To run the examples, you can either directly use

    $> lua ../luam.lua run-tests.lua

or make a shortcut to luam.lua on your path.

    $> luam run-tests.lua

`run-tests.lua` is both an example of using macros and a little test harness for the package.

The shortcut should look something like this

    @echo off
    rem luam.bat
    lua c:\mylua\LuaMacro\luam.lua %*

or this:

    # luam
    lua /home/frodo/lua/LuaMacro/luam.lua $*

and then should be copied somewhere on your executable path.

