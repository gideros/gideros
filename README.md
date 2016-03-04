## Gideros Cross-Platform Mobile Development Environment

[![GitHub release](https://img.shields.io/github/release/gideros/gideros.svg)]() [![Github Releases](https://img.shields.io/github/downloads/gideros/gideros/latest/total.svg)]() [![Github All Releases](https://img.shields.io/github/downloads/gideros/gideros/total.svg)]() [![Twitter Follow](https://img.shields.io/twitter/follow/GiderosMobile.svg?style=social)]() [![GitHub stars](https://img.shields.io/github/stars/gideros/gideros.svg?style=social&label=Star)]()

![Gideros](http://giderosmobile.com/cms.design/images/slider/apps.jpg)

Gideros is a cross-platform mobile development environment for creating amazing games. In a couple of hours, you’ll find yourself building and running your next great game. Developers trust Gideros in building 1000s of games across AppStore, Google Play, Amazon Store, Ouya and more.

## Benefits

* **Free:** Gideros is an open source project. Just download and use it for FREE.
* **Instant testing:** While developing your game, it can be tested on a real device through Wifi in only 1 second – you don’t waste your time with an export or deploy process.
* **Native speed:** Developed on top of C/C++ and OpenGL, your game runs at native speed and fully utilizes the power of CPUs and GPUs underneath.
* **Full development set:** Get everything you need from the start, including lightweight IDE, players for Desktop and devices, Texture packer, Font Creator and there are also lots of 3rd party tools.
* **Cross-platform:** Apart from supporting multiple platforms, Gideros also provides automatic screen scaling and automatic selecting of proper image resolution, which makes supporting different screen resolutions and creating universal projects an easy task.
* **Extensive plugins** You can easily extend the core with plugins. Import your existing (C, C++, Java or Obj-C) code, bind to Lua and interpret them directly. Dozens of open-source plugins are already developed and ready to use.
* **Fast development** Easy learning curve, instant testing, OOP coding practices and ability to create needed custom plugins reduces the development time. And because of reusable code, each your next app will be developed even faster.
* **Clean OOP approach** Gideros provides its own class system with all the basic OOP standards, enabling you to write clean and reusable code for any of your future games.

## Example code

Lua is used to build games with Gideros. Below you can find a few simple examples to show how easy to show basic screen elemens.

### Displaying an image

To display image, we firstly create the Texture object with its name and optional boolean parameter which indicates if the image should be filtere (anti-alised). Then we create Bitmap object, position it at some coordinate (default are 0,0) and add it to the stage.

<pre>
local bmp = Bitmap.new(Texture.new("images/ball.png", true))
bmp:setPosition(100, 100)
stage:addChild(bmp)
</pre>

## Drawing a shape

We will use a solid red color the fill style and 5px width blue line with 1 alpha (or full opacity). Easy, isn't it?

<pre>
local shape = Shape.new()
shape:setFillStyle(Shape.SOLID, 0xff0000)
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
</pre>

## Where to start? 

* [**Introduction:**](http://docs.giderosmobile.com/) Everything from creating your first project and running it on device, to the basic concepts of OOP, File system and Events. A must read for all new Developers. 
* [**Reference Guide:**](http://docs.giderosmobile.com/reference) Information about every class, method, event, property, plugin avaialble with main Gideros SDK bundle and with examples. Bookmark it, you'll be using it a lot.
* [**Developer Center:**](http://giderosmobile.com/DevCenter/index.php/Main_Page) Developer Center contains different articles and posts about Gideros, that might help you deal with some specific problems.
* [**Everything else...**](http://giderosmobile.com/guide) All developer related documentation... 

##3rd party tools 

Gideros runs with many 3rd party applications, from Particle Candy to Physics Editor to Texture Packer. [Here'a an incomplete list](http://giderosmobile.com/tools).

## Join Gideros community

Gideros has an active, vivid community. We have a lively, helpful team of Gideros experts, users and newcomers discussing future of Gideros and mobile development. [Join us here](http://giderosmobile.com/forum)











