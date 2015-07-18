require "googlebilling"
 
 
googlebilling:setPublicKey("aMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAhaOIx58WDmoLsVacdFHsb/LFZ6cSJPutIYhqibPneF0jYdMTGKUZX98e87sVf+PdCTjIECQxuqkCrq0uNu+9ugYxJhifbA65PONT98eEXN6MJsrxXQORWT+bBvh1R2hn0aZHYuLF4QmCy8QpxESNAtNLlSTU+LPEBm7Gl6A8a+K/hzc2qJ/cJneH5W88lufiMEiUFWm5tVh8t/HMuxaJAWx+Jo/O0FDtlGfSfiuPVwyM++AGqOkAvAlXW72INfiMOZ+/3uq1G1ThfWKCbAo5nLTDlQB0thpiyinVpkgFZ06hSocv2YSE984kqnaE/rTUkVOIXsnuShjuLGiKQCzH0wIDAQAB")

function REQUEST_PURCHASE_COMPLETE(event)
	print("REQUEST_PURCHASE_COMPLETE")
end
 
local function PURCHASE_STATE_CHANGE(event)
	print("PURCHASE_STATE_CHANGE")
	print(event.purchaseState, event.productId, event.notificationId, event.purchaseTime, event.developerPayload)
	googlebilling:confirmNotification(event.notificationId)
end
 
googlebilling:addEventListener(Event.REQUEST_PURCHASE_COMPLETE, REQUEST_PURCHASE_COMPLETE)
googlebilling:addEventListener(Event.PURCHASE_STATE_CHANGE, PURCHASE_STATE_CHANGE)

googlebilling:requestPurchase("android.test.purchased")
