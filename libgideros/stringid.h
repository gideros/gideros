#ifndef STRINGID_H
#define STRINGID_H

#include <map>
#include <string>
#include <vector>
#include <string.h>
#include "gideros_p.h"

enum StringIdType
{
	eStringIdEnterFrame = 0,

	eStringIdComplete,

	eStringIdX,
	eStringIdY,
	eStringIdZ,
	eStringIdRotation,
	eStringIdRotationX,
	eStringIdRotationY,
	eStringIdScale,
	eStringIdScaleX,
	eStringIdScaleY,
	eStringIdScaleZ,
    eStringIdAnchorX,
    eStringIdAnchorY,
    eStringIdAnchorZ,
	eStringIdAlpha,
    eStringIdRedMultiplier,
    eStringIdGreenMultiplier,
    eStringIdBlueMultiplier,
    eStringIdAlphaMultiplier,
    eStringIdSkewX,
    eStringIdSkewY,

	eStringIdLinear,
	eStringIdInQuad,
	eStringIdOutQuad,
	eStringIdInOutQuad,
	eStringIdOutInQuad,
	eStringIdInCubic,
	eStringIdOutCubic,
	eStringIdInOutCubic,
	eStringIdOutInCubic,
	eStringIdInQuart,
	eStringIdOutQuart,
	eStringIdInOutQuart,
	eStringIdOutInQuart,
	eStringIdInQuint,
	eStringIdOutQuint,
	eStringIdInOutQuint,
	eStringIdOutInQuint,
	eStringIdInSine,
	eStringIdOutSine,
	eStringIdInOutSine,
	eStringIdOutInSine,
	eStringIdInExpo,
	eStringIdOutExpo,
	eStringIdInOutExpo,
	eStringIdOutInExpo,
	eStringIdInCirc,
	eStringIdOutCirc,
	eStringIdInOutCirc,
	eStringIdOutInCirc,
	eStringIdInElastic,
	eStringIdOutElastic,
	eStringIdInOutElastic,
	eStringIdOutInElastic,
	eStringIdInBack,
	eStringIdOutBack,
	eStringIdInOutBack,
	eStringIdOutInBack,
	eStringIdOutBounce,
	eStringIdInBounce,
	eStringIdInOutBounce,
	eStringIdOutInBounce,
	eStringIdInCurve,
	eStringIdOutCurve,
	eStringIdSineCurve,
	eStringIdCosineCurve,

	eLastStringId,
};

class GIDEROS_API StringId
{
public:
	static StringId& instance();

	int id(const char* str);
	int id(const std::string& str);

private:
	StringId();
	~StringId();

	struct ltstr
	{
		bool operator()(const char* s1, const char* s2) const
		{
			return strcmp(s1, s2) < 0;
		}
	};

	typedef std::map<const char*, int, ltstr> map_t;
	map_t idmap_;
	int nextid_;

	std::vector<char*> deletelist_;
};

#endif
