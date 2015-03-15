
--[[
	initializing store, 
	will differ for each store
	but is mandatory to set
]]

--create iab instance based on store you want to use
iab = IAB.new("amazon")

iab:setUp()
--provide list of products you will be using 
--(key is your internal products id, value is store specific product id)
iab:setProducts({p1 = "money10", p2 = "money22", p3 = "doublemoney"})



