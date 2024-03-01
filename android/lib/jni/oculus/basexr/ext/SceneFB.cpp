/*
 * SceneFB.cpp
 *
 *  Created on: 13 oct. 2023
 *      Author: User
 */

#include "SceneFB.h"
#include "common.h"
#include "openxr_program.h"
#include <openxr-oculus/fb_scene.h>
#include "luautil.h"

SceneFB *SceneFB::instance=NULL;

void SceneFB::SessionStart(XrInstance m_instance,XrSession m_session,XrSystemId m_systemId)
{
	XrResult result;
	Log::Write(Log::Level::Warning,"Scene EXT Init\n");

	result = xrGetInstanceProcAddr(m_instance, "xrQuerySpacesFB", (PFN_xrVoidFunction*)&pfnXrQuerySpacesFBX);
	if (XR_FAILED(result))
		Log::Write(Log::Level::Warning,"Failed to obtain the function pointer for xrQuerySpacesFB.\n");
	result = xrGetInstanceProcAddr(m_instance, "xrRetrieveSpaceQueryResultsFB", (PFN_xrVoidFunction*)&pfnXrRetrieveSpaceQueryResultsFBX);
	if (XR_FAILED(result))
		Log::Write(Log::Level::Warning,"Failed to obtain the function pointer for xrRetrieveSpaceQueryResultsFB.\n");
	result = xrGetInstanceProcAddr(m_instance, "xrRequestSceneCaptureFB", (PFN_xrVoidFunction*)&pfnXrRequestSceneCaptureFBX);
	if (XR_FAILED(result))
		Log::Write(Log::Level::Warning,"Failed to obtain the function pointer for xrRequestSceneCaptureFB.\n");
	result = xrGetInstanceProcAddr(m_instance, "xrGetSpaceRoomLayoutFB", (PFN_xrVoidFunction*)&pfnXrGetSpaceRoomLayoutFBX);
	if (XR_FAILED(result))
		Log::Write(Log::Level::Warning,"Failed to obtain the function pointer for xrGetSpaceRoomLayoutFB.\n");
	result = xrGetInstanceProcAddr(m_instance, "xrGetSpaceContainerFB", (PFN_xrVoidFunction*)&pfnXrGetSpaceContainerFBX);
	if (XR_FAILED(result))
		Log::Write(Log::Level::Warning,"Failed to obtain the function pointer for xrGetSpaceContainerFB.\n");
	result = xrGetInstanceProcAddr(m_instance, "xrGetSpaceComponentStatusFB", (PFN_xrVoidFunction*)&pfnXrGetSpaceComponentStatusFBX);
	if (XR_FAILED(result))
		Log::Write(Log::Level::Warning,"Failed to obtain the function pointer for xrGetSpaceComponentStatusFB.\n");
	result = xrGetInstanceProcAddr(m_instance, "xrGetSpaceSemanticLabelsFB", (PFN_xrVoidFunction*)&pfnXrGetSpaceSemanticLabelsFBX);
	if (XR_FAILED(result))
		Log::Write(Log::Level::Warning,"Failed to obtain the function pointer for xrGetSpaceSemanticLabelsFB.\n");
	result = xrGetInstanceProcAddr(m_instance, "xrGetSpaceBoundary2DFB", (PFN_xrVoidFunction*)&pfnXrGetSpaceBoundary2DFBX);
	if (XR_FAILED(result))
		Log::Write(Log::Level::Warning,"Failed to obtain the function pointer for xrGetSpaceBoundary2DFB.\n");
	result = xrGetInstanceProcAddr(m_instance, "xrGetSpaceBoundingBox2DFB", (PFN_xrVoidFunction*)&pfnXrGetSpaceBoundingBox2DFBX);
	if (XR_FAILED(result))
		Log::Write(Log::Level::Warning,"Failed to obtain the function pointer for xrGetSpaceBoundingBox2DFB.\n");
	result = xrGetInstanceProcAddr(m_instance, "xrGetSpaceBoundingBox3DFB", (PFN_xrVoidFunction*)&pfnXrGetSpaceBoundingBox3DFBX);
	if (XR_FAILED(result))
		Log::Write(Log::Level::Warning,"Failed to obtain the function pointer for xrGetSpaceBoundingBox3DFB.\n");
	result = xrGetInstanceProcAddr(m_instance, "xrSetSpaceComponentStatusFB", (PFN_xrVoidFunction*)&pfnXrSetSpaceComponentStatusFBX);
	if (XR_FAILED(result))
		Log::Write(Log::Level::Warning,"Failed to obtain the function pointer for xrSetSpaceComponentStatusFB.\n");



	session=m_session;
	instance=this;
	luaCb=0;
}

