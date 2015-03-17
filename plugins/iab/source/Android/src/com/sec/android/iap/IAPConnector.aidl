package com.sec.android.iap;

import com.sec.android.iap.IAPServiceCallback;

interface IAPConnector {

	boolean requestCmd(IAPServiceCallback callback, in Bundle bundle);
	
	boolean unregisterCallback(IAPServiceCallback callback);
	
	Bundle init(int mode);
	
	Bundle getItemList(int mode, String packageName, String itemGroupId, int startNum, int endNum, String itemType);
	
	Bundle getItemsInbox(String packageName, String itemGroupId, int startNum, int endNum, String startDate, String endDate);
}