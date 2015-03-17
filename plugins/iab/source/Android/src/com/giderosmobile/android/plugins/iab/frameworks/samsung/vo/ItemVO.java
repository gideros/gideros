package com.giderosmobile.android.plugins.iab.frameworks.samsung.vo;

import org.json.JSONException;
import org.json.JSONObject;

import android.util.Log;

public class ItemVO extends BaseVO
{
    private static final String TAG = ItemVO.class.getSimpleName();
    
    private String mType;
    private String mSubscriptionDurationUnit;       // ģ� ķ��źø°ź°„ ė‹�ģ�„
    private String mSubscriptionDurationMultiplier; // ģ� ķ��źø°ź°„
    
    public ItemVO(){}

    public ItemVO( String _jsonString )
    {
        super( _jsonString );
        
        Log.i( TAG, _jsonString );
        
        try
        {
            JSONObject jObject = new JSONObject( _jsonString );
            
            setType( jObject.getString( "mType" ) );
                
            setSubscriptionDurationUnit( 
                            jObject.getString( "mSubscriptionDurationUnit" ) );
                
            setSubscriptionDurationMultiplier( 
                      jObject.getString( "mSubscriptionDurationMultiplier" ) );
        }
        catch( JSONException e )
        {
            e.printStackTrace();
        }
    }

    public String getType()
    {
        return mType;
    }

    public void setType( String _type )
    {
        mType = _type;
    }
    
    public String getSubscriptionDurationUnit()
    {
        return mSubscriptionDurationUnit;
    }

    public void setSubscriptionDurationUnit( String _subscriptionDurationUnit )
    {
        mSubscriptionDurationUnit = _subscriptionDurationUnit;
    }

    public String getSubscriptionDurationMultiplier()
    {
        return mSubscriptionDurationMultiplier;
    }

    public void setSubscriptionDurationMultiplier(
                                       String _subscriptionDurationMultiplier )
    {
        mSubscriptionDurationMultiplier = _subscriptionDurationMultiplier;
    }

    public String dump()
    {
        String dump = super.dump() + "\n";
        
        dump += "Type : " + getType() + "\n" +
                "SubscriptionDurationUnit : "
                                       + getSubscriptionDurationUnit() + "\n" +
                "SubscriptionDurationMultiplier : " + 
                                           getSubscriptionDurationMultiplier();
        return dump;
    }
}