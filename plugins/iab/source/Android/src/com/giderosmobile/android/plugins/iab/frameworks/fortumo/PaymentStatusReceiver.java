package com.giderosmobile.android.plugins.iab.frameworks.fortumo;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;
import android.os.Bundle;

import com.fortumo.android.Fortumo;
import com.fortumo.android.PaymentResponse;
import com.giderosmobile.android.plugins.iab.Iab;
import com.giderosmobile.android.plugins.iab.frameworks.IabFortumo;

public class PaymentStatusReceiver extends BroadcastReceiver {
	private static String TAG = "PaymentStatusReceiver";

  	@Override
  	public void onReceive(Context context, Intent intent) {
  		PaymentResponse response = new PaymentResponse(intent);
  		if(response.getBillingStatus() == Fortumo.MESSAGE_STATUS_BILLED) {
  			Iab.purchaseComplete(new IabFortumo(), response.getProductName(), response.getMessageId()+"");
  		} 
  		else if(response.getBillingStatus() == Fortumo.MESSAGE_STATUS_FAILED){
  			Iab.purchaseError(new IabFortumo(), "Purchase failed");
  		}
  		else if(response.getBillingStatus() == Fortumo.MESSAGE_STATUS_NOT_SENT){
  			Iab.purchaseError(new IabFortumo(), "Purchase not sent");
  		}
  		else if(response.getBillingStatus() == Fortumo.MESSAGE_STATUS_USE_ALTERNATIVE_METHOD){
  			Iab.purchaseError(new IabFortumo(), "User alternative method");
  		}
  	}
}