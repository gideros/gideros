# cute_c2 LUA binding for [Gideros mobile](http://giderosmobile.com/)
[cute_c2](https://github.com/RandyGaul/cute_headers)

# API
```lua
require "CuteC2"
```

## Global functions

### Objects
```lua
-- x, y (numbers): position 
-- r (number): radius
Circle = CuteC2.circle(x, y, r)

-- minX, minY (numbers): top left corner position 
-- maxX, maxY (numbers): bottom right corner position
-- anchor point is in top left corner
AABB = CuteC2.aabb(minX, minY, maxX, maxY)

-- x, y (numbers): position 
-- h (number): height
-- r (number): radius
Capsule = CuteC2.capsule(x, y, h, r)

-- points (table): table of x/y pairs (example: {0,0, 100,50, 60,30, 40,90})
-- !!!IMPORTANT NOTE!!! maximum amount of points pairs is 8, everything beyond this limit is ignored
Poly = CuteC2.poly(points)

-- x1, y1 (numbers): start position
-- x2, y2 (numbers): destenation position (gets normalized)
-- len (number): ray lenght (default: distance between (x1, y1) and (x2, y2))
Ray = CuteC2.ray(x1, y1, x2, y2 [, len])
-- x, y (numbers): start position
-- rotation (number): facing angle (in radians)
-- len (number): ray lenght (default: 1)
Ray = CuteC2.rayFromRotation(x, y, rotation [, len])

-- x, y (numbers): position (default: (0, 0))
-- r (number): rotation (default: 0)
Transform = CuteC2.transform([x, y, r])
```

### Functions
```lua
bool = CuteC2.circleToCircle(circle1, circle2)
bool = CuteC2.circleToAABB(circle, aabb)
bool = CuteC2.circleToCapsule(circle, capsule)
bool = CuteC2.AABBtoAABB(aabb1, aabb2)
bool = CuteC2.AABBtoCapsule(aabb, capsule)
bool = CuteC2.capsuleToCapsule(capsule1, capsule2)
bool = CuteC2.circleToPoly(circle, poly [, transform])
bool = CuteC2.AABBtoPoly(aabb, poly [, transform])
bool = CuteC2.capsuleToPoly(capsule, poly [, transform])
bool = CuteC2.polyToPoly(poly1, poly2 [, transform1, transform2])
bbol = CuteC2.collided(object1, object2 [, transform1, transform2])

bool = CuteC2.AABBtoPoint(aabb, x, y)
bool = CuteC2.circleToPoint(circle, x, y)
bool = CuteC2.capsuleToPoint(capsule, x, y)
bool = CuteC2.polyToPoint(poly, x, y [, transform])

result, normalX, normalY, t = CuteC2.castRay(ray, object [, transform])
result, normalX, normalY, t = CuteC2.castRay(rayX1, rayY1, rayX2, rayY2, rayLen, object [, transform])

-- t (number): value from raycast result
x, y = CuteC2.impact(ray, t)
x, y = CuteC2.impact(rayX1, rayY1, rayX2, rayY2, rayLen, t)

-- Mainfold (table):
-- {
--		count = number,
--		depth = {number1, number2},
--		contact_points = {
--			{x = number, y = number}, 
--			{x = number, y = number}
--		}, 
--		normal = {x = number, y = number}
-- }
Mainfold = CuteC2.collide(object1, object2 [, transform1, transform2])
Mainfold = CuteC2.circleToCircleManifold(circle1, circle2)
Mainfold = CuteC2.circleToAABBManifold(circle, aabb)
Mainfold = CuteC2.circleToCapsuleManifold(circle, capsule)
Mainfold = CuteC2.AABBtoAABBManifold(aabb1, aabb2)
Mainfold = CuteC2.AABBtoCapsuleManifold(aabb, capsule)
Mainfold = CuteC2.capsuleToCapsuleManifold(capsule1, capsule2)
Mainfold = CuteC2.circletoPolyManifold(circle, poly)
Mainfold = CuteC2.AABBtoPolyManifold(aabb, poly)
Mainfold = CuteC2.capsuleToPolyManifold(capsule, poly)
Mainfold = CuteC2.polyToPolyManifold(poly1, poly2)
```
### Advanced functions
Check the [source code](https://github.com/RandyGaul/cute_headers/blob/df3b63e072afa275a72ce8aa7fce0428a5966e0c/cute_c2.h#L342) for more information
```lua
distance, aX, aY, bX, bY, iterations = CuteC2.GJK(object1, object2 [, transform1, transform2])
-- hit (bool): true if shapes were touching at the TOI, false if they never hit.
-- toi (number): The time of impact between two shapes.
-- nx, ny (numbers): Surface normal from shape A to B at the time of impact.
-- px, py (numbers): Point of contact between shapes A and B at time of impact.
-- iterations (number): Number of iterations the solver underwent.
hit, toi, nx, ny, px, py, iterations = CuteC2.TOI(object1, v1x, v1y, object2, v2x, v2y [, use_radius, transform1, transform2])
```

## Circle
```lua
Circle:setPosition(x, y)
x, y = Circle:getPosition()

Circle:setX(x)
x = Circle:getX()

Circle:setY(y)
y = Circle:getY()

Circle:setRadius(radius)
radius = Circle:getRadius()

minX, minY, maxX, maxY = Circle:getBoundingBox()

x, y, radius = Circle:getData()

Circle:move(dx, dy)

Circle:inflate(skin_factor)

bool = Circle:hitTest(x, y)

result, normalX, normalY, t = Circle:rayTest(ray [, transform])
result, normalX, normalY, t = Circle:rayTest(rayX1, rayY1, rayX2, rayY2, rayLen [, transform])
```

## AABB
```lua
-- x, y (number): top left corner position
AABB:setMinPosition(x, y)
x, y = AABB:getMinPosition()

-- x, y (number): bottom right corner position
AABB:setMaxPosition(x, y)
x, y = AABB:getMaxPosition()

-- x, y (number): center position
AABB:setPosition(x, y)
x, y = AABB:getPosition()

AABB:setMinX(x)
x = AABB:getMinX()

AABB:setMinY(y)
y = AABB:getMinY()

AABB:setMaxX(x)
x = AABB:getMaxX()

AABB:setMaxY(y)
y = AABB:getMaxY()

AABB:setX(x)
x = AABB:getX()

AABB:setY(y)
y = AABB:getY()

-- w, h (number): full size (relative to shape center)
AABB:setSize(w, h)
w, h = AABB:getSize()

-- w, h (number): half size (relative to shape center)
AABB:setHalfSize(w, h)
w, h = AABB:getHalfSize()

-- w (number): full width (relative to shape center)
AABB:setWidth(w)
w = AABB:getWidth()

-- h (number): full height (relative to shape center)
AABB:setHeight(h)
h = AABB:getHeight()

-- w (number): half width (relative to shape center)
AABB:setHalfWidth(w)
w = AABB:getHalfWidth()

-- h (number): half height (relative to shape center)
AABB:setHalfHeight(h)
h = AABB:getHalfHeight()

minX, minY, maxX, maxY = AABB:getBoundingBox()

minX, minY, maxX, maxY, fullW, fullH = AABB:getData()

AABB:move(dx, dy)

AABB:inflate(skin_factor)

bool = AABB:hitTest(x, y)
result, normalX, normalY, t = AABB:rayTest(ray [, transform])
result, normalX, normalY, t = AABB:rayTest(rayX1, rayY1, rayX2, rayY2, rayLen [, transform])
```

## Capsule
```lua
         , - ~ ~ ~ - ,-------------.-
     , '               ' ,         |
   ,                       ,       |  Radius
  ,                         ,      |
 ,- - - - - - -X-<Tip - - - -,-----|-
 ,                           ,     |
 ,                           ,     |
 ,                           ,     |
 ,             X < Center    ,     | Height
 ,                           ,     |
 ,                           ,     |
 ,             |             ,     |
 ,- - - - - - -X-<Base- - - -,-----|-
  ,                         ,      |
   ,                       ,       | Radius
     ,                  , '        |
       ' - , _ _ _ ,  '_ _ _ _ _ _ |_
```

```lua
-- x, y (number): center position
Capsule:setPosition(x, y)
x, y = Capsule:getPosition()

-- x, y (number): position of the top point
Capsule:setTipPosition(x, y)
x, y = Capsule:getTipPosition()

-- x, y (number): position of the bottom point
Capsule:setBasePosition(x, y)
x, y = Capsule:getBasePosition()

Capsule:setX(x)
x = Capsule:getX()

Capsule:setY(y)
y = Capsule:getY()

Capsule:setTipX(x)
x = Capsule:getTipX()

Capsule:setTipY(y)
y = Capsule:getTipY()

Capsule:setBaseX(x)
x = Capsule:getBaseX()

Capsule:setBaseY(y)
y = Capsule:getBaseY()

-- h (number): height
Capsule:setHeight(h)
h = Capsule:getHeight()

-- r (number): radius
Capsule:setRadius(r)
r = Capsule:getRadius()

Capsule:setSize(radius, height)
radius, height = Capsule:getSize()

minX, minY, maxX, maxY = Capsule:getBoundingBox()

tipX, tipY, baseX, baseY, radius = Capsule:getData()

Capsule:move(dx, dy)
Capsule:inflate(skin_factor)
bool = Capsule:hitTest(x, y)
result, normalX, normalY, t = Capsule:rayTest(ray [, transform])
result, normalX, normalY, t = Capsule:rayTest(rayX1, rayY1, rayX2, rayY2, rayLen [, transform])
```

## Poly
```lua
-- points (table): table of x/y pairs (example: {0,0, 100,50, 60,30, 40,90})
Poly:setPoints(points)

-- points (table): table of x/y pairs (example: {{x=0,y=0}, {x=100,y=50}, {x=60,y=30}, {x=40,y=90}})
Poly:setPointsXY(points)

-- returns a table of x/ pairs ({{x=0,y=0,}, {x=10,y=0}, ...})
table = Poly:getPoints() 
table = Poly:getNormals()
table = Poly:getRotatedPoints(transform)
table = Poly:getRotatedNormals(transform)

Poly:setVertex(index, x, y)
x, y = Poly:getVertex(index)
x = Poly:getVertexX(index)
y = Poly:getVertexY(index)
x, y = Poly:getRotatedVertex(index, transform)
x = Poly:getRotatedVertexX(index, transform)
y = Poly:getRotatedVertexY(index, transform)

-- return number of vertices
n = Poly:getVertexCount()
-- removes 
Poly:removeVertex(index)
-- add new vertex
-- x, y (number): point coordinats (in local space)
-- index (number): where to insert (default: last index)
-- transform (Transform): Transform object to add point with respect to shape position and rotation
Poly:insertVertex(x, y [, index, transform])

-- if vertices is too far from origin, sets the origin in the middle of its bounding box
Poly:updateCenter()

-- scales object by a factor
Poly:inflate(skin_factor)
bool = Poly:hitTest(x, y [, transform])
minX, minY, maxX, maxY = Poly:getBoundingBox()
-- return a table with points & normals: {points = {...}, normals = {...}}
table = Poly:getData()

result, normalX, normalY, t = Poly:rayTest(ray [, transform])
result, normalX, normalY, t = Poly:rayTest(rayX1, rayY1, rayX2, rayY2, rayLen [, transform])
```

## Ray
Please check (this thread)[https://github.com/RandyGaul/cute_headers/issues/30] for more information about ray direction

### Example
```
┌───────────────────────────────────┐
│  0  1  2  3  4  5  6  7  8  9     │
│  ┌──┬──┬──┬──┬──┬──┬──┬──┬──┬──►  │
│  │  │  │  │  │  │  │  │  │  │  x  │
│1 ├──┼──S##┼──┼──┼──┼──┼──┼──┼──   │
│  │  │  ## ## │  │  │  │  │  │     │
│2 ├──┼──#─#┼─##──┼──┼──┼──┼──┼───  │
│  │  │  #  #  │##│  │  │  │  │     │
│3 ├──┼──#──┼#─┼──##─┼──┼──┼──┼───  │
│  │  │  #  │ #│  │ ##  │  │  │     │
│4 ├──┼──#──┼──#──┼──┼##┼──┼──┼───  │
│  │  │  #  │  │# │  │  ## │  │     │
│5 ├──┼──#──┼──┼─#┼──┼──┼─#N──┼───  │
│  │  │  #  │  │  #  │  │  │  │     │
│6 ├──┼──P──┼──┼──┼#─┼──┼──┼──┼───  │
│  │  │  │  │  │  │ #│  │  │  │     │
│7 ├──┼──┼──┼──┼──┼──T──┼──┼──┼───  │
│  │  │  │  │  │  │  │  │  │  │     │
│  │                                │
│  ▼y                               │
└───────────────────────────────────┘
S = (2; 1) - origin position
T = (6; 7) - direction position
N = (8; 5) - new ray direction
P = (2; 6) - new ray direction
```

```lua
ray = CuteC2.ray(2, 1, 5, 7)

print(ray:getPosition()) --> 2, 1 (point "S")
print(ray:getTargetPosition()) --> 5, 7 (point "T")
print(ray:getLength()) --> 6.70

ray:normalize()
print(ray:getPosition()) --> 2, 1 (point "S")
print(ray:getTargetPosition()) --> (0.45, 0.89) direction vector
print(ray:getLength()) --> 1

ray:faceTo(8, 5) -- point "N"

print(ray:getLength()) --> 7.21
print(ray:getTargetPosition()) --> (0.83, 0.55)
print(ray:getFacePosition()) --> (8, 5)

ray:setDirection(math.pi - math.pi / 2) -- 90 deg
ray:setLength(5) -- point "P"

print(ray:getTargetPosition()) --> ~(0, 1)
print(ray:getFacePosition()) --> ~(2, 6)
```

### API
```lua
ray:setPosition(x, y)
x, y = Ray:getPosition()

-- x, y (number): destenation point
ray:setTargetPosition(x, y)
x, y = Ray:getTargetPosition()

-- x, y (number): destenation point  (target position gets normalized)
Ray:faceTo(x, y)

-- returns point in global space
x, y = Ray:getFacePosition()
x = Ray:getFaceX()
y = Ray:getFaceY()

Ray:setStartX(x)
x = Ray:getStartX()

Ray:setStartY(y)
y = Ray:getStartY()

Ray:setTargetX(x)
x = Ray:getTargetX()

Ray:setTargetY(y)
y = Ray:getTargetY()

Ray:setLength(len)
len = Ray:getLength()

-- normalzies the target position 
-- set_len_to_one (bool) 
Ray:normalize([set_len_to_one])

-- normalzies the target position 
Ray:setDirection(radians)
radians = Ray:getDirection()

-- move original position (but not target!)
Ray:move(dx, dy)
startX, startY, directionX, directionY, len = Poly:getData()
```

## Transform
```lua
Transform:setPosition(x, y)
x, y = Transform:getPosition()
Transform:setX(x)
x = Transform:getX()
Transform:setY(y)
y = Transform:getY()
Transform:move(deltaX, deltaY)
Transform:setRotation(radians)
radians = Transform:getRotation()
Transform:rotate(amount_in_radians)
Transform:move(dx, dy)
-- returns cos & sin of current rotation angle
cos, sin = Transform:getCosSin()
```

## Shape types
```lua
CuteC2.TYPE_NONE
CuteC2.TYPE_CIRCLE
CuteC2.TYPE_AABB
CuteC2.TYPE_CAPSULE
CuteC2.TYPE_POLY
```

Example:
```lua
circle = CuteC2.circle(100, 200, 15)
print(circle.__shapeType == CuteC2.TYPE_CIRCLE) -- true
```
