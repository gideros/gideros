package com.giderosmobile.android.player;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.text.Editable;
import android.text.InputType;
import android.text.TextWatcher;
import android.view.inputmethod.InputMethodManager;
import android.widget.EditText;

public class TextInputBox implements DialogInterface.OnShowListener, DialogInterface.OnClickListener, DialogInterface.OnCancelListener, DialogInterface.OnDismissListener, TextWatcher
{
	private native void completeCallback(String text, int buttonIndex, String buttonText, long udata);
	
	private AlertDialog alertDialog_;
	private EditText editText_;
	private boolean isVisible_ = false;
	private long udata_;
	
	private String cancel_;
	private String button1_;
	private String button2_;
	
	String text_;
	int inputType_;
	boolean secureInput_;

	static private final int TEXT = 0;
	static private final int NUMBER = 1;
	static private final int PHONE = 2;
	static private final int EMAIL = 3;
	static private final int URL = 4;
	
	public TextInputBox(String title, String message, String text, String cancel, String button1, String button2, long udata)
	{
		cancel_ = cancel;
		button1_ = button1;
		button2_ = button2;
		
		udata_ = udata;
		
		text_ = text;
		inputType_ = TEXT;
		secureInput_ = false;
		
		final TextInputBox this2 = this;
		final String title2 = title;
		final String message2 = message;	
		final String text2 = text;
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
        		
        		editText_ = new EditText(WeakActivityHolder.get());
        		editText_.setText(text2);
        		editText_.requestFocus();
        		editText_.addTextChangedListener(this2);
        		alertDialog_.setView(editText_);

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
	
	void setText(String text)
	{
		text_ = text;

		WeakActivityHolder.get().runOnUiThread(new Runnable()
		{
            @Override
            public void run()
            {
        		editText_.setText(text_);
            }
        });
	}

	String getText()
	{
		return text_;
	}

	private static int convertInputType(int inputType, boolean secureInput)
	{
		int password = secureInput ? InputType.TYPE_TEXT_VARIATION_PASSWORD : 0;

		switch (inputType)
		{
        case TEXT:
        	return InputType.TYPE_CLASS_TEXT | password;
        case NUMBER:
        	return InputType.TYPE_CLASS_NUMBER | password;
        case PHONE:
        	return InputType.TYPE_CLASS_PHONE | password;
        case EMAIL:
        	return InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_EMAIL_ADDRESS | password;
        case URL:
        	return InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_URI | password;        		
		}
		
    	return 0;	
	}
	
	void setInputType(int inputType)
	{
		if (inputType != TEXT && inputType != NUMBER && inputType != PHONE && inputType != EMAIL && inputType != URL)
			return;
		
		inputType_ = inputType;
		
		WeakActivityHolder.get().runOnUiThread(new Runnable()
		{
            @Override
            public void run()
            {
        		editText_.setInputType(convertInputType(inputType_, secureInput_));
            }
        });
	}
	
	int getInputType()
	{
		return inputType_;
	}
	
	void setSecureInput(boolean secureInput)
	{
		secureInput_ = secureInput;

		WeakActivityHolder.get().runOnUiThread(new Runnable()
		{
            @Override
            public void run()
            {
				int inputType = editText_.getInputType();
				if (secureInput_)
					editText_.setInputType(inputType | InputType.TYPE_TEXT_VARIATION_PASSWORD);
				else
					editText_.setInputType(inputType & ~InputType.TYPE_TEXT_VARIATION_PASSWORD);		
            }
        });
	}
	
	boolean isSecureInput()
	{
		return secureInput_;
	}

	@Override
	public void onClick(DialogInterface dialog, int which)
	{
		switch (which)
		{
		case DialogInterface.BUTTON_NEGATIVE:
			completeCallback(editText_.getText().toString(), 0, cancel_, udata_);
			break;
		case DialogInterface.BUTTON_POSITIVE:
			completeCallback(editText_.getText().toString(), 1, button1_, udata_);
			break;
		case DialogInterface.BUTTON_NEUTRAL:
			completeCallback(editText_.getText().toString(), 2, button2_, udata_);
			break;
		}
	}

	@Override
	public void onCancel(DialogInterface dialog)
	{
		completeCallback(editText_.getText().toString(), 0, cancel_, udata_);
	}

	@Override
	public void onShow(DialogInterface dialog)
	{
		isVisible_ = true;
		InputMethodManager imm = (InputMethodManager)WeakActivityHolder.get().getSystemService(Context.INPUT_METHOD_SERVICE);
		imm.showSoftInput(editText_, InputMethodManager.SHOW_IMPLICIT);
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

	@Override
	public void afterTextChanged(Editable s)
	{
		text_ = s.toString();
	}

	@Override
	public void beforeTextChanged(CharSequence s, int start, int count, int after)
	{
	}

	@Override
	public void onTextChanged(CharSequence s, int start, int before, int count)
	{
	}
}
