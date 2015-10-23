require_ "proto"

Function bonzo (a: number, b: string) : string
    return a .. b
end

print (bonzo(10,"hello"))
print (bonzo("hello"))  ---> blows up!


