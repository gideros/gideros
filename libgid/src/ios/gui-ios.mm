#include <gui.h>
#include <UIKit/UIKit.h>
#include <map>
#include <deque>

#include <stdexcept>

class UIManager
{
public:
    UIManager();
    ~UIManager();
	
    g_id createAlertDialog(const char *title,
                           const char *message,
                           const char *cancelButton,
                           const char *button1,
                           const char *button2,
                           gevent_Callback callback,
                           void *udata);
	
    g_id createTextInputDialog(const char *title,
                               const char *message,
                               const char *text,
                               const char *cancelButton,
                               const char *button1,
                               const char *button2,
                               gevent_Callback callback,
                               void *udata);
	
    void show(g_id gid);
    void hide(g_id gid);
    void deleteWidget(g_id gid);
    bool isVisible(g_id gid);
	
    void setText(g_id gid, const char* text);
    const char *getText(g_id gid);
    void setInputType(g_id gid, int inputType);
    int getInputType(g_id gid);
    void setSecureInput(g_id gid, bool secureInput);
    bool isSecureInput(g_id gid);
	
private:
    std::map<g_id, id> map_;
};

#if TARGET_OS_TV == 1
/* We override presses to handle Cancel button when MENU button is pressed on Apple TV */
@interface UIAlertController (pressesOverride)
@end

@implementation UIAlertController (pressesOverride)
-(void)pressesBegan:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event
{
    if (presses.anyObject.type == UIPressTypeMenu) {
        [self dismissViewControllerAnimated:YES completion:^{
            [[NSNotificationCenter defaultCenter]
             postNotificationName:@"UIAlertControllerPressMenuButton"
             object:self];
        }];
        return;
    }
    
    [super pressesBegan:presses withEvent:event];

    return;
}

@end
#endif

@interface GGAlertDialog : UIViewController
{
	id alertView_;
	gevent_Callback callback_;
	void *udata_;
	g_id gid_;
}

@end


@implementation GGAlertDialog

- (void) forceCancel {
	[[NSNotificationCenter defaultCenter] removeObserver:self
                                                    name:@"UIAlertControllerPressMenuButton"
                                                  object:nil];
        [self buttonPressed:@"" withIndex:0];
}

- (id)initWithTitle:(NSString *)title
			message:(NSString *)message
	   cancelButton:(NSString *)cancelButton 
			button1:(NSString *)button1
			button2:(NSString *)button2
		   callback:(gevent_Callback)callback
			  udata:(void*)udata
				gid:(g_id)gid
{
    if (self = [super init])
    {
	[[NSNotificationCenter defaultCenter] addObserver:self
                                           selector:@selector(forceCancel)
                                               name:@"UIAlertControllerPressMenuButton"
                                             object:nil];
	if ([[UIDevice currentDevice].systemVersion floatValue] >= 8) {
		alertView_ = [UIAlertController alertControllerWithTitle:title
                               message:message
                               preferredStyle:UIAlertControllerStyleAlert];
		[alertView_ retain];

		UIAlertAction* defaultAction = [UIAlertAction actionWithTitle:cancelButton style:UIAlertActionStyleDefault   
			handler:^(UIAlertAction * action) {
				[self buttonPressed:cancelButton withIndex:0];
			}];

		[alertView_ addAction:defaultAction];


		if (button1) {
			UIAlertAction* button1Action = [UIAlertAction actionWithTitle:button1 style:UIAlertActionStyleDefault
                        handler:^(UIAlertAction * action) {
				[self buttonPressed:button1 withIndex:1];
                        }];
			[alertView_ addAction:button1Action];
		}

		if (button2) {
			UIAlertAction* button2Action = [UIAlertAction actionWithTitle:button2 style:UIAlertActionStyleDefault
                        handler:^(UIAlertAction * action) {
				[self buttonPressed:button2 withIndex:2];
                        }];
                        [alertView_ addAction:button2Action];
		}

		callback_ = callback;
		udata_ = udata;
		gid_ = gid;
	}
#if TARGET_OS_TV == 0
	else
	{
                alertView_ = [[UIAlertView alloc] initWithTitle:title
                                                                                                message:message
                                                                                           delegate:self
                                                                          cancelButtonTitle:cancelButton
                                                                          otherButtonTitles:nil];

                if (button1)
                        [alertView_ addButtonWithTitle:button1];

                if (button2)
                        [alertView_ addButtonWithTitle:button2];

                callback_ = callback;
                udata_ = udata;
                gid_ = gid;	
	}
#endif
    }
    return self;
}

- (void)dealloc
{
#if TARGET_OS_TV == 0
		((UIAlertView *)alertView_).delegate = nil;
	        [alertView_ dismissWithClickedButtonIndex:-1 animated:NO];
#else
	if ([alertView_ isViewLoaded]) [alertView_ dismissViewControllerAnimated:YES completion:nil];
#endif
        	[alertView_ release];

	[super dealloc];
}

- (void)show
{	
	if ([[UIDevice currentDevice].systemVersion floatValue] >= 8) {
		[[UIApplication sharedApplication].keyWindow.rootViewController presentViewController:alertView_ animated:YES completion:nil];
	}
#if TARGET_OS_TV == 0
	else
	{
		[alertView_ show];
	}
#endif
}

- (void)hide
{
	if ([[UIDevice currentDevice].systemVersion floatValue] >= 8) {
		if ([alertView_ isViewLoaded]) [alertView_ dismissViewControllerAnimated:YES completion:nil];
	}
#if TARGET_OS_TV == 0
	else
	{	
		[alertView_ dismissWithClickedButtonIndex:-1 animated:YES];
	}
#endif
}

- (BOOL)isVisible
{
	if ([[UIDevice currentDevice].systemVersion floatValue] >= 8) {
		return [alertView_ isViewLoaded];
	}
#if TARGET_OS_TV == 0
	else
	{
		return [alertView_ isVisible];
	}
#endif
	return NO;
}

- (void)buttonPressed:(NSString *)text withIndex:(NSInteger)buttonIndex 
{
	const char *buttonText = [text UTF8String];
        size_t size = sizeof(gui_AlertDialogCompleteEvent) + strlen(buttonText) + 1;
        gui_AlertDialogCompleteEvent *event = (gui_AlertDialogCompleteEvent*)malloc(size);
        event->gid = gid_;
        event->buttonIndex = (int)buttonIndex;
        event->buttonText = (char*)event + sizeof(gui_AlertDialogCompleteEvent);
        strcpy((char*)event->buttonText, buttonText);

        gevent_EnqueueEvent(gid_, callback_, GUI_ALERT_DIALOG_COMPLETE_EVENT, event, 1, udata_);
}

#if TARGET_OS_TV == 0
- (void)alertView:(UIAlertView *)alertView didDismissWithButtonIndex:(NSInteger)buttonIndex
{
        if (buttonIndex >= 0)
        {
        [self buttonPressed:[alertView_ buttonTitleAtIndex:buttonIndex] withIndex:buttonIndex];
        }
}
#endif

@end


#if TARGET_OS_TV == 0

@interface GGAlertView : UIAlertView
{
}

@property(nonatomic, retain) UITextField *textFieldEx;
@property(nonatomic, retain) UILabel *messageEx;

- (void)orientationDidChange:(NSNotification *)notification;

@end

@implementation GGAlertView

@synthesize textFieldEx = textField_;
@synthesize messageEx = message_;

- (id)initWithTitle:(NSString *)title
			message:(NSString *)message
		   delegate:(id)delegate
  cancelButtonTitle:(NSString *)cancelButtonTitle
{
    if (self = [super initWithTitle:title
                            message:nil
                           delegate:delegate
                  cancelButtonTitle:cancelButtonTitle
                  otherButtonTitles:nil])
    {
        if (message != nil && message.length > 0)
            self.message = @"\n\n\n";
        else
            self.message = @"\n\n";
		
        message_ = [[UILabel alloc] initWithFrame:CGRectZero];
        message_.backgroundColor = [UIColor clearColor];        
        message_.textAlignment = NSTextAlignmentCenter;
        message_.textColor = [UIColor whiteColor];
        message_.font = [UIFont systemFontOfSize:16];
        message_.text = message;
        [self addSubview:message_];        
        
        textField_ = [[UITextField alloc] initWithFrame:CGRectZero];
        textField_.backgroundColor = [UIColor whiteColor];
        textField_.borderStyle = UITextBorderStyleBezel;
        textField_.font = [UIFont systemFontOfSize:19];
        [textField_ becomeFirstResponder];
        [self addSubview:textField_];
        
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(orientationDidChange:) name:UIDeviceOrientationDidChangeNotification object:nil];        
        [[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];    
    }
    
    return self;
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self name:UIDeviceOrientationDidChangeNotification object:nil];
    [[UIDevice currentDevice] endGeneratingDeviceOrientationNotifications];
    
    [message_ release];
    [textField_ release];
    
    [super dealloc];
}

- (void)layoutSubviews
{
    // We assume keyboard is on.
    if ([[UIDevice currentDevice] isGeneratingDeviceOrientationNotifications])
    {
        CGRect screenRect = [[UIScreen mainScreen] bounds];
        CGFloat screenWidth = screenRect.size.width;
        CGFloat screenHeight = screenRect.size.height;
        CGFloat height = self.bounds.size.height;
        BOOL portrait = UIInterfaceOrientationIsPortrait([[UIApplication sharedApplication] statusBarOrientation]);
        BOOL iphone = ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPhone);
        
        if (portrait)
        {
            self.center = CGPointMake(screenWidth / 2, (screenHeight - 20 - height) / 2 + 12.0f);
        }
        else
        {
            self.center = CGPointMake(screenHeight / 2, (screenWidth - 20 - height) / 2 + 12.0f);
        }
        
        if (iphone && !portrait)
        {
            message_.frame = CGRectMake(12, self.bounds.size.height - 125, 260, 21);
            textField_.frame = CGRectMake(12, self.bounds.size.height - 95, 260, 31);
        }
        else
        {
            message_.frame = CGRectMake(12, self.bounds.size.height - 135, 260, 21);
            textField_.frame = CGRectMake(12, self.bounds.size.height - 100, 260, 31);            
        }
    }
}

- (void)orientationDidChange:(NSNotification *)notification
{
    [self setNeedsLayout];
}

@end

#endif


@interface GGTextInputDialog : UIViewController
{
#if TARGET_OS_TV == 0
	GGAlertView *alertView4_;   // for <  iOS 5
#endif
	id alertView5_;	// for >= iOS 5
    id alertView_;
    UITextField *textField_;
	gevent_Callback callback_;
	void *udata_;
	g_id gid_;
}

@end


@implementation GGTextInputDialog

- (void) forceCancel {
	[[NSNotificationCenter defaultCenter] removeObserver:self
                                                    name:@"UIAlertControllerPressMenuButton"
                                                  object:nil];
	[self buttonPressed:@"" withIndex:0];
}

- (id)initWithTitle:(NSString *)title
			message:(NSString *)message
			   text:(NSString *)text
	   cancelButton:(NSString *)cancelButton 
			button1:(NSString *)button1
			button2:(NSString *)button2
		   callback:(gevent_Callback)callback
			  udata:(void*)udata
				gid:(g_id)gid
{
    if (self = [super init])
    {
		[[NSNotificationCenter defaultCenter] addObserver:self
                                           selector:@selector(forceCancel)
                                               name:@"UIAlertControllerPressMenuButton"
                                             object:nil];
		NSString *currSysVer = [[UIDevice currentDevice] systemVersion];
		BOOL ios5 = ([currSysVer compare:@"5.0" options:NSNumericSearch] != NSOrderedAscending);

		if (!ios5)
		{
#if TARGET_OS_TV == 0
			alertView4_ = [[GGAlertView alloc] initWithTitle:title
													message:message
												   delegate:self
										  cancelButtonTitle:cancelButton];

			if (button1) {
				[alertView4_ addButtonWithTitle:button1];
			}

			if (button2) {
                                [alertView4_ addButtonWithTitle:button2];
                        }
            
	            alertView_ = (UIAlertController *)alertView4_;
        	    textField_ = alertView4_.textFieldEx;

                    if (button1)
                        [alertView_ addButtonWithTitle:button1];

                    if (button2)
                        [alertView_ addButtonWithTitle:button2];
#endif
		}
		else if ([[UIDevice currentDevice].systemVersion floatValue] >= 8)
		{
	                alertView5_ = [UIAlertController alertControllerWithTitle:title
                               message:message
                               preferredStyle:UIAlertControllerStyleAlert];
			[alertView5_ retain];

	                UIAlertAction* defaultAction = [UIAlertAction actionWithTitle:cancelButton style:UIAlertActionStyleDefault
       	        	         handler:^(UIAlertAction * action) {
                        	        [self buttonPressed:cancelButton withIndex:0];
        	                }];

	                [alertView5_ addAction:defaultAction];

			[alertView5_ addTextFieldWithConfigurationHandler:^(UITextField *textField) {
			    textField_ = textField;
			}];


	                if (button1) {
        	                UIAlertAction* button1Action = [UIAlertAction actionWithTitle:button1 style:UIAlertActionStyleDefault
                	        handler:^(UIAlertAction * action) {
                        	        [self buttonPressed:button1 withIndex:1];
	                        }];
        	                [alertView5_ addAction:button1Action];
                	}

			if (button2) {
                                UIAlertAction* button2Action = [UIAlertAction actionWithTitle:button2 style:UIAlertActionStyleDefault
                                handler:^(UIAlertAction * action) {
                                        [self buttonPressed:button2 withIndex:2];
                                }];
                                [alertView5_ addAction:button2Action];
                        }

            
	            alertView_ = alertView5_;
		}
#if TARGET_OS_TV == 0
		else
		{
                    alertView5_ =[[UIAlertView alloc] initWithTitle:title
                                                      message:message
                                                      delegate:self
                                                      cancelButtonTitle:cancelButton
                                                      otherButtonTitles:nil];
                    ((UIAlertView *)alertView5_).alertViewStyle = UIAlertViewStylePlainTextInput;

        	    alertView_ = alertView5_;
	            textField_ = [alertView5_ textFieldAtIndex:0];

                    if (button1)
                        [alertView_ addButtonWithTitle:button1];

                    if (button2)
                        [alertView_ addButtonWithTitle:button2];
		}
#endif
        
	        textField_.text = text;

		
		callback_ = callback;
		udata_ = udata;
		gid_ = gid;
    }
    return self;
}

- (void)dealloc
{
#if TARGET_OS_TV == 0
	UIAlertView * tmpAlertView = (UIAlertView*)alertView_;
	tmpAlertView.delegate = nil;
	[tmpAlertView dismissWithClickedButtonIndex:-1 animated:NO];
#endif
	[alertView_ release];
	[super dealloc];
}

- (void)show
{
#if TARGET_OS_TV == 0
    if ([[UIDevice currentDevice].systemVersion floatValue] < 8) {
	[((UIAlertView *)alertView_) show];
    } else {
#endif
        [[UIApplication sharedApplication].keyWindow.rootViewController presentViewController:alertView_ animated:YES completion:nil];
#if TARGET_OS_TV == 0
    }
#endif
}

- (void)hide
{
#if TARGET_OS_TV == 0
    if ([[UIDevice currentDevice].systemVersion floatValue] < 8) {
	[alertView_ dismissWithClickedButtonIndex:-1 animated:YES];
    } else {
#endif
        if ([alertView_ isViewLoaded]) [alertView_ dismissViewControllerAnimated:YES completion:nil];
#if TARGET_OS_TV == 0
    }
#endif
}

- (BOOL)isVisible
{
#if TARGET_OS_TV == 0
    if ([[UIDevice currentDevice].systemVersion floatValue] < 8) {
	return [alertView_ isVisible];
    } else {
#endif
        return [alertView_ isViewLoaded];
#if TARGET_OS_TV == 0
    }
#endif
}

- (void)setText:(NSString *)text
{
    textField_.text = text;
}

- (NSString *)getText
{
    return textField_.text;
}

- (void)setInputType:(int)inputType
{
    switch (inputType)
    {
        case GUI_TEXT_INPUT_DIALOG_TEXT:
            textField_.keyboardType = UIKeyboardTypeDefault;
            break;
        case GUI_TEXT_INPUT_DIALOG_NUMBER:
            textField_.keyboardType = UIKeyboardTypeNumberPad;
            break;
        case GUI_TEXT_INPUT_DIALOG_PHONE:
            textField_.keyboardType = UIKeyboardTypePhonePad;
            break;
        case GUI_TEXT_INPUT_DIALOG_EMAIL:
            textField_.keyboardType = UIKeyboardTypeEmailAddress;
            break;
        case GUI_TEXT_INPUT_DIALOG_URL:
            textField_.keyboardType = UIKeyboardTypeURL;
            break;
    }
}

- (int)getInputType
{
    switch (textField_.keyboardType)
    {
        case UIKeyboardTypeDefault:
            return GUI_TEXT_INPUT_DIALOG_TEXT;
        case UIKeyboardTypeNumberPad:
            return GUI_TEXT_INPUT_DIALOG_NUMBER;
        case UIKeyboardTypePhonePad:
            return GUI_TEXT_INPUT_DIALOG_PHONE;
        case UIKeyboardTypeEmailAddress:
            return GUI_TEXT_INPUT_DIALOG_EMAIL;
        case UIKeyboardTypeURL:
            return GUI_TEXT_INPUT_DIALOG_URL;
        default:
            return GUI_TEXT_INPUT_DIALOG_TEXT;
    }
    
    return GUI_TEXT_INPUT_DIALOG_TEXT;
}

- (void)setSecureInput:(BOOL)secureInput
{
    textField_.secureTextEntry = secureInput;
}

- (BOOL)isSecureInput
{
    return textField_.secureTextEntry;
}

- (void)buttonPressed:(NSString *)mtext withIndex:(NSInteger)buttonIndex {
	const char *text = [[self getText] UTF8String];
        const char *buttonText = [mtext UTF8String];
        size_t size = sizeof(gui_TextInputDialogCompleteEvent) + strlen(text) + 1 + strlen(buttonText) + 1;
        gui_TextInputDialogCompleteEvent *event = (gui_TextInputDialogCompleteEvent*)malloc(size);
                event->gid = gid_;
                event->text = (char*)event + sizeof(gui_TextInputDialogCompleteEvent);
                event->buttonIndex = (int)buttonIndex;
                event->buttonText = (char*)event + sizeof(gui_TextInputDialogCompleteEvent) + strlen(text) + 1;
        strcpy((char*)event->text, text);
        strcpy((char*)event->buttonText, buttonText);

                gevent_EnqueueEvent(gid_, callback_, GUI_TEXT_INPUT_DIALOG_COMPLETE_EVENT, event, 1, udata_);
}

#if TARGET_OS_TV == 0
- (void)alertView:(UIAlertView *)alertView didDismissWithButtonIndex:(NSInteger)buttonIndex
{
	if (buttonIndex >= 0)
	{
	[self buttonPressed:[alertView_ buttonTitleAtIndex:buttonIndex] withIndex:buttonIndex];
	}
}
#endif

@end

UIManager::UIManager()
{
}

UIManager::~UIManager()
{
	std::map<g_id, id>::iterator iter, e = map_.end();
	for (iter = map_.begin(); iter != e; ++iter)
		[iter->second release];
}

g_id UIManager::createAlertDialog(const char *title,
                                  const char *message,
                                  const char *cancelButton,
                                  const char *button1,
                                  const char *button2,
                                  gevent_Callback callback,
                                  void *udata)
{
    g_id gid = g_NextId();

	NSString *title2 = [NSString stringWithUTF8String:title];
	NSString *message2 = [NSString stringWithUTF8String:message];
	NSString *cancelButton2 = [NSString stringWithUTF8String:cancelButton];
	NSString *button12 = button1 ? [NSString stringWithUTF8String:button1] : nil;
	NSString *button22 = button2 ? [NSString stringWithUTF8String:button2] : nil;

	GGAlertDialog *alertDialog = [[GGAlertDialog alloc] initWithTitle:title2 
															  message:message2 
														 cancelButton:cancelButton2 
															  button1:button12 
															  button2:button22
															 callback:callback
																udata:udata
																  gid:gid];

    map_[gid] = alertDialog;
	
    return gid;
}

g_id UIManager::createTextInputDialog(const char *title,
                                      const char *message,
                                      const char *text,
                                      const char *cancelButton,
                                      const char *button1,
                                      const char *button2,
                                      gevent_Callback callback,
                                      void *udata)
{
    g_id gid = g_NextId();
	
	NSString *title2 = [NSString stringWithUTF8String:title];
	NSString *message2 = [NSString stringWithUTF8String:message];
	NSString *text2 = [NSString stringWithUTF8String:text];
	NSString *cancelButton2 = [NSString stringWithUTF8String:cancelButton];
	NSString *button12 = button1 ? [NSString stringWithUTF8String:button1] : nil;
	NSString *button22 = button2 ? [NSString stringWithUTF8String:button2] : nil;
	
	GGTextInputDialog *textInputDialog = [[GGTextInputDialog alloc] initWithTitle:title2 
																		  message:message2 
																			 text:text2
																	 cancelButton:cancelButton2 
																		  button1:button12 
																		  button2:button22
																		 callback:callback
																			udata:udata
																			  gid:gid];
	
    map_[gid] = textInputDialog;
	
    return gid;
}

void UIManager::show(g_id gid)
{
    std::map<g_id, id>::iterator iter = map_.find(gid);
	
    if (iter == map_.end())
        throw std::runtime_error("invalid gid");
	
    [iter->second show];
}

void UIManager::hide(g_id gid)
{
    std::map<g_id, id>::iterator iter = map_.find(gid);
	
    if (iter == map_.end())
        throw std::runtime_error("invalid gid");
	
    [iter->second hide];
}


void UIManager::deleteWidget(g_id gid)
{
    std::map<g_id, id>::iterator iter = map_.find(gid);
	
    if (iter == map_.end())
        throw std::runtime_error("invalid gid");

    [iter->second release];
    map_.erase(iter);

    gevent_RemoveEventsWithGid(gid);
}

bool UIManager::isVisible(g_id gid)
{
    std::map<g_id, id>::iterator iter = map_.find(gid);
	
    if (iter == map_.end())
        throw std::runtime_error("invalid gid");
	
    return [iter->second isVisible];
}

void UIManager::setText(g_id gid, const char* text)
{
    std::map<g_id, id>::iterator iter = map_.find(gid);
	
    if (iter == map_.end())
        throw std::runtime_error("invalid gid");

    GGTextInputDialog *textInputDialog = [iter->second isMemberOfClass:[GGTextInputDialog class]] ? iter->second : nil;

	if (textInputDialog == NULL)
        throw std::runtime_error("invalid gid");

	NSString *text2 = [NSString stringWithUTF8String:text];
	
	[textInputDialog setText:text2];
}

const char *UIManager::getText(g_id gid)
{
    std::map<g_id, id>::iterator iter = map_.find(gid);
	
    if (iter == map_.end())
        throw std::runtime_error("invalid gid");
	
    GGTextInputDialog *textInputDialog = [iter->second isMemberOfClass:[GGTextInputDialog class]] ? iter->second : nil;
	
	if (textInputDialog == nil)
        throw std::runtime_error("invalid gid");
	
	return [[textInputDialog getText] UTF8String];
}


void UIManager::setInputType(g_id gid, int inputType)
{
    std::map<g_id, id>::iterator iter = map_.find(gid);
	
    if (iter == map_.end())
        throw std::runtime_error("invalid gid");
	
    GGTextInputDialog *textInputDialog = [iter->second isMemberOfClass:[GGTextInputDialog class]] ? iter->second : nil;
	
	if (textInputDialog == nil)
        throw std::runtime_error("invalid gid");
    
    [textInputDialog setInputType:inputType];
}

int UIManager::getInputType(g_id gid)
{
    std::map<g_id, id>::iterator iter = map_.find(gid);
	
    if (iter == map_.end())
        throw std::runtime_error("invalid gid");

    GGTextInputDialog *textInputDialog = [iter->second isMemberOfClass:[GGTextInputDialog class]] ? iter->second : nil;
	
	if (textInputDialog == nil)
        throw std::runtime_error("invalid gid");
	
    return [textInputDialog getInputType];
}

void UIManager::setSecureInput(g_id gid, bool secureInput)
{
    std::map<g_id, id>::iterator iter = map_.find(gid);
	
    if (iter == map_.end())
        throw std::runtime_error("invalid gid");
	
    GGTextInputDialog *textInputDialog = [iter->second isMemberOfClass:[GGTextInputDialog class]] ? iter->second : nil;
	
	if (textInputDialog == nil)
        throw std::runtime_error("invalid gid");
    
    [textInputDialog setSecureInput:secureInput];
}

bool UIManager::isSecureInput(g_id gid)
{
    std::map<g_id, id>::iterator iter = map_.find(gid);
	
    if (iter == map_.end())
        throw std::runtime_error("invalid gid");
	
    GGTextInputDialog *textInputDialog = [iter->second isMemberOfClass:[GGTextInputDialog class]] ? iter->second : nil;
	
	if (textInputDialog == nil)
        throw std::runtime_error("invalid gid");
    
    return [textInputDialog isSecureInput];
}

static UIManager *s_manager = NULL;

extern "C" {

G_API void gui_init()
{
    s_manager = new UIManager;
}

G_API void gui_cleanup()
{
    delete s_manager;
    s_manager = NULL;
}

G_API g_id gui_createAlertDialog(const char *title,
                                 const char *message,
                                 const char *cancelButton,
                                 const char *button1,
                                 const char *button2,
                                 gevent_Callback callback,
                                 void *udata)
{
    return s_manager->createAlertDialog(title, message, cancelButton, button1, button2, callback, udata);
}

G_API g_id gui_createTextInputDialog(const char *title,
                                     const char *message,
                                     const char *text,
                                     const char *cancelButton,
                                     const char *button1,
                                     const char *button2,
                                     gevent_Callback callback,
                                     void *udata)
{
    return s_manager->createTextInputDialog(title, message, text, cancelButton, button1, button2, callback, udata);
}

G_API void gui_show(g_id gid)
{
    s_manager->show(gid);
}

G_API void gui_hide(g_id gid)
{
    s_manager->hide(gid);
}

G_API void gui_delete(g_id gid)
{
    s_manager->deleteWidget(gid);
}

G_API int gui_isVisible(g_id gid)
{
    return s_manager->isVisible(gid);
}

G_API void gui_setText(g_id gid, const char* text)
{
    s_manager->setText(gid, text);
}

G_API const char *gui_getText(g_id gid)
{
    return s_manager->getText(gid);
}

G_API void gui_setInputType(g_id gid, int inputType)
{
    s_manager->setInputType(gid, inputType);
}

G_API int gui_getInputType(g_id gid)
{
    return s_manager->getInputType(gid);
}

G_API void gui_setSecureInput(g_id gid, int secureInput)
{
    s_manager->setSecureInput(gid, secureInput);
}

G_API int gui_isSecureInput(g_id gid)
{
    return s_manager->isSecureInput(gid);
}


}
