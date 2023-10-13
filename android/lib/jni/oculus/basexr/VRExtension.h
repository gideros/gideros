/*
 * VRExtension.h
 *
 *  Created on: 13 oct. 2023
 *      Author: User
 */

#ifndef ANDROID_LIB_JNI_OCULUS_BASEXR_VREXTENSION_H_
#define ANDROID_LIB_JNI_OCULUS_BASEXR_VREXTENSION_H_
#include "options.h"
#include "openxr_program.h"
#include <vector>

class VRExtension {
public:
	virtual ~VRExtension() {};
	virtual void SessionStart(XrInstance m_instance,XrSession m_session,XrSystemId m_systemId) {};
	virtual void SessionEnd() {};
	virtual void Composition(std::vector<XrCompositionLayerBaseHeader*> &layers) {};
};

#endif /* ANDROID_LIB_JNI_OCULUS_BASEXR_VREXTENSION_H_ */
