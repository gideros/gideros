#include "colortransform.h"

ColorTransform::ColorTransform(	float redMultiplier,
								float greenMultiplier,
								float blueMultiplier,
								float alphaMultiplier,
								float redOffset,
								float greenOffset,
								float blueOffset,
								float alphaOffset) :
	redMultiplier_(redMultiplier),
	greenMultiplier_(greenMultiplier),
	blueMultiplier_(blueMultiplier),
	alphaMultiplier_(alphaMultiplier),
	redOffset_(redOffset),
	greenOffset_(greenOffset),
	blueOffset_(blueOffset),
	alphaOffset_(alphaOffset)
{

}

