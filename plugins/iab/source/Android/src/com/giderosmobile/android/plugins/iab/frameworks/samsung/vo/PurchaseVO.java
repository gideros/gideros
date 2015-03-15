package com.giderosmobile.android.plugins.iab.frameworks.samsung.vo;

import org.json.JSONException;
import org.json.JSONObject;

import android.util.Log;

public class PurchaseVO extends BaseVO
{
    private static final String TAG = PurchaseVO.class.getSimpleName();
    
    private String mPaymentId;
    private String mPurchaseId;
    private String mPurchaseDate;
    private String mVerifyUrl;
    
    public PurchaseVO( String _jsonString )
    {
        super( _jsonString );
        
        try
        {
            Log.i( TAG, _jsonString );
            
            JSONObject jObject = new JSONObject( _jsonString );

            setPaymentId( jObject.getString( "mPaymentId" ) );
            setPurchaseId( jObject.getString( "mPurchaseId" ) );
            
            setPurchaseDate( 
                       getDateString( jObject.getString( "mPurchaseDate" ) ) );
            
            setVerifyUrl( jObject.getString( "mVerifyUrl" ) );
        }
        catch( JSONException e )
        {
            e.printStackTrace();
        }
    }

    public String getPaymentId()
    {
        return mPaymentId;
    }

    public void setPaymentId( String _paymentId )
    {
        mPaymentId = _paymentId;
    }
    
    public String getPurchaseId()
    {
        return mPurchaseId;
    }

    public void setPurchaseId( String _purchaseId )
    {
        mPurchaseId = _purchaseId;
    }

    public String getPurchaseDate()
    {
        return mPurchaseDate;
    }

    public void setPurchaseDate( String _purchaseDate )
    {
        mPurchaseDate = _purchaseDate;
    }

    public String getVerifyUrl()
    {
        return mVerifyUrl;
    }
    
    public void setVerifyUrl(String _verifyUrl)
    {
        mVerifyUrl = _verifyUrl;
    }
    
    public String dump()
    {
        String dump = super.dump() + "\n";
        
        dump += "PaymentID    : " + getPaymentId()    + "\n" +
                "PurchaseId   : " + getPurchaseId()   + "\n" +
                "PurchaseDate : " + getPurchaseDate() + "\n" +
                "VerifyUrl    : " + getVerifyUrl();
        
        return dump;
    }
}