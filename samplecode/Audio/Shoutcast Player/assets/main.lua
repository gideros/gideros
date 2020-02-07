-- Create Shoutcast stream
u=UrlLoader.new()
u:setStreaming(true) --Enable streaming mode
-- Create our mp3 Buffer
u.mp3=Buffer.new("buffer.mp3",true)
u.icy=0
-- The header listener will receive HTTP response status before actual data
u:addEventListener(Event.HEADER,function (e)
	-- Read shoutcast metadata interval
	u.icy=tonumber(e.headers["icy-metaint"] or 0)
	-- Set up metadata state
	u.icys=""
	u.icyc=u.icy
	u.meta={}
end)

-- Process stream data
u:addEventListener(Event.PROGRESS,function (e)
	local chunk=e.chunk
	--If metadata is present, process it
	if u.icy>0 then 
		while (#chunk)>0 do
			if u.icyc==0 then
				--We are at the the start of a metadata chunk
				u.icys=u.icys..chunk
				local cn=u.icys:byte(1)*16
				if #u.icys>=cn then
					local ic=u.icys:sub(2,1+cn)
					chunk=u.icys:sub(cn+2)
					u.icyc=u.icy
					u.icys=""
					-- Parse and call metadata handler
					if #ic>0 and u.metaHandler then
						ic:gsub("(%a+)='(.-)';",function(k,v) u.meta[k]=v end)
						u.metaHandler(u,u.meta)
					end
				else
					chunk=""
				end
			end
			if u.icyc>0 then
				--The remaining is just mp3 data
				local cn=(#chunk)><u.icyc
				u.icyc-=cn
				u.mp3:append(chunk:sub(1,cn))
				chunk=chunk:sub(cn+1)
			end
		end
	else
		-- No metadata in this stream, just add to mp3 stream
		u.mp3:append(chunk)
	end
	if u.mp3:size()>100000 and not u.mp3.started then
		-- Start playing is we buffered more than 100k bytes
		print("Starting")
		Sound.new("|B|buffer.mp3"):play() --Play the buffer content by name
		u.mp3.started=true
	end
	if u.mp3.started then
		-- Periodically trim already played data from buffer
		u.mp3:trim(u.mp3:tell())
	end
end)
print("Buffering")
u:load("http://uk5.internet-radio.com:8256/",UrlLoader.GET,{ ["Icy-Metadata"]="1"})

-- Minimalist UI: just show the song title
title=TextField.new(nil,"Buffering")
stage:addChild(title)
title:setPosition(10,10)
u.metaHandler=function(u,meta)
	print("Title: "..meta.StreamTitle or "")
	title:setText(meta.StreamTitle or "")
end

