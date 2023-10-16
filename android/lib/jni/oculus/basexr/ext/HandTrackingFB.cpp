/*
 * HandTrackingFB.cpp
 *
 *  Created on: 13 oct. 2023
 *      Author: User
 */

#include "HandTrackingFB.h"

void HandTrackingFB::SessionStart(XrInstance m_instance,XrSession m_session,XrSystemId m_systemId)
{
	// Inspect hand tracking system properties
	 XrSystemHandTrackingPropertiesEXT handTrackingSystemProperties{
		 XR_TYPE_SYSTEM_HAND_TRACKING_PROPERTIES_EXT};
	 XrSystemProperties systemProperties{
		 XR_TYPE_SYSTEM_PROPERTIES, &handTrackingSystemProperties};
	 CHECK_XRCMD(xrGetSystemProperties(m_instance, m_systemId, &systemProperties));
	 if (!handTrackingSystemProperties.supportsHandTracking) {
		 // The system does not support hand tracking
		 Log::Write(Log::Level::Warning,"xrGetSystemProperties XR_TYPE_SYSTEM_HAND_TRACKING_PROPERTIES_EXT FAILED.");
	 } else {
		 Log::Write(Log::Level::Info,
			 "xrGetSystemProperties XR_TYPE_SYSTEM_HAND_TRACKING_PROPERTIES_EXT OK - initiallizing hand tracking...");
	 }

	 /// Hook up extensions for hand tracking
	 CHECK_XRCMD(xrGetInstanceProcAddr(
			 m_instance,
		 "xrCreateHandTrackerEXT",
		 (PFN_xrVoidFunction*)(&xrCreateHandTrackerEXT_)));
	 CHECK_XRCMD(xrGetInstanceProcAddr(
			 m_instance,
		 "xrDestroyHandTrackerEXT",
		 (PFN_xrVoidFunction*)(&xrDestroyHandTrackerEXT_)));
	 CHECK_XRCMD(xrGetInstanceProcAddr(
			 m_instance,
		 "xrLocateHandJointsEXT",
		 (PFN_xrVoidFunction*)(&xrLocateHandJointsEXT_)));

	 /// Hook up extensions for hand rendering
	 CHECK_XRCMD(xrGetInstanceProcAddr(
			 m_instance, "xrGetHandMeshFB", (PFN_xrVoidFunction*)(&xrGetHandMeshFB_)));
}

void HandTrackingFB::SessionEnd()
{

}
