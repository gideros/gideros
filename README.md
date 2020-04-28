## Gideros Cross-Platform Development Environment

[![GitHub release](https://img.shields.io/github/release/gideros/gideros.svg)]() [![Github Releases](https://img.shields.io/github/downloads/gideros/gideros/latest/total.svg)]() [![Github All Releases](https://img.shields.io/github/downloads/gideros/gideros/total.svg)]() [![Twitter Follow](https://img.shields.io/twitter/follow/GiderosMobile.svg?style=social)]() [![GitHub stars](https://img.shields.io/github/stars/gideros/gideros.svg?style=social&label=Star)]()

![Gideros](http://giderosmobile.com/cms.design/images/slider/apps.jpg)

Gideros is a cross-platform development environment for creating amazing games and interactive applications in 2D or 3D. It is easy to pick up, quick to develop and robust to deploy. Code your game once and deploy to Android, iOS, MacOS, tvOS, Windows, HTML 5 and more.

## Benefits

* **Free:** Gideros is an open source project. It is completely free for personal and commercial projects.
* **Instant testing:** While developing your game, it can be tested on a real device through Wifi in only 1 second – you don’t waste your time with an export or deploy process.
* **Native speed:** Developed on top of C/C++ and OpenGL or Metal, your game runs at native speed and fully utilizes the power of CPUs and GPUs underneath.
* **Full development set:** Get everything you need from the start, including a lightweight IDE, players for Desktop and mobile devices as well as tools to manage your assets (Texture Packer, Font Creator).
* **Cross-platform:** Apart from supporting multiple platforms, Gideros also provides automatic screen scaling and automatic selection of proper image resolution, which makes supporting different screen resolutions, aspect ratios and universal projects an easy task.
* **Extensive plugins** You can easily extend the core with plugins. Import your existing (C, C++, Java or Obj-C) code, bind to Lua and interpret them directly. Dozens of open-source plugins are already developed and ready to use: ads, in-app purchases, physics for 2d or 3d, Steam integration and many more.
* **Fast development** Easy learning curve, instant testing and the ability to create custom native plugins reduces development time.
* **Clean OOP approach** Gideros provides its own class system on top of Lua with all the basic OOP standards, enabling you to write clean and reusable code for any of your future games.
* **Well-established API** Gideros is a mature software with years of development on its back and is influenced by the Flash API - as such it will be instantly familiar to seasoned developers and newcomers.

## Example code

Gideros uses Lua as its scripting language. It is a fast and friendly language which is well established in the world of game development.

### Displaying an image

To display an image, we first create the Texture object with its reference to an image file and an optional boolean parameter which indicates if the image should be anti-aliased. Then we create a Bitmap object with our Texture, position it at coordinates (100, 100) and add it to the stage, which is the main container for all objects that should be drawn on screen.

```lua
local tex = Texture.new("images/ball.png", true)
local bmp = Bitmap.new(tex)
bmp:setPosition(100, 100)
stage:addChild(bmp)
```

## Drawing a custom shape

Gideros provides an API for drawing custom shapes. In this example we use a solid red color with a solid fill and a 5px-wide blue line with a fully opaque border. Easy, isn't it?

```lua
local shape = Shape.new()
shape:setFillStyle(Shape.SOLID, 0xff0000) -- RGB-color red
shape:setLineStyle(5, 0x0000ff, 1)
shape:beginPath()
shape:moveTo(0,0)
shape:lineTo(0, 100)
shape:lineTo(100, 100)
shape:lineTo(100, 0)
shape:lineTo(0, 0)
shape:endPath()
shape:setPosition(200, 100)
stage:addChild(shape)
```

## Where to start? 

* [**Introduction:**](http://docs.giderosmobile.com/) Everything from creating your first project and running it on device, to the basic concepts of OOP, File system and Events. A must read for all new Developers. 
* [**Reference Guide:**](http://docs.giderosmobile.com/reference) API information about every class, method, event, property and plugin available in Gideros with examples of how to use them. Bookmark it, you'll be using it a lot.
* [**Gideros Wiki:**](http://wiki.giderosmobile.com/index.php/Welcome!) The Gideros Wiki, containing a wealth of information with links to tutorials, community contributed classes and code snippets and tips and tricks on how to develop with Gideros.
* [**Everything else...**](http://giderosmobile.com/guide) Even more developer related documentation... 

## Join the Gideros community

Gideros has an active, friendly community. We have a lively, helpful team of Gideros experts, users and newcomers discussing the future of Gideros and games development. [Join us here](http://giderosmobile.com/forum)











