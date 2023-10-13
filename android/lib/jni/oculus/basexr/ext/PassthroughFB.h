/*
 * PassthroughFB.h
 *
 *  Created on: 13 oct. 2023
 *      Author: User
 */

#ifndef ANDROID_LIB_JNI_OCULUS_BASEXR_EXT_PASSTHROUGHFB_H_
#define ANDROID_LIB_JNI_OCULUS_BASEXR_EXT_PASSTHROUGHFB_H_
#include "VRExtension.h"

class PassthroughFB: public VRExtension {
	virtual void SessionStart(XrInstance m_instance,XrSession m_session,XrSystemId m_systemId);
	virtual void SessionEnd();
	virtual void Composition(std::vector<XrCompositionLayerBaseHeader*> &layers);
private:
	XrCompositionLayerPassthroughFB passthroughCompLayer = {XR_TYPE_COMPOSITION_LAYER_PASSTHROUGH_FB};
    XrPassthroughLayerFB passthroughLayer = XR_NULL_HANDLE;
};

#endif /* ANDROID_LIB_JNI_OCULUS_BASEXR_EXT_PASSTHROUGHFB_H_ */
