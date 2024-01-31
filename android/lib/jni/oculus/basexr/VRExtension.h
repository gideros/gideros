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
#include "lua.hpp"


class VRExtension {
public:
	virtual ~VRExtension() {};
	virtual void Setup(std::shared_ptr<IOpenXrProgram> program) {};
	virtual void SessionStart(XrInstance m_instance,XrSession m_session,XrSystemId m_systemId) {};
	virtual void SessionEnd() {};
	virtual void Composition(std::vector<XrCompositionLayerBaseHeader*> &layers) {};
	virtual void SetupAPI(lua_State *L) {};
	virtual bool HandleEvent(const XrEventDataBaseHeader* event) { return false; };
	virtual void StartFrame(XrTime time, XrSpace viewSpace) {};
};

#endif /* ANDROID_LIB_JNI_OCULUS_BASEXR_VREXTENSION_H_ */
