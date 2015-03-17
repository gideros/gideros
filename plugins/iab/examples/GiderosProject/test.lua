
--[[
	initializing store, 
	will differ for each store
	but is mandatory to set
]]

--create iab instance based on store you want to use
iab = IAB.new("test")

--provide key(s) needed for the store
iab:setUp("testkey", "testkey2", "testkey3")

--provide list of products you will be using 
--(key is your internal products id, value is store specific product id)
iab:setProducts({p1 = "product1", p2 = "product2", p3 = "upgrade1"})

--which of your products are consumables, 
--that people can buy many times (provide internal product ids)
iab:setConsumables({"p1", "p2"})