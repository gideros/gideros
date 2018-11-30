#include "stringid.h"

#define populate(key,val) idmap_[key]=val; reversemap_[val]=key;
StringId::StringId()
{
	populate("enterFrame", eStringIdEnterFrame);

	populate("complete", eStringIdComplete);
	populate("soundComplete", eStringIdComplete);

	populate("x", eStringIdX);
	populate("y", eStringIdY);
	populate("z", eStringIdZ);
	populate("rotation", eStringIdRotation);
	populate("rotationX", eStringIdRotationX);
	populate("rotationY", eStringIdRotationY);
	populate("scale", eStringIdScale);
	populate("scaleX", eStringIdScaleX);
	populate("scaleY", eStringIdScaleY);
	populate("scaleZ", eStringIdScaleZ);
    populate("skewX", eStringIdSkewX);
    populate("skewY", eStringIdSkewY);
	populate("anchorX", eStringIdAnchorX);
    populate("anchorY", eStringIdAnchorY);
    populate("anchorZ", eStringIdAnchorZ);
	populate("alpha", eStringIdAlpha);
    populate("redMultiplier", eStringIdRedMultiplier);
    populate("greenMultiplier", eStringIdGreenMultiplier);
    populate("blueMultiplier", eStringIdBlueMultiplier);
    populate("alphaMultiplier", eStringIdAlphaMultiplier);

	populate("linear",      eStringIdLinear);		//
	populate("inQuadratic",	 eStringIdInQuad);		//
	populate("outQuadratic", eStringIdOutQuad);		//
	populate("inOutQuadratic", eStringIdInOutQuad);	//
//	populate("outInQuadratic", eStringIdOutInQuad);	// no
	populate("inCubic",		eStringIdInCubic);		//
	populate("outCubic",	eStringIdOutCubic);		//
	populate("inOutCubic",	eStringIdInOutCubic);	//
//	populate("outInCubic",	eStringIdOutInCubic);	// no
	populate("inQuartic",	eStringIdInQuart);		//
	populate("outQuartic",	eStringIdOutQuart);		//
	populate("inOutQuartic", eStringIdInOutQuart);	//
//	populate("outInQuartic", eStringIdOutInQuart);	// no
	populate("inQuintic", eStringIdInQuint);			//
	populate("outQuintic",	eStringIdOutQuint);		//
	populate("inOutQuintic", eStringIdInOutQuint);	//
//	populate("outInQuintic", eStringIdOutInQuint);	// no
	populate("inSine",		eStringIdInSine);		//
	populate("outSine",		eStringIdOutSine);		//
	populate("inOutSine",	eStringIdInOutSine);		//
//	populate("outInSine",	eStringIdOutInSine);		// no
	populate("inExponential", eStringIdInExpo);			//
	populate("outExponential", eStringIdOutExpo);		//
	populate("inOutExponential", eStringIdInOutExpo);	//
//	populate("outInExponential", eStringIdOutInExpo);	// no
	populate("inCircular",	eStringIdInCirc);			//
	populate("outCircular",	eStringIdOutCirc);			//
	populate("inOutCircular",eStringIdInOutCirc);		//
//	populate("outInCircular",eStringIdOutInCirc);		// no
	populate("inElastic",	eStringIdInElastic);			//
	populate("outElastic",	eStringIdOutElastic);		//
	populate("inOutElastic",eStringIdInOutElastic);		//
//	populate("outInElastic",eStringIdOutInElastic);		// no
	populate("inBack",		eStringIdInBack);			//
	populate("outBack",		eStringIdOutBack);			//
	populate("inOutBack",	eStringIdInOutBack);			//
//	populate("outInBack",	eStringIdOutInBack);			// no
	populate("inBounce",	eStringIdInBounce);			//
	populate("outBounce",	eStringIdOutBounce);			//
	populate("inOutBounce",	eStringIdInOutBounce);		//
//	populate("outInBounce",	eStringIdOutInBounce);		// no
//	populate("inCurve",		eStringIdInCurve);			// no
//	populate("outCurve",	eStringIdOutCurve);			// no
//	populate("sineCurve",	eStringIdSineCurve);			// no
//	populate("cosineCurve",	eStringIdCosineCurve);		// no


	nextid_ = eLastStringId;
}

StringId::~StringId()
{
	for (std::size_t i = 0; i < deletelist_.size(); ++i)
		delete [] deletelist_[i];
}

StringId& StringId::instance()
{
	static StringId stringId;
	return stringId;
}

int StringId::id(const char* str)
{
	map_t::iterator iter = idmap_.find(str);

	if (iter == idmap_.end())
	{
		nextid_++;

		char* newstr = new char[strlen(str) + 1];
		strcpy(newstr, str);

		deletelist_.push_back(newstr);

		idmap_[newstr] = nextid_;
		reversemap_[nextid_]=newstr;

		return nextid_;
	}

	return iter->second;
}

const char *StringId::str(int id) {
	return reversemap_[id];
}

int StringId::id(const std::string& str)
{
	return id(str.c_str());
}
