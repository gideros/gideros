--[[ ==============================================================================
TODO
  - add rounded rectangle to class - why are rounded rects jagged?
  - make sure currentpoint and firstpoint are always set when drawing
  - add comments/documentation
  - add examples
          https://developer.mozilla.org/samples/canvas-tutorial/2_4_canvas_arc.html
          https://developer.mozilla.org/samples/canvas-tutorial/2_7_canvas_combined.html
          https://developer.mozilla.org/en/Canvas_tutorial/Drawing_shapes
  -  add generic polygon
  -  create simple line/fill style functions
-- ================================================================================]]


--[[ ==============================================================================
   Bezier curve code - from http://www.giderosmobile.com/forum/discussion/461#Item_4
   bezier.lua
   http://www.fawzma.com
   source ported from http://paulbourke.net/geometry/bezier/index2.html	
-- ================================================================================]]
local function bezier3(p1,p2,p3,mu)
   local mum1,mum12,mu2
   local p = {}
   mu2 = mu * mu
   mum1 = 1 - mu
   mum12 = mum1 * mum1
   p.x = p1.x * mum12 + 2 * p2.x * mum1 * mu + p3.x * mu2
   p.y = p1.y * mum12 + 2 * p2.y * mum1 * mu + p3.y * mu2
   return p
end

local function bezier4(p1,p2,p3,p4,mu)
   local mum1,mum13,mu3;
   local p = {}
   mum1 = 1 - mu
   mum13 = mum1 * mum1 * mum1
   mu3 = mu * mu * mu
   p.x = mum13*p1.x + 3*mu*mum1*mum1*p2.x + 3*mu*mu*mum1*p3.x + mu3*p4.x
   p.y = mum13*p1.y + 3*mu*mum1*mum1*p2.y + 3*mu*mu*mum1*p3.y + mu3*p4.y
   return p	
end

--[[ ================================================================================
   NdShape class
-- ================================================================================]]
NdShape = Core.class(Shape)
function NdShape:init()
   self.stack        = {}
   self.lineStyle    = nil
   self.fillStyle    = nil
   self.currentPoint = nil
   self.startPoint   = nil
end

function NdShape:setFillStyle(type,colorOrTexture,alphaOrMatrix)
   self.fillStyle = {type,colorOrTexture,alphaOrMatrix}
   Shape.setFillStyle(self,type,colorOrTexture,alphaOrMatrix)
end

function NdShape:setLineStyle(width,color,alpha)
   self.lineStyle = {width,color,alpha}
   Shape.setLineStyle(self,width,color,alpha)
end

function NdShape:beginPath()
   Shape.beginPath(self)
end

function NdShape:moveTo(x,y)
   Shape.moveTo(self,x,y)
   self.currentPoint = { x, y }
   if not self.startPoint then
      self.startPoint = { x, y }
   end
end

function NdShape:lineTo(x,y)
   Shape.lineTo(self,x,y)
   self.currentPoint = { x, y }
   if not self.startPoint then
      self.startPoint = { x, y }
   end
end

function NdShape:closePath()
   Shape.closePath(self)
   self.currentPoint = self.startPoint
   self.startPoint   = nil
end

function NdShape:endPath()
   Shape.endPath(self)
   self.currentPoint = nil
   self.startPoint   = nil
end

function NdShape:clear()
   Shape.clear(self)
   self.lineStyle    = nil
   self.fillStyle    = nil
   self.currentPoint = nil
   self.startPoint   = nil
end

function NdShape:stroke()
   local currentFillStyle = self.fillStyle
   Shape.setFillStyle(self, Shape.NONE)
   self:endPath()
   if currentFillStyle then
      Shape.setFillStyle(self, unpack(currentFillStyle))
   end
end

function NdShape:fill()
   self:endPath()
end

function NdShape:rect(x,y,w,h)
   self:moveTo(x-w/2,y-h/2)
   self:lineTo(x+w/2,y-h/2)
   self:lineTo(x+w/2,y+h/2)
   self:lineTo(x-w/2,y+h/2)
   self:lineTo(x-w/2,y-h/2)
end

function NdShape:fillRect(x,y,w,h)
   self:beginPath()
   self:rect(x,y,w,h)
   self:endPath()
end

function NdShape:strokeRect(x,y,w,h)
   local currentFillStyle = self.fillStyle
   Shape.setFillStyle(self, Shape.NONE)
   self:beginPath()
   self:rect(x,y,w,h)
   self:endPath()
   if currentFillStyle then 
      Shape.setFillStyle(self, unpack(self.fillStyle)) 
   end
