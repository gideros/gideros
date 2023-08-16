package com.giderosmobile.android.plugins.ads.frameworks;

import java.lang.ref.WeakReference;
import java.util.Hashtable;

import android.app.Activity;
import android.content.DialogInterface;
import androidx.fragment.app.FragmentActivity;
import android.util.SparseArray;
import android.view.KeyEvent;

import com.giderosmobile.android.plugins.ads.*;

import me.kiip.sdk.Kiip;
import me.kiip.sdk.KiipFragment;
import me.kiip.sdk.KiipFragmentCompat;
import me.kiip.sdk.Modal;
import me.kiip.sdk.Poptart;


public class AdsKiip implements AdsInterface, Kiip.OnContentListener, DialogInterface.OnShowListener, DialogInterface.OnDismissListener {

    private WeakReference<Activity> sActivity;
    private AdsManager mngr;
    private final static String KIIP_TAG = "kiip_fragment_tag";
    private KiipFragmentCompat mKiipFragment;
    static private AdsKiip me;
    Hashtable<Poptart,String> popMap=new Hashtable<Poptart, String>();

    public void onCreate(WeakReference<Activity> activity)
    {
        me = this;
        sActivity = activity;
        mngr = new AdsManager();

        // Create or re-use KiipFragment.
        mKiipFragment = (KiipFragmentCompat) ((FragmentActivity)activity.get()).getSupportFragmentManager().findFragmentByTag(KIIP_TAG);
        if (mKiipFragment==null) {
            mKiipFragment = new KiipFragmentCompat();
            ((FragmentActivity)activity.get()).getSupportFragmentManager().beginTransaction().add(mKiipFragment, KIIP_TAG).commit();
        }
        mKiipFragment.setOnShowListener(this);
        mKiipFragment.setOnDismissListener(this);
    }

    //on destroy event
    public void onDestroy(){}

    public void onStart(){
        Kiip.getInstance().startSession(new Kiip.Callback() {
            @Override
            public void onFailed(Kiip kiip, Exception exception) {
                // handle failure
                Ads.adFailed(me,"startSession",exception.toString());
            }

            @Override
            public void onFinished(Kiip kiip, Poptart poptart) {
                onPoptart("startSession",poptart);
            }
        });
    }

    public void onStop(){
        Kiip.getInstance().endSession(new Kiip.Callback() {
            @Override
            public void onFailed(Kiip kiip, Exception exception) {
                // handle failure
                Ads.adFailed(me,"startSession",exception.toString());
            }

            @Override
            public void onFinished(Kiip kiip, Poptart poptart) {
                onPoptart("endSession",poptart);
            }
        });
    }

    public void onPause()
    {
    }

    public void onResume()
    {
    }

    public boolean onKeyUp(int keyCode, KeyEvent event) {
        return false;
    }

    public void onPoptart(final String type,Poptart poptart) {
        if (poptart!=null) {
            popMap.put(poptart,type);
            mKiipFragment.showPoptart(poptart);
        }
    }

    public void setKey(final Object parameters){
        SparseArray<String> param = (SparseArray<String>)parameters;
        Kiip.init(sActivity.get().getApplication(), "GIDEROS_KIIP_APP_KEY", param.get(0));
        Kiip.getInstance().setOnContentListener(this);
    }

    @Override
    public void onShow(DialogInterface dialogInterface) {
        String type=popMap.get(dialogInterface);
        Ads.adDisplayed(me,type);
    }

    @Override
    public void onDismiss(DialogInterface dialogInterface) {
        String type=popMap.get(dialogInterface);
        Ads.adDismissed(me,type);
        mngr.reset(type,true);
        popMap.remove(dialogInterface);
    }


    private static class AdContext {
        Poptart poptart;
    }

    //load an Ad
    public void loadAd(final Object parameters)
    {
        final SparseArray<String> param = (SparseArray<String>)parameters;
        final String type = param.get(0);
        AdContext adctx=new AdContext();
        Kiip.getInstance().saveMoment(type, new Kiip.Callback() {
            @Override
            public void onFinished(Kiip kiip, Poptart reward) {
                AdContext adctx= (AdContext) mngr.get(type);
                adctx.poptart=reward;
                if (reward!=null)
                {
                    Ads.adReceived(me,type);
                    mngr.load(type);
                }
                else
                    Ads.adFailed(me,type,"no fill");
            }

            @Override
            public void onFailed(Kiip kiip, Exception exception) {
                // handle failure
                Ads.adFailed(me,type,exception.toString());
            }
        });
        mngr.set(adctx, type, new AdsStateChangeListener(){
            @Override
            public void onShow() {
                AdContext adctx= (AdContext) mngr.get(type);
                onPoptart(type,adctx.poptart);
            }
            @Override
            public void onDestroy() {}
            @Override
            public void onHide() {
                AdContext adctx= (AdContext) mngr.get(type);
                mKiipFragment.dismissPoptart(adctx.poptart);
            }
            @Override
            public void onRefresh() {}
        });
        mngr.setAutoKill(type,false);
    }

    public void showAd(final Object parameters)
    {
        SparseArray<String> param = (SparseArray<String>)parameters;
        String type = param.get(0);
        if(mngr.get(type) == null)
        {
            loadAd(parameters);
        }
        mngr.show(type);
    }

    //remove ad
    public void hideAd(String type)
    {
        mngr.hide(type);
    }

    public int getWidth(){
        return 0;
    }

    public int getHeight(){
        return 0;
    }

    @Override
    public void enableTesting() {
        Kiip.getInstance().setTestMode(true);
    }

	@Override
	public boolean checkConsent(boolean underAge) { return false; }
	
    @Override
    public void onContent(Kiip kiip, String momentId, int quantity, String transactionId,
                          String signature) {
            Ads.adRewarded(this,momentId,quantity);
    }

}