void SceneFB::SessionEnd()
{

}

void SceneFB::StartFrame(XrTime time, XrSpace viewSpace)
{
	XrResult res = XR_SUCCESS;
	for (auto &it : spaceLocations) {
		XrSpaceLocation& sloc=it.second;
		res=xrLocateSpace(it.first,viewSpace,time,&sloc);
		if (XR_FAILED(res)) {
			sloc.locationFlags=0;
		}
	}
}

static void pushVector(lua_State *L,XrVector3f v) {
	lua_newtable(L);
	lua_pushnumber(L,v.x); lua_rawseti(L,-2,1);
	lua_pushnumber(L,v.y); lua_rawseti(L,-2,2);
	lua_pushnumber(L,v.z); lua_rawseti(L,-2,3);
}

static void pushVector4(lua_State *L,XrQuaternionf v) {
	lua_newtable(L);
	lua_pushnumber(L,v.x); lua_rawseti(L,-2,1);
	lua_pushnumber(L,v.y); lua_rawseti(L,-2,2);
	lua_pushnumber(L,v.z); lua_rawseti(L,-2,3);
	lua_pushnumber(L,v.w); lua_rawseti(L,-2,4);
}

bool SceneFB::HandleEvent(const XrEventDataBaseHeader* event)
{
	switch (event->type) {
	case XR_TYPE_EVENT_DATA_SPACE_QUERY_RESULTS_AVAILABLE_FB:
	{
		XrEventDataSpaceQueryResultsAvailableFB *e=(XrEventDataSpaceQueryResultsAvailableFB*)event;
		if (e->requestId==requestId) {
			XrSpaceQueryResultsFB queryRes = {
			    XR_TYPE_SPACE_QUERY_RESULTS_FB,
			    nullptr,
			    0,
			    0,
			    nullptr};
			pfnXrRetrieveSpaceQueryResultsFBX(session, requestId, &queryRes);
			queryRes.results=new XrSpaceQueryResultFB[queryRes.resultCountOutput];
			queryRes.resultCapacityInput=queryRes.resultCountOutput;
			pfnXrRetrieveSpaceQueryResultsFBX(session, requestId, &queryRes);
			for (size_t i=0;i<queryRes.resultCountOutput;i++)
				lSpaces.push_back(queryRes.results[i]);
			delete[] queryRes.results;
		}
		return true;
	}
	case XR_TYPE_EVENT_DATA_SPACE_QUERY_COMPLETE_FB:
	{
		XrEventDataSpaceQueryCompleteFB *e=(XrEventDataSpaceQueryCompleteFB *)event;
		if (e->requestId==requestId) {
			lua_State *L=luaL;
			lua_getref(L,luaCb);
			lua_unref(L,luaCb);
			lua_newtable(L);
			int k=1;
			for (auto it=lSpaces.begin();it!=lSpaces.end();it++) {
				lua_newtable(L);
				lua_pushlstring(L,(const char *) ((*it).uuid.data),XR_UUID_SIZE_EXT);
				lua_rawsetfield(L,-2,"uuid");
				lua_pushlightuserdata(L,(void *)((*it).space));
				lua_rawsetfield(L,-2,"space");

				XrSpaceComponentStatusFB cStatus={
						XR_TYPE_SPACE_COMPONENT_STATUS_FB,
						nullptr
				};
				pfnXrGetSpaceComponentStatusFBX((*it).space,XR_SPACE_COMPONENT_TYPE_SEMANTIC_LABELS_FB,&cStatus);
				if (cStatus.enabled) {
					bool ext=!supportedLabels.empty();
					XrSemanticLabelsSupportInfoFB supInfo={
							XR_TYPE_SEMANTIC_LABELS_SUPPORT_INFO_FB,
							nullptr,
							multiLabels?XR_SEMANTIC_LABELS_SUPPORT_MULTIPLE_SEMANTIC_LABELS_BIT_FB:0,
							supportedLabels.c_str()
					};
					XrSemanticLabelsFB semLabel={
							XR_TYPE_SEMANTIC_LABELS_FB,
							ext?(&supInfo):nullptr,
							0,0,nullptr
					};

					if (pfnXrGetSpaceSemanticLabelsFBX(session,(*it).space,&semLabel)==XR_SUCCESS)
					{
						semLabel.buffer=new char[semLabel.bufferCountOutput];
						semLabel.bufferCapacityInput=semLabel.bufferCountOutput;
						pfnXrGetSpaceSemanticLabelsFBX(session,(*it).space,&semLabel);
						lua_pushlstring(L,semLabel.buffer,semLabel.bufferCountOutput);
						lua_rawsetfield(L,-2,"label");
						delete[] semLabel.buffer;
					}

					pfnXrGetSpaceComponentStatusFBX((*it).space,XR_SPACE_COMPONENT_TYPE_BOUNDED_2D_FB,&cStatus);
					if (cStatus.enabled) {
						XrRect2Df bbox;
						if (pfnXrGetSpaceBoundingBox2DFBX(session,(*it).space,&bbox)==XR_SUCCESS) {
							lua_newtable(L);
							lua_pushnumber(L,bbox.offset.x); lua_rawsetfield(L,-2,"x");
							lua_pushnumber(L,bbox.offset.y); lua_rawsetfield(L,-2,"y");
							lua_pushnumber(L,bbox.extent.width); lua_rawsetfield(L,-2,"w");
							lua_pushnumber(L,bbox.extent.height); lua_rawsetfield(L,-2,"h");
							lua_rawsetfield(L,-2,"bbox2d");
						}
						XrBoundary2DFB shape= {
								XR_TYPE_BOUNDARY_2D_FB,
								nullptr,
								0,0,nullptr
						};
						if (pfnXrGetSpaceBoundary2DFBX(session,(*it).space,&shape)==XR_SUCCESS) {
							shape.vertices=new XrVector2f[shape.vertexCountOutput];
							shape.vertexCapacityInput=shape.vertexCountOutput;
							if (pfnXrGetSpaceBoundary2DFBX(session,(*it).space,&shape)==XR_SUCCESS) {
								lua_newtable(L);
								for (int k=0;k<shape.vertexCountOutput;k++) {
									lua_newtable(L);
									lua_pushnumber(L,shape.vertices[k].x); lua_rawseti(L,-2,1);
									lua_pushnumber(L,shape.vertices[k].y); lua_rawseti(L,-2,2);
									lua_rawseti(L,-2,k+1);
								}
								lua_rawsetfield(L,-2,"shape2d");
							}
							delete[] shape.vertices;
						}
					}
					pfnXrGetSpaceComponentStatusFBX((*it).space,XR_SPACE_COMPONENT_TYPE_BOUNDED_3D_FB,&cStatus);
					if (cStatus.enabled) {
						XrRect3DfFB bbox;
						if (pfnXrGetSpaceBoundingBox3DFBX(session,(*it).space,&bbox)==XR_SUCCESS) {
							lua_newtable(L);
							lua_pushnumber(L,bbox.offset.x); lua_rawsetfield(L,-2,"x");
							lua_pushnumber(L,bbox.offset.y); lua_rawsetfield(L,-2,"y");
							lua_pushnumber(L,bbox.offset.z); lua_rawsetfield(L,-2,"z");
							lua_pushnumber(L,bbox.extent.width); lua_rawsetfield(L,-2,"w");
							lua_pushnumber(L,bbox.extent.height); lua_rawsetfield(L,-2,"h");
							lua_pushnumber(L,bbox.extent.depth); lua_rawsetfield(L,-2,"d");
							lua_rawsetfield(L,-2,"bbox3d");
						}
					}
				}

				lua_rawseti(L,-2,k++);
			}
			luaCb=0;
			//lua_call(L,1,0);
		    if (lua_pcall_traceback(L, 1, 0, 0))
			{
				Log::Write(Log::Level::Error,lua_tostring(L, -1));
		        lua_pop(L, 1);
			}
		}
		return true;
	}
	case XR_TYPE_EVENT_DATA_SCENE_CAPTURE_COMPLETE_FB:
	{
		XrEventDataSceneCaptureCompleteFB *e=(XrEventDataSceneCaptureCompleteFB *)event;
		if (e->requestId==requestId) {
			lua_State *L=luaL;
			lua_getref(L,luaCb);
			lua_unref(L,luaCb);
			lua_pushinteger(L,e->result);
			luaCb=0;
			lua_call(L,1,0);
		}
		return true;
	}
	default: return false;
	}
}

