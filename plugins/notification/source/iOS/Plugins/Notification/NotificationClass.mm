//
//  NotificationClass.mm
//  NotificatinTest
//
//  Created by Arturs Sosins on 3/13/13.
//  Copyright (c) 2013 Arturs Sosins. All rights reserved.
//
#import "NotificationClass.h"
#import "GNotification.h"
#include "notification_wrapper.h"

@interface NSString (NSArrayFormatExtension)

+ (id)stringWithFormat:(NSString *)format array:(NSArray*) arguments;

@end

@implementation NSString (NSArrayFormatExtension)

+ (id)stringWithFormat:(NSString *)format array:(NSArray*) arguments;
{
    char *argList = (char *)malloc(sizeof(NSString *) * [arguments count]);
    [arguments getObjects:(id *)argList];
    NSString* result = [[[NSString alloc] initWithFormat:format arguments:argList] autorelease];
    free(argList);
    return result;
}

@end

@implementation NotificationClass

static bool canDispatch = false;
@synthesize notifics = _notifics;
-(id)initialize{
    self.notifics = [NSMutableDictionary dictionary];
    //subscribe to events
    NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
    [center addObserver:self selector:@selector(onActive) name:UIApplicationDidBecomeActiveNotification object:nil];
    
    [center addObserver:self selector:@selector(onPushRegistration:) name:@"onPushRegistration" object:nil];
    [center addObserver:self selector:@selector(onPushError:) name:@"onPushRegistrationError" object:nil];
    [center addObserver:self selector:@selector(onPrePushHandler:) name:@"onPushNotification" object:nil];
    [center addObserver:self selector:@selector(onPreLocalHandler:) name:@"onLocalNotification" object:nil];
    return self;
}

-(void)onActive{
    // Look for any local or remote notifications that launched the app.  If found
    // nil them out so we don't pick them up a second time.
    id<UIApplicationDelegate> app = [UIApplication sharedApplication].delegate;
    UILocalNotification *launchLocalNotification = [app performSelector:@selector(launchLocalNotification)];
    
    if (launchLocalNotification) {
        [self onLocalHandler:launchLocalNotification wasLaunched:[NSNumber numberWithBool:TRUE]];
        [app performSelector:@selector(setLaunchLocalNotification:) withObject:nil];
    }
    
    NSDictionary *launchRemoteNotification = [app performSelector:@selector(launchRemoteNotification)];
    
    if (launchRemoteNotification) {
        [self onPushHandler:launchRemoteNotification wasLaunched:[NSNumber numberWithBool:TRUE]];
        [app performSelector:@selector(setLaunchRemoteNotification:) withObject:nil];
    }
    
    // Clear the application badge.
    [UIApplication sharedApplication].applicationIconBadgeNumber = 0;
}

-(void)deinitialize{
    NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
    [center removeObserver:self];

    for(NSString *key in self.notifics){
        GNotification *note = [self.notifics objectForKey:key];
        [note release];
    }
    
    canDispatch = false;
}

-(void)init:(int) nid{
 
    GNotification *note = [[GNotification alloc] init:self];
        
    note.nid = nid;
    NSMutableDictionary *dic = [self get:nid fromRepo:@"NotificationLocal"];
    if(dic != NULL)
    {
        note.isDispatched = true;
        note.title = [dic objectForKey:@"title"];
        note.body = [dic objectForKey:@"body"];
        note.sound = [dic objectForKey:@"sound"];
        note.customData = [dic objectForKey:@"custom"];
        note.number = [[dic objectForKey:@"title"] intValue];
    }
    [self.notifics setObject:note forKey:[NSString stringWithFormat:@"%d", nid]];
        
}

-(void)cleanup:(int) nid{
    GNotification *note = [self.notifics objectForKey:[NSString stringWithFormat:@"%d", nid]];
    if(note != NULL){
        [self.notifics removeObjectForKey:[NSString stringWithFormat:@"%d", nid]];
        note = nil;
    }
}

-(void)setTitle:(NSString*)title withID:(int) nid{
    GNotification *note = [self.notifics objectForKey:[NSString stringWithFormat:@"%d", nid]];
    if(note != NULL){
        note.title = title;
    }
}

-(NSString*)getTitle:(int) nid{
    GNotification *note = [self.notifics objectForKey:[NSString stringWithFormat:@"%d", nid]];
    if(note != NULL){
        return note.title;
    }
    return @"";
}

