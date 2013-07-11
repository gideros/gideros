classes = {}

classes["Graphics"] =
{

}

classes["EventDispatcher"] =
{

}

classes["Sprite"] =
{
	base = "EventDispatcher",

	description =
	[[
		The Sprite class is a basic display list building block: a display list node that can display graphics and can also contain children.

		A Sprite object is similar to a movie clip, but does not have a timeline. Sprite is an appropriate base class for objects that do not require timelines. For example, Sprite would be a logical base class for user interface (UI) components that typically do not use the timeline.

		The Sprite class is new in ActionScript 3.0. It provides an alternative to the functionality of the MovieClip class, which retains all the functionality of previous ActionScript releases to provide backward compatibility.
	]],


	properties =
	{
		{
			name = "graphics",
			type = "Graphics",
			description =
			[[
			]]
		}
	},

	functions =
	{
		{
			name = "addChild",
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


classes["TextField"] =
{
	base = "Sprite",
}

classes["Bitmap"] =
{

	base = "Sprite",
}

classes["Timer"] =
{
	base = "EventDispatcher",
}

--[=[

subclasses = {}

for name,t in pairs(classes) do
	if (t.base ~= nil) then
		subclasses[t.base] = subclasses[t.base] or {}
		table.insert(subclasses[t.base], name)
	end
end

for k, v in pairs(subclasses) do
	io.write(k.." subclasses: ")
	for _,s in ipairs(v) do
		io.write(s.." ")
	end
	print()
end

--]=]



--[[
require("cosmo")
template = "$message{rank = 'Ace', suit = 'Spades'}"
values = {message = function(arg) return arg.rank.." of "..arg.suit end }
print(cosmo.fill(template, values))
--]]

--[=[
require("cosmo")
template = "$do_cards{1, 2, 3}[[$rank of $suit, ]],[[$rank - $suit, ]],[[$rank + $suit, ]]"

cards =
{
	{"1", "2"},
	{"2", "4"},
	{"3", "6"},
}

function do_cards(arg)
	for i, v in ipairs(cards) do
		cosmo.yield{rank=v[1], suit=v[2], _template = arg[i]}
	end
end

print(cosmo.fill(template, {do_cards = do_cards}))
--]=]

--[=[
$val
$val|key1|key2|...|keyn
$selector{arg1 = 'value1', arg2 = 'value2'}
$val[[subtemplate]]
--]=]


require("cosmo")

template = [=[
<html>
<body>
<table>
	$functions[[
	<tr>
	<td>$name</td>
	<td>$description</td>
	</tr>
	]]
</table>
</body>
</html>
]=]


values =
{
	functions =
	{
		{name = "a", description = "b"},
		{name = "c", description = "d"},
		{name = "e", description = "f"},
	}
}

print(cosmo.fill(template, values))
