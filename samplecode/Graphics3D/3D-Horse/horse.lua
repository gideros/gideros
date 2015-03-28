function buildHorse()
local horse=loadObj("horse","LD_HorseRtime02.obj")
horse:setScale(40,-40,40)
horse:setRotationX(0)
horse:setRotationY(90)
horse:setPosition(300,300,-400)
return horse
end
