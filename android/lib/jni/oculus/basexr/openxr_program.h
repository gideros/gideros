// Copyright (c) 2017-2023, The Khronos Group Inc.
//
// SPDX-License-Identifier: Apache-2.0

#pragma once
#include "graphicsplugin.h"
#include "options.h"
#include <array>
#include <map>
#include <string>
#include <vector>

class VRExtension;

namespace Side {
const int LEFT = 0;
const int RIGHT = 1;
const int COUNT = 2;
};  // namespace Side

struct IOpenXrProgram {
    struct InputState {
        XrActionSet actionSet{XR_NULL_HANDLE};
        XrAction grabAction{XR_NULL_HANDLE};
        XrAction indexAction{XR_NULL_HANDLE};
        XrAction poseAction{XR_NULL_HANDLE};
        XrAction vibrateAction{XR_NULL_HANDLE};
        XrAction quitAction{XR_NULL_HANDLE};
        XrAction btnAAction{XR_NULL_HANDLE};
        XrAction btnBAction{XR_NULL_HANDLE};
        XrAction btnXAction{XR_NULL_HANDLE};
        XrAction btnYAction{XR_NULL_HANDLE};
        XrAction stickAction{XR_NULL_HANDLE};
        std::array<XrPath, Side::COUNT> handSubactionPath;
        std::array<XrSpace, Side::COUNT> handSpace;
        std::array<XrVector2f, Side::COUNT> handStick = {{{0.0f,0.0f}, {0.0f,0.0f}}};
        std::array<int, Side::COUNT> handButtons = {{0,0}};
        std::array<float, Side::COUNT> handScale = {{1.0f, 1.0f}};
        std::array<float, Side::COUNT> handGrip = {{0.0f, 0.0f}};
        std::array<float, Side::COUNT> handIndex = {{0.0f, 0.0f}};
        std::array<XrBool32, Side::COUNT> handActive;
    };


    virtual ~IOpenXrProgram() = default;
    virtual std::map<std::string,bool> &ProbeExtensions() = 0;
    virtual std::map<std::string,bool> &EnabledExtensions() = 0;
    virtual void SetViewSpace(std::string s) =0;
    virtual XrSpace GetSceneSpace()=0;
    virtual XrTime GetTime()=0;

    // Create an Instance and other basic instance-level initialization.
    virtual void CreateInstance(std::vector<std::string> &extra) = 0;

    // Select a System for the view configuration specified in the Options
    virtual void InitializeSystem() = 0;

    // Initialize the graphics device for the selected system.
    virtual void InitializeDevice() = 0;

    // Create a Session and other basic session-level initialization.
    virtual void InitializeSession() = 0;

    // Create a Swapchain which requires coordinating with the graphics plugin to select the format, getting the system graphics
    // properties, getting the view configuration and grabbing the resulting swapchain images.
    virtual void CreateSwapchains() = 0;

    // Process any events in the event queue.
    virtual void PollEvents(bool* exitRenderLoop, bool* requestRestart) = 0;

    // Manage session lifecycle to track if RenderFrame should be called.
    virtual bool IsSessionRunning() const = 0;

    // Manage session state to track if input should be processed.
    virtual bool IsSessionFocused() const = 0;

    // Sample input actions and generate haptic feedback.
    virtual void PollActions() = 0;

    // Create and submit a frame.
    virtual void RenderFrame() = 0;

    // Get preferred blend mode based on the view configuration specified in the Options
    virtual XrEnvironmentBlendMode GetPreferredBlendMode() const = 0;

    std::vector<VRExtension *> VRExts;
    void AddExtension(VRExtension *e) { VRExts.push_back(e); };

    void (*StartOfFrame)();
    void (*HandleInput)(IOpenXrProgram::InputState *m_input,XrSpace m_appSpace,XrTime predictedDisplayTime,XrSpaceLocation *head,XrSpaceVelocity *headSpd);
};

struct Swapchain {
    XrSwapchain handle;
    int32_t width;
    int32_t height;
};

std::shared_ptr<IOpenXrProgram> CreateOpenXrProgram(const std::shared_ptr<Options>& options,
                                                    const std::shared_ptr<IPlatformPlugin>& platformPlugin,
                                                    const std::shared_ptr<IGraphicsPlugin>& graphicsPlugin);
