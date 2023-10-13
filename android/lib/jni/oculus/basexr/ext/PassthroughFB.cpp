/*
 * PassthroughFB.cpp
 *
 *  Created on: 13 oct. 2023
 *      Author: User
 */

#include "PassthroughFB.h"
#include "common.h"
#include "openxr_program.h"

void PassthroughFB::SessionStart(XrInstance m_instance,XrSession m_session,XrSystemId m_systemId)
{
	XrResult result;

	PFN_xrCreatePassthroughFB pfnXrCreatePassthroughFBX = nullptr;
	result = xrGetInstanceProcAddr(m_instance, "xrCreatePassthroughFB", (PFN_xrVoidFunction*)(&pfnXrCreatePassthroughFBX));
	if (XR_FAILED(result)) {
		Log::Write(Log::Level::Warning,"Failed to obtain the function pointer for xrCreatePassthroughFB.\n");
	}
	PFN_xrCreatePassthroughLayerFB pfnXrCreatePassthroughLayerFBX = nullptr;
	result = xrGetInstanceProcAddr(m_instance, "xrCreatePassthroughLayerFB", (PFN_xrVoidFunction*)(&pfnXrCreatePassthroughLayerFBX));
	if (XR_FAILED(result)) {
		Log::Write(Log::Level::Warning,"Failed to obtain the function pointer for XrCreatePassthroughLayerFBX.\n");
	}

	// Create the feature
	XrPassthroughFB passthroughFeature = XR_NULL_HANDLE;

	XrPassthroughCreateInfoFB passthroughCreateInfo = {XR_TYPE_PASSTHROUGH_CREATE_INFO_FB};
	passthroughCreateInfo.flags = XR_PASSTHROUGH_IS_RUNNING_AT_CREATION_BIT_FB;

	result = pfnXrCreatePassthroughFBX(m_session, &passthroughCreateInfo, &passthroughFeature);
	if (XR_FAILED(result)) {
		Log::Write(Log::Level::Warning, "Failed to create the passthrough feature.\n");
	}

	XrPassthroughLayerCreateInfoFB layerCreateInfo = {XR_TYPE_PASSTHROUGH_LAYER_CREATE_INFO_FB};
	layerCreateInfo.passthrough = passthroughFeature;
	layerCreateInfo.purpose = XR_PASSTHROUGH_LAYER_PURPOSE_RECONSTRUCTION_FB;
	layerCreateInfo.flags = XR_PASSTHROUGH_IS_RUNNING_AT_CREATION_BIT_FB;
	//layerCreateInfo.flags|= XR_PASSTHROUGH_LAYER_DEPTH_BIT_FB;

	result = pfnXrCreatePassthroughLayerFBX(m_session, &layerCreateInfo, &passthroughLayer);
	if (XR_FAILED(result)) {
		Log::Write(Log::Level::Warning, "Failed to create and start a passthrough layer");
	}
}

void PassthroughFB::Composition(std::vector<XrCompositionLayerBaseHeader*> &layers)
{
	passthroughCompLayer.layerHandle = passthroughLayer;
	passthroughCompLayer.flags = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT;
	passthroughCompLayer.space = XR_NULL_HANDLE;
	if (passthroughLayer) {
		layers.insert(layers.begin(),reinterpret_cast<XrCompositionLayerBaseHeader*>(&passthroughCompLayer));
	}
}

void PassthroughFB::SessionEnd()
{

}

