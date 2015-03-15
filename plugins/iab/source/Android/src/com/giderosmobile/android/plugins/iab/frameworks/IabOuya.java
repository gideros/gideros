package com.giderosmobile.android.plugins.iab.frameworks;

import java.io.UnsupportedEncodingException;
import java.lang.ref.WeakReference;
import java.security.InvalidAlgorithmParameterException;
import java.security.InvalidKeyException;
import java.security.KeyFactory;
import java.security.NoSuchAlgorithmException;
import java.security.NoSuchProviderException;
import java.security.PublicKey;
import java.security.SecureRandom;
import java.security.spec.X509EncodedKeySpec;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.Hashtable;
import java.util.List;

import javax.crypto.BadPaddingException;
import javax.crypto.Cipher;
import javax.crypto.IllegalBlockSizeException;
import javax.crypto.NoSuchPaddingException;
import javax.crypto.SecretKey;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;

import org.json.JSONException;
import org.json.JSONObject;

import com.giderosmobile.android.plugins.iab.Iab;
import com.giderosmobile.android.plugins.iab.IabInterface;

import tv.ouya.console.api.OuyaAuthenticationHelper;
import tv.ouya.console.api.OuyaEncryptionHelper;
import tv.ouya.console.api.OuyaErrorCodes;
import tv.ouya.console.api.OuyaFacade;
import tv.ouya.console.api.OuyaPurchaseHelper;
import tv.ouya.console.api.OuyaResponseListener;
import tv.ouya.console.api.Product;
import tv.ouya.console.api.Purchasable;
import tv.ouya.console.api.Receipt;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Base64;
import android.util.SparseArray;

public class IabOuya implements IabInterface {
	public static WeakReference<Activity> sActivity;
	public static OuyaFacade ouyaFacade;
	public static HashMap<String,String> mOutstandingPurchaseRequests;
	public static PublicKey mPublicKey;
	public static final int PURCHASE_AUTHENTICATION_ACTIVITY_ID = 1;
	
	public static Boolean isInstalled(){
		if("cardhu".equals(android.os.Build.DEVICE) || "ouya_1_1".equals(android.os.Build.DEVICE))
			return true;
		return false;
	}

	@Override
	public void onCreate(WeakReference<Activity> activity) {
		sActivity = activity;
		ouyaFacade = OuyaFacade.getInstance();
		mOutstandingPurchaseRequests = new HashMap<String, String>();
	}

	@Override
	public void onDestroy() {
		ouyaFacade.shutdown();
	}
	
	@Override
	public void onStart() {

	}
	
	public void onActivityResult(final int requestCode, final int resultCode, final Intent data) {
        if(resultCode == Activity.RESULT_OK) {
            switch (requestCode) {
                case PURCHASE_AUTHENTICATION_ACTIVITY_ID:
                    restartInterruptedPurchase();
                    break;
            }
        }
    }

	@Override
	public void init(Object parameters) {
		SparseArray<byte[]> p = (SparseArray<byte[]>)parameters;
		try {
			ouyaFacade.init(sActivity.get(), new String(p.get(0), "UTF-8"));
		} catch (UnsupportedEncodingException e1) {
		}
		try {
            X509EncodedKeySpec keySpec = new X509EncodedKeySpec(p.get(1));
            KeyFactory keyFactory = KeyFactory.getInstance("RSA");
            mPublicKey = keyFactory.generatePublic(keySpec);
        } catch (Exception e) {
        }
	}

	@Override
	public void check() {
		if (ouyaFacade.isRunningOnOUYAHardware())
		{
			Iab.available(this);
		}
		else
		{
			Iab.notAvailable(this);
		}
	}
	
	@Override
	public void request(Hashtable<String, String> products) {
		List<Purchasable> list = new ArrayList<Purchasable>();
    	Enumeration<String> e = products.keys();
		while(e.hasMoreElements())
		{
			String prodName = e.nextElement();
        	list.add(new Purchasable(products.get(prodName)));
        }
        ouyaFacade.requestProductList(list, new IabOuyaProductRequestListener(this));
	}

