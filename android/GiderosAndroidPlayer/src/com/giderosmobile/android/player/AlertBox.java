package com.giderosmobile.android.player;

import android.app.AlertDialog;
import android.content.DialogInterface;

public class AlertBox implements DialogInterface.OnShowListener, DialogInterface.OnClickListener, DialogInterface.OnCancelListener, DialogInterface.OnDismissListener
{
	private native void completeCallback(int buttonIndex, String buttonText, long udata);
	
	private AlertDialog alertDialog_;
	private boolean isVisible_ = false;
	private long udata_;
	
	private String cancel_;
	private String button1_;
	private String button2_;
	
	public AlertBox(String title, String message, String cancel, String button1, String button2, long udata)
	{
		cancel_ = cancel;
		button1_ = button1;
		button2_ = button2;
		
		udata_ = udata;
		
		final AlertBox this2 = this;
		final String title2 = title;
		final String message2 = message;	
		final String cancel2 = cancel;
		final String button12 = button1;
		final String button22 = button2;
		
		WeakActivityHolder.get().runOnUiThread(new Runnable()
		{
            @Override
            public void run()
            {
        		alertDialog_ = new AlertDialog.Builder(WeakActivityHolder.get()).create();
        		alertDialog_.setTitle(title2);
        		alertDialog_.setMessage(message2);
        		alertDialog_.setCancelable(true);
        		alertDialog_.setOnShowListener(this2);        		
        		alertDialog_.setButton(DialogInterface.BUTTON_NEGATIVE, cancel2, this2);
        		if (button12 != null)
        			alertDialog_.setButton(DialogInterface.BUTTON_POSITIVE, button12, this2);
        		if (button22 != null)
        			alertDialog_.setButton(DialogInterface.BUTTON_NEUTRAL, button22, this2);
        		alertDialog_.setOnCancelListener(this2);
        		alertDialog_.setOnDismissListener(this2);
            }
        });
	}

	public void show()
	{
		WeakActivityHolder.get().runOnUiThread(new Runnable()
		{
            @Override
            public void run()
            {
        		alertDialog_.show();
            }
        });
	}

	public void hide()
	{
		WeakActivityHolder.get().runOnUiThread(new Runnable()
		{
            @Override
            public void run()
            {
        		alertDialog_.hide();
            }
        });
	}
	
	public void delete()
	{
        alertDialog_.dismiss();
	}

	@Override
	public void onClick(DialogInterface dialog, int which)
	{
		switch (which)
		{
		case DialogInterface.BUTTON_NEGATIVE:
			completeCallback(0, cancel_, udata_);
			break;
		case DialogInterface.BUTTON_POSITIVE:
			completeCallback(1, button1_, udata_);
			break;
		case DialogInterface.BUTTON_NEUTRAL:
			completeCallback(2, button2_, udata_);
			break;
		}
	}

	@Override
	public void onCancel(DialogInterface dialog)
	{
		completeCallback(0, cancel_, udata_);
	}

	@Override
	public void onShow(DialogInterface dialog)
	{
		isVisible_ = true;
	}
	
	@Override
	public void onDismiss(DialogInterface dialog)
	{
		isVisible_ = false;
	}

	public boolean isVisible()
	{
		return isVisible_;
	}
}
