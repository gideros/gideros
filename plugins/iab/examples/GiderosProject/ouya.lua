
--[[
	initializing store, 
	will differ for each store
	but is mandatory to set
]]

--create iab instance based on store you want to use
iab = IAB.new("ouya")

--read in ouya key
local file = io.open("key.der", "rb")
local appKey = file:read( "*a" )
io.close( file )

iab:setUp("ouya dev key", appKey)

--provide list of products you will be using 
--(key is your internal products id, value is store specific product id)
iab:setProducts({p1 = "money65", p2 = "money10", p3 = "doublemoney"})
