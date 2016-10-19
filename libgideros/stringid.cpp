#include "stringid.h"

StringId::StringId()
{
	idmap_["enterFrame"] = eStringIdEnterFrame;

	idmap_["complete"] = eStringIdComplete;
	idmap_["soundComplete"] = eStringIdComplete;

	idmap_["x"] = eStringIdX;
	idmap_["y"] = eStringIdY;
	idmap_["z"] = eStringIdZ;
	idmap_["rotation"] = eStringIdRotation;
	idmap_["rotationX"] = eStringIdRotationX;
	idmap_["rotationY"] = eStringIdRotationY;
	idmap_["scale"] = eStringIdScale;
	idmap_["scaleX"] = eStringIdScaleX;
	idmap_["scaleY"] = eStringIdScaleY;
	idmap_["scaleZ"] = eStringIdScaleZ;
    idmap_["skewX"] = eStringIdSkewX;
    idmap_["skewY"] = eStringIdSkewY;
	idmap_["anchorX"] = eStringIdAnchorX;
    idmap_["anchorY"] = eStringIdAnchorY;
    idmap_["anchorZ"] = eStringIdAnchorZ;
	idmap_["alpha"] = eStringIdAlpha;
    idmap_["redMultiplier"] = eStringIdRedMultiplier;
    idmap_["greenMultiplier"] = eStringIdGreenMultiplier;
    idmap_["blueMultiplier"] = eStringIdBlueMultiplier;
    idmap_["alphaMultiplier"] = eStringIdAlphaMultiplier;

	idmap_["linear"] =      eStringIdLinear;		//
	idmap_["inQuadratic"] =	 eStringIdInQuad;		//
	idmap_["outQuadratic"] = eStringIdOutQuad;		//
	idmap_["inOutQuadratic"] = eStringIdInOutQuad;	//
//	idmap_["outInQuadratic"] = eStringIdOutInQuad;	// no
	idmap_["inCubic"] =		eStringIdInCubic;		//
	idmap_["outCubic"] =	eStringIdOutCubic;		//
	idmap_["inOutCubic"] =	eStringIdInOutCubic;	//
//	idmap_["outInCubic"] =	eStringIdOutInCubic;	// no
	idmap_["inQuartic"] =	eStringIdInQuart;		//
	idmap_["outQuartic"] =	eStringIdOutQuart;		//
	idmap_["inOutQuartic"] = eStringIdInOutQuart;	//
//	idmap_["outInQuartic"] = eStringIdOutInQuart;	// no
	idmap_["inQuintic"] = eStringIdInQuint;			//
	idmap_["outQuintic"] =	eStringIdOutQuint;		//
	idmap_["inOutQuintic"] = eStringIdInOutQuint;	//
//	idmap_["outInQuintic"] = eStringIdOutInQuint;	// no
	idmap_["inSine"] =		eStringIdInSine;		//
	idmap_["outSine"] =		eStringIdOutSine;		//
	idmap_["inOutSine"] =	eStringIdInOutSine;		//
//	idmap_["outInSine"] =	eStringIdOutInSine;		// no
	idmap_["inExponential"] = eStringIdInExpo;			//
	idmap_["outExponential"] = eStringIdOutExpo;		//
	idmap_["inOutExponential"] = eStringIdInOutExpo;	//
//	idmap_["outInExponential"] = eStringIdOutInExpo;	// no
	idmap_["inCircular"] =	eStringIdInCirc;			//
	idmap_["outCircular"] =	eStringIdOutCirc;			//
	idmap_["inOutCircular"] =eStringIdInOutCirc;		//
//	idmap_["outInCircular"] =eStringIdOutInCirc;		// no
	idmap_["inElastic"] =	eStringIdInElastic;			//
	idmap_["outElastic"] =	eStringIdOutElastic;		//
	idmap_["inOutElastic"] =eStringIdInOutElastic;		//
//	idmap_["outInElastic"] =eStringIdOutInElastic;		// no
	idmap_["inBack"] =		eStringIdInBack;			//
	idmap_["outBack"] =		eStringIdOutBack;			//
	idmap_["inOutBack"] =	eStringIdInOutBack;			//
//	idmap_["outInBack"] =	eStringIdOutInBack;			// no
	idmap_["inBounce"] =	eStringIdInBounce;			//
	idmap_["outBounce"] =	eStringIdOutBounce;			//
	idmap_["inOutBounce"] =	eStringIdInOutBounce;		//
//	idmap_["outInBounce"] =	eStringIdOutInBounce;		// no
//	idmap_["inCurve"] =		eStringIdInCurve;			// no
//	idmap_["outCurve"] =	eStringIdOutCurve;			// no
//	idmap_["sineCurve"] =	eStringIdSineCurve;			// no
//	idmap_["cosineCurve"] =	eStringIdCosineCurve;		// no


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

		return nextid_;
	}

	return iter->second;
}

int StringId::id(const std::string& str)
{
	return id(str.c_str());
}
