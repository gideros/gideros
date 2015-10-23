@let env = os.getenv
@if env 'P'
print 'P env was set'
@if A
print 'Global A was true'
@end
@elseif A
print 'Global A was true, no P'
@else
print 'Neither P or A'
@end


