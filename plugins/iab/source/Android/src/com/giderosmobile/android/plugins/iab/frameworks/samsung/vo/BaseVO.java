package com.giderosmobile.android.plugins.iab.frameworks.samsung.vo;

import org.json.JSONException;
import org.json.JSONObject;

import android.text.TextUtils;
import android.text.format.DateFormat;

public class BaseVO 
{
    private String mItemId;
    private String mItemName;
    private Double mItemPrice;
    private String mItemPriceString;
    private String mCurrencyUnit;
    private String mItemDesc;
    private String mItemImageUrl;
    private String mItemDownloadUrl;
    
    public BaseVO(){}

    public BaseVO( String _jsonString )
    {
        try
        {
            JSONObject jObject = new JSONObject( _jsonString );
            
            setItemId( jObject.getString( "mItemId" ) );
            setItemName( jObject.getString( "mItemName" ) );
            setItemPrice( jObject.getDouble("mItemPrice" ) );
            setCurrencyUnit( jObject.getString( "mCurrencyUnit" ) );
            setItemDesc( jObject.getString( "mItemDesc" ) );
            setItemImageUrl( jObject.getString( "mItemImageUrl" ) );
            setItemDownloadUrl( jObject.getString( "mItemDownloadUrl" ) );
            setItemPriceString( jObject.getString( "mItemPriceString" ) );
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

    public Double getItemPrice()
    {
        return mItemPrice;
    }

    public void setItemPrice( Double _itemPrice )
    {
        mItemPrice = _itemPrice;
    }
    
    public String getItemPriceString()
    {
        return mItemPriceString;
    }

    public void setItemPriceString( String _itemPriceString )
    {
        mItemPriceString = _itemPriceString;
    }

    public String getCurrencyUnit()
    {
        return mCurrencyUnit;
    }

    public void setCurrencyUnit( String _currencyUnit )
    {
        mCurrencyUnit = _currencyUnit;
    }

    public String getItemDesc()
    {
        return mItemDesc;
    }

    public void setItemDesc( String _itemDesc )
    {
        mItemDesc = _itemDesc;
    }

    public String getItemImageUrl()
    {
        return mItemImageUrl;
    }

    public void setItemImageUrl( String _itemImageUrl )
    {
        mItemImageUrl = _itemImageUrl;
    }

    public String getItemDownloadUrl()
    {
        return mItemDownloadUrl;
    }

    public void setItemDownloadUrl( String _itemDownloadUrl )
    {
        mItemDownloadUrl = _itemDownloadUrl;
    }

    public String dump()
    {
        String dump = null;
        
        dump = "ItemId          : " + getItemId()          + "\n" + 
               "ItemName        : " + getItemName()        + "\n" +
               "ItemPrice       : " + getItemPrice()       + "\n" +
               "ItemPriceString : " + getItemPriceString() + "\n" +
               "CurrencyUnit    : " + getCurrencyUnit()    + "\n" +
               "ItemDesc        : " + getItemDesc()        + "\n" +
               "ItemImageUrl    : " + getItemImageUrl()    + "\n" +
               "ItemDownloadUrl : " + getItemDownloadUrl();
        
        return dump;
    }
    
    protected String getDateString( String _strTimeMills )
    {
        String result     = "";
        String dateFormat = "yyyy.MM.dd hh:mm:ss";
        
        if( false == TextUtils.isEmpty( _strTimeMills ) )
        {
            try
            {
                long timeMills = Long.parseLong( _strTimeMills );
                result = DateFormat.format( dateFormat, timeMills ).toString();
            }
            catch( Exception e )
            {
                e.printStackTrace();
                result = "";
            }
        }
        
        return result;
    }
}