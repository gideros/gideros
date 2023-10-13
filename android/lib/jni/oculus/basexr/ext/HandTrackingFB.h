/*
 * HandTrackingFB.h
 *
 *  Created on: 13 oct. 2023
 *      Author: User
 */

#ifndef ANDROID_LIB_JNI_OCULUS_BASEXR_EXT_HANDTRACKINGFB_H_
#define ANDROID_LIB_JNI_OCULUS_BASEXR_EXT_HANDTRACKINGFB_H_
#include "VRExtension.h"
class HandTrackingFB: public VRExtension {
public:
	virtual void SessionStart(XrInstance m_instance,XrSession m_session,XrSystemId m_systemId);
	virtual void SessionEnd();
protected:
    PFN_xrCreateHandTrackerEXT xrCreateHandTrackerEXT_ = nullptr;
     PFN_xrDestroyHandTrackerEXT xrDestroyHandTrackerEXT_ = nullptr;
     PFN_xrLocateHandJointsEXT xrLocateHandJointsEXT_ = nullptr;
     /// Hands - FB mesh rendering extensions
     PFN_xrGetHandMeshFB xrGetHandMeshFB_ = nullptr;
};

#endif /* ANDROID_LIB_JNI_OCULUS_BASEXR_EXT_HANDTRACKINGFB_H_ */
