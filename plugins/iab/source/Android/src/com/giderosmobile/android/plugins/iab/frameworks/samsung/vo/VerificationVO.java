package com.giderosmobile.android.plugins.iab.frameworks.samsung.vo;

import org.json.JSONException;
import org.json.JSONObject;

import android.util.Log;

public class VerificationVO
{
    private static final String TAG = VerificationVO.class.getSimpleName();
    
    private String mItemId;
    private String mItemName;
    private String mItemDesc;
    private String mPurchaseDate;
    private String mPaymentId;
    private String mPaymentAmount;
    private String mStatus;
    
    public VerificationVO( String _jsonString )
    {
        Log.i( TAG, _jsonString );
        
        try
        {
            JSONObject jObject = new JSONObject( _jsonString );
            
            setItemId( jObject.getString( "itemId" ) );
            setItemName( jObject.getString( "itemName" ) );
            setItemDesc( jObject.getString( "itemDesc" ) );
            setPurchaseDate( jObject.getString( "purchaseDate" ) );
            setPaymentId( jObject.getString( "paymentId" ) );
            setPaymentAmount( jObject.getString( "paymentAmount" ) );
            setStatus( jObject.getString( "status" ) );
        }
        catch( JSONException e )
        {
            e.printStackTrace();
        }
    }

    public String getItemId()
    {
        return mItemId;
    }

    public void setItemId( String _itemId )
    {
        mItemId = _itemId;
    }

    public String getItemName()
    {
        return mItemName;
    }

    public void setItemName( String _itemName )
    {
        mItemName = _itemName;
    }

    public String getItemDesc()
    {
        return mItemDesc;
    }

    public void setItemDesc( String _itemDesc )
    {
        mItemDesc = _itemDesc;
    }

    public String getPurchaseDate()
    {
        return mPurchaseDate;
    }

    public void setPurchaseDate( String _purchaseDate )
    {
        mPurchaseDate = _purchaseDate;
    }

    public String getPaymentId()
    {
        return mPaymentId;
    }

    public void setPaymentId( String _paymentId )
    {
        mPaymentId = _paymentId;
    }

    public String getPaymentAmount()
    {
        return mPaymentAmount;
    }

    public void setPaymentAmount( String _paymentAmount )
    {
        mPaymentAmount = _paymentAmount;
    }

    public String getStatus()
    {
        return mStatus;
    }

    public void setStatus( String _status )
    {
        mStatus = _status;
    }

    public String dump()
    {
        String dump = null;
        
        dump = "ItemId        : " + getItemId()        + "\n" +
               "ItemName      : " + getItemName()      + "\n" +
               "ItemDesc      : " + getItemDesc()      + "\n" +
               "PurchaseDate  : " + getPurchaseDate()  + "\n" +
               "PaymentId     : " + getPaymentId()     + "\n" +
               "PaymentAmount : " + getPaymentAmount() + "\n" +
               "Status        : " + getStatus();
        
        return dump;
    }
}