	@Override
	public void purchase(String productId) {
		try{
    		makePurchase(productId);
    	} catch (Exception ex) {
            Iab.purchaseError(this, ex.getMessage());
        }
	}

	@Override
	public void restore() {
		ouyaFacade.requestReceipts(new IabOuyaReceiptRequestListener(this));
	}
	
	public void restartInterruptedPurchase() {
        final String suspendedPurchaseId = OuyaPurchaseHelper.getSuspendedPurchase(sActivity.get());
        if(suspendedPurchaseId == null) {
            return;
        }
        purchase(suspendedPurchaseId);
    }
	
	public void makePurchase(String productId) throws NoSuchAlgorithmException, JSONException, NoSuchProviderException, NoSuchPaddingException, InvalidKeyException, InvalidAlgorithmParameterException, IllegalBlockSizeException, BadPaddingException, UnsupportedEncodingException {
   	 SecureRandom sr = SecureRandom.getInstance("SHA1PRNG");

        // This is an ID that allows you to associate a successful purchase with
        // it's original request. The server does nothing with this string except
        // pass it back to you, so it only needs to be unique within this instance
        // of your app to allow you to pair responses with requests.
        String uniqueId = Long.toHexString(sr.nextLong());

        JSONObject purchaseRequest = new JSONObject();
        purchaseRequest.put("uuid", uniqueId);
        purchaseRequest.put("identifier", productId);
        String purchaseRequestJson = purchaseRequest.toString();

        byte[] keyBytes = new byte[16];
        sr.nextBytes(keyBytes);
        SecretKey key = new SecretKeySpec(keyBytes, "AES");

        byte[] ivBytes = new byte[16];
        sr.nextBytes(ivBytes);
        IvParameterSpec iv = new IvParameterSpec(ivBytes);

        Cipher cipher = Cipher.getInstance("AES/CBC/PKCS5Padding", "BC");
        cipher.init(Cipher.ENCRYPT_MODE, key, iv);
        byte[] payload = cipher.doFinal(purchaseRequestJson.getBytes("UTF-8"));

        cipher = Cipher.getInstance("RSA/ECB/PKCS1Padding", "BC");
        cipher.init(Cipher.ENCRYPT_MODE, mPublicKey);
        byte[] encryptedKey = cipher.doFinal(keyBytes);

        Purchasable purchasable =
                new Purchasable(
                        productId,
                        Base64.encodeToString(encryptedKey, Base64.NO_WRAP),
                        Base64.encodeToString(ivBytes, Base64.NO_WRAP),
                        Base64.encodeToString(payload, Base64.NO_WRAP) );

        synchronized (mOutstandingPurchaseRequests) {
            mOutstandingPurchaseRequests.put(uniqueId, productId);
        }
        ouyaFacade.requestPurchase(purchasable, new IabOuyaPurchaseListener(this, productId));
	}
}

class IabOuyaProductRequestListener implements OuyaResponseListener<ArrayList<Product>> {
	IabOuya caller;
	public IabOuyaProductRequestListener(IabOuya iabOuya) {
		caller = iabOuya;
	}
	
    @Override
    public void onSuccess(ArrayList<Product> products) {
    	SparseArray<Bundle> arr = new SparseArray<Bundle>();
    	int i = 0; 
        for(Product p : products) {
        	Bundle map = new Bundle();
        	map.putString("productId", p.getIdentifier());
        	map.putString("title", p.getName());
        	map.putString("description", "");
        	map.putString("price", ((float)p.getPriceInCents()/100) + "");
        	arr.put(i, map);
        	i++;
        }
        Iab.productsComplete(caller, arr);
    }

    @Override
    public void onFailure(int errorCode, String errorMessage, Bundle errorBundle) {
    	Iab.productsError(caller, errorMessage);
    }

	@Override
	public void onCancel() {}
};

class IabOuyaPurchaseListener implements OuyaResponseListener<String> {
	private String mProduct;
	private IabOuya caller;
	
	IabOuyaPurchaseListener(IabOuya iabOuya, String product) {
        mProduct = product;
        caller = iabOuya;
    }
	
