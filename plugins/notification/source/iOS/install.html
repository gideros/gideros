<h2>IOS</h2>
<h4>1) Gideros project</h4>
<ul>
<li>Create Gideros project</li>
<li>Export it as IOS project</li>
<li>Open project in Xcode</li>
</ul>
<h4>2) Copying files</h4>
<ul>
<li>Copy Plugins/Notification folder int your Xcode project's Plugins folder</li>
<li>Add files to your Xcode project:
<ul>
<li>Right click on Plugins folder in your Xcode project</li>
<li>Select Add file to "Your project name"</li>
<li>Select: Create groups for any added folders</li>
<li>select Notification folder and click Add</li>
</ul>
</li>
</ul>
<h4>3) Modify AppDelegate file</h4>
<ul>
<li>Add these properties to AppDelegate.h file class:</li>
<ul>
	<li><pre>@property (nonatomic, retain) UILocalNotification *launchLocalNotification;</pre></li>
	<li><pre>@property (nonatomic, retain) NSDictionary *launchRemoteNotification;</pre></li>
</ul>
<li>Add these lines in the start of the <code>application didFinishLaunchingWithOptions</code> method in the AppDelegate.mm file:</li>
<ul>
	<li><pre>self.launchLocalNotification =
    [launchOptions objectForKey:UIApplicationLaunchOptionsLocalNotificationKey];</pre></li>
	<li><pre>self.launchRemoteNotification =
    [launchOptions objectForKey:UIApplicationLaunchOptionsRemoteNotificationKey];</pre></li>
</ul>
<li>Add these methods at the end of AppDelegate.m file class:</li>
<ul>
<li><pre>- (void)application:(UIApplication *)application didReceiveLocalNotification:(UILocalNotification *)notification {
    UIApplicationState state = application.applicationState;
    NSNumber *launched = [NSNumber numberWithBool:(state ==UIApplicationStateInactive)];
    NSDictionary *dic = [NSDictionary dictionaryWithObjectsAndKeys:notification, @"notification", launched, @"launched", nil];
    [[NSNotificationCenter defaultCenter] postNotificationName:@"onLocalNotification" object:self userInfo:dic];
}</pre></li>
<li><pre>- (void)application:(UIApplication*)application didReceiveRemoteNotification:(NSDictionary*)userInfo
{
    UIApplicationState state = application.applicationState;
    NSNumber *launched = [NSNumber numberWithBool:(state ==UIApplicationStateInactive)];
    NSDictionary *dic = [NSDictionary dictionaryWithObjectsAndKeys:userInfo, @"payLoad", launched, @"launched", nil];
    [[NSNotificationCenter defaultCenter] postNotificationName:@"onPushNotification" object:self userInfo:dic];
}</pre></li>
<li><pre>- (void)application:(UIApplication*)application didRegisterForRemoteNotificationsWithDeviceToken:(NSData*)deviceToken
{
    NSDictionary *dic = [NSDictionary dictionaryWithObject:[deviceToken description] forKey:@"token"];
    [[NSNotificationCenter defaultCenter] postNotificationName:@"onPushRegistration" object:self userInfo:dic];
}</pre></li>
<li><pre>- (void)application:(UIApplication*)application didFailToRegisterForRemoteNotificationsWithError:(NSError*)error
{
    NSDictionary *dic = [NSDictionary dictionaryWithObject:error forKey:@"error"];
    [[NSNotificationCenter defaultCenter] postNotificationName:@"onPushRegistrationError" object:self userInfo:dic];
}</pre></li>
</ul>
</ul>