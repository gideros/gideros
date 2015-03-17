/*  Copyright (c) 2011 The ORMMA.org project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

(function() {

   var ormmaview = window.ormmaview = {};

 
 
   /****************************************************/
   /********** PROPERTIES OF THE ORMMA BRIDGE **********/
   /****************************************************/
 
   /** Expand Properties */
    var expandProperties = {
        useBackground:false,
        backgroundColor:'#ffffff',
        backgroundOpacity:1.0,
        lockOrientation:false
    };
 
 
   /** The set of listeners for ORMMA Native Bridge Events */
   var listeners = { };
 
 
   /** A Queue of Calls to the Native SDK that still need execution */
   var nativeCallQueue = [ ];
 
   /** Identifies if a native call is currently in progress */
   var nativeCallInFlight = false;
 
   /** timer for identifying iframes */
   var timer;
   var totalTime;

 
 
   /**********************************************/
   /********** OBJECTIVE-C ENTRY POINTS **********/
   /**********************************************/
 
   /**
    * Called by the Objective-C SDK when an asset has been fully cached.
    *
    * @returns string, "OK"
    */
   ormmaview.fireAssetReadyEvent = function( alias, URL ) {
      var handlers = listeners["assetReady"];
      if ( handlers != null ) {
         for ( var i = 0; i < handlers.length; i++ ) {
            handlers[i]( alias, URL );
         }
      }
 
      return "OK";
   };
 
 
   /**
    * Called by the Objective-C SDK when an asset has been removed from the
	* cache at the request of the creative.
    *
    * @returns string, "OK"
    */
   ormmaview.fireAssetRemovedEvent = function( alias ) {
      var handlers = listeners["assetRemoved"];
      if ( handlers != null ) {
         for ( var i = 0; i < handlers.length; i++ ) {
            handlers[i]( alias );
         }
      }
 
      return "OK";
   };
 
 
   /**
    * Called by the Objective-C SDK when an asset has been automatically
	* removed from the cache for reasons outside the control of the creative.
    *
    * @returns string, "OK"
    */
   ormmaview.fireAssetRetiredEvent = function( alias ) {
      var handlers = listeners["assetRetired"];
      if ( handlers != null ) {
         for ( var i = 0; i < handlers.length; i++ ) {
            handlers[i]( alias );
         }
      }
 
      return "OK";
   };
 
 
   /**
	* Called by the Objective-C SDK when various state properties have changed.
    *
    * @returns string, "OK"
	*/
   ormmaview.fireChangeEvent = function( properties ) {
      var handlers = listeners["change"];
      if ( handlers != null ) {
         for ( var i = 0; i < handlers.length; i++ ) {
		    handlers[i]( properties );
         }
      }
 
      return "OK";
   };
 
 
   /**
    * Called by the Objective-C SDK when an error has occured.
    *
    * @returns string, "OK"
    */
   ormmaview.fireErrorEvent = function( message, action ) {
      var handlers = listeners["error"];
      if ( handlers != null ) {
         for ( var i = 0; i < handlers.length; i++ ) {
            handlers[i]( message, action );
         }
      }
 
      return "OK";
   };
 
 
   /**
    * Called by the Objective-C SDK when the user shakes the device.
    *
    * @returns string, "OK"
    */
   ormmaview.fireShakeEvent = function() {
      var handlers = listeners["shake"];
      if ( handlers != null ) {
         for ( var i = 0; i < handlers.length; i++ ) {
            handlers[i]();
         }
      }
 
      return "OK";
   };
 
 
   /**
    * nativeCallComplete notifies the abstraction layer that a native call has
    * been completed..
    *
    * NOTE: This function is called by the native code and is not intended to be
    *       used by anything else.
    *
    * @returns string, "OK"
    */
   ormmaview.nativeCallComplete = function( cmd ) {
 
      // anything left to do?
      if ( nativeCallQueue.length == 0 ) {
         nativeCallInFlight = false;
         return;
      }
 
       // still have something to do
       var bridgeCall = nativeCallQueue.pop();
       window.location = bridgeCall;

      return "OK";
   };
 
 
   /**
    *
    */
   ormmaview.showAlert = function( message ) {
      alert( message );
   };
 
 
   /*********************************************/
   /********** INTERNALLY USED METHODS **********/
   /*********************************************/
 
 
   /**
    *
    */
   ormmaview.zeroPad = function( number ) {
      var text = "";
      if ( number < 10 ) {
         text += "0";
      }
	  text += number;
      return text;
   }
 
   /**
    *
    */
   ormmaview.executeNativeCall = function( command ) {
      // build iOS command
      var bridgeCall = "ormma://" + command;
      var value;
      var firstArg = true;
      for ( var i = 1; i < arguments.length; i += 2 ) {
         value = arguments[i + 1];
         if ( value == null ) {
            // no value, ignore the property
            continue;
         }
 
         // add the correct separator to the name/value pairs
         if ( firstArg ) {
            bridgeCall += "?";
            firstArg = false;
         }
         else {
            bridgeCall += "&";
         }
         bridgeCall += arguments[i] + "=" + escape( value );
      }
 
      // add call to queue
      if ( nativeCallInFlight ) {
         // call pending, queue up request
         nativeCallQueue.push( bridgeCall );
      }
      else {
         // no call currently in process, execute it directly
         nativeCallInFlight = true;
         window.location = bridgeCall;
      }
   };
 
 
 
   /***************************************************************************/
   /********** LEVEL 0 (not part of spec, but required by public API **********/
   /***************************************************************************/
 
   /**
    *
    */
   ormmaview.activate = function( event ) {
      this.executeNativeCall( "service", 
							  "name", event, 
							  "enabled", "Y" );
   };

 
   /**
    *
    */
   ormmaview.addEventListener = function( event, listener ) {
      var handlers = listeners[event];
	  if ( handlers == null ) {
		 // no handlers defined yet, set it up
         listeners[event] = [];
         handlers = listeners[event];
      }
 
      // see if the listener is already present
	  for ( var handler in handlers ) {
	     if ( listener == handler ) {
		    // listener already present, nothing to do
			return;
		}
	  }
 
      // not present yet, go ahead and add it
      handlers.push( listener );
   };

 
   /**
    *
    */
   ormmaview.deactivate = function( event ) {
      this.executeNativeCall( "service", 
							  "name", event, 
							  "enabled", "N" );
   };

 
   /**
    *
    */
   ormmaview.removeEventListener = function( event, listener ) {
	  var handlers = listeners[event];
	  if ( handlers != null ) {
         handlers.remove( listener );
	  }
   };
 

 
   /*****************************/
   /********** LEVEL 1 **********/
   /*****************************/

   /**
    *
    */
   ormmaview.close = function() {
	  this.executeNativeCall( "close" );
   };
 
 
   /**
    *
    */
   ormmaview.expand = function( dimensions, URL ) {
	  try {
		 var cmd = "this.executeNativeCall( 'expand'";
		 if ( URL != null ) {
			cmd += ", 'url', '" + URL + "'";
		 }
		 if ( ( typeof dimensions.x != "undefined" ) && ( dimensions.x != null ) ) {
			cmd += ", 'x', '" + dimensions.x + "'";
		 }
		 if ( ( typeof dimensions.y != "undefined" ) && ( dimensions.y != null ) ) {
			cmd += ", 'y', '" + dimensions.y + "'";
		 }
		 if ( ( typeof dimensions.width != "undefined" ) && ( dimensions.width != null ) ) {
			cmd += ", 'w', '" + dimensions.width + "'";
		 }
		 if ( ( typeof dimensions.height != "undefined" ) && ( dimensions.height != null ) ) {
			cmd += ", 'h', '" + dimensions.height + "'";
		 }
		 if ( ( typeof expandProperties.useBackground != "undefined" ) && ( expandProperties.useBackground != null ) ) {
			 cmd += ", 'useBG', '" + ( expandProperties.useBackground ? "Y" : "N" ) + "'";
		 }
		 if ( ( typeof expandProperties.backgroundColor != "undefined" ) && ( expandProperties.backgroundColor != null ) ) {
			cmd += ", 'bgColor', '" + expandProperties.backgroundColor + "'";
		 }
		 if ( ( typeof expandProperties.backgroundOpacity != "undefined" ) && ( expandProperties.backgroundOpacity != null ) ) {
			cmd += ", 'bgOpacity', " + expandProperties.backgroundOpacity;
		 }
		 if ( ( typeof expandProperties.lockOrientation != "undefined" ) && ( expandProperties.lockOrientation != null ) ) {
			cmd += ", 'lkOrientation', '" + ( expandProperties.lockOrientation ? "Y" : "N" ) + "'";
		 }
		 
		 
		 
		 cmd += " );";
		 eval( cmd );
	  } catch ( e ) {
	     alert( "executeNativeExpand: " + e + ", cmd = " + cmd );
	  }
   };

 
   /**
    *
    */
   ormmaview.hide = function() {
	  this.executeNativeCall( "hide" );
   };
 
 

 
   /**
    *
    */
   ormmaview.open = function( URL, controls ) {
	  // the navigation parameter is an array, break it into its parts
	  var back = false;
	  var forward = false;
	  var refresh = false;
	  if ( controls == null ) {
		 back = true;
		 forward = true;
		 refresh = true;
	  }
	  else {
		 for ( var i = 0; i < controls.length; i++ ) {
			if ( ( controls[i] == "none" ) && ( i > 0 ) ) {
			   // error
			   self.fireErrorEvent( "none must be the only navigation element present.", "open" );
			   return;
			}
			else if ( controls[i] == "all" ) {
			   if ( i > 0 ) {
				   // error
				   self.fireErrorEvent( "none must be the only navigation element present.", "open" );
				   return;
				}
				
				// ok
				back = true;
				forward = true;
				refresh = true;
			}
			else if ( controls[i] == "back" ) {
				back = true;
			}
			else if ( controls[i] == "forward" ) {
				forward = true;
			}
			else if ( controls[i] == "refresh" ) {
				refresh = true;
			}
	     }
	  }
	
	
	  this.executeNativeCall( "open",
							  "url", URL,
							  "back", ( back ? "Y" : "N" ),
							  "forward", ( forward ? "Y" : "N" ),
							  "refresh", ( refresh ? "Y" : "N" ) );
   };
    
    
    /**
     *
     */
    ormmaview.openMap = function( URL, fullscreen ) {
        
        this.executeNativeCall( "openMap", 
                                "url", URL,
                                "fullscreen", fullscreen
                              );
    };
 
   /**
    *
    */
   ormmaview.resize = function( width, height ) {
	  this.executeNativeCall( "resize", 
							  "w", width, 
							  "h", height );
   };

    
    ormmaview.getExpandProperties = function(){
        return expandProperties;
    }
    
   /**
    *
    */
   ormmaview.setExpandProperties = function( properties ) {
	  expandProperties = properties;
   };

 
   /**
    *
    */
   ormmaview.show = function() {
	  this.executeNativeCall( "show" );
   };
    
    
   /**
     *
     */
   ormmaview.playAudio = function( URL, properties ) {
       
       var cmd = "this.executeNativeCall( 'playAudio'";
       
       cmd += ", 'url', '" + URL + "'";
    
       if ( properties != null ) {
           
           if ( ( typeof properties.autoplay != "undefined" ) && ( properties.autoplay != null ) ) {
               cmd += ", 'autoplay', 'Y'";
           }
           else{
               cmd += ", 'autoplay', 'N'";
           }
       
           if ( ( typeof properties.controls != "undefined" ) && ( properties.controls != null ) ) {
               cmd += ", 'controls', 'Y'";
           }
           else{
               cmd += ", 'controls', 'N'";
           }
           
           if ( ( typeof properties.loop != "undefined" ) && ( properties.loop != null ) ) {
               cmd += ", 'loop', 'Y'";
           }
           else{
               cmd += ", 'loop', 'N'";
           }
           
           if ( ( typeof properties.position != "undefined" ) && ( properties.position != null ) ) {
               cmd += ", 'position', 'Y'";
           }
           else{
               cmd += ", 'position', 'N'";
           }
           
           //TODO check valid values...           
           
           if ( ( typeof properties.startStyle != "undefined" ) && ( properties.startStyle != null ) ) {
                cmd += ", 'startStyle', '" + properties.startStyle + "'";
           }
           else{
               cmd += ", 'startStyle', 'normal'";
           }
           
           if ( ( typeof properties.stopStyle != "undefined" ) && ( properties.stopStyle != null ) ) {
               cmd += ", 'stopStyle', '" + properties.stopStyle + "'";
           }
           else{
               cmd += ", 'stopStyle', 'normal'";
           }
                    
       }    
           
       cmd += " );";
       
       eval( cmd );
   };
    
    
    /**
     *
     */
    ormmaview.playVideo = function( URL, properties ) {
        var cmd = "this.executeNativeCall( 'playVideo'";
        
        cmd += ", 'url', '" + URL + "'";
        
        if ( properties != null ) {
            
            if ( ( typeof properties.audio != "undefined" ) && ( properties.audio != null ) ) {
                cmd += ", 'audioMuted', 'Y'";
            }
            else{
                cmd += ", 'audioMuted', 'N'";
            }
            
            if ( ( typeof properties.autoplay != "undefined" ) && ( properties.autoplay != null ) ) {
                cmd += ", 'autoplay', 'Y'";
            }
            else{
                cmd += ", 'autoplay', 'N'";
            }
            
            if ( ( typeof properties.controls != "undefined" ) && ( properties.controls != null ) ) {
                cmd += ", 'controls', 'Y'";
            }
            else{
                cmd += ", 'controls', 'N'";
            }
            
            if ( ( typeof properties.loop != "undefined" ) && ( properties.loop != null ) ) {
                cmd += ", 'loop', 'Y'";
            }
            else{
                cmd += ", 'loop', 'N'";
            }
            
            if ( ( typeof properties.position != "undefined" ) && ( properties.position != null ) ) {
                cmd += ", 'position_top', '" + properties.position.top + "'";
                cmd += ", 'position_left', '" + properties.position.left + "'";
                
                if ( ( typeof properties.width != "undefined" ) && ( properties.width != null ) ) {
                    cmd += ", 'position_width', '" + properties.width + "'";
                }
                else{
                    //TODO ERROR
                }
                
                if ( ( typeof properties.height != "undefined" ) && ( properties.height != null ) ) {
                    cmd += ", 'position_height', '" + properties.height + "'";
                }
                else{
                    //TODO ERROR
                }
            }
          
            
            if ( ( typeof properties.startStyle != "undefined" ) && ( properties.startStyle != null ) ) {
                cmd += ", 'startStyle', '" + properties.startStyle + "'";
            }
            else{
                cmd += ", 'startStyle', 'normal'";
            }
            
            if ( ( typeof properties.stopStyle != "undefined" ) && ( properties.stopStyle != null ) ) {
                cmd += ", 'stopStyle', '" + properties.stopStyle + "'";
            }
            else{
                cmd += ", 'stopStyle', 'normal'";
            }
            
        }    
        
        cmd += " );";
        
        eval( cmd );

    };
 
 
 
   /*****************************/
   /********** LEVEL 2 **********/
   /*****************************/

   /**
    *
    */
   ormmaview.createEvent = function( date, title, body ) {
 //      var year = date.getFullYear();
 //      var month = date.getMonth() + 1;
 //      var day = date.getDate();
 //      var hours = date.getHours();
 //      var minutes = date.getMinutes();
 //
 //
 //      var dateString = year + this.zeroPad( month ) + this.zeroPad( day ) + this.zeroPad( hours ) + this.zeroPad( minutes );
 // alert (dateString);
 //	  this.executeNativeCall( "calendar",
 //							  "date", dateString,
 //							  "title", title,
 //							  "body", body );
 this.executeNativeCall( "calendar",
                        "date", date,
                        "title", title,
                        "body", body );
 
   };
 
   /**
    *
    */
   ormmaview.makeCall = function( phoneNumber ) {
	  this.executeNativeCall( "phone",
							  "number", phoneNumber );
   };
 
 
   /**
    *
    */
   ormmaview.sendMail = function( recipient, subject, body ) {
	  this.executeNativeCall( "email",
							  "to", recipient,
							  "subject", subject,
							  "body", body,
							  "html", "N" );
   };
 

   /**
    *
    */
   ormmaview.sendSMS = function( recipient, body ) {
	  this.executeNativeCall( "sms",
							  "to", recipient,
							  "body", body );
   };
 
   /**
    *
    */
   ormmaview.setShakeProperties = function( properties ) {
   };
 
 
 
   /*****************************/
   /********** LEVEL 3 **********/
   /*****************************/

   /**
    *
    */
   ormmaview.addAsset = function( URL, alias ) {
	  this.executeNativeCall( "addasset", 
							  "uri", url,
							  "alias", alias );
   };
 
 
   /**
    *
    */
   ormmaview.request = function( URI, display ) {
	  this.executeNativeCall( "request", 
							  "uri", uri, 
							  "display", display );
   };

 
   /**
    *
    */
   ormmaview.removeAsset = function( alias ) {
	  this.executeNativeCall( "removeasset", 
							  "alias", alias );
   };
})();
