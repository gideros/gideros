-- shows how a macro module pulled in with require_
-- can return a substitution value. In this case,
-- it would be better to use include_, but this
-- method is more general
return function()
    return 'local sin,cos = math.sin, math.cos\n'
end
