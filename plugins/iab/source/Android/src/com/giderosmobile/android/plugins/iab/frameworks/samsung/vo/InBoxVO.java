package com.giderosmobile.android.plugins.iab.frameworks.samsung.vo;

import org.json.JSONException;
import org.json.JSONObject;

import android.util.Log;

public class InBoxVO extends BaseVO
{
    private static final String TAG = InBoxVO.class.getSimpleName();

    private String mType;
    private String mPaymentId;
    private String mPurchaseDate;
    
    // Subscription ķ�€ģ˛… ģ•„ģ¯´ķ…�ģ¯� ģ� ķ��źø°ź°„ ė§�ė£�ģ¯¼
    // ========================================================================
    private String mSubscriptionEndDate;
    // ========================================================================
    
    public InBoxVO( String _jsonString )
    {
        super( _jsonString );
        
        Log.i( TAG, _jsonString );
        
        try
        {
            JSONObject jObject = new JSONObject( _jsonString );
            
            setType( jObject.getString( "mType" ) );
            setPaymentId( jObject.getString( "mPaymentId" ) );
            setPurchaseDate( 
                       getDateString( jObject.getString( "mPurchaseDate" ) ) );
            setSubscriptionEndDate(
                getDateString( jObject.getString( "mSubscriptionEndDate" ) ) );
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

    public String getPurchaseDate()
    {
        return mPurchaseDate;
    }

    public void setPurchaseDate( String _purchaseDate )
    {
        mPurchaseDate = _purchaseDate;
    }

    
    public String getSubscriptionEndDate()
    {
        return mSubscriptionEndDate;
    }

    public void setSubscriptionEndDate( String _subscriptionEndDate )
    {
        mSubscriptionEndDate = _subscriptionEndDate;
    }
    
    public String getType()
    {
        return mType;
    }

    public void setType( String _type )
    {
        mType = _type;
    }
    
    public String dump()
    {
        String dump = super.dump() + "\n";
        
        dump += "Type                : " + getType()                + "\n" + 
                "PurchaseDate        : " + getPurchaseDate()        + "\n" +
                "SubscriptionEndDate : " + getSubscriptionEndDate() + "\n" +
                "PaymentID           : " + getPaymentId();
        
        return dump;
    }
}