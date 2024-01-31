/*
 * PassthroughFB.h
 *
 *  Created on: 13 oct. 2023
 *      Author: User
 */

#ifndef ANDROID_LIB_JNI_OCULUS_BASEXR_EXT_SCENEFB_H_
#define ANDROID_LIB_JNI_OCULUS_BASEXR_EXT_SCENEFB_H_
#include "VRExtension.h"
#include <openxr-oculus/fb_scene.h>
#include <openxr-oculus/fb_scene_capture.h>
class SceneFB: public VRExtension {
	virtual void StartFrame(XrTime time, XrSpace viewSpace);
	virtual void SessionStart(XrInstance m_instance,XrSession m_session,XrSystemId m_systemId);
	virtual void SessionEnd();
	virtual void SetupAPI(lua_State *L);
	virtual bool HandleEvent(const XrEventDataBaseHeader* event);
public:
	int getScene(lua_State *L);
	int captureScene(lua_State *L);
	int getSceneComponents(lua_State *L);
	int setLabelsSupport(lua_State *L);
	int getComponentPose(lua_State *L);
	int trackComponentPose(lua_State *L);
	static SceneFB *instance;
private:
	XrSession session;
	PFN_xrQuerySpacesFB pfnXrQuerySpacesFBX;
	PFN_xrRetrieveSpaceQueryResultsFB pfnXrRetrieveSpaceQueryResultsFBX;
	PFN_xrRequestSceneCaptureFB pfnXrRequestSceneCaptureFBX;
	PFN_xrGetSpaceRoomLayoutFB pfnXrGetSpaceRoomLayoutFBX;
	PFN_xrGetSpaceContainerFB pfnXrGetSpaceContainerFBX;
	PFN_xrGetSpaceComponentStatusFB pfnXrGetSpaceComponentStatusFBX;
	PFN_xrGetSpaceSemanticLabelsFB pfnXrGetSpaceSemanticLabelsFBX;
	PFN_xrGetSpaceBoundary2DFB pfnXrGetSpaceBoundary2DFBX;
	PFN_xrGetSpaceBoundingBox2DFB pfnXrGetSpaceBoundingBox2DFBX;
	PFN_xrGetSpaceBoundingBox3DFB pfnXrGetSpaceBoundingBox3DFBX;
	PFN_xrSetSpaceComponentStatusFB pfnXrSetSpaceComponentStatusFBX;
	//
	lua_State *luaL;
	int luaCb;
	XrAsyncRequestIdFB requestId;
	std::vector<XrSpaceQueryResultFB> lSpaces;
	int reqType;
	//
	std::vector<XrUuidEXT> wallUuids;
	XrUuidEXT floorUuid;
	XrUuidEXT ceilingUuid;
	std::vector<XrUuidEXT> extraUuids;
	//
	bool multiLabels;
	std::string supportedLabels;
	//
	std::map<XrSpace,XrSpaceLocation> spaceLocations;
};

#endif /* ANDROID_LIB_JNI_OCULUS_BASEXR_EXT_SCENEFB_H_ */
