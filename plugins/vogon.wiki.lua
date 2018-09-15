--
--
--    vogon.wiki.lua
--
--        example of the Lua file format used by create-wiki-pages.lua which 
--        allows easier creation of wiki doc pages for wiki.giderosmobile.com
--
--        the generated output is published at: wiki.giderosmobile.com/Vogon
--
--

return {
  title = "Vogon",
  supported_platforms = { "android", "ios", "pc", "mac", "winrt", "win32", "html5" },
  available_since = "2042.11",
  class = "Vogon",
  description =
[==[
This is just a test of docs gen from the create-wiki-pages.lua script in the plugins folder of the Gideros 
repository using the included vogon.wiki.lua file. From a Lua description file, multiple pages are created 
which can then be relatively easily copied and pasted into new pages. Run create-wiki-pages.lua  without 
arguments for instructions.<br/>
<br/>
Feel free to experiment with vogon.wiki.lua and publish revisions of the Vogon API. 
]==]
  ,
  inherits_from = {
    "Cryptography"
  },
  examples = {
    {
      title = "Standard use of a Vogon.",
      code =
[==[
require "Vogon"
local vogon = Vogon.new()
local poetry = vogon:writePoetry(topic)
local number_of_deaths = vogon:recitePoetry()
]==]
    }
  }
  -- a one line syntax example is created for each method on that method's wiki page, so
  -- the examples section is for examples with more context. See the format used above...
  -- each example is a table within the main examples table containing "title" and "code"
  -- fields
  ,
  methods = {
  --------------------------------------------------------------------
    {
      name = "new",
      delimiter = ".",
      short = "Create a new Vogon",
      long =
[==[
]==],
      examples = { 
        --[[ one table per example: title and code fields ]]
        {
          title = "Create an administration department of Vogons",
          code =
[==[
local admin_vogons = {}
for i = 1, 42 do
  admin_vogons[#admin_vogons + 1] = Vogon.new()
end
]==]
        },
        -- example of multiple examples
        {
          title = "Create a string quarter of Vogons",
          code =
[==[
local musical_vogons = {}
for i = 1, 4 do
  musical_vogons[#musical_vogons + 1] = Vogon.new()
end
]==]
        }
      },
      parameters = { --[[ one table per parameter: _type, name and desc fields ]] },
      returns = {
        {
          _type = "Vogon",
          name = "vogon",
          desc = "new Vogon object"
        }
      } -- /returns
    } -- /this method
    , -- remember the comma between entries
  --------------------------------------------------------------------
    {
      name = "writePoetry",
      delimiter = ":",
      short = "Creates abnormally long poem.",
      long =
[==[
Creates abnormally long poem. Needs at least 64GB of RAM installed, preferably on a very big computer.
]==]
      ,
      examples = { --[[ one table per example ]] },
      parameters = {
        {
          _type = "string",
          name = "topic",
          desc = "Suggested starting topic eg gabbleblotchits."
        }
      } -- /parameters
      ,
      returns = {
        {
          _type = "boolean",
          name = "segfault",
          desc = "success or failure"
        }
      } -- /returns
    } -- /this method
    ,
  --------------------------------------------------------------------
    {
      name = "recitePoetry",
      delimiter = ":",
      short = "Recites the third worst poetry in the known universe.",
      long =
[==[
Endangers neighbours, friends and confidantes with text renditions of computer generated Vogon poetry.
]==]
      ,
      examples = { },
      parameters = {
        {
          _type = "string",
          name = "topic",
          desc = "Suggested starting topic eg gabbleblotchits."
        }
      } -- /parameters
      ,
      returns = {
        {
          _type = "boolean",
          name = "segfault",
          desc = "success or failure"
        }
      } -- /returns
    } -- /this method
  } -- /methods section
  ,
  -- in these sections we supply the name of the table/class as well, in case there is 
  -- more than one table name used for events or constants
  events = {
  --------------------------------------------------------------------
    {
      name = "Vogon.IMMINENT_DESTRUCTION",
      desc = "Fires when it's towel-grabbing time."
    }
    ,
  --------------------------------------------------------------------
    {
      name = "Vogon.POST_DESTRUCTION",
      desc = "Rarely received."
    }
  } -- /events section
  ,
  constants = {
  --------------------------------------------------------------------
    {
      name = "Vogon.STATUS_SIGN_POSTED",
      value = "1",
      desc = ""
    }
    ,
  --------------------------------------------------------------------
    {
      name = "Vogon.STATUS_DESTRUCTION_APPROVED",
      value = "2", 
      desc = ""
    }
  } -- /constants section
}
