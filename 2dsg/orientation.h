#ifndef ORIENTATION_H
#define ORIENTATION_H

enum Orientation
{
	ePortrait,					// The device is in portrait mode, with the device held upright and the home button on the bottom.
	eLandscapeLeft,				// The device is in landscape mode, with the device held upright and the home button on the right side.
	ePortraitUpsideDown,		// The device is in portrait mode but upside down, with the device held upright and the home button at the top.
	eLandscapeRight,			// The device is in landscape mode, with the device held upright and the home button on the left side.
	eFixed,						// The device cannot rotate
};

enum LogicalScaleMode
{
	eNoScale,
	eCenter,
	ePixelPerfect,
	eLetterBox,
	eCrop,
	eStretch,
	eFitWidth,
	eFitHeight,
};

#endif
