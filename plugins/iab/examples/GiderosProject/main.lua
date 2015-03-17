--[[
	and now comes iap implementation into your app, which can be the same for any store
]]

--which of your products are consumables, 
--that people can buy many times (provide internal product ids)
iab:setConsumables({"p1", "p2"})

--[[ CHECKING IF STORE IS AVAILABLE ]]--
iab:addEventListener(Event.AVAILABLE, function(e)
	print("is available")
	--usually here we would set a flag that it is possible to make purchases
	--basically you can allow doing all the iap stuff after this event is called
	
	--[[ REQUESTING PRODUCTS ]]--
	--if this event is called, we received the list of products and information about them
	iab:addEventListener(Event.PRODUCTS_COMPLETE, function(e)
		for i = 1, #e.products do
			local p = e.products[i]
			--id, title, description and price
			print(p.productId, p.title, p.description, p.price)
		end
	end)
	
	--else we could not retrieve information about products now
	iab:addEventListener(Event.PRODUCTS_ERROR, function(e)
		print(e:getType(), e.error)
	end)
	
	--requesting products
	iab:requestProducts()
	
	--[[ PURCHASING ]]--
	
	iab:addEventListener(Event.PURCHASE_COMPLETE, function(e)
		--purchases successfully completed 
		--here you need to check if receiptId was not already saved previously
		--as in if purchase was not already made
		--then you can unlock the item here
		--and store receiptId presistently to know 
		--that you have already provided this item to user
		print(e:getType(), e.productId, e.receiptId)
	end)
	iab:addEventListener(Event.PURCHASE_ERROR, function(e)
		--it was not possible to complete the purchase, 
		--inform user
		print(e:getType(), e.error)
	end)
	
	stage:addEventListener(Event.MOUSE_DOWN, function()
		print("purchasing")
		iab:purchase("p1")
	end)
	
	--[[ RESTORING PURCHASES ]]--
	-- we can either provide button to restore purchases or launch it on every app start
	-- this will simply launch purchase event for every unconsumable item
	iab:restore()

end)
--this function will call Event.AVAILABLE if purchases are available
--until then you must assume that it is not available
iab:isAvailable()