-- similar syntax to tests.bat, but more portable and aware of errors.
require_ 'forall'
require_ 'qw'
local lua52 = _VERSION:match '5.2'
local lua51 = not lua52
def_ put io.stderr:write

local tests = qw(dollar,lambda,try,block,forall,scope,do,const,with,case,mod,test,rawhash)

local luam = lua51 and 'luam' or 'luam52'

function run (f)
  put(f,': ')
  local res = os.execute(luam..' '..f)
  if (lua52 and not res) or (lua51 and res ~= 0) then
    put 'failed!\n'
    os.exit(1)
  else
    put 'ok\n'
  end
end

forall f in tests  do
  f = 'test-'..f..'.lua'
  run(f)
end

run '-lcskin test-cskin.lua'

if pcall(require,'pl') then
  run 'test-list.lua'
  run 'test-pl-list.lua'
end