    @Override
    public void onSuccess(String result) {
    	String productId;
    	String receiptId = "";
        try {
            OuyaEncryptionHelper helper = new OuyaEncryptionHelper();

            JSONObject response = new JSONObject(result);
            if(response.has("key") && response.has("iv")) {
            	receiptId = helper.decryptPurchaseResponse(response, IabOuya.mPublicKey);
                String storedProduct;
                synchronized (IabOuya.mOutstandingPurchaseRequests) {
                    storedProduct = IabOuya.mOutstandingPurchaseRequests.remove(receiptId);
                }
                if(storedProduct == null || !storedProduct.equals(mProduct)) {
                    onFailure(
                        OuyaErrorCodes.THROW_DURING_ON_SUCCESS, 
                        "No purchase outstanding for the given purchase request",
                        Bundle.EMPTY);
                    return;
                }
                productId = storedProduct;
            } else {
                Product product = new Product(new JSONObject(result));
                productId = product.getIdentifier();
                if(!mProduct.equals(product.getIdentifier())) {
                    onFailure(
                        OuyaErrorCodes.THROW_DURING_ON_SUCCESS, 
                        "Purchased product is not the same as purchase request product", 
                        Bundle.EMPTY);
                    return;
                }
            }
            Iab.purchaseComplete(caller, productId, receiptId);
        } catch (Exception e) {
        	onFailure(
                    OuyaErrorCodes.THROW_DURING_ON_SUCCESS, 
                    "Purchase failed", 
                    Bundle.EMPTY);
        }
    }

    @Override
    public void onFailure(int errorCode, String errorMessage, Bundle errorBundle) {
    	 boolean wasHandledByAuthHelper =
                 OuyaAuthenticationHelper.
                         handleError(
                                 IabOuya.sActivity.get(), errorCode, errorMessage,
                                 errorBundle, IabOuya.PURCHASE_AUTHENTICATION_ACTIVITY_ID,
                                 new OuyaResponseListener<Void>() {
                                     @Override
                                     public void onSuccess(Void result) {
                                         caller.restartInterruptedPurchase();   // Retry the purchase if the error was handled.
                                     }

                                     @Override
                                     public void onFailure(int errorCode, String errorMessage,
                                                           Bundle optionalData) {
                                    	 Iab.purchaseError(caller, errorMessage);
                                     }

                                     @Override
                                     public void onCancel() {
                                    	 Iab.purchaseError(caller, "User canceled purchase");
                                     }
                                 });


         if(!wasHandledByAuthHelper) {
        	 Iab.purchaseError(caller, errorMessage);
         }
    }

	@Override
	public void onCancel() {
		Iab.purchaseError(caller, "User canceled purchase");		
	}
};

class IabOuyaReceiptRequestListener implements OuyaResponseListener<String> {
	private IabOuya caller;
    public IabOuyaReceiptRequestListener(IabOuya iabOuya) {
		caller = iabOuya;
	}

	@Override
    public void onSuccess(String receiptResponse) {
    	OuyaEncryptionHelper helper = new OuyaEncryptionHelper();
        List<Receipt> receipts = null;
        try {
            JSONObject response = new JSONObject(receiptResponse);
            if(response.has("key") && response.has("iv")) {
                receipts = helper.decryptReceiptResponse(response, IabOuya.mPublicKey);
            } else {
                receipts = helper.parseJSONReceiptResponse(receiptResponse);
            }
        } catch (Exception e) {
        	onFailure(
                    OuyaErrorCodes.THROW_DURING_ON_SUCCESS, 
                    e.getMessage(), 
                    Bundle.EMPTY);
        }
        for (Receipt r : receipts) {
        	Iab.purchaseComplete(caller, r.getIdentifier(), r.getUuid());
        }
        Iab.restoreComplete(caller);
    }

    @Override
    public void onFailure(int errorCode, String errorMessage, Bundle errorBundle) {
    	Iab.restoreError(caller, errorMessage);
    }

	@Override
	public void onCancel() {}
};
