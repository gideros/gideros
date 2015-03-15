<h1>Notification installation instructions</h1>
<h2>Android</h2>
<h4>1) Gideros project</h4>
<ul>
<li>Create Gideros project</li>
<li>Export it as Android project</li>
<li>Import it in Eclipse</li>
</ul>
<h4>2) Copying files</h4>
<ul>
<li>Copy .so files from libs folder to each separate armeabi folder</li>
<li>Add gcm.to the project
<ul>
<li>Copy gcm.jar into libs folder</li>
<li>Right click on Reference libraries</li>
<li>Select Build path -> Configure Build Path</li>
<li>Click Add JARs and navigate to your project libs folder.</li>
<li>Select gcm.jar and click OK (if it's not there, but you have copied it, refresh the project)</li>
<li>Go to Order and Export tab and check gcm.jar</li>
</ul>
</li>
<li>Copy to src/giderosmobile/android/plugins folder/notification folder into project's src/giderosmobile/android/plugins folder</li>
</ul>
<h4>3) Modify Android manifest</h4>
<ul>
<li>Add permissions: (replace com.yourdomain.yourapp with your bundle name)
<ul>
<li><pre>&lt;permission android:name="com.yourdomain.yourapp.permission.C2D_MESSAGE" android:protectionLevel="signature" /&gt;</pre></li>
<li><pre>&lt;uses-permission android:name="com.yourdomain.yourapp.permission.C2D_MESSAGE" /&gt;</pre></li>
<li><pre>&lt;uses-permission android:name="com.google.android.c2dm.permission.RECEIVE" /&gt;</pre></li>
<li><pre>&lt;uses-permission android:name="android.permission.GET_ACCOUNTS" /&gt;</pre></li>
<li><pre>&lt;uses-permission android:name="android.permission.WAKE_LOCK" /&gt;</pre></li>
</ul>
</li>
<li>Add services and receivers to application tag (replace com.yourdomain.yourapp with your bundle name) example manifest included:
<ul>
<li><pre>&lt;receiver android:name="com.giderosmobile.android.plugins.notification.NotificationClass"&gt;&lt;/receiver&gt;</pre></li>
<li><pre>&lt;receiver android:name="com.giderosmobile.android.plugins.notification.NotificationRestore" &gt;
   	&lt;intent-filter&gt;
    	&lt;action android:name="android.intent.action.BOOT_COMPLETED" /&gt;
    &lt;/intent-filter&gt;
&lt;/receiver&gt;</pre></li>
<li><pre>&lt;receiver android:name="com.giderosmobile.android.plugins.notification.GCMReceiver" android:permission="com.google.android.c2dm.permission.SEND" &gt;
  	&lt;intent-filter&gt;
  		&lt;action android:name="com.google.android.c2dm.intent.RECEIVE" /&gt;
  		&lt;action android:name="com.google.android.c2dm.intent.REGISTRATION" /&gt;
  		&lt;category android:name="com.yourdomain.yourapp" /&gt;
  	&lt;/intent-filter&gt;
&lt;/receiver&gt;</pre></li>
<li><pre>&lt;service android:name="com.giderosmobile.android.plugins.notification.GCMIntentService" /&gt;</pre></li>
</ul>
</li>
</ul>
<h4>4) Modify Main activity file</h4>
<ul>
<li>Load notification library: System.loadLibrary("notification");</li>
<li>Add external class: "com.giderosmobile.android.plugins.notification.NotificationClass"</li>
<li>Add intent handing: <pre>@Override
protected void onNewIntent(Intent intent) {
    super.onNewIntent(intent);
    setIntent(intent);
}</pre></li>
</ul>
