require_ 'macro.forall'
require_ 'qw'
local function execute (name)
   local file = 'test-'..name..'.lua'
   print('executing '..file)
   os.execute('luam '..file)
end
forall name in qw(dollar lambda try block scope do const rawhash include test) do
    execute (name)
end

if pcall(require,'pl') then
    execute 'list'
end

local function exec (cmd)
    print (cmd)
    os.execute(cmd)
end

exec 'luam -lcskin test-cskin.lua'
exec 'luam test-atm.lua'
exec 'luam -VA test-atm.lua'
exec 'set P=1 && luam test-atm.lua'
exec 'set P=1 && luam -VA test-atm.lua'