-(void)setBody:(NSString*)body withID:(int) nid{
    GNotification *note = [self.notifics objectForKey:[NSString stringWithFormat:@"%d", nid]];
    if(note != NULL){
        note.body = body;
    }
}

-(NSString*)getBody:(int) nid{
    GNotification *note = [self.notifics objectForKey:[NSString stringWithFormat:@"%d", nid]];
    if(note != NULL){
        return note.body;
    }
    return @"";
}

-(void)setSound:(NSString*)sound withID:(int) nid{
    GNotification *note = [self.notifics objectForKey:[NSString stringWithFormat:@"%d", nid]];
    if(note != NULL){
        note.sound = sound;
    }
}

-(NSString*)getSound:(int) nid{
    GNotification *note = [self.notifics objectForKey:[NSString stringWithFormat:@"%d", nid]];
    if(note != NULL){
        return note.sound;
    }
    return @"";
}

-(void)setCustom:(NSString *)custom withID:(int)nid{
    GNotification *note = [self.notifics objectForKey:[NSString stringWithFormat:@"%d", nid]];
    if(note != NULL){
        note.customData = custom;
    }
}

-(NSString*)getCustom:(int) nid{
    GNotification *note = [self.notifics objectForKey:[NSString stringWithFormat:@"%d", nid]];
    if(note != NULL){
        return note.customData;
    }
    return @"";
}

-(void)setNumber:(int)number withID:(int) nid{
    GNotification *note = [self.notifics objectForKey:[NSString stringWithFormat:@"%d", nid]];
    if(note != NULL){
        note.number = number;
    }
}

-(int)getNumber:(int) nid{
    GNotification *note = [self.notifics objectForKey:[NSString stringWithFormat:@"%d", nid]];
    if(note != NULL){
        return note.number;
    }
    return 0;
}

-(void)dispatchNow:(int) nid{
    GNotification *note = [self.notifics objectForKey:[NSString stringWithFormat:@"%d", nid]];
    if(note != NULL){
        [note createNotification];
    }
}

-(void)dispatchOn:(int) nid onDate:(NSMutableDictionary*) date {
    GNotification *note = [self.notifics objectForKey:[NSString stringWithFormat:@"%d", nid]];
    if(note != NULL){
        NSDateComponents *comps = [self getDate:date];
        note.ftime = [[NSCalendar currentCalendar] dateFromComponents:comps];
        [note createNotification];
    }
}

-(void)dispatchOn:(int) nid onDate:(NSMutableDictionary*) date repeating:(NSMutableDictionary*)repeat {
    GNotification *note = [self.notifics objectForKey:[NSString stringWithFormat:@"%d", nid]];
    if(note != NULL){
        NSDateComponents *comps = [self getDate:date];
        note.ftime = [[NSCalendar currentCalendar] dateFromComponents:comps];
        if ([[date objectForKey:@"year"] intValue] > 0) {
            note.repeat = NSYearCalendarUnit;
        }
        else if ([[date objectForKey:@"month"] intValue] > 0) {
            note.repeat = NSMonthCalendarUnit;
        }
        else if ([[date objectForKey:@"day"] intValue] >= 7) {
            note.repeat = NSWeekCalendarUnit;
        }
        else if ([[date objectForKey:@"day"] intValue] > 0) {
            note.repeat = NSDayCalendarUnit;
        }
        else if ([[date objectForKey:@"hour"] intValue] > 0) {
            note.repeat = NSHourCalendarUnit;
        }
        else if ([[date objectForKey:@"min"] intValue] > 0) {
            note.repeat = NSMinuteCalendarUnit;
        }
        else if ([[date objectForKey:@"sec"] intValue] > 0) {
            note.repeat = NSSecondCalendarUnit;
        }

        [note createNotification];
    }
}

-(void)dispatchAfter:(int) nid onDate:(NSMutableDictionary*) date {
    GNotification *note = [self.notifics objectForKey:[NSString stringWithFormat:@"%d", nid]];
    if(note != NULL){
        NSDateComponents *comps = [self getDate:date];
        note.ftime = [[NSCalendar currentCalendar] dateByAddingComponents:comps toDate:[NSDate date] options:0];
        [note createNotification];
    }
}