end

function NdShape:save()
   table.insert(self.stack, { lineStyle    = self.lineStyle, 
                              fillStyle    = self.fillStyle, 
                              currentPoint = self.currentPoint,
                              startPoint   = self.startPoint, } )
end

function NdShape:restore()
   local context = table.remove(self.stack)
   if context then
      self.lineStyle    = context.lineStyle
      self.fillStyle    = context.fillStyle
      self.currentPoint = context.currentPoint
      self.startPoint   = context.startPoint
   end
end

function NdShape:scale(x,y)
   self:setScale(x or 1, y or 1)
end

function NdShape:rotate(angle)
   self:setRotation(angle or 0)
end

function NdShape:translate(x,y)
   local x0,y0 = self:getPosition()
   self:setPosition(x0 + (x or 0), y0 + (y or 0))
end

function NdShape:transform(m11, m12, m21, m22, tx, ty)
   local Bm11, Bm12, Bm21, Bm22, Btx, Bty = self:getMatrix():getElements()
   self:setMatrix(Matrix.new(
                     m11 * Bm11 + m12 * Bm21,          -- m11
                     m11 * Bm12 + m12 * Bm22,          -- m12
                     m21 * Bm11 + m22 * Bm21,          -- m21
                     m21 * Bm12 + m22 * Bm22,          -- m22
                     m11 * Btx  + m12 * Bty   +  tx,  --  tx
                     m21 * Btx  + m22 * Bty   +  ty   --  ty
                 ))
end

function NdShape:setTransform(m11, m12, m21, m22, tx, ty)
   self:setMatrix(Matrix.new(m11, m12, m21, m22, tx, ty))
end

function NdShape:quadraticCurveTo(cpx, cpy, x, y, mu)
   if self.currentPoint then
      local inc = mu or 0.1 -- need a better default
      for i = 0,1,inc do
         local p = bezier3(
            { x=self.currentPoint[1], y=self.currentPoint[2] },
            { x=cpx, y=cpy },
            { x=x, y=y },
            i)
         Shape.lineTo(self,p.x,p.y)
      end
   end
   self.currentPoint = { x, y }
end

function NdShape:bezierCurveTo(cp1x, cp1y, cp2x, cp2y, x, y, mu)
   if self.currentPoint then
      local inc = mu or 0.1 -- need a better default
      for i = 0,1,inc do  
         local p = bezier4(
            { x=self.currentPoint[1], y=self.currentPoint[2] },
            { x=cp1x, y=cp1y },
            { x=cp2x, y=cp2y },
            { x=x, y=y },
            i)
         Shape.lineTo(self,p.x,p.y)
      end
   end
   self.currentPoint = { x, y }
end

-- very hacky function, there's got to be a better way
function NdShape:ellipse(x,y,xradius,yradius,startAngle,endAngle,anticlockwise)
   local sides = (xradius + yradius) / 2  -- need a better default
   local dist  = 0

   -- handle missing entries
   if startAngle == nil then startAngle = 0 end
   if endAngle   == nil then endAngle   = 2*math.pi end

   -- Find clockwise distance (convert negative distances to positive)
   dist = endAngle - startAngle
   if (dist < 0) then
      dist = 2*math.pi - ((-dist) % (2*math.pi))
   end

   -- handle clockwise/anticlockwise
   if anticlockwise == nil or anticlockwise == false then
      -- CW
      -- Handle special case where mod of the two angles is equal but
      -- they're really not equal 
      if dist == 0 and startAngle ~= endAngle then
         dist = 2*math.pi
      end
   else
      -- CCW
      dist = dist - 2*math.pi

      -- Handle special case where mod of the two angles is equal but
      -- they're really not equal 
      if dist == 0 and startAngle ~= endAngle then
         dist = -2*math.pi
      end

   end

   -- add the lines
   for i=0,sides do
      local angle = (i/sides) *  dist + startAngle
      Shape.lineTo(self, x + math.cos(angle) * xradius,
                         y + math.sin(angle) * yradius)
   end

end

function NdShape:arc(x,y,radius,startAngle,endAngle,anticlockwise)
   self:ellipse(x,y,radius,radius,startAngle,endAngle,anticlockwise)
end

function NdShape:circle(x,y,radius,anticlockwise)
   self:ellipse(x,y,radius,radius,0,2*math.pi,anticlockwise)
end

function NdShape:path(v)
   for i,p in ipairs(v) do
      self:lineTo(p[1], p[2])
   end
end
