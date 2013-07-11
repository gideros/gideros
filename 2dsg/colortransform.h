#ifndef COLORTRANSFORM_H
#define COLORTRANSFORM_H

class ColorTransform
{
public:
	ColorTransform(	float redMultiplier = 1,
					float greenMultiplier = 1,
					float blueMultiplier = 1,
					float alphaMultiplier = 1,
					float redOffset = 0,
					float greenOffset = 0,
					float blueOffset = 0,
					float alphaOffset = 0);

	float redMultiplier() const
	{
		return redMultiplier_;
	}

	float greenMultiplier() const
	{
		return greenMultiplier_;
	}

	float blueMultiplier() const
	{
		return blueMultiplier_;
	}

	float alphaMultiplier() const
	{
		return alphaMultiplier_;
	}

	float redOffset() const
	{
		return redOffset_;
	}

	float greenOffset() const
	{
		return greenOffset_;
	}

	float blueOffset() const
	{
		return blueOffset_;
	}

	float alphaOffset() const
	{
		return alphaOffset_;
	}

	void setRedMultiplier(float redMultiplier)
	{
		redMultiplier_ = redMultiplier;
	}

	void setGreenMultiplier(float greenMultiplier)
	{
		greenMultiplier_ = greenMultiplier;
	}

	void setBlueMultiplier(float blueMultiplier)
	{
		blueMultiplier_ = blueMultiplier;
	}

	void setAlphaMultiplier(float alphaMultiplier)
	{
		alphaMultiplier_ = alphaMultiplier;
	}

private:
	float redMultiplier_;
	float greenMultiplier_;
	float blueMultiplier_;
	float alphaMultiplier_;
	float redOffset_;
	float greenOffset_;
	float blueOffset_;
	float alphaOffset_;
};

#endif
