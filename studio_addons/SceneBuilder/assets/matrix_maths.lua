MatrixMaths={}

function MatrixMaths.getScale(m)
	local function cmag(m,n)
		n=1+(n-1)*4
		return ((m[n]^2)+(m[n+1]^2)+(m[n+2]^2)+(m[n+3]^2))^.5
	end
	return vector(cmag(m,1),cmag(m,2),cmag(m,3))
end


local function qquaternionToEuler(w,x,y,z)
   -- roll (x-axis rotation)
   local sinr_cosp = 2 * (w * x + y * z)
   local cosr_cosp = 1 - 2 * (x * x + y * y)
   local rx = math.atan2(sinr_cosp, cosr_cosp)

   -- pitch (y-axis rotation)
    local sinp = 2 * (w * y - z * x)
	local ry
    if (math.abs(sinp) >= 1) then
        ry= 3.141592654 / 2
		if sinp<0 then ry=-ry end
    else
        ry = math.asin(sinp)
	end

    -- yaw (z-axis rotation)
    local siny_cosp = 2 * (w * z + x * y)
    local cosy_cosp = 1 - 2 * (y * y + z * z)
    local rz = math.atan2(siny_cosp, cosy_cosp)
	
	return vector(^>rx,^>ry,^>rz)
end

local function quaternionToEuler(w,x,y,z)
	local test = x * y + z * w
    if test>0.499 then
		return vector(0,-2*^>math.atan2(x,w),-90)
	elseif test<-.499 then
		return vector(0,2*^>math.atan2(x,w),90)
	else
        local rx=^>math.atan2(2*x*w-2*y*z,1-2*x*x-2*z*z)
        local ry=^>math.atan2(2*y*w-2*x*z,1-2*y*y-2*z*z)
		local rz=^>math.asin(2*x*y+2*z*w)
		return vector(-rx,-ry,-rz)
	end
end
		 
function MatrixMaths.getRotation(m)
	local s=MatrixMaths.getScale(m)
	local m00=m[1]/s.x
	local m01=m[2]/s.y
	local m02=m[3]/s.z
	local m10=m[5]/s.x
	local m11=m[6]/s.y
	local m12=m[7]/s.z
	local m20=m[9]/s.x
	local m21=m[10]/s.y
	local m22=m[11]/s.z

	local qw=((0<>(1+m00+m11+m22))^.5)/2
	local qx=((0<>(1+m00-m11-m22))^.5)/2
	local qy=((0<>(1-m00+m11-m22))^.5)/2
	local qz=((0<>(1-m00-m11+m22))^.5)/2
	
	if qx*(m21-m12)<0 then qx=-qx end
	if qy*(m02-m20)<0 then qy=-qy end
	if qz*(m10-m01)<0 then qz=-qz end
	
	local qm=(qw*qw+qx*qx+qy*qy+qz*qz)^.5
	return quaternionToEuler(qw/qm,qx/qm,qy/qm,qz/qm)
end	 
