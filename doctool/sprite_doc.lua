require("lp")


class =
{
	name = "Sprite",
	base = "EventDispatcher",

	description =
	[[
		<p> The Sprite class is a basic display list building block: a display list node that can display graphics and can also contain children.</p>
	]],


	functions =
	{
		{
			name = "addChild",
			brief = "addChild brief",
			description =
			[[
				Adds a child Sprite instance to this Sprite instance. The child is added to the front (top) of all other children in this Sprite instance.
				If you add a child object that already has a different display object container as a parent, the object is removed from the child list of the other display object container.
			]],
			parameters =
			{
				{name = "child", type = "Sprite"},
			}
		},
		{
			name = "addChildAt",
			brief = "addChildAt brief",
			description =
			[[
				Adds a child DisplayObject instance to this DisplayObjectContainer instance. The child is added at the index position specified. An index of 0 represents the back (bottom) of the display list for this DisplayObjectContainer object.
				For example, the following example shows three display objects, labeled a, b, and c, at index positions 0, 2, and 1, respectively:
				If you add a child object that already has a different display object container as a parent, the object is removed from the child list of the other display object container.
			]],
			parameters =
			{
				{name = "child", type = "Sprite"},
				{name = "index", type = "Number"},
			}
		},
	}

}


template = [[
<html>
<body>
<h1><%= class.name %> : <%= class.base %></h1>
<p>
<%= class.description %>
</p>

<% for _,f in ipairs(class.functions) do %>

<p><b>function:</b> <%= f.name %></p>
<p><b>description:</b> <%= f.description %></p>

<% end %>


</body>
</html>
]]


io.input("template.lp", "r")
template = io.read("*all");


io.output("sprite.html", "w")
loadstring(lp.translate(template))()