int SceneFB::getScene(lua_State *L) {
	luaL=L;
	if (luaCb)
	{
		lua_pushfstring(L,"XR: FB Scene operation already in progress");
		lua_error(L);
	}
	luaL_checktype(L,1,LUA_TFUNCTION);
	luaCb=lua_ref(L,1);
	uint32_t mret=luaL_optinteger(L,2,1);
	XrSpaceStorageLocationFilterInfoFB storageLocationFilterInfo = {
	    XR_TYPE_SPACE_STORAGE_LOCATION_FILTER_INFO_FB,
	    nullptr,
	    XR_SPACE_STORAGE_LOCATION_LOCAL_FB};

	XrSpaceComponentFilterInfoFB componentFilterInfo = {
	    XR_TYPE_SPACE_COMPONENT_FILTER_INFO_FB,
	    &storageLocationFilterInfo,
	    XR_SPACE_COMPONENT_TYPE_ROOM_LAYOUT_FB};

	XrSpaceQueryInfoFB queryInfo = {
	    XR_TYPE_SPACE_QUERY_INFO_FB,
	    nullptr,
	    XR_SPACE_QUERY_ACTION_LOAD_FB,
	    mret, //MAX results
		XR_INFINITE_DURATION ,
	    (XrSpaceFilterInfoBaseHeaderFB*)&componentFilterInfo,
	    nullptr};
	lSpaces.clear();

	reqType=0;
	pfnXrQuerySpacesFBX(session, (XrSpaceQueryInfoBaseHeaderFB*)&queryInfo, &requestId);

	return 0;
}

