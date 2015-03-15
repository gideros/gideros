
--[[
	initializing store, 
	will differ for each store
	but is mandatory to set
]]

--create iab instance based on store you want to use
iab = IAB.new("google")

--provide key(s) needed for the store
iab:setUp("google billing key")

--provide list of products you will be using 
--(key is your internal products id, value is store specific product id)
iab:setProducts({p1 = "android.test.purchased", p2 = "android.test.purchased", p3 = "android.test.purchased"})