-(void)dispatchAfter:(int) nid onDate:(NSMutableDictionary*) date repeating:(NSMutableDictionary*)repeat {
    GNotification *note = [self.notifics objectForKey:[NSString stringWithFormat:@"%d", nid]];
    if(note != NULL){
        NSDateComponents *comps = [self getDate:date];
        note.ftime = [[NSCalendar currentCalendar] dateByAddingComponents:comps toDate:[NSDate date] options:0];
        
        if ([[date objectForKey:@"year"] intValue] > 0) {
            note.repeat = NSYearCalendarUnit;
        }
        else if ([[date objectForKey:@"month"] intValue] > 0) {
            note.repeat = NSMonthCalendarUnit;
        }
        else if ([[date objectForKey:@"day"] intValue] >= 7) {
            note.repeat = NSWeekCalendarUnit;
        }
        else if ([[date objectForKey:@"day"] intValue] > 0) {
            note.repeat = NSDayCalendarUnit;
        }
        else if ([[date objectForKey:@"hour"] intValue] > 0) {
            note.repeat = NSHourCalendarUnit;
        }
        else if ([[date objectForKey:@"min"] intValue] > 0) {
            note.repeat = NSMinuteCalendarUnit;
        }
        else if ([[date objectForKey:@"sec"] intValue] > 0) {
            note.repeat = NSSecondCalendarUnit;
        }

        [note createNotification];
    }
}

-(void)registerForPushNotifications{
    // Let the device know we want to receive push notifications
    [[UIApplication sharedApplication] registerForRemoteNotificationTypes:
     (UIRemoteNotificationTypeBadge | UIRemoteNotificationTypeSound | UIRemoteNotificationTypeAlert)];
}

-(void)unRegisterForPushNotifications{
    // Let the device know we don't want to receive push notifications
    [[UIApplication sharedApplication] unregisterForRemoteNotifications];
}

-(void)readyForEvents{
    canDispatch = true;
    [self dispatchEvents:@"NotificationLocalEvent"];
    [self dispatchEvents:@"NotificationPushEvent"];
}

-(void)dispatchEvents:(NSString*)type{
    NSMutableDictionary *arr = [self getAll:type];
    if(arr)
    {
        for (NSString *key in arr) {
            NSMutableDictionary *dic = [arr objectForKey:key];
            if([type isEqualToString:@"NotificationLocalEvent"])
            {
                [self onLocalNotification:dic];
            }
            else if([type isEqualToString:@"NotificationPushEvent"])
            {
                [self onPushNotification:dic];
            }
        }
        [self deleteAll:type];
    }
}

-(void)onPreLocalHandler: (NSNotification*) note{
    if (note) {
        NSNumber *launched = [[note userInfo] objectForKey:@"launched"];
        [self onLocalHandler:[[note userInfo] objectForKey:@"notification"] wasLaunched:launched];
    }
}

-(void)onPrePushHandler: (NSNotification*) note{
    if (note) {
        NSDictionary *payload = [[note userInfo] objectForKey:@"payload"];
        NSNumber *launched = [[note userInfo] objectForKey:@"launched"];
        [self onPushHandler:payload wasLaunched:launched];
    }
}

-(void)onLocalHandler: (UILocalNotification*)note wasLaunched:(NSNumber *)launched {
    if (note) {
        if (!launched) {
            launched = [NSNumber numberWithBool:FALSE];
        }
        if ([launched boolValue]) {
            NSString *noteId = [[note userInfo] objectForKey:@"nid"] ?: @"0";
            [self setLaunchedFlag:launched forId:noteId forRepo:@"NotificationLocal"];
        }
        if(canDispatch)
        {
            NSMutableDictionary *dic = [NSMutableDictionary dictionary];
            [dic setObject:[note alertAction] forKey:@"title"];
            [dic setObject:[note alertBody] forKey:@"body"];
            [dic setObject:[note soundName] ?: @"" forKey:@"sound"];
            [dic setObject:[NSString stringWithFormat:@"%d", [note applicationIconBadgeNumber] ?: 0] forKey:@"number"];
            [dic setObject:[[note userInfo] objectForKey:@"nid"] ?: @"0" forKey:@"id"];
            [dic setObject:[[note userInfo] objectForKey:@"custom"] ?: @"" forKey:@"custom"];
            [dic setObject:launched forKey:@"launched"];
            [self onLocalNotification:dic];
        }
        else
        {
            [self safe:[[[note userInfo] objectForKey:@"nid"] intValue] title:[note alertAction] body:[note alertBody] sound:[note soundName] ?: @"" number:[note applicationIconBadgeNumber] ?: 0 custom:[[note userInfo] objectForKey:@"custom"] launched:launched inRepo:@"NotificationLocalEvent"];
        }
    }
}