int SceneFB::captureScene(lua_State *L) {
	luaL=L;
	if (luaCb)
	{
		lua_pushfstring(L,"XR: FB Scene operation already in progress");
		lua_error(L);
	}
	luaL_checktype(L,1,LUA_TFUNCTION);
	size_t sl;
	const char *s=luaL_checklstring(L,2,&sl);
	luaCb=lua_ref(L,1);

	XrSceneCaptureRequestInfoFB req={
			XR_TYPE_SCENE_CAPTURE_REQUEST_INFO_FB,
		    nullptr,
		    (uint32_t)sl,s
	};

	reqType=0;
	pfnXrRequestSceneCaptureFBX(session, &req, &requestId);

	return 0;
}

static bool ValidUuid(XrUuidEXT u) {
	for (int k=0;k<16;k++)
		if (u.data[k]) return true;
	return false;
}

int SceneFB::getSceneComponents(lua_State *L) {
	luaL=L;
	if (luaCb)
	{
		lua_pushfstring(L,"XR: FB Scene operation already in progress");
		lua_error(L);
	}
	luaL_checktype(L,1,LUA_TFUNCTION);
	luaL_checktype(L,2,LUA_TLIGHTUSERDATA);
	luaCb=lua_ref(L,1);
	XrSpace space=(XrSpace) lua_tolightuserdata(L,2);

	XrRoomLayoutFB roomLayout={
			XR_TYPE_ROOM_LAYOUT_FB,
			nullptr,
			{0,},{0,},
			0,0,nullptr
	};
	pfnXrGetSpaceRoomLayoutFBX(session,space,&roomLayout);

	wallUuids.resize(roomLayout.wallUuidCountOutput);
	roomLayout.wallUuidCapacityInput = wallUuids.size();
	roomLayout.wallUuids = wallUuids.data();
	pfnXrGetSpaceRoomLayoutFBX(session,space,&roomLayout);

	std::vector<XrUuidEXT> uuids;
	for (auto it=wallUuids.begin();it!=wallUuids.end();it++)
		uuids.push_back((*it));
	if (ValidUuid(roomLayout.ceilingUuid))
		uuids.push_back(roomLayout.ceilingUuid);
	if (ValidUuid(roomLayout.floorUuid))
		uuids.push_back(roomLayout.floorUuid);
	floorUuid=roomLayout.floorUuid;
	ceilingUuid=roomLayout.ceilingUuid;

	XrSpaceContainerFB spaceContainer = {
			XR_TYPE_SPACE_CONTAINER_FB,
			nullptr,
			0,0,nullptr
	};
	// First call
	pfnXrGetSpaceContainerFBX(session, space, &spaceContainer);
	// Second call
	extraUuids.resize(spaceContainer.uuidCountOutput);
	spaceContainer.uuidCapacityInput = extraUuids.size();
	spaceContainer.uuids = extraUuids.data();
	pfnXrGetSpaceContainerFBX(session, space, &spaceContainer);

	for (uint32_t i = 0; i < spaceContainer.uuidCountOutput; i++) {
	    uuids.push_back(spaceContainer.uuids[i]);
	}

	XrSpaceStorageLocationFilterInfoFB storageLocationFilterInfo = {
	    XR_TYPE_SPACE_STORAGE_LOCATION_FILTER_INFO_FB,
	    nullptr,
	    XR_SPACE_STORAGE_LOCATION_LOCAL_FB};

	XrSpaceUuidFilterInfoFB uuidFilterInfo = {
	    XR_TYPE_SPACE_UUID_FILTER_INFO_FB,
	    &storageLocationFilterInfo,
	    (uint32_t)uuids.size(),
		uuids.data()};

	XrSpaceQueryInfoFB queryInfo = {
	    XR_TYPE_SPACE_QUERY_INFO_FB,
	    nullptr,
	    XR_SPACE_QUERY_ACTION_LOAD_FB,
		(uint32_t)(uuids.size()),
	    0,
	    (XrSpaceFilterInfoBaseHeaderFB*)&uuidFilterInfo,
	    nullptr};

	lSpaces.clear();
	reqType=1;
	pfnXrQuerySpacesFBX(session, (XrSpaceQueryInfoBaseHeaderFB*)&queryInfo, &requestId);
	return 0;
}

int SceneFB::setLabelsSupport(lua_State *L) {
	supportedLabels=luaL_checkstring(L,2);
	multiLabels=lua_toboolean(L,1);
	return 0;
}

int SceneFB::getComponentPose(lua_State *L) {
	luaL_checktype(L,1,LUA_TLIGHTUSERDATA);
	XrSpace space=(XrSpace) lua_tolightuserdata(L,1);
	if (spaceLocations.count(space)==0) {
		return 0;
	}
	XrSpaceLocation& sloc=spaceLocations[space];
	lua_newtable(L);
	lua_pushinteger(L,sloc.locationFlags);
	lua_setfield(L, -2,	"poseStatus");
	pushVector(L,sloc.pose.position);
	lua_setfield(L, -2, "position");
	pushVector4(L,sloc.pose.orientation);
	lua_setfield(L, -2, "rotation");
	return 1;
}

int SceneFB::trackComponentPose(lua_State *L) {
	luaL_checktype(L,1,LUA_TLIGHTUSERDATA);
	bool en=lua_toboolean(L,2);
	XrSpace space=(XrSpace) lua_tolightuserdata(L,1);
	auto it=spaceLocations.find(space);
	if (en) {
		if (it==spaceLocations.end()) {
			XrSpaceComponentStatusFB cStatus={
					XR_TYPE_SPACE_COMPONENT_STATUS_FB,
					nullptr
			};
			pfnXrGetSpaceComponentStatusFBX(space,XR_SPACE_COMPONENT_TYPE_LOCATABLE_FB,&cStatus);
			if (!cStatus.enabled) {
                XrSpaceComponentStatusSetInfoFB request = {
                     XR_TYPE_SPACE_COMPONENT_STATUS_SET_INFO_FB,
                     nullptr,
                     XR_SPACE_COMPONENT_TYPE_LOCATABLE_FB,
                     true,
                     0};
                 XrAsyncRequestIdFB requestId;
                 pfnXrSetSpaceComponentStatusFBX(space, &request, &requestId);
			}
			spaceLocations[space]={
				XR_TYPE_SPACE_LOCATION,
				nullptr,
				0, {{0, 0, 0, 1}, {0, 0, 0}}
			};
		}
	}
	else {
		if (it!=spaceLocations.end())
			spaceLocations.erase(it);
	}
	return 0;
}

static int _getScene(lua_State *L) {
	return SceneFB::instance->getScene(L);
}
static int _captureScene(lua_State *L) {
	return SceneFB::instance->captureScene(L);
}
static int _getSceneComponents(lua_State *L) {
	return SceneFB::instance->getSceneComponents(L);
}
static int _setLabelsSupport(lua_State *L) {
	return SceneFB::instance->setLabelsSupport(L);
}
static int _getComponentPose(lua_State *L) {
	return SceneFB::instance->getComponentPose(L);
}
static int _trackComponent(lua_State *L) {
	return SceneFB::instance->trackComponentPose(L);
}

void SceneFB::SetupAPI(lua_State *L)
{
	Log::Write(Log::Level::Warning,"Scene EXT API\n");
    static const luaL_Reg functionList[] = {
    	{"getScene", _getScene},
		{"captureScene", _captureScene},
    	{"getComponents", _getSceneComponents},
    	{"setLabelsSupport", _setLabelsSupport},
    	{"getComponentPose", _getComponentPose},
		{"trackComponentPose",_trackComponent},
        {NULL, NULL},
    };
    lua_getglobal(L, "Oculus");
    lua_newtable(L);
    luaL_register(L, NULL, functionList);;
    lua_setfield(L,-2,"FBScene");
    lua_pop(L,1);
}

