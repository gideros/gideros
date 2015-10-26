require_ "rawhash"

function test ()
    Tab mytable, another

    t = {1,3}

    -- Here # is short for #mytable
    mytable[#+1] = 1
    mytable[#+1] = 2

    -- without indexing, behaves just like a table reference
    assert(type(mytable)=='table')

    -- it is still possible to use #t explicitly
    assert(mytable [#]==mytable[#t])

    assert(mytable[#-1] == mytable[1])

    for i = 1,10 do another[#+1] = i end
    for i = 1,10 do assert(another[i] == i) end

end

test()

-- although mytable is a macro, its scope is limited to test()
assert(mytable == nil)







