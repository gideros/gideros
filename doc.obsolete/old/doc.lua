require("lp")

module_docs = {}

function Sprite()
	local name = "Sprite"
	local description = [[
	The Sprite class is a basic display list building
	block: a display list node that can display graphics
	and can also contain children. <br/><br/>

	The Sprite class is the base class for all objects that
	can serve as display object containers on the display list.
	The display list manages all objects displayed in Gideros.
	Use the Sprite class to arrange the display objects in the
	display list. Each Sprite object has its own child list for
	organizing the z-order of the objects. The z-order is the
	front-to-back order that determines which object is drawn in
	front, which is behind, and so on.
	]]

	--local author = {"Atilim Cetin", "Deniz Cetin", "Ela Cetin"}
	--local copyright = "tekno disko"
	local release = "1.0"

	local properties =
	{
		"colorTransform",
		"graphics",
		"height",
		"numChildren",
		"parent",
		"rotation",
		"scaleX",
		"scaleY",
		"visible",
		"width",
		"x",
		"y",

		graphics =
		{
			type = "Graphics",
			summary = "Specifies the Graphics object that belongs to this sprite where vector drawing commands can occur.",
			description = "Specifies the Graphics object that belongs to this sprite where vector drawing commands can occur.",
			read_only = true,
		},

		height =
		{
			type = "number",
			summary = "Indicates the height of the display object, in pixels.",
			description = "Indicates the height of the display object, in pixels. The height is calculated based on the bounds of the content of the display object.",
			read_only = true,
		},

		width =
		{
			type = "number",
			summary = "Indicates the width of the display object, in pixels.",
			description = "Indicates the width of the display object, in pixels. The width is calculated based on the bounds of the content of the display object.",
			read_only = true,
		},

		parent =
		{
			type = "Sprite",
			summary = "Indicates the Sprite object that contains this display object.",
			description = "Indicates the Sprite object that contains this display object. Use the parent property to specify a relative path to display objects that are above the current display object in the display list hierarchy.",
			read_only = true,
		},

		rotation =
		{
			type = "number",
			summary = "Indicates the rotation of the Sprite instance, in degrees, from its original orientation.",
			description = "Indicates the rotation of the Sprite instance, in degrees, from its original orientation. Values from 0 to 180 represent clockwise rotation; values from 0 to -180 represent counterclockwise rotation. Values outside this range are added to or subtracted from 360 to obtain a value within the range. For example, the statement <code>my_sprite.rotation = 450</code> is the same as <code>my_sprite.rotation = 90</code>.",
		},

		scaleX =
		{
			type = "number",
			summary = "Indicates the horizontal scale (percentage) of the object as applied from the registration point.",
			description = "Indicates the horizontal scale (percentage) of the object as applied from the registration point. The default registration point is (0,0). 1.0 equals 100% scale.",
		},

		scaleY =
		{
			type = "number",
			summary = "Indicates the vertical scale (percentage) of the object as applied from the registration point.",
			description = "Indicates the vertical scale (percentage) of the object as applied from the registration point. The default registration point is (0,0). 1.0 equals 100% scale.",
		},

		visible =
		{
			type = "boolean",
			summary = "Whether or not the display object is visible.",
			description = "Whether or not the display object is visible. Display objects that are not visible are also taken into consideration while calculating bounds.",
		},

		x =
		{
			type = "number",
			summary = "Indicates the x coordinate of the Sprite instance relative to the local coordinates of the parent Sprite.",
			description = "Indicates the x coordinate of the Sprite instance relative to the local coordinates of the parent Sprite. If the object is inside a Sprite that has transformations, it is in the local coordinate system of the enclosing Sprite. Thus, for a Sprite rotated 90° counterclockwise, the Sprite's children inherit a coordinate system that is rotated 90° counterclockwise. The object's coordinates refer to the registration point position.",
		},

		y =
		{
			type = "number",
			summary = "Indicates the y coordinate of the Sprite instance relative to the local coordinates of the parent Sprite.",
			description = "Indicates the y coordinate of the Sprite instance relative to the local coordinates of the parent Sprite. If the object is inside a Sprite that has transformations, it is in the local coordinate system of the enclosing Sprite. Thus, for a Sprite rotated 90° counterclockwise, the Sprite's children inherit a coordinate system that is rotated 90° counterclockwise. The object's coordinates refer to the registration point position.",
		},

		colorTransform =
		{
			type = "ColorTransform",
			summary = "A ColorTransform object containing values that universally adjust the colors in the display object.",
			description = "A ColorTransform object containing values that universally adjust the colors in the display object.",
		},

		numChildren =
		{
			type = "number",
			summary = "Returns the number of children of this object.",
			description = "Returns the number of children of this object.",
			read_only = true,
		},
	}


	local functions =
	{
		"addChild",
		"contains",
		"removeChild",
		"globalToLocal",
		"localToGlobal",
		"child",

		addChild =
		{
			summary = "Adds a child Sprite instance to this Sprite instance.",
			description =
	[[
	Adds a child Sprite instance to this Sprite instance.
	The child is added to the front (top) of all other children in
	this Sprite instance.<br/><br/>

	If you add a child object that already has a different display
	object container as a parent, the object is removed from the
	child list of the other display object container.
	]],
			param =
			{
				"child",
				child =
				{
					type = "Sprite",
					description = "The Sprite instance to add as a child of this Sprite instance.",
				}
			}
			--usage = {"func1 usage", "hebe", "gube"}, --burasi string ya da array de olabilir
			--ret = "Graphics",
			--see = "see also of func1",	-- bu link atiyor
			--private = true
		},

		contains =
		{
			summary = "Determines whether the specified display object is a child of the Sprite instance or the instance itself.",
			description =
	[[
	Determines whether the specified display object is a child of the Sprite instance or the instance itself.
	The search includes the entire display list including this Sprite instance.
	Grandchildren, great-grandchildren, and so on each return <code>true</code>.
	]],
			param =
			{
				"child",
				child =
				{
					type = "Sprite",
					description = "The child object to test.",
				}
			},
			ret = "<code>true</code> if the child object is a child of the Sprite or the container itself; otherwise <code>false</code>.",
		},

		removeChild =
		{
			summary = "Removes the specified child Sprite instance from the child list of the Sprite instance.",
			description =
	[[
	Removes the specified child Sprite instance from the child
	list of the Sprite instance. The parent property
	of the removed child is set to <code>nil</code> , and the object is garbage
	collected if no other references to the child exist. The index
	positions of any display objects above the child in the Sprite
	are decreased by 1.<br/><br/>

	The garbage collector reallocates unused memory space. When a variable
	or object is no longer actively referenced or stored somewhere,
	the garbage collector sweeps through and wipes out the memory space
	it used to occupy if no other references to it exist.
	]],
			param =
			{
				"child",
				child =
				{
					type = "Sprite",
					description = "The Sprite instance to remove.",
				}
			}
		},


		globalToLocal =
		{
			summary = "Converts the x,y coordinates from the global to the display object's (local) coordinates.",
			description =
	[[
	Converts the x,y coordinates from the global to the display object's
	(local) coordinates. <br/><br/>

	The x and y values that you pass to <code>globalToLocal()</code> method represent global
	coordinates because they relate to the origin (0,0) of the main
	display area. The method returns x and y values that relate to
	the origin of the display object.
	]],
			param =
			{
				"x",
				"y",
				x =
				{
					type = "number",
					description = "x coordinate of the global coordinate.",
				},
				y =
				{
					type = "number",
					description = "y coordinate of the global coordinate.",
				},
			},
			ret = {"x coordinate relative to the display object.", "y coordinate relative to the display object."},
		},

		localToGlobal =
		{
			summary = "Converts the x,y coordinates from the display object's (local) coordinates to the global coordinates.",
			description =
	[[
	Converts the x,y coordinates from the display object's (local) coordinates
	to the global coordinates. <br/><br/>

	This method allows you to convert any given x and y coordinates from
	values that are relative to the origin (0,0) of a specific display
	object (local coordinates) to values that are in the global coordinates.

	The x and y values that you pass to <code>localToGlobal()</code> method represent local
	coordinates related to the origin (0,0) of the display object. The method returns
	x and y values that relate to the origin of the display area.
	]],
			param =
			{
				"x",
				"y",
				x =
				{
					type = "number",
					description = "x coordinate of the local coordinate.",
				},
				y =
				{
					type = "number",
					description = "y coordinate of the local coordinate.",
				},
			},
			ret = {"x coordinate relative to the display area.", "y coordinate relative to the display area."},
		},

		child =
		{
			summary = "Returns the child display object instance that exists at the specified index.",
			description = "Returns the child display object instance that exists at the specified index. First child is at index 1.",
			param =
			{
				"index",
				index =
				{
					type = "number",
					"The index position of the child object.",
				},
			},
			ret = "The child display object at the specified index position.",
		}
	}

	--[[
	tables =
	{
		"table1",

		table1 =
		{
			name = "table1",
			summary = "table1 summary",
			description = "description",
			field =
			{
				"field1",
				field1 = "my field1",
			}
		},

	}
	--]]

	local tables = {}

	local module_doc = {}
	module_doc.name = name
	module_doc.description = description
	module_doc.author = author
	module_doc.copyright = copyright
	module_doc.release = release
	module_doc.properties = properties
	module_doc.functions = functions
	module_doc.tables = tables


	module_docs[#module_docs + 1] = module_doc
end

function EventDispatcher()
	local name = "EventDispatcher"
	local description = [[
	The EventDispatcher class is the base class for all classes that
	dispatch events. The EventDispatcher class is the base class for the
	Sprite class. The EventDispatcher class allows any object on the display
	list to be an event target and as such, to use the methods of the
	EventDispatcher class.<br/><br/>

	Event targets are an important part of the Gideros event model. The event
	target serves as the focal point for how events flow through the display
	list hierarchy. When an event such as a touch event, Gideros dispatches an
	event object into the event flow from the root of the display list.<br/><br/>

	In general, the easiest way for a user-defined class to gain event dispatching
	capabilities is to extend EventDispatcher.
	]]

	local release = "1.0"

	local properties =
	{
	}


	local functions =
	{
		"addEventListener",
		"removeEventListener",
		"dispatchEvent",
		"hasEventListener",

		addEventListener =
		{
			summary = "Registers a function (and optionally a data) so that the function receives notification of an event.",
			description = "Registers a function (and optionally a data) so that the function receives notification of an event.",
			param =
			{
				"type",
				"listener",
				"data",

				type =
				{
					type = "string",
					description = "The type of event.",
				},
				listener =
				{
					type = "function",
					description = "The listener function that processes the event.",
				},
				data =
				{
					type = "",
					description = "An optional data parameter to be passed as a first argument to the listener function."
				},
			}
		},


		removeEventListener =
		{
			summary = "Removes a listener from the EventDispatcher object.",
			description = "Removes a listener from the EventDispatcher object. If there is no matching listener registered with the EventDispatcher object, a call to this method has no effect.",
			param =
			{
				"type",
				"listener",
				"data",

				type =
				{
					type = "string",
					description = "The type of event.",
				},
				listener =
				{
					type = "function",
					description = "The listener object to remove.",
				},
				data =
				{
					type = "",
					description = "The data parameter that is used while registering the event."
				}
			}
		},

		dispatchEvent =
		{
			summary = "Dispatches an event into the event flow.",
			description = "Dispatches an event into the event flow. The event target is the listener function upon which the <code>dispatchEvent()</code> method is called.",
			param =
			{
				"event",

				event =
				{
					type = "Event",
					description = "The Event object that is dispatched into the event flow.",
				}
			}
		},
		hasEventListener =
		{
			summary = "Checks whether the EventDispatcher object has any listeners registered for a specific type of event.",
			description = "",
			param =
			{
				"type",

				type =
				{
					type = "string",
					description = "The type of event.",
				}
			},
			ret = "A value of <code>true</code> if a listener of the specified type is registered; <code>false</code> otherwise."
		}
	}

	local tables = {}

	local module_doc = {}
	module_doc.name = name
	module_doc.description = description
	module_doc.author = author
	module_doc.copyright = copyright
	module_doc.release = release
	module_doc.properties = properties
	module_doc.functions = functions
	module_doc.tables = tables

	module_docs[#module_docs + 1] = module_doc
end

function Event()
	local name = "Event"
	local description = [[
	The Event class is used as the base class for the
	creation of Event objects, which are passed as parameters
	to event listeners when an event occurs. <br/><br/>

	The properties of the Event class carry basic information
	about an event, such as the event's type and event's target. For many events,
	such as the events represented by the Event class constants,
	this basic information is sufficient. Other events, however,
	may require more detailed information. Events associated with
	a touch event, for example, need to include additional information
	about the locations of the touch event. Gideros API defines several
	Event subclasses for common events that require additional
	information. Events associated with each of the Event subclasses are
	described in the documentation for each class. <br/><br/>

	Touch and mouse events are dispatched to the all targets on
	the display list. For these events, the direction of event flow is
	reverse of the drawing order. Therefore, the display object that is on top
	the display list recives the mouse or touch event first.<br/><br/>

	You can make the current event listener the last one to
	process an event by calling the <code>stopPropagation()</code> method.
	]]

	local release = "1.0"

	local properties =
	{
		"target",
		"type",

		target =
		{
			type = "EventDispatcher",
			summary = "The event target.",
			description = "The event target. This property contains the target node.",
			read_only = true,
		},

		type =
		{
			type = "string",
			summary = "The type of event.",
			description = "The type of event. The type is case-sensitive.",
			read_only = true,
		},
	}

	local fields =
	{
		"ENTER_FRAME",
		"ADDED_TO_STAGE",
		"REMOVED_FROM_STAGE",
		"SOUND_COMPLETE",

		ENTER_FRAME =
		{
			type = "string",
			value = '"enterFrame"',
			summary = 'The value of <code>Event.ENTER_FRAME</code> is the string name <code>"enterFrame"</code> and defines the value of the type property of an enter frame event.',
			description =
[[
Enter frame event is dispatched when player is entering a new frame.
This event is dispatched simultaneously to all display objects
listenting for this event. The framerate is set fixed to 60hz
but the framerate may drop according to the complexity of the
stage. <br/><br/>

The value of <code>Event.ENTER_FRAME</code> is the string name
<code>"enterFrame"</code> and defines the value of the type
property of an enter frame event.<br/><br/>

When enter frame event is dispatched, the <code>Event</code> value sent to
listener function contains 4 additional fields to make the timing and
syncronization easier. These 4 fields are
<code>time</code>,
<code>deltaTime</code>,
<code>frameCount</code> and
<code>deltaFrameCount</code>.

<br/><br/>

<b>Example</b>

<pre class="example">
function onEnterFrame(event)
    print("time", event.time)
    print("deltaTime", event.deltaTime)
    print("frameCount", event.frameCount)
    print("deltaFrameCount", event.deltaFrameCount)
end

s = Sprite.new()
s:addEventListener(Event.ENTER_FRAME, onEnterFrame)
</pre>
]],
		},

		ADDED_TO_STAGE =
		{
			type = "string",
			value = '"addedToStage"',
			summary = 'The value of <code>Event.ADDED_TO_STAGE</code> is the string name <code>"addedToStage"</code> and defines the value of the type property of an added to stage event.',
			description =
[[
Added to stage event is dispatched when a display object is added
to the on stage display list, either directly or through the addition
of a sub tree in which the display object is contained.<br/><br/>

The value of <code>Event.ADDED_TO_STAGE</code> is the
string name <code>"addedToStage"</code> and defines
the value of the type property of an added to stage event.
]],
		},

		REMOVED_FROM_STAGE =
		{
			type = "string",
			value = '"removedFromStage"',
			summary = 'The value of <code>Event.REMOVED_FROM_STAGE</code> is the string name <code>"removedFromStage"</code> and defines the value of the type property of a removed from stage event.',
			description =
[[
Removed from stage event is dispatched when a display object
is about to be removed from the display list, either directly or
through the removal of a sub tree in which the display object is
contained.<br/><br/>

The value of <code>Event.REMOVED_FROM_STAGE</code> is the string
name <code>"removedFromStage"</code> and defines the value of
the type property of a removed from stage event.
]],
		},

		SOUND_COMPLETE =
		{
			type = "string",
			value = '"soundComplete"',
			summary = 'The value of <code>Event.SOUND_COMPLETE</code> is the string name <code>"soundComplete"</code> and defines the value of the type property of a sound complete event.',
			description =
[[
Sound complete event is dispatched when a sound has finished
playing. <br/><br/>

The value of <code>Event.SOUND_COMPLETE</code> is the string
name <code>"soundComplete"</code> and defines the value of
the type property of a sound complete event.
]]
		},
	}

	local functions =
	{
		"stopPropagation",

		stopPropagation =
		{
			summary = "Prevents processing of any event listeners in nodes subsequent to the current node in the event flow.",
			description = [[
Prevents processing of any event listeners in nodes subsequent
to the current node in the event flow. <br/><br/>
]],
			param = {},
		},

	}

	local tables = {}

	local module_doc = {}
	module_doc.name = name
	module_doc.description = description
	module_doc.author = author
	module_doc.copyright = copyright
	module_doc.release = release
	module_doc.properties = properties
	module_doc.functions = functions
	module_doc.fields = fields
	module_doc.tables = tables

	module_docs[#module_docs + 1] = module_doc
end



function Timer()
	local name = "Timer"
	local description = [[
	The Timer class is the interface to timers, which let you
	run code on a specified time sequence. Use the <code>start()</code> method
	to start a timer. Add an event listener for the timer event
	to set up code to be run on the timer interval.<br/><br/>

	You can create Timer objects to run once or repeat at specified intervals
	to execute code on a schedule. The dispatch interval of timer events are bound
	to the framerate of the player. Gideros player is set to play at 60 frames per
	second (fps) by default, which is ~16.6 millisecond intervals, but your timer is set to
	fire an event at 10 milliseconds, the event will be dispatched close to the 16
	millisecond interval. Stage complexity and memory-intensive applications may also
	offset the events.
	]]

	local release = "1.0"

	local properties =
	{
		"currentCount",
		"delay",
		"repeatCount",
		"running",

		currentCount =
		{
			type = "number",
			summary = "The total number of times the timer has fired since it started at zero.",
			description = "The total number of times the timer has fired since it started at zero. If the timer has been reset, only the fires since the reset are counted.",
			read_only = true,
		},

		delay =
		{
			type = "number",
			summary = "The delay, in milliseconds, between timer events.",
			description = "The delay, in milliseconds, between timer events. If you set the delay interval while the timer is running, the timer will restart at the same <code>repeatCount</code> iteration. ",
		},

		repeatCount =
		{
			type = "number",
			summary = "The total number of times the timer is set to run.",
			description =
			[[
			The total number of times the timer is set to run. If the repeat
			count is set to 0, the timer continues forever or until the <code>stop()</code> method
			is invoked or the program stops. If the repeat count is nonzero, the
			timer runs the specified number of times. If <code>repeatCount</code> is set to a
			total that is the same or less then <code>currentCount</code> the timer stops and
			will not fire again.
			]],
		},
		running =
		{
			type = "boolean",
			summary = "The timer's current state; true if the timer is running, otherwise false.",
			description = "The timer's current state; <code>true</code> if the timer is running, otherwise <code>false</code>.",
			read_only = true,
		},
	}

	local fields =
	{
	}

	local functions =
	{
		"new",
		"reset",
		"start",
		"stop",

		new =
		{
			summary = "Constructs a new Timer object with the specified delay and repeatCount states.",
			description =
			[[
			Constructs a new Timer object with the specified
			<code>delay</code> and <code>repeatCount</code> states. <br/><br/>

			The timer does not start automatically; you must call
			the <code>start()</code> method to start it.
			]],
			param =
			{
				"delay",
				"repeatCount",

				delay =
				{
					type = "number",
					description = "The delay between timer events, in milliseconds.",
				},

				repeatCount =
				{
					type = "number",
					description = "Specifies the number of repetitions. If zero, the timer repeats infinitely. If nonzero, the timer runs the specified number of times and then stops.",
					default = "0",
				}
			},
		},

		reset =
		{
			summary = "Stops the timer, if it is running, and sets the currentCount property back to 0, like the reset button of a stopwatch.",
			description = "Stops the timer, if it is running, and sets the <code>currentCount</code> property back to 0, like the reset button of a stopwatch. Then, when <code>start()</code> is called, the timer instance runs for the specified number of repetitions, as set by the <code>repeatCount</code> value. ",
			param = {},
		},

		start =
		{
			summary = "Starts the timer, if it is not already running.",
			description = "Starts the timer, if it is not already running.",
			param = {},
		},

		stop =
		{
			summary = "Stops the timer.",
			description = "Stops the timer. When <code>start()</code> is called after <code>stop()</code>, the timer instance runs for the remaining number of repetitions, as set by the <code>repeatCount</code> property. ",
			param = {},
		},
	}

	local tables = {}

	local module_doc = {}
	module_doc.name = name
	module_doc.description = description
	module_doc.author = author
	module_doc.copyright = copyright
	module_doc.release = release
	module_doc.properties = properties
	module_doc.functions = functions
	module_doc.fields = fields
	module_doc.tables = tables

	module_docs[#module_docs + 1] = module_doc
end

function TimerEvent()
	local name = "TimerEvent"
	local base = "Event"
	local description = [[
	A Timer object dispatches a TimerEvent objects whenever the Timer
	object reaches the interval specified by the <code>Timer.delay</code> property.
	]]

	local release = "1.0"

	local properties =
	{
	}

	local fields =
	{
		"TIMER",
		"TIMER_COMPLETE",

		TIMER =
		{
			type = "string",
			value = '"timer"',
			summary = 'The value of <code>TimerEvent.TIMER</code> is the string name <code>"timer"</code> and defines the value of the type property of a timer event.',
			description =
			[[
			Timer event is dispatched whenever a Timer object reaches an interval specified
			according to the <code>Timer.delay</code> property.<br/><br/>

			The value of <code>TimerEvent.TIMER</code> is the string name <code>"timer"</code>
			and defines the value of the type property of a timer event.
			]],
		},

		TIMER_COMPLETE =
		{
			type = "string",
			value = '"timerComplete"',
			summary = 'The value of <code>TimerEvent.TIMER_COMPLETE</code> is the string name <code>"timerComplete"</code> and defines the value of the type property of a timer complete event.',
			description =
			[[
			Timer complete event is dispatched whenever a Timer object has completed the
			number of requests set by <code>Timer.repeatCount</code>.<br/><br/>

			The value of <code>TimerEvent.TIMER_COMPLETE</code> is the string
			name <code>"timerComplete"</code> and defines the value of the type
			property of a timer complete event.
			]],
		},
	}

	local functions =
	{
	}

	local tables = {}

	local module_doc = {}
	module_doc.name = name
	module_doc.base = base
	module_doc.description = description
	module_doc.author = author
	module_doc.copyright = copyright
	module_doc.release = release
	module_doc.properties = properties
	module_doc.functions = functions
	module_doc.fields = fields
	module_doc.tables = tables

	module_docs[#module_docs + 1] = module_doc
end

function Texture()
	local name = "Texture"
	local description =
	[[
	The texture class lets you load an .PNG image file and use as a texture.
	]]

	local release = "1.0"

	local properties =
	{
		"height",
		"width",

		height =
		{
			type = "number",
			summary = "The height of the texture in pixels.",
			description = "The height of the texture in pixels.",
			read_only = true,
		},

		width =
		{
			type = "number",
			summary = "The width of the texture in pixels.",
			description = "The width of the texture in pixels.",
			read_only = true,
		},

	}

	local fields =
	{
	}

	local functions =
	{
		"new",

		new =
		{
			summary = "Constructs a new Texture object with the specified file name.",
			description =
			[[
			Constructs a new Texture object with the specified file name.
			]],
			param =
			{
				"fileName",

				fileName =
				{
					type = "string",
					description = "",
				},
			},
		},

	}

	local tables = {}

	local module_doc = {}
	module_doc.name = name
	module_doc.base = base
	module_doc.description = description
	module_doc.author = author
	module_doc.copyright = copyright
	module_doc.release = release
	module_doc.properties = properties
	module_doc.functions = functions
	module_doc.fields = fields
	module_doc.tables = tables

	module_docs[#module_docs + 1] = module_doc
end

function TextureRegion()
	local name = "TextureRegion"
	local description =
	[[
	TextureRegion class lets you specify a texture with a region in it.<br/><br/>

	A TextureRegion object can share its Texture reference among several TextureRegion objects.
	Because you can create multiple TextureRegion objects that reference the same Texture object,
	multiple TextureRegion objects can use the same complex Texture object without incurring the memory
	overhead of a Texture object for each TextureRegion instance.
	]]

	local release = "1.0"

	local properties =
	{
	}

	local fields =
	{
	}

	local functions =
	{
		"new",

		new =
		{
			summary = "Constructs a new TextureRegion object with the specified Texture and rectangular region inside the texture.",
			description =
			[[
			Constructs a new TextureRegion object with the specified Texture and rectangle region inside the texture.
			]],
			param =
			{
				"texture",
				"minx",			--TODO: sirayi tam hatirlayamadim (sirayi flash'taki Rect ile ayni yapalim)
				"maxx",
				"miny",
				"maxy",

				texture =
				{
					type = "Texture",
					description = "",
				},

				minx =
				{
					type = "number",
					description = "",
				},

				miny =
				{
					type = "number",
					description = "",
				},

				maxx =
				{
					type = "number",
					description = "",
				},
				maxy =
				{
					type = "number",
					description = "",
				},

			},
		},
	}

	local tables = {}

	local module_doc = {}
	module_doc.name = name
	module_doc.base = base
	module_doc.description = description
	module_doc.author = author
	module_doc.copyright = copyright
	module_doc.release = release
	module_doc.properties = properties
	module_doc.functions = functions
	module_doc.fields = fields
	module_doc.tables = tables

	module_docs[#module_docs + 1] = module_doc
end

function Bitmap()
	local name = "Bitmap"
	local base = "Sprite"
	local description =
	[[
	The Bitmap class represents display objects that represent bitmap images.<br/><br/>

	The <code>Bitmap.new()</code> function allows you to create a Bitmap object that contains a
	reference to a TextureRegion object. After you create a Bitmap object, use the <code>addChild()</code>
	function of the parent Sprite instance to place the bitmap on the display list.
	]]

	local release = "1.0"

	local properties =
	{
	}

	local fields =
	{
	}

	local functions =
	{
		"new",

		new =
		{
			summary = "",
			description = "",
			param =
			{
				"textureRegion",

				textureRegion =
				{
					type = "TextureRegion",
					description = "",
				}
			},
		},
	}

	local tables = {}

	local module_doc = {}
	module_doc.name = name
	module_doc.base = base
	module_doc.description = description
	module_doc.author = author
	module_doc.copyright = copyright
	module_doc.release = release
	module_doc.properties = properties
	module_doc.functions = functions
	module_doc.fields = fields
	module_doc.tables = tables

	module_docs[#module_docs + 1] = module_doc
end

function Sound()
	local name = "Sound"
	local description =
	[[
	The Sound class lets you work with sound in an application. The Sound class
	lets you create a new Sound object to load and play an external
	WAV or MP3 file. <br/><br/>

	More detailed control of the sound is performed through the
	SoundChannel object for the sound.
	]]

	local release = "1.0"

	local properties =
	{
		"length",

		length =
		{
			type = "number",
			summary = "The length of the current sound in milliseconds.",
			description = "The length of the current sound in milliseconds.",
			read_only = true,
		},
	}

	local fields =
	{
	}

	local functions =
	{
		"new",
		"play",

		new =
		{
			summary = "",
			description = "",
			param =
			{
				"fileName",

				fileName =
				{
					type = "string",
					description = "",
				},
			},
		},

		play =
		{
			summary = "Generates a new SoundChannel object to play back the sound.",
			description =
			[[
			Generates a new SoundChannel object to play back the sound.
			This method returns a SoundChannel object, which you access to monitor and stop the sound.
			]],
			param =
			{
				"startTime",
				"loops",

				startTime =
				{
					type = "number",
					description = "The initial position in milliseconds at which playback should start.",
					default = "0",
				},

				loops =
				{
					type = "number",
					description = "Defines the number of times a sound loops back to the <code>startTime</code> value before the sound channel stops playback. ",
					default = "0",
				}
			},
			ret = "A SoundChannel object, which you use to control the sound. This method returns <code>nil</code> if you want to play more then one MP3 file or if you run out of available sound channels.",
		},
	}

	local tables = {}

	local module_doc = {}
	module_doc.name = name
	module_doc.base = base
	module_doc.description = description
	module_doc.author = author
	module_doc.copyright = copyright
	module_doc.release = release
	module_doc.properties = properties
	module_doc.functions = functions
	module_doc.fields = fields
	module_doc.tables = tables

	module_docs[#module_docs + 1] = module_doc
end

function SoundChannel()
	local name = "SoundChannel"
	local description =
	[[
	The SoundChannel class controls a sound in an application.
	Every sound is assigned to a sound channel, and the application
	can have multiple sound channels that are mixed together. The SoundChannel
	class contains a <code>stop()</code> method and property for monitoring position of the channel.<br/><br/>

	<code>Event.SOUND_COMPLETE</code> event is dispatched when a sound has finished playing.
	]]

	local release = "1.0"

	local properties =
	{
		"position",

		position =
		{
			type = "number",
			summary = "When the sound is playing, the position property indicates the current point that is being played in the sound file.",
			description =
			[[
			When the sound is playing, the position property indicates the current point
			that is being played in the sound file. When the sound is stopped or paused, the
			position property indicates the last point that was played in the sound file.<br/><br/>

			A common use case is to save the value of the position property when the sound is stopped.
			You can resume the sound later by restarting it from that saved position.
			]],
			read_only = true,
		},
	}

	local fields =
	{
	}

	local functions =
	{
		"stop",

		stop =
		{
			summary = "Stops the sound playing in the channel.",
			description = "Stops the sound playing in the channel.",
			param = {},
		},
	}

	local tables = {}

	local module_doc = {}
	module_doc.name = name
	module_doc.base = base
	module_doc.description = description
	module_doc.author = author
	module_doc.copyright = copyright
	module_doc.release = release
	module_doc.properties = properties
	module_doc.functions = functions
	module_doc.fields = fields
	module_doc.tables = tables

	module_docs[#module_docs + 1] = module_doc
end

function TextField()
	local name = "TextField"
	local description =
	[[
	The TextField class is used to create display objects for text display.
	]]

	local release = "1.0"

	local properties =
	{
		"text",
		text =
		{
			type = "string",
			summary = "A string that is the current text in the text field.",
			description = "A string that is the current text in the text field.",
		},
	}

	local fields =
	{
	}

	local functions =
	{
		"new",

		new =
		{
			summary = "",
			description = "",
			param = {},
		},
	}

	local tables = {}

	local module_doc = {}
	module_doc.name = name
	module_doc.base = base
	module_doc.description = description
	module_doc.author = author
	module_doc.copyright = copyright
	module_doc.release = release
	module_doc.properties = properties
	module_doc.functions = functions
	module_doc.fields = fields
	module_doc.tables = tables

	module_docs[#module_docs + 1] = module_doc
end

function TexturePack()
	local name = "TexturePack"
	local description =
	[[
	]]

	local release = "1.0"

	local properties =
	{
	}

	local fields =
	{
	}

	local functions =
	{
	}

	local tables = {}

	local module_doc = {}
	module_doc.name = name
	module_doc.base = base
	module_doc.description = description
	module_doc.author = author
	module_doc.copyright = copyright
	module_doc.release = release
	module_doc.properties = properties
	module_doc.functions = functions
	module_doc.fields = fields
	module_doc.tables = tables

	module_docs[#module_docs + 1] = module_doc
end

function Font()
	local name = "Font"
	local description =
	[[
	]]

	local release = "1.0"

	local properties =
	{
	}

	local fields =
	{
	}

	local functions =
	{
	}

	local tables = {}

	local module_doc = {}
	module_doc.name = name
	module_doc.base = base
	module_doc.description = description
	module_doc.author = author
	module_doc.copyright = copyright
	module_doc.release = release
	module_doc.properties = properties
	module_doc.functions = functions
	module_doc.fields = fields
	module_doc.tables = tables

	module_docs[#module_docs + 1] = module_doc
end

function MouseEvent()
	local name = "MouseEvent"
	local description =
	[[
	]]

	local release = "1.0"

	local properties =
	{
	}

	local fields =
	{
	}

	local functions =
	{
	}

	local tables = {}

	local module_doc = {}
	module_doc.name = name
	module_doc.base = base
	module_doc.description = description
	module_doc.author = author
	module_doc.copyright = copyright
	module_doc.release = release
	module_doc.properties = properties
	module_doc.functions = functions
	module_doc.fields = fields
	module_doc.tables = tables

	module_docs[#module_docs + 1] = module_doc
end

function TouchEvent()
	local name = "TouchEvent"
	local description =
	[[
	]]

	local release = "1.0"

	local properties =
	{
	}

	local fields =
	{
	}

	local functions =
	{
	}

	local tables = {}

	local module_doc = {}
	module_doc.name = name
	module_doc.base = base
	module_doc.description = description
	module_doc.author = author
	module_doc.copyright = copyright
	module_doc.release = release
	module_doc.properties = properties
	module_doc.functions = functions
	module_doc.fields = fields
	module_doc.tables = tables

	module_docs[#module_docs + 1] = module_doc
end


Sprite()
EventDispatcher()
Event()
Timer()
TimerEvent()
Texture()
TextureRegion()
Bitmap()
Sound()
SoundChannel()
TextField()
TexturePack()
Font()
MouseEvent()
TouchEvent()


links = {}

for _,v in ipairs(module_docs) do
	links[v.name] = v.name .. ".html"
end

function tolink(keyword)
	return links[keyword] and ('<a href="' .. links[keyword] .. '">' .. keyword .. "</a>") or keyword
end


io.input("module.lp", "r")
template = io.read("*all");

for _,v in ipairs(module_docs) do
	module_doc = v

	io.output(links[v.name], "w")
	loadstring(lp.translate(template))()
end
