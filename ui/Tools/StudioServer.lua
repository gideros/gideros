-- Studio server for addons support routines

function RunStudioQuery(clientId,jsonQuery)
  local ok,luaQuery=pcall(json.decode,jsonQuery)
  local res= {}
  if not ok then
    res.error={ code=-32700, message=luaQuery }
  else
    res.id=luaQuery.id
    local ok,resp=pcall(function ()
      if luaQuery.method then
        local mtd=loadstring('return '..luaQuery.method..'(...)') --return a wrapper around the called method
        Studio.CURRENT_CLIENT=clientId
        if luaQuery.params then
          luaQuery.params.n=#luaQuery.params
          for i=1,luaQuery.params.n do if luaQuery.params[i]==json.null then luaQuery.params[i]=nil end end
          return mtd(unpack(luaQuery.params))
        else
          return mtd()
        end
      end
    end)
    if ok then 
      res.result=resp or json.null
    else
      res.error={ code=-32602, message=resp}
    end
  end
  return json.encode(res)
end

Studio=Studio or {} -- Create the studio namespace if not already done
Studio.EVENTLISTENERS={}
Studio.notifyClient=function (cid,data) Studio._notifyClient(cid,json.encode(data)) end
Studio.registerEvent=function (type,subtype)
  local ctab=Studio.EVENTLISTENERS[Studio.CURRENT_CLIENT] or {}
  Studio.EVENTLISTENERS[Studio.CURRENT_CLIENT]=ctab
  table.insert(ctab, { type=type, subtype=subtype })
end
Studio.notifyEvent=function(type,subtype,args)
  for c,el in pairs(Studio.EVENTLISTENERS) do
    for _,v in ipairs(el) do
      if v.type==type and (v.subtype==nil or v.subtype==subtype) then
        Studio.notifyClient(c,{type=type, subtype=subtype, args=args})
      end
    end
  end
end
Studio.clientDisconnected=function (cid) Studio.EVENTLISTENERS[cid]=nil end