/**
 Validates and formats push notification 'custom' field.  We expect it to be a
 string but if it's something else we JSON convert it.
 */
-(NSString *)formatCustom:(NSDictionary*)note {
    NSString *result = @"";
    NSObject *custom = [note objectForKey:@"custom"];
    
    if (custom) {
        // NSString, NSNumber, NSArray, NSDictionary
        if ([custom isKindOfClass:[NSString class]]) {
            result = (NSString *)custom;
        }
        else if ([custom isKindOfClass:[NSNumber class]]) {
            result = [NSString stringWithFormat:@"%@", custom];
        }
        else if ([NSJSONSerialization isValidJSONObject:custom]) {
            NSData *jsonData = [NSJSONSerialization dataWithJSONObject:custom options:0 error:nil];
            result = [[NSString alloc] initWithData:jsonData encoding:NSUTF8StringEncoding];
        }
    }
    
    return result;
}

-(void)onPushHandler: (NSDictionary*)note wasLaunched:(NSNumber *)launched {
    if(note){
        if (!launched) {
            launched = [NSNumber numberWithBool:FALSE];
        }
        if ([launched boolValue]) {
            NSString *noteId = [[note objectForKey:@"aps"] objectForKey:@"id"] ?: @"0";
            [self setLaunchedFlag:launched forId:noteId forRepo:@"NotificationPush"];
        }
        NSDictionary *aps = [note valueForKey:@"aps"];
        NSMutableDictionary *dic = [NSMutableDictionary dictionary];
        if([[aps valueForKey:@"alert"] isKindOfClass:[NSDictionary class]]){
            NSDictionary *alert = [aps valueForKey:@"alert"];
            
            // Message body, either from 'loc-key' + 'loc-args' or 'body'.
            NSString *body = @"";
            NSString *locKey =[alert valueForKey:@"loc-key"];
            
            if (locKey){
                NSArray *locArgs = [alert valueForKey:@"loc-args"];
                if (locArgs) {
                    body = [NSString stringWithFormat:NSLocalizedString(locKey, @""), locArgs];
                }
                else {
                    body = NSLocalizedString(locKey, @"");
                }
            }
            else{
                body = [alert valueForKey:@"body"];
            }
            
            [dic setObject:body forKey:@"body"];
            
            [dic setObject:[alert valueForKey:@"action-loc-key"] ?: [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleDisplayName"] forKey:@"title"];
        }
        else{
            [dic setObject:[[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleDisplayName"] forKey:@"title"];
            [dic setObject:[aps valueForKey:@"alert"] ?: @"" forKey:@"body"];
        }
        
        [dic setObject:[aps valueForKey:@"sound"] ?: @"" forKey:@"sound"];
        [dic setObject:[aps valueForKey:@"badge"] ?: @"0" forKey:@"number"];
        [dic setObject:[aps valueForKey:@"id"] ?: @"0" forKey:@"id"];
        NSString *custom = [self formatCustom:note];
        [dic setObject:custom forKey:@"custom"];
        [dic setObject:launched forKey:@"launched"];
        
        if(canDispatch)
        {
            [self onPushNotification:dic];
        }
        else
        {
            [self safe:[[dic objectForKey:@"id"] intValue] title:[dic objectForKey:@"title"] body:[dic objectForKey:@"body"] sound:[dic objectForKey:@"sound"] number:[[dic objectForKey:@"number"] intValue] custom:[dic objectForKey:@"custom"] launched:launched inRepo:@"NotificationPushEvent"];
        }
        [self safe:[[dic objectForKey:@"id"] intValue] title:[dic objectForKey:@"title"] body:[dic objectForKey:@"body"] sound:[dic objectForKey:@"sound"] number:[[dic objectForKey:@"number"] intValue] custom:[dic objectForKey:@"custom"] launched:launched inRepo:@"NotificationPush"];
    }
}

-(void)onLocalNotification: (NSMutableDictionary*) note{
    //call the C API
    int nid = [[note objectForKey:@"id"] intValue];
    const char *title = [[note objectForKey:@"title"] UTF8String];
    const char *text = [[note objectForKey:@"body"] UTF8String];
    int number = [[note objectForKey:@"number"] intValue];
    const char *sound = [[note objectForKey:@"sound"] UTF8String];
    const char *custom = [[note objectForKey:@"custom"] UTF8String];
    bool launched = !![[note objectForKey:@"launched"] boolValue];
    gnotification_onLocalNotification(nid, title, text, number, sound, custom, launched);
}

-(void)onPushNotification: (NSMutableDictionary*) note{
    //call the C API
    int nid = [[note objectForKey:@"id"] intValue];
    const char *title = [[note objectForKey:@"title"] UTF8String];
    const char *text = [[note objectForKey:@"body"] UTF8String];
    int number = [[note objectForKey:@"number"] intValue];
    const char *sound = [[note objectForKey:@"sound"] UTF8String];
    const char *custom = [[note objectForKey:@"custom"] UTF8String];
    bool launched = !![[note objectForKey:@"launched"] boolValue];
    gnotification_onPushNotification(nid, title, text, number, sound, custom, launched);
}

-(void)onPushRegistration: (NSNotification*) n{
    NSString *token = [[n userInfo] objectForKey:@"token"];
    token = [token stringByTrimmingCharactersInSet:[NSCharacterSet characterSetWithCharactersInString:@"<>"]];
	token = [token stringByReplacingOccurrencesOfString:@" " withString:@""];
    gnotification_onPushRegistration([token UTF8String]);
}

-(void)onPushError: (NSNotification*) n{
    NSError *error = [[n userInfo] objectForKey:@"error"];
    gnotification_onPushRegistrationError([[error localizedDescription] UTF8String]);
}

-(void)cancel:(int) nid{
    [self cleanup: nid];
    [self internalCancel:nid];
}

-(void)internalCancel:(int) nid{
    NSString *strId = [NSString stringWithFormat:@"%d", nid];
    NSArray *notifications = [[UIApplication sharedApplication] scheduledLocalNotifications];
    for(UILocalNotification *note in notifications){
        NSString *notId = [note.userInfo objectForKey:@"nid"];
        if([strId isEqualToString:notId])
        {
            [[UIApplication sharedApplication] cancelLocalNotification:note];
        }
    }
    [self del:nid inRepo:@"NotificationLocal"];
}

-(void)cancelAll{
    for(NSString *key in self.notifics){
        GNotification *note = [self.notifics objectForKey:key];
        [note release];
        note = nil;
    }
    [self.notifics removeAllObjects];
    [[UIApplication sharedApplication] cancelAllLocalNotifications];
    NSMutableDictionary *local = [self getAll:@"NotificationLocal"];
    if(local)
    {
        for(NSString *key in local) {
            NSMutableDictionary *dic = [local objectForKey:key];
            if([self checkNotification:[dic objectForKey:@"id"]])
            {
                [local removeObjectForKey:[dic objectForKey:@"id"]];
            }
        }
    
        NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
        [defaults setObject:local forKey:@"NotificationLocal"];
        [defaults synchronize];
    }
}

-(NSMutableDictionary*)getScheduledNotifications{
    NSMutableDictionary *ret = [NSMutableDictionary dictionary];
    
    NSArray *notifications = [[UIApplication sharedApplication] scheduledLocalNotifications];
    for(UILocalNotification *note in notifications){
        
        NSMutableDictionary *d = [NSMutableDictionary dictionary];
        int nid = [[note.userInfo objectForKey:@"nid"] intValue];
        [d setObject:[NSNumber numberWithInt:nid] forKey:@"id"];
        [d setObject:note.alertAction forKey:@"title"];
        [d setObject:note.alertBody forKey:@"body"];
        [d setObject:note.soundName forKey:@"sound"];
        [d setObject:[note.userInfo objectForKey:@"custom"] forKey:@"custom"];
        NSNumber *number = [NSNumber numberWithInt:note.applicationIconBadgeNumber];
        [d setObject:number forKey:@"number"];
        
        [ret setObject:d forKey:[NSString stringWithFormat:@"%d", nid]];
    }
    return ret;
}

-(NSMutableDictionary*)getLocalNotifications{
    NSMutableDictionary *local = [self getAll:@"NotificationLocal"];
    NSMutableDictionary *ret = [NSMutableDictionary dictionary];
    if(local)
    {
        for(NSString *key in local) {
            NSMutableDictionary *dic = [local objectForKey:key];
            if(![self checkNotification:[dic objectForKey:@"id"]])
            {
                [ret setObject:dic forKey:[dic objectForKey:@"id"]];
            }
        }
    }
    return ret;
}

-(NSMutableDictionary*)getPushNotifications{
    return [self getAll:@"NotificationPush"];
}

typedef BOOL (^RepoVisitor)(NSString *nid, NSDictionary *note, NSMutableDictionary *repo);

-(void)visitRepo:(NSString *)repoKey visitor:(RepoVisitor)visitor {
    NSMutableDictionary *repo = [self getAll:repoKey];
    if(repo != nil)
    {
        for(NSString *key in [repo allKeys]) {
            if (visitor(key, [repo objectForKey:key], repo)) {
                break;
            }
        }
        
        NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
        [defaults setObject:repo forKey:repoKey];
        [defaults synchronize];
    }
}

-(void) setLaunchedFlag:(NSNumber *)launched forId:(NSString *)nid forRepo:(NSString *)repo {
    [self visitRepo:repo visitor:^BOOL(NSString *key, NSDictionary *note, NSMutableDictionary *repo) {
        if ([key isEqualToString:nid]) {
            NSMutableDictionary *newNote = [note mutableCopy];
            [newNote setObject:[launched copy] forKey:@"launched"];
            [repo removeObjectForKey:nid];
            [repo setObject:newNote forKey:nid];
            return TRUE;
        }
        return FALSE;
    }];
}

-(void)clearLocalNotifications{
    NSMutableDictionary *local = [self getAll:@"NotificationLocal"];
    if(local != nil)
    {
        for(NSString *key in [local allKeys]) {
            NSMutableDictionary *dic = [local objectForKey:key];
            if(![self checkNotification:[dic objectForKey:@"id"]])
            {
                [local removeObjectForKey:[dic objectForKey:@"id"]];
            }
        }
        NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
        [defaults setObject:local forKey:@"NotificationLocal"];
        [defaults synchronize];
    }
}

-(void)clearPushNotifications{
    [self deleteAll:@"NotificationPush"];
}

-(NSDateComponents*)getDate:(NSMutableDictionary*) date{
    NSDateComponents *comps = [[NSDateComponents alloc] init];
    [comps setSecond:[[date objectForKey:@"sec"] intValue]];
    [comps setMinute:[[date objectForKey:@"min"] intValue]];
    [comps setHour:[[date objectForKey:@"hour"] intValue]];
    [comps setDay:[[date objectForKey:@"day"] intValue]];
    [comps setMonth:[[date objectForKey:@"month"] intValue]];
    [comps setYear:[[date objectForKey:@"year"] intValue]];
    return comps;
}

-(void)safe:(int)nid title:(NSString*)title body:(NSString*)body sound:(NSString*)sound number:(int)number custom:(NSString*)custom launched:(NSNumber *)launched inRepo:(NSString*) repo{
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSMutableDictionary *arr = [[defaults objectForKey:repo] mutableCopy];
    if(arr == NULL)
    {
        arr = [NSMutableDictionary dictionary];
    }
    NSMutableDictionary *dic = [NSMutableDictionary dictionary];
    [dic setObject:[NSString stringWithFormat:@"%d", nid] forKey:@"id"];
    [dic setObject:title forKey:@"title"];
    [dic setObject:body forKey:@"body"];
    [dic setObject:sound forKey:@"sound"];
    [dic setObject:custom forKey:@"custom"];
    [dic setObject:[NSString stringWithFormat:@"%d", number] forKey:@"number"];
    [arr setObject:dic forKey:[NSString stringWithFormat:@"%d", nid]];
    [defaults setObject:arr forKey:repo];
    [defaults synchronize];
}

-(void)del:(int) nid inRepo:(NSString*) repo{
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSMutableDictionary *arr = [[defaults objectForKey:repo] mutableCopy];
    if(arr)
    {
        [arr removeObjectForKey:[NSString stringWithFormat:@"%d", nid]];
        [defaults setObject:arr forKey:repo];
        [defaults synchronize];
    }
}

-(void)deleteAll:(NSString*) repo{
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    [defaults removeObjectForKey:repo];
    [defaults synchronize];
}

-(NSMutableDictionary*)get:(int) nid fromRepo:(NSString*) repo{
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSMutableDictionary *arr = [[defaults objectForKey:repo] mutableCopy];
    return [arr objectForKey:[NSString stringWithFormat:@"%d", nid]];
}

-(NSMutableDictionary*)getAll:(NSString*) repo{
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    return [[defaults objectForKey:repo] mutableCopy];
}

-(bool)checkNotification:(NSString*)nid{
    bool ret = false;
    NSArray *notifications = [[UIApplication sharedApplication] scheduledLocalNotifications];
    for(UILocalNotification *note in notifications){
        NSString *notId = [note.userInfo objectForKey:@"nid"];
        if([nid isEqualToString:notId])
        {
            ret = true;
            break;
        }
    }
    return ret;
}

@end