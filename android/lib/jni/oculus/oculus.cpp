/************************************************************************************

Filename    :   VrCubeWorld_SurfaceView.c
Content     :   This sample uses a plain Android SurfaceView and handles all
                Activity and Surface life cycle events in native code. This sample
                does not use the application framework.
                This sample only uses the VrApi.
Created     :   March, 2015
Authors     :   J.M.P. van Waveren

Copyright   :   Copyright (c) Facebook Technologies, LLC and its affiliates. All rights reserved.

*************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/prctl.h> // for prctl( PR_SET_NAME )
#include <android/log.h>
#include <android/native_window_jni.h> // for native window JNI
#include <android/input.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

#include "luaapplication.h"
#include "oculus.h"
#include "debugging.h"

void setupApi(lua_State *L);
static LuaApplication *_gapp=NULL;

#if !defined(EGL_OPENGL_ES3_BIT_KHR)
#define EGL_OPENGL_ES3_BIT_KHR 0x0040
#endif

// EXT_texture_border_clamp
#ifndef GL_CLAMP_TO_BORDER
#define GL_CLAMP_TO_BORDER 0x812D
#endif

#ifndef GL_TEXTURE_BORDER_COLOR
#define GL_TEXTURE_BORDER_COLOR 0x1004
#endif

#if !defined(GL_EXT_multisampled_render_to_texture)
typedef void(GL_APIENTRY* PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC)(
    GLenum target,
    GLsizei samples,
    GLenum internalformat,
    GLsizei width,
    GLsizei height);
typedef void(GL_APIENTRY* PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC)(
    GLenum target,
    GLenum attachment,
    GLenum textarget,
    GLuint texture,
    GLint level,
    GLsizei samples);
#endif

#if !defined(GL_OVR_multiview)
/// static const int GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_NUM_VIEWS_OVR       = 0x9630;
/// static const int GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_BASE_VIEW_INDEX_OVR = 0x9632;
/// static const int GL_MAX_VIEWS_OVR                                      = 0x9631;
typedef void(GL_APIENTRY* PFNGLFRAMEBUFFERTEXTUREMULTIVIEWOVRPROC)(
    GLenum target,
    GLenum attachment,
    GLuint texture,
    GLint level,
    GLint baseViewIndex,
    GLsizei numViews);
#endif

#if !defined(GL_OVR_multiview_multisampled_render_to_texture)
typedef void(GL_APIENTRY* PFNGLFRAMEBUFFERTEXTUREMULTISAMPLEMULTIVIEWOVRPROC)(
    GLenum target,
    GLenum attachment,
    GLuint texture,
    GLint level,
    GLsizei samples,
    GLint baseViewIndex,
    GLsizei numViews);
#endif

#include "Include/VrApi.h"
#include "Include/VrApi_Helpers.h"
#include "Include/VrApi_SystemUtils.h"
#include "Include/VrApi_Input.h"

#define DEBUG 1
#define OVR_LOG_TAG "VrGideros"

#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, OVR_LOG_TAG, __VA_ARGS__)
#if DEBUG
#define ALOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, OVR_LOG_TAG, __VA_ARGS__)
#else
#define ALOGV(...)
#endif

static const int CPU_LEVEL = 2;
static const int GPU_LEVEL = 3;
static const int NUM_MULTI_SAMPLES = 4;

#define MULTI_THREADED 0

/*
================================================================================

System Clock Time

================================================================================
*/

static double GetTimeInSeconds() {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (now.tv_sec * 1e9 + now.tv_nsec) * 0.000000001;
}

/*
================================================================================

OpenGL-ES Utility Functions

================================================================================
*/

typedef struct {
    bool multi_view; // GL_OVR_multiview, GL_OVR_multiview2
    bool EXT_texture_border_clamp; // GL_EXT_texture_border_clamp, GL_OES_texture_border_clamp
} OpenGLExtensions_t;

OpenGLExtensions_t glExtensions;

static void EglInitExtensions() {
    const char* allExtensions = (const char*)glGetString(GL_EXTENSIONS);
    if (allExtensions != NULL) {
        glExtensions.multi_view = strstr(allExtensions, "GL_OVR_multiview2") &&
            strstr(allExtensions, "GL_OVR_multiview_multisampled_render_to_texture");

        glExtensions.EXT_texture_border_clamp =
            strstr(allExtensions, "GL_EXT_texture_border_clamp") ||
            strstr(allExtensions, "GL_OES_texture_border_clamp");
    }
}

static const char* EglErrorString(const EGLint error) {
    switch (error) {
        case EGL_SUCCESS:
            return "EGL_SUCCESS";
        case EGL_NOT_INITIALIZED:
            return "EGL_NOT_INITIALIZED";
        case EGL_BAD_ACCESS:
            return "EGL_BAD_ACCESS";
        case EGL_BAD_ALLOC:
            return "EGL_BAD_ALLOC";
        case EGL_BAD_ATTRIBUTE:
            return "EGL_BAD_ATTRIBUTE";
        case EGL_BAD_CONTEXT:
            return "EGL_BAD_CONTEXT";
        case EGL_BAD_CONFIG:
            return "EGL_BAD_CONFIG";
        case EGL_BAD_CURRENT_SURFACE:
            return "EGL_BAD_CURRENT_SURFACE";
        case EGL_BAD_DISPLAY:
            return "EGL_BAD_DISPLAY";
        case EGL_BAD_SURFACE:
            return "EGL_BAD_SURFACE";
        case EGL_BAD_MATCH:
            return "EGL_BAD_MATCH";
        case EGL_BAD_PARAMETER:
            return "EGL_BAD_PARAMETER";
        case EGL_BAD_NATIVE_PIXMAP:
            return "EGL_BAD_NATIVE_PIXMAP";
        case EGL_BAD_NATIVE_WINDOW:
            return "EGL_BAD_NATIVE_WINDOW";
        case EGL_CONTEXT_LOST:
            return "EGL_CONTEXT_LOST";
        default:
            return "unknown";
    }
}

static const char* GlFrameBufferStatusString(GLenum status) {
    switch (status) {
        case GL_FRAMEBUFFER_UNDEFINED:
            return "GL_FRAMEBUFFER_UNDEFINED";
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            return "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            return "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
        case GL_FRAMEBUFFER_UNSUPPORTED:
            return "GL_FRAMEBUFFER_UNSUPPORTED";
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
            return "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
        default:
            return "unknown";
    }
}

#ifdef CHECK_GL_ERRORS

static const char* GlErrorString(GLenum error) {
    switch (error) {
        case GL_NO_ERROR:
            return "GL_NO_ERROR";
        case GL_INVALID_ENUM:
            return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE:
            return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION:
            return "GL_INVALID_OPERATION";
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            return "GL_INVALID_FRAMEBUFFER_OPERATION";
        case GL_OUT_OF_MEMORY:
            return "GL_OUT_OF_MEMORY";
        default:
            return "unknown";
    }
}

static void GLCheckErrors(int line) {
    for (int i = 0; i < 10; i++) {
        const GLenum error = glGetError();
        if (error == GL_NO_ERROR) {
            break;
        }
        ALOGE("GL error on line %d: %s", line, GlErrorString(error));
    }
}

#define GL(func) \
    func;        \
    GLCheckErrors(__LINE__);

#else // CHECK_GL_ERRORS

#define GL(func) func;

#endif // CHECK_GL_ERRORS

/*
================================================================================

ovrEgl

================================================================================
*/

typedef struct {
    EGLint MajorVersion;
    EGLint MinorVersion;
    EGLDisplay Display;
    EGLConfig Config;
    EGLSurface TinySurface;
    EGLSurface MainSurface;
    EGLContext Context;
} ovrEgl;

static void ovrEgl_Clear(ovrEgl* egl) {
    egl->MajorVersion = 0;
    egl->MinorVersion = 0;
    egl->Display = 0;
    egl->Config = 0;
    egl->TinySurface = EGL_NO_SURFACE;
    egl->MainSurface = EGL_NO_SURFACE;
    egl->Context = EGL_NO_CONTEXT;
}

static void ovrEgl_CreateContext(ovrEgl* egl, const ovrEgl* shareEgl) {
    if (egl->Display != 0) {
        return;
    }

    egl->Display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    ALOGV("        eglInitialize( Display, &MajorVersion, &MinorVersion )");
    eglInitialize(egl->Display, &egl->MajorVersion, &egl->MinorVersion);
    // Do NOT use eglChooseConfig, because the Android EGL code pushes in multisample
    // flags in eglChooseConfig if the user has selected the "force 4x MSAA" option in
    // settings, and that is completely wasted for our warp target.
    const int MAX_CONFIGS = 1024;
    EGLConfig configs[MAX_CONFIGS];
    EGLint numConfigs = 0;
    if (eglGetConfigs(egl->Display, configs, MAX_CONFIGS, &numConfigs) == EGL_FALSE) {
        ALOGE("        eglGetConfigs() failed: %s", EglErrorString(eglGetError()));
        return;
    }
    const EGLint configAttribs[] = {
        EGL_RED_SIZE,
        8,
        EGL_GREEN_SIZE,
        8,
        EGL_BLUE_SIZE,
        8,
        EGL_ALPHA_SIZE,
        8, // need alpha for the multi-pass timewarp compositor
        EGL_DEPTH_SIZE,
        0,
        EGL_STENCIL_SIZE,
        0,
        EGL_SAMPLES,
        0,
        EGL_NONE};
    egl->Config = 0;
    for (int i = 0; i < numConfigs; i++) {
        EGLint value = 0;

        eglGetConfigAttrib(egl->Display, configs[i], EGL_RENDERABLE_TYPE, &value);
        if ((value & EGL_OPENGL_ES3_BIT_KHR) != EGL_OPENGL_ES3_BIT_KHR) {
            continue;
        }

        // The pbuffer config also needs to be compatible with normal window rendering
        // so it can share textures with the window context.
        eglGetConfigAttrib(egl->Display, configs[i], EGL_SURFACE_TYPE, &value);
        if ((value & (EGL_WINDOW_BIT | EGL_PBUFFER_BIT)) != (EGL_WINDOW_BIT | EGL_PBUFFER_BIT)) {
            continue;
        }

        int j = 0;
        for (; configAttribs[j] != EGL_NONE; j += 2) {
            eglGetConfigAttrib(egl->Display, configs[i], configAttribs[j], &value);
            if (value != configAttribs[j + 1]) {
                break;
            }
        }
        if (configAttribs[j] == EGL_NONE) {
            egl->Config = configs[i];
            break;
        }
    }
    if (egl->Config == 0) {
        ALOGE("        eglChooseConfig() failed: %s", EglErrorString(eglGetError()));
        return;
    }
    EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
    ALOGV("        Context = eglCreateContext( Display, Config, EGL_NO_CONTEXT, contextAttribs )");
    egl->Context = eglCreateContext(
        egl->Display,
        egl->Config,
        (shareEgl != NULL) ? shareEgl->Context : EGL_NO_CONTEXT,
        contextAttribs);
    if (egl->Context == EGL_NO_CONTEXT) {
        ALOGE("        eglCreateContext() failed: %s", EglErrorString(eglGetError()));
        return;
    }
    const EGLint surfaceAttribs[] = {EGL_WIDTH, 16, EGL_HEIGHT, 16, EGL_NONE};
    ALOGV("        TinySurface = eglCreatePbufferSurface( Display, Config, surfaceAttribs )");
    egl->TinySurface = eglCreatePbufferSurface(egl->Display, egl->Config, surfaceAttribs);
    if (egl->TinySurface == EGL_NO_SURFACE) {
        ALOGE("        eglCreatePbufferSurface() failed: %s", EglErrorString(eglGetError()));
        eglDestroyContext(egl->Display, egl->Context);
        egl->Context = EGL_NO_CONTEXT;
        return;
    }
    ALOGV("        eglMakeCurrent( Display, TinySurface, TinySurface, Context )");
    if (eglMakeCurrent(egl->Display, egl->TinySurface, egl->TinySurface, egl->Context) ==
        EGL_FALSE) {
        ALOGE("        eglMakeCurrent() failed: %s", EglErrorString(eglGetError()));
        eglDestroySurface(egl->Display, egl->TinySurface);
        eglDestroyContext(egl->Display, egl->Context);
        egl->Context = EGL_NO_CONTEXT;
        return;
    }
}

static void ovrEgl_DestroyContext(ovrEgl* egl) {
    if (egl->Display != 0) {
        ALOGE("        eglMakeCurrent( Display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT )");
        if (eglMakeCurrent(egl->Display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) ==
            EGL_FALSE) {
            ALOGE("        eglMakeCurrent() failed: %s", EglErrorString(eglGetError()));
        }
    }
    if (egl->Context != EGL_NO_CONTEXT) {
        ALOGE("        eglDestroyContext( Display, Context )");
        if (eglDestroyContext(egl->Display, egl->Context) == EGL_FALSE) {
            ALOGE("        eglDestroyContext() failed: %s", EglErrorString(eglGetError()));
        }
        egl->Context = EGL_NO_CONTEXT;
    }
    if (egl->TinySurface != EGL_NO_SURFACE) {
        ALOGE("        eglDestroySurface( Display, TinySurface )");
        if (eglDestroySurface(egl->Display, egl->TinySurface) == EGL_FALSE) {
            ALOGE("        eglDestroySurface() failed: %s", EglErrorString(eglGetError()));
        }
        egl->TinySurface = EGL_NO_SURFACE;
    }
    if (egl->Display != 0) {
        ALOGE("        eglTerminate( Display )");
        if (eglTerminate(egl->Display) == EGL_FALSE) {
            ALOGE("        eglTerminate() failed: %s", EglErrorString(eglGetError()));
        }
        egl->Display = 0;
    }
}


/*
================================================================================

ovrFramebuffer

================================================================================
*/

typedef struct {
    int Width;
    int Height;
    int Multisamples;
    int TextureSwapChainLength;
    int TextureSwapChainIndex;
    bool UseMultiview;
    ovrTextureSwapChain* ColorTextureSwapChain;
    GLuint* DepthBuffers;
    GLuint* FrameBuffers;
} ovrFramebuffer;

static void ovrFramebuffer_Clear(ovrFramebuffer* frameBuffer) {
    frameBuffer->Width = 0;
    frameBuffer->Height = 0;
    frameBuffer->Multisamples = 0;
    frameBuffer->TextureSwapChainLength = 0;
    frameBuffer->TextureSwapChainIndex = 0;
    frameBuffer->UseMultiview = false;
    frameBuffer->ColorTextureSwapChain = NULL;
    frameBuffer->DepthBuffers = NULL;
    frameBuffer->FrameBuffers = NULL;
}

static bool ovrFramebuffer_Create(
    ovrFramebuffer* frameBuffer,
    const bool useMultiview,
    const GLenum colorFormat,
    const int width,
    const int height,
    const int multisamples) {
    PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC glRenderbufferStorageMultisampleEXT =
        (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC)eglGetProcAddress(
            "glRenderbufferStorageMultisampleEXT");
    PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC glFramebufferTexture2DMultisampleEXT =
        (PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC)eglGetProcAddress(
            "glFramebufferTexture2DMultisampleEXT");

    PFNGLFRAMEBUFFERTEXTUREMULTIVIEWOVRPROC glFramebufferTextureMultiviewOVR =
        (PFNGLFRAMEBUFFERTEXTUREMULTIVIEWOVRPROC)eglGetProcAddress(
            "glFramebufferTextureMultiviewOVR");
    PFNGLFRAMEBUFFERTEXTUREMULTISAMPLEMULTIVIEWOVRPROC glFramebufferTextureMultisampleMultiviewOVR =
        (PFNGLFRAMEBUFFERTEXTUREMULTISAMPLEMULTIVIEWOVRPROC)eglGetProcAddress(
            "glFramebufferTextureMultisampleMultiviewOVR");

    frameBuffer->Width = width;
    frameBuffer->Height = height;
    frameBuffer->Multisamples = multisamples;
    frameBuffer->UseMultiview =
        (useMultiview && (glFramebufferTextureMultiviewOVR != NULL)) ? true : false;

    frameBuffer->ColorTextureSwapChain = vrapi_CreateTextureSwapChain3(
        frameBuffer->UseMultiview ? VRAPI_TEXTURE_TYPE_2D_ARRAY : VRAPI_TEXTURE_TYPE_2D,
        colorFormat,
        width,
        height,
        1,
        3);
    frameBuffer->TextureSwapChainLength =
        vrapi_GetTextureSwapChainLength(frameBuffer->ColorTextureSwapChain);
    frameBuffer->DepthBuffers =
        (GLuint*)malloc(frameBuffer->TextureSwapChainLength * sizeof(GLuint));
    frameBuffer->FrameBuffers =
        (GLuint*)malloc(frameBuffer->TextureSwapChainLength * sizeof(GLuint));

    ALOGV("        frameBuffer->UseMultiview = %d", frameBuffer->UseMultiview);

    for (int i = 0; i < frameBuffer->TextureSwapChainLength; i++) {
        // Create the color buffer texture.
        const GLuint colorTexture =
            vrapi_GetTextureSwapChainHandle(frameBuffer->ColorTextureSwapChain, i);
        GLenum colorTextureTarget = frameBuffer->UseMultiview ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;
        GL(glBindTexture(colorTextureTarget, colorTexture));
        GL(glTexParameteri(colorTextureTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
        GL(glTexParameteri(colorTextureTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));
        GLfloat borderColor[] = {0.0f, 0.0f, 0.0f, 0.0f};
        GL(glTexParameterfv(colorTextureTarget, GL_TEXTURE_BORDER_COLOR, borderColor));
        GL(glTexParameteri(colorTextureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        GL(glTexParameteri(colorTextureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        GL(glBindTexture(colorTextureTarget, 0));

        if (frameBuffer->UseMultiview) {
            // Create the depth buffer texture.
            GL(glGenTextures(1, &frameBuffer->DepthBuffers[i]));
            GL(glBindTexture(GL_TEXTURE_2D_ARRAY, frameBuffer->DepthBuffers[i]));
            GL(glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH_COMPONENT24, width, height, 2));
            GL(glBindTexture(GL_TEXTURE_2D_ARRAY, 0));

            // Create the frame buffer.
            GL(glGenFramebuffers(1, &frameBuffer->FrameBuffers[i]));
            GL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBuffer->FrameBuffers[i]));
            if (multisamples > 1 && (glFramebufferTextureMultisampleMultiviewOVR != NULL)) {
                GL(glFramebufferTextureMultisampleMultiviewOVR(
                    GL_DRAW_FRAMEBUFFER,
                    GL_DEPTH_ATTACHMENT,
                    frameBuffer->DepthBuffers[i],
                    0 /* level */,
                    multisamples /* samples */,
                    0 /* baseViewIndex */,
                    2 /* numViews */));
                GL(glFramebufferTextureMultisampleMultiviewOVR(
                    GL_DRAW_FRAMEBUFFER,
                    GL_COLOR_ATTACHMENT0,
                    colorTexture,
                    0 /* level */,
                    multisamples /* samples */,
                    0 /* baseViewIndex */,
                    2 /* numViews */));
            } else {
                GL(glFramebufferTextureMultiviewOVR(
                    GL_DRAW_FRAMEBUFFER,
                    GL_DEPTH_ATTACHMENT,
                    frameBuffer->DepthBuffers[i],
                    0 /* level */,
                    0 /* baseViewIndex */,
                    2 /* numViews */));
                GL(glFramebufferTextureMultiviewOVR(
                    GL_DRAW_FRAMEBUFFER,
                    GL_COLOR_ATTACHMENT0,
                    colorTexture,
                    0 /* level */,
                    0 /* baseViewIndex */,
                    2 /* numViews */));
            }

            GL(GLenum renderFramebufferStatus = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));
            GL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
            if (renderFramebufferStatus != GL_FRAMEBUFFER_COMPLETE) {
                ALOGE(
                    "Incomplete frame buffer object: %s",
                    GlFrameBufferStatusString(renderFramebufferStatus));
                return false;
            }
        } else {
            if (multisamples > 1 && glRenderbufferStorageMultisampleEXT != NULL &&
                glFramebufferTexture2DMultisampleEXT != NULL) {
                // Create multisampled depth buffer.
                GL(glGenRenderbuffers(1, &frameBuffer->DepthBuffers[i]));
                GL(glBindRenderbuffer(GL_RENDERBUFFER, frameBuffer->DepthBuffers[i]));
                GL(glRenderbufferStorageMultisampleEXT(
                    GL_RENDERBUFFER, multisamples, GL_DEPTH_COMPONENT24, width, height));
                GL(glBindRenderbuffer(GL_RENDERBUFFER, 0));

                // Create the frame buffer.
                // NOTE: glFramebufferTexture2DMultisampleEXT only works with GL_FRAMEBUFFER.
                GL(glGenFramebuffers(1, &frameBuffer->FrameBuffers[i]));
                GL(glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer->FrameBuffers[i]));
                GL(glFramebufferTexture2DMultisampleEXT(
                    GL_FRAMEBUFFER,
                    GL_COLOR_ATTACHMENT0,
                    GL_TEXTURE_2D,
                    colorTexture,
                    0,
                    multisamples));
                GL(glFramebufferRenderbuffer(
                    GL_FRAMEBUFFER,
                    GL_DEPTH_ATTACHMENT,
                    GL_RENDERBUFFER,
                    frameBuffer->DepthBuffers[i]));
                GL(GLenum renderFramebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER));
                GL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
                if (renderFramebufferStatus != GL_FRAMEBUFFER_COMPLETE) {
                    ALOGE(
                        "Incomplete frame buffer object: %s",
                        GlFrameBufferStatusString(renderFramebufferStatus));
                    return false;
                }
            } else {
                // Create depth buffer.
                GL(glGenRenderbuffers(1, &frameBuffer->DepthBuffers[i]));
                GL(glBindRenderbuffer(GL_RENDERBUFFER, frameBuffer->DepthBuffers[i]));
                GL(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height));
                GL(glBindRenderbuffer(GL_RENDERBUFFER, 0));

                // Create the frame buffer.
                GL(glGenFramebuffers(1, &frameBuffer->FrameBuffers[i]));
                GL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBuffer->FrameBuffers[i]));
                GL(glFramebufferRenderbuffer(
                    GL_DRAW_FRAMEBUFFER,
                    GL_DEPTH_ATTACHMENT,
                    GL_RENDERBUFFER,
                    frameBuffer->DepthBuffers[i]));
                GL(glFramebufferTexture2D(
                    GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0));
                GL(GLenum renderFramebufferStatus = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));
                GL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
                if (renderFramebufferStatus != GL_FRAMEBUFFER_COMPLETE) {
                    ALOGE(
                        "Incomplete frame buffer object: %s",
                        GlFrameBufferStatusString(renderFramebufferStatus));
                    return false;
                }
            }
        }
    }

    return true;
}

static void ovrFramebuffer_Destroy(ovrFramebuffer* frameBuffer) {
    GL(glDeleteFramebuffers(frameBuffer->TextureSwapChainLength, frameBuffer->FrameBuffers));
    if (frameBuffer->UseMultiview) {
        GL(glDeleteTextures(frameBuffer->TextureSwapChainLength, frameBuffer->DepthBuffers));
    } else {
        GL(glDeleteRenderbuffers(frameBuffer->TextureSwapChainLength, frameBuffer->DepthBuffers));
    }
    vrapi_DestroyTextureSwapChain(frameBuffer->ColorTextureSwapChain);

    free(frameBuffer->DepthBuffers);
    free(frameBuffer->FrameBuffers);

    ovrFramebuffer_Clear(frameBuffer);
}

static void ovrFramebuffer_SetCurrent(ovrFramebuffer* frameBuffer) {
    GL(glBindFramebuffer(
        GL_DRAW_FRAMEBUFFER, frameBuffer->FrameBuffers[frameBuffer->TextureSwapChainIndex]));
}

static void ovrFramebuffer_SetNone() {
    GL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
}

static void ovrFramebuffer_Resolve(ovrFramebuffer* frameBuffer) {
    // Discard the depth buffer, so the tiler won't need to write it back out to memory.
    const GLenum depthAttachment[1] = {GL_DEPTH_ATTACHMENT};
    glInvalidateFramebuffer(GL_DRAW_FRAMEBUFFER, 1, depthAttachment);

    // We now let the resolve happen implicitly.
}

static void ovrFramebuffer_Advance(ovrFramebuffer* frameBuffer) {
    // Advance to the next texture from the set.
    frameBuffer->TextureSwapChainIndex =
        (frameBuffer->TextureSwapChainIndex + 1) % frameBuffer->TextureSwapChainLength;
}

/*
================================================================================

ovrRenderer

================================================================================
*/

typedef struct {
    ovrFramebuffer FrameBuffer[VRAPI_FRAME_LAYER_EYE_MAX];
    int NumBuffers;
} ovrRenderer;

static void ovrRenderer_Clear(ovrRenderer* renderer) {
    for (int eye = 0; eye < VRAPI_FRAME_LAYER_EYE_MAX; eye++) {
        ovrFramebuffer_Clear(&renderer->FrameBuffer[eye]);
    }
    renderer->NumBuffers = VRAPI_FRAME_LAYER_EYE_MAX;
}

static void
ovrRenderer_Create(ovrRenderer* renderer, const ovrJava* java, const bool useMultiview) {
    renderer->NumBuffers = useMultiview ? 1 : VRAPI_FRAME_LAYER_EYE_MAX;

    // Create the frame buffers.
    for (int eye = 0; eye < renderer->NumBuffers; eye++) {
        ovrFramebuffer_Create(
            &renderer->FrameBuffer[eye],
            useMultiview,
            GL_RGBA8,
            vrapi_GetSystemPropertyInt(java, VRAPI_SYS_PROP_SUGGESTED_EYE_TEXTURE_WIDTH),
            vrapi_GetSystemPropertyInt(java, VRAPI_SYS_PROP_SUGGESTED_EYE_TEXTURE_HEIGHT),
            NUM_MULTI_SAMPLES);
    }
}

static void ovrRenderer_Destroy(ovrRenderer* renderer) {
    for (int eye = 0; eye < renderer->NumBuffers; eye++) {
        ovrFramebuffer_Destroy(&renderer->FrameBuffer[eye]);
    }
}

#define ASSIGNV(a,b) a.x=b.x; a.y=b.y; a.z=b.z;
#define ASSIGNV4(a,b) a.x=b.x; a.y=b.y; a.z=b.z; a.w=b.w;
static bool roomEnabled=true,roomScreenEnabled=true,roomFloor=false;
static ovrTracking2 last_Head;
static ovrLayerProjection2 ovrRenderer_RenderFrame(
    ovrRenderer* renderer,
    const ovrJava* java,
	double elapsed,
    const ovrTracking2* tracking,
    ovrMobile* ovr) {

	if (tracking)
		last_Head=*tracking;
	ovrInputCapabilityHeader capsHeader;
	uint32_t did=0;
	while ( vrapi_EnumerateInputDevices( ovr, did, &capsHeader ) >= 0 ) {
		if ( capsHeader.Type == ovrControllerType_TrackedRemote ) {
			ovrInputTrackedRemoteCapabilities caps;
			caps.Header = capsHeader;
			vrapi_GetInputDeviceCapabilities( ovr, &caps.Header );
			ovrInputStateTrackedRemote state;
			oculus::Input input;
			state.Header.ControllerType = capsHeader.Type;
			if ( vrapi_GetCurrentInputState( ovr, capsHeader.DeviceID, &state.Header ) >= 0 ) {
				input.deviceId=capsHeader.DeviceID;
				input.deviceType=capsHeader.Type;
				input.batteryPercent=state.BatteryPercentRemaining;
				input.recenterCount=state.RecenterCount;
				input.caps=caps.ControllerCapabilities;
				input.buttons=state.Buttons;
				input.stickX=state.JoystickNoDeadZone.x;
				input.stickY=state.JoystickNoDeadZone.y;
				input.gripTrigger=state.GripTrigger;
				input.indexTrigger=state.IndexTrigger;
				input.trackpadStatus=state.TrackpadStatus;
				input.trackpadX=state.TrackpadPosition.x;
				input.trackpadY=state.TrackpadPosition.y;
				input.touches=state.Touches;
				ovrTracking trackingState;
				input.poseStatus=0;
				if ( vrapi_GetInputTrackingState( ovr, capsHeader.DeviceID, 0, &trackingState ) >= 0 )		{
					input.poseStatus=trackingState.Status;
					ASSIGNV(input.pos,trackingState.HeadPose.Pose.Position);
					ASSIGNV4(input.rot,trackingState.HeadPose.Pose.Orientation);
					ASSIGNV(input.velPos,trackingState.HeadPose.LinearVelocity);
					ASSIGNV(input.velRot,trackingState.HeadPose.AngularVelocity);
					ASSIGNV(input.accPos,trackingState.HeadPose.LinearAcceleration);
					ASSIGNV(input.accRot,trackingState.HeadPose.AngularAcceleration);
				}
				oculus::doInputEvent(input);
			}
		}
		else if ( capsHeader.Type == ovrControllerType_Hand ) {
			ovrInputHandCapabilities caps;
			caps.Header = capsHeader;
			vrapi_GetInputDeviceCapabilities( ovr, &caps.Header );
			ovrInputStateHand state;
			oculus::Input input;
			state.Header.ControllerType = capsHeader.Type;
			if ( vrapi_GetCurrentInputState( ovr, capsHeader.DeviceID, &state.Header ) >= 0 ) {
				input.deviceId=capsHeader.DeviceID;
				input.deviceType=capsHeader.Type;
				input.caps=caps.HandCapabilities;
				//input.buttons=state.PinchStrength
				//ASSIGNV(input.pos,state.PointerPose.Position);
				//ASSIGNV4(input.rot,state.PointerPose.Orientation);
				ovrHandPose trackingState;
				trackingState.Header.Version=ovrHandVersion_1;
				input.poseStatus=0;
				if ( vrapi_GetHandPose( ovr, capsHeader.DeviceID, 0, &trackingState.Header ) >= 0 )		{
					input.poseStatus=trackingState.Status;
					ASSIGNV(input.pos,trackingState.RootPose.Position);
					ASSIGNV4(input.rot,trackingState.RootPose.Orientation);
					for (int k=0;k<ovrHandBone_Max;k++) {
						ASSIGNV4(input.handBone[k],trackingState.BoneRotations[k]);
					}
					input.handScale=trackingState.HandScale;
				}
				oculus::doInputEvent(input);
			}
		}
		did++;
	}
    oculus::doTick(elapsed);

    ovrTracking2 updatedTracking = *tracking;

    ovrMatrix4f eyeViewMatrixTransposed[2];
    eyeViewMatrixTransposed[0] = ovrMatrix4f_Transpose(&updatedTracking.Eye[0].ViewMatrix);
    eyeViewMatrixTransposed[1] = ovrMatrix4f_Transpose(&updatedTracking.Eye[1].ViewMatrix);

    ovrMatrix4f projectionMatrixTransposed[2];
    projectionMatrixTransposed[0] = ovrMatrix4f_Transpose(&updatedTracking.Eye[0].ProjectionMatrix);
    projectionMatrixTransposed[1] = ovrMatrix4f_Transpose(&updatedTracking.Eye[1].ProjectionMatrix);

    ovrLayerProjection2 layer = vrapi_DefaultLayerProjection2();
    layer.HeadPose = updatedTracking.HeadPose;
    for (int eye = 0; eye < VRAPI_FRAME_LAYER_EYE_MAX; eye++) {
        ovrFramebuffer* frameBuffer = &renderer->FrameBuffer[renderer->NumBuffers == 1 ? 0 : eye];
        layer.Textures[eye].ColorSwapChain = frameBuffer->ColorTextureSwapChain;
        layer.Textures[eye].SwapChainIndex = frameBuffer->TextureSwapChainIndex;
        layer.Textures[eye].TexCoordsFromTanAngles =
            ovrMatrix4f_TanAngleMatrixFromProjection(&updatedTracking.Eye[eye].ProjectionMatrix);
    }
    layer.Header.Flags |= VRAPI_FRAME_LAYER_FLAG_CHROMATIC_ABERRATION_CORRECTION;

    // Render the eye images.
    for (int eye = 0; eye < renderer->NumBuffers; eye++) {
        // NOTE: In the non-mv case, latency can be further reduced by updating the sensor
        // prediction for each eye (updates orientation, not position)
        ovrFramebuffer* frameBuffer = &renderer->FrameBuffer[eye];
        ovrFramebuffer_SetCurrent(frameBuffer);

        float vmat[16],pmat[16];
        float *m=vmat; for (int j=0;j<4;j++) for (int i=0;i<4;i++) *(m++)=eyeViewMatrixTransposed[eye].M[j][i];
        m=pmat; for (int j=0;j<4;j++) for (int i=0;i<4;i++) *(m++)=projectionMatrixTransposed[eye].M[j][i];
        oculus::doRender(vmat,pmat,frameBuffer->Width,frameBuffer->Height,roomEnabled,roomScreenEnabled,roomFloor);

        ovrFramebuffer_Resolve(frameBuffer);
        ovrFramebuffer_Advance(frameBuffer);
    }

    ovrFramebuffer_SetNone();

    return layer;
}

/*
================================================================================

ovrRenderThread

================================================================================
*/

#if MULTI_THREADED

typedef enum { RENDER_FRAME, RENDER_LOADING_ICON, RENDER_BLACK_FINAL } ovrRenderType;

typedef struct {
    JavaVM* JavaVm;
    jobject ActivityObject;
    const ovrEgl* ShareEgl;
    pthread_t Thread;
    int Tid;
    bool UseMultiview;
    // Synchronization
    bool Exit;
    bool WorkAvailableFlag;
    bool WorkDoneFlag;
    pthread_cond_t WorkAvailableCondition;
    pthread_cond_t WorkDoneCondition;
    pthread_mutex_t Mutex;
    // Latched data for rendering.
    ovrMobile* Ovr;
    ovrRenderType RenderType;
    long long FrameIndex;
    double DisplayTime;
    int SwapInterval;
    ovrScene* Scene;
    ovrSimulation Simulation;
    ovrTracking2 Tracking;
} ovrRenderThread;

void* RenderThreadFunction(void* parm) {
    ovrRenderThread* renderThread = (ovrRenderThread*)parm;
    renderThread->Tid = gettid();

    ovrJava java;
    java.Vm = renderThread->JavaVm;
    (*java.Vm)->AttachCurrentThread(java.Vm, &java.Env, NULL);
    java.ActivityObject = renderThread->ActivityObject;

    // Note that AttachCurrentThread will reset the thread name.
    prctl(PR_SET_NAME, (long)"OVR::Renderer", 0, 0, 0);

    ovrEgl egl;
    ovrEgl_CreateContext(&egl, renderThread->ShareEgl);

    ovrRenderer renderer;
    ovrRenderer_Create(&renderer, &java, renderThread->UseMultiview);

    ovrScene* lastScene = NULL;

    for (;;) {
        // Signal work completed.
        pthread_mutex_lock(&renderThread->Mutex);
        renderThread->WorkDoneFlag = true;
        pthread_cond_signal(&renderThread->WorkDoneCondition);
        pthread_mutex_unlock(&renderThread->Mutex);

        // Wait for work.
        pthread_mutex_lock(&renderThread->Mutex);
        while (!renderThread->WorkAvailableFlag) {
            pthread_cond_wait(&renderThread->WorkAvailableCondition, &renderThread->Mutex);
        }
        renderThread->WorkAvailableFlag = false;
        pthread_mutex_unlock(&renderThread->Mutex);

        // Check for exit.
        if (renderThread->Exit) {
            break;
        }

        // Make sure the scene has VAOs created for this context.
        if (renderThread->Scene != NULL && renderThread->Scene != lastScene) {
            if (lastScene != NULL) {
                ovrScene_DestroyVAOs(lastScene);
            }
            ovrScene_CreateVAOs(renderThread->Scene);
            lastScene = renderThread->Scene;
        }

        // Render.
        ovrLayer_Union2 layers[ovrMaxLayerCount] = {0};
        int layerCount = 0;
        int frameFlags = 0;

        if (renderThread->RenderType == RENDER_FRAME) {
            ovrLayerProjection2 layer;
            layer = ovrRenderer_RenderFrame(
                &renderer,
                &java,
                renderThread->Scene,
                &renderThread->Simulation,
                &renderThread->Tracking,
                renderThread->Ovr);

            layers[layerCount++].Projection = layer;
        } else if (renderThread->RenderType == RENDER_LOADING_ICON) {
            ovrLayerProjection2 blackLayer = vrapi_DefaultLayerBlackProjection2();
            blackLayer.Header.Flags |= VRAPI_FRAME_LAYER_FLAG_INHIBIT_SRGB_FRAMEBUFFER;
            layers[layerCount++].Projection = blackLayer;

            ovrLayerLoadingIcon2 iconLayer = vrapi_DefaultLayerLoadingIcon2();
            iconLayer.Header.Flags |= VRAPI_FRAME_LAYER_FLAG_INHIBIT_SRGB_FRAMEBUFFER;
            layers[layerCount++].LoadingIcon = iconLayer;

            frameFlags |= VRAPI_FRAME_FLAG_FLUSH;
        } else if (renderThread->RenderType == RENDER_BLACK_FINAL) {
            ovrLayerProjection2 layer = vrapi_DefaultLayerBlackProjection2();
            layer.Header.Flags |= VRAPI_FRAME_LAYER_FLAG_INHIBIT_SRGB_FRAMEBUFFER;
            layers[layerCount++].Projection = layer;

            frameFlags |= VRAPI_FRAME_FLAG_FLUSH | VRAPI_FRAME_FLAG_FINAL;
        }

        const ovrLayerHeader2* layerList[ovrMaxLayerCount] = {0};
        for (int i = 0; i < layerCount; i++) {
            layerList[i] = &layers[i].Header;
        }

        ovrSubmitFrameDescription2 frameDesc = {0};
        frameDesc.Flags = frameFlags;
        frameDesc.SwapInterval = renderThread->SwapInterval;
        frameDesc.FrameIndex = renderThread->FrameIndex;
        frameDesc.DisplayTime = renderThread->DisplayTime;
        frameDesc.LayerCount = layerCount;
        frameDesc.Layers = layerList;

        vrapi_SubmitFrame2(renderThread->Ovr, &frameDesc);
    }

    if (lastScene != NULL) {
        ovrScene_DestroyVAOs(lastScene);
    }

    ovrRenderer_Destroy(&renderer);
    ovrEgl_DestroyContext(&egl);

    (*java.Vm)->DetachCurrentThread(java.Vm);

    return NULL;
}

static void ovrRenderThread_Clear(ovrRenderThread* renderThread) {
    renderThread->JavaVm = NULL;
    renderThread->ActivityObject = NULL;
    renderThread->ShareEgl = NULL;
    renderThread->Thread = 0;
    renderThread->Tid = 0;
    renderThread->UseMultiview = false;
    renderThread->Exit = false;
    renderThread->WorkAvailableFlag = false;
    renderThread->WorkDoneFlag = false;
    renderThread->Ovr = NULL;
    renderThread->RenderType = RENDER_FRAME;
    renderThread->FrameIndex = 1;
    renderThread->DisplayTime = 0;
    renderThread->SwapInterval = 1;
    renderThread->Scene = NULL;
    ovrSimulation_Clear(&renderThread->Simulation);
}

static void ovrRenderThread_Create(
    ovrRenderThread* renderThread,
    const ovrJava* java,
    const ovrEgl* shareEgl,
    const bool useMultiview) {
    renderThread->JavaVm = java->Vm;
    renderThread->ActivityObject = java->ActivityObject;
    renderThread->ShareEgl = shareEgl;
    renderThread->Thread = 0;
    renderThread->Tid = 0;
    renderThread->UseMultiview = useMultiview;
    renderThread->Exit = false;
    renderThread->WorkAvailableFlag = false;
    renderThread->WorkDoneFlag = false;
    pthread_cond_init(&renderThread->WorkAvailableCondition, NULL);
    pthread_cond_init(&renderThread->WorkDoneCondition, NULL);
    pthread_mutex_init(&renderThread->Mutex, NULL);

    const int createErr =
        pthread_create(&renderThread->Thread, NULL, RenderThreadFunction, renderThread);
    if (createErr != 0) {
        ALOGE("pthread_create returned %i", createErr);
    }
}

static void ovrRenderThread_Destroy(ovrRenderThread* renderThread) {
    pthread_mutex_lock(&renderThread->Mutex);
    renderThread->Exit = true;
    renderThread->WorkAvailableFlag = true;
    pthread_cond_signal(&renderThread->WorkAvailableCondition);
    pthread_mutex_unlock(&renderThread->Mutex);

    pthread_join(renderThread->Thread, NULL);
    pthread_cond_destroy(&renderThread->WorkAvailableCondition);
    pthread_cond_destroy(&renderThread->WorkDoneCondition);
    pthread_mutex_destroy(&renderThread->Mutex);
}

static void ovrRenderThread_Submit(
    ovrRenderThread* renderThread,
    ovrMobile* ovr,
    ovrRenderType type,
    long long frameIndex,
    double displayTime,
    int swapInterval,
	double elapsed,
    const ovrTracking2* tracking) {
    // Wait for the renderer thread to finish the last frame.
    pthread_mutex_lock(&renderThread->Mutex);
    while (!renderThread->WorkDoneFlag) {
        pthread_cond_wait(&renderThread->WorkDoneCondition, &renderThread->Mutex);
    }
    renderThread->WorkDoneFlag = false;
    // Latch the render data.
    renderThread->Ovr = ovr;
    renderThread->RenderType = type;
    renderThread->FrameIndex = frameIndex;
    renderThread->DisplayTime = displayTime;
    renderThread->SwapInterval = swapInterval;
    renderThread->Scene = scene;
    if (simulation != NULL) {
        renderThread->Simulation = *simulation;
    }
    if (tracking != NULL) {
        renderThread->Tracking = *tracking;
    }
    // Signal work is available.
    renderThread->WorkAvailableFlag = true;
    pthread_cond_signal(&renderThread->WorkAvailableCondition);
    pthread_mutex_unlock(&renderThread->Mutex);
}

static void ovrRenderThread_Wait(ovrRenderThread* renderThread) {
    // Wait for the renderer thread to finish the last frame.
    pthread_mutex_lock(&renderThread->Mutex);
    while (!renderThread->WorkDoneFlag) {
        pthread_cond_wait(&renderThread->WorkDoneCondition, &renderThread->Mutex);
    }
    pthread_mutex_unlock(&renderThread->Mutex);
}

static int ovrRenderThread_GetTid(ovrRenderThread* renderThread) {
    ovrRenderThread_Wait(renderThread);
    return renderThread->Tid;
}

#endif // MULTI_THREADED

/*
================================================================================

ovrApp

================================================================================
*/

typedef struct {
    ovrJava Java;
    ovrEgl Egl;
    ANativeWindow* NativeWindow;
    bool Resumed;
    ovrMobile* Ovr;
    long long FrameIndex;
    double DisplayTime;
    int SwapInterval;
    int CpuLevel;
    int GpuLevel;
    int MainThreadTid;
    int RenderThreadTid;
    bool GamePadBackButtonDown;
#if MULTI_THREADED
    ovrRenderThread RenderThread;
#else
    ovrRenderer Renderer;
#endif
    bool UseMultiview;
} ovrApp;

static void ovrApp_Clear(ovrApp* app) {
    app->Java.Vm = NULL;
    app->Java.Env = NULL;
    app->Java.ActivityObject = NULL;
    app->NativeWindow = NULL;
    app->Resumed = false;
    app->Ovr = NULL;
    app->FrameIndex = 1;
    app->DisplayTime = 0;
    app->SwapInterval = 1;
    app->CpuLevel = 2;
    app->GpuLevel = 2;
    app->MainThreadTid = 0;
    app->RenderThreadTid = 0;
    app->GamePadBackButtonDown = false;
    app->UseMultiview = false;

    ovrEgl_Clear(&app->Egl);
#if MULTI_THREADED
    ovrRenderThread_Clear(&app->RenderThread);
#else
    ovrRenderer_Clear(&app->Renderer);
#endif
}

static void ovrApp_HandleVrModeChanges(ovrApp* app) {
    if (app->Resumed != false && app->NativeWindow != NULL) {
        if (app->Ovr == NULL) {
            ovrModeParms parms = vrapi_DefaultModeParms(&app->Java);
            // Must reset the FLAG_FULLSCREEN window flag when using a SurfaceView
            parms.Flags |= VRAPI_MODE_FLAG_RESET_WINDOW_FULLSCREEN;

            parms.Flags |= VRAPI_MODE_FLAG_NATIVE_WINDOW;
            parms.Display = (size_t)app->Egl.Display;
            parms.WindowSurface = (size_t)app->NativeWindow;
            parms.ShareContext = (size_t)app->Egl.Context;

            ALOGV("        eglGetCurrentSurface( EGL_DRAW ) = %p", eglGetCurrentSurface(EGL_DRAW));

            ALOGV("        vrapi_EnterVrMode()");

            app->Ovr = vrapi_EnterVrMode(&parms);

            ALOGV("        eglGetCurrentSurface( EGL_DRAW ) = %p", eglGetCurrentSurface(EGL_DRAW));

            // If entering VR mode failed then the ANativeWindow was not valid.
            if (app->Ovr == NULL) {
                ALOGE("Invalid ANativeWindow!");
                app->NativeWindow = NULL;
            }

            // Set performance parameters once we have entered VR mode and have a valid ovrMobile.
            if (app->Ovr != NULL) {
                vrapi_SetClockLevels(app->Ovr, app->CpuLevel, app->GpuLevel);

                ALOGV("		vrapi_SetClockLevels( %d, %d )", app->CpuLevel, app->GpuLevel);

                vrapi_SetPerfThread(app->Ovr, VRAPI_PERF_THREAD_TYPE_MAIN, app->MainThreadTid);

                ALOGV("		vrapi_SetPerfThread( MAIN, %d )", app->MainThreadTid);

                vrapi_SetPerfThread(
                    app->Ovr, VRAPI_PERF_THREAD_TYPE_RENDERER, app->RenderThreadTid);

                ALOGV("		vrapi_SetPerfThread( RENDERER, %d )", app->RenderThreadTid);
            }
        }
    } else {
        if (app->Ovr != NULL) {
#if MULTI_THREADED
            // Make sure the renderer thread is no longer using the ovrMobile.
            ovrRenderThread_Wait(&app->RenderThread);
#endif
            ALOGV("        eglGetCurrentSurface( EGL_DRAW ) = %p", eglGetCurrentSurface(EGL_DRAW));

            ALOGV("        vrapi_LeaveVrMode()");

            vrapi_LeaveVrMode(app->Ovr);
            app->Ovr = NULL;

            ALOGV("        eglGetCurrentSurface( EGL_DRAW ) = %p", eglGetCurrentSurface(EGL_DRAW));
        }
    }
}

static void ovrApp_HandleInput(ovrApp* app) {}

static int ovrApp_HandleKeyEvent(ovrApp* app, const int keyCode, const int action) {
    // Handle back button.
    if (keyCode == AKEYCODE_BACK || keyCode == AKEYCODE_BUTTON_B) {
        if (action == AKEY_EVENT_ACTION_DOWN) {
            app->GamePadBackButtonDown = true;
        } else if (action == AKEY_EVENT_ACTION_UP) {
            app->GamePadBackButtonDown = false;
        }
        return 1;
    }
    return 0;
}

static void ovrApp_HandleVrApiEvents(ovrApp* app) {
    ovrEventDataBuffer eventDataBuffer = {};

    // Poll for VrApi events
    for (;;) {
        ovrEventHeader* eventHeader = (ovrEventHeader*)(&eventDataBuffer);
        ovrResult res = vrapi_PollEvent(eventHeader);
        if (res != ovrSuccess) {
            break;
        }

        switch (eventHeader->EventType) {
            case VRAPI_EVENT_DATA_LOST:
                ALOGV("vrapi_PollEvent: Received VRAPI_EVENT_DATA_LOST");
                break;
            case VRAPI_EVENT_VISIBILITY_GAINED:
                ALOGV("vrapi_PollEvent: Received VRAPI_EVENT_VISIBILITY_GAINED");
                break;
            case VRAPI_EVENT_VISIBILITY_LOST:
                ALOGV("vrapi_PollEvent: Received VRAPI_EVENT_VISIBILITY_LOST");
                break;
            case VRAPI_EVENT_FOCUS_GAINED:
                // FOCUS_GAINED is sent when the application is in the foreground and has
                // input focus. This may be due to a system overlay relinquishing focus
                // back to the application.
                ALOGV("vrapi_PollEvent: Received VRAPI_EVENT_FOCUS_GAINED");
                break;
            case VRAPI_EVENT_FOCUS_LOST:
                // FOCUS_LOST is sent when the application is no longer in the foreground and
                // therefore does not have input focus. This may be due to a system overlay taking
                // focus from the application. The application should take appropriate action when
                // this occurs.
                ALOGV("vrapi_PollEvent: Received VRAPI_EVENT_FOCUS_LOST");
                break;
            default:
                ALOGV("vrapi_PollEvent: Unknown event");
                break;
        }
    }
}

/*
================================================================================

ovrMessageQueue

================================================================================
*/

typedef enum {
    MQ_WAIT_NONE, // don't wait
    MQ_WAIT_RECEIVED, // wait until the consumer thread has received the message
    MQ_WAIT_PROCESSED // wait until the consumer thread has processed the message
} ovrMQWait;

#define MAX_MESSAGE_PARMS 8
#define MAX_MESSAGES 1024

typedef struct {
    int Id;
    ovrMQWait Wait;
    long long Parms[MAX_MESSAGE_PARMS];
} ovrMessage;

static void ovrMessage_Init(ovrMessage* message, const int id, const int wait) {
    message->Id = id;
    message->Wait = (ovrMQWait) wait;
    memset(message->Parms, 0, sizeof(message->Parms));
}

static void ovrMessage_SetPointerParm(ovrMessage* message, int index, void* ptr) {
    *(void**)&message->Parms[index] = ptr;
}
static void* ovrMessage_GetPointerParm(ovrMessage* message, int index) {
    return *(void**)&message->Parms[index];
}
static void ovrMessage_SetIntegerParm(ovrMessage* message, int index, int value) {
    message->Parms[index] = value;
}
static int ovrMessage_GetIntegerParm(ovrMessage* message, int index) {
    return (int)message->Parms[index];
}
/// static void		ovrMessage_SetFloatParm( ovrMessage * message, int index, float value ) {
/// *(float *)&message->Parms[index] = value; } static float	ovrMessage_GetFloatParm( ovrMessage
/// * message, int index ) { return *(float *)&message->Parms[index]; }

// Cyclic queue with messages.
typedef struct {
    ovrMessage Messages[MAX_MESSAGES];
    volatile int Head; // dequeue at the head
    volatile int Tail; // enqueue at the tail
    ovrMQWait Wait;
    volatile bool EnabledFlag;
    volatile bool PostedFlag;
    volatile bool ReceivedFlag;
    volatile bool ProcessedFlag;
    pthread_mutex_t Mutex;
    pthread_cond_t PostedCondition;
    pthread_cond_t ReceivedCondition;
    pthread_cond_t ProcessedCondition;
} ovrMessageQueue;

static void ovrMessageQueue_Create(ovrMessageQueue* messageQueue) {
    messageQueue->Head = 0;
    messageQueue->Tail = 0;
    messageQueue->Wait = MQ_WAIT_NONE;
    messageQueue->EnabledFlag = false;
    messageQueue->PostedFlag = false;
    messageQueue->ReceivedFlag = false;
    messageQueue->ProcessedFlag = false;

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutex_init(&messageQueue->Mutex, &attr);
    pthread_mutexattr_destroy(&attr);
    pthread_cond_init(&messageQueue->PostedCondition, NULL);
    pthread_cond_init(&messageQueue->ReceivedCondition, NULL);
    pthread_cond_init(&messageQueue->ProcessedCondition, NULL);
}

static void ovrMessageQueue_Destroy(ovrMessageQueue* messageQueue) {
    pthread_mutex_destroy(&messageQueue->Mutex);
    pthread_cond_destroy(&messageQueue->PostedCondition);
    pthread_cond_destroy(&messageQueue->ReceivedCondition);
    pthread_cond_destroy(&messageQueue->ProcessedCondition);
}

static void ovrMessageQueue_Enable(ovrMessageQueue* messageQueue, const bool set) {
    messageQueue->EnabledFlag = set;
}

static void ovrMessageQueue_PostMessage(ovrMessageQueue* messageQueue, const ovrMessage* message) {
    if (!messageQueue->EnabledFlag) {
        return;
    }
    while (messageQueue->Tail - messageQueue->Head >= MAX_MESSAGES) {
        usleep(1000);
    }
    pthread_mutex_lock(&messageQueue->Mutex);
    messageQueue->Messages[messageQueue->Tail & (MAX_MESSAGES - 1)] = *message;
    messageQueue->Tail++;
    messageQueue->PostedFlag = true;
    pthread_cond_broadcast(&messageQueue->PostedCondition);
    if (message->Wait == MQ_WAIT_RECEIVED) {
        while (!messageQueue->ReceivedFlag) {
            pthread_cond_wait(&messageQueue->ReceivedCondition, &messageQueue->Mutex);
        }
        messageQueue->ReceivedFlag = false;
    } else if (message->Wait == MQ_WAIT_PROCESSED) {
        while (!messageQueue->ProcessedFlag) {
            pthread_cond_wait(&messageQueue->ProcessedCondition, &messageQueue->Mutex);
        }
        messageQueue->ProcessedFlag = false;
    }
    pthread_mutex_unlock(&messageQueue->Mutex);
}

static void ovrMessageQueue_SleepUntilMessage(ovrMessageQueue* messageQueue) {
    if (messageQueue->Wait == MQ_WAIT_PROCESSED) {
        messageQueue->ProcessedFlag = true;
        pthread_cond_broadcast(&messageQueue->ProcessedCondition);
        messageQueue->Wait = MQ_WAIT_NONE;
    }
    pthread_mutex_lock(&messageQueue->Mutex);
    if (messageQueue->Tail > messageQueue->Head) {
        pthread_mutex_unlock(&messageQueue->Mutex);
        return;
    }
    while (!messageQueue->PostedFlag) {
        pthread_cond_wait(&messageQueue->PostedCondition, &messageQueue->Mutex);
    }
    messageQueue->PostedFlag = false;
    pthread_mutex_unlock(&messageQueue->Mutex);
}

static bool ovrMessageQueue_GetNextMessage(
    ovrMessageQueue* messageQueue,
    ovrMessage* message,
    bool waitForMessages) {
    if (messageQueue->Wait == MQ_WAIT_PROCESSED) {
        messageQueue->ProcessedFlag = true;
        pthread_cond_broadcast(&messageQueue->ProcessedCondition);
        messageQueue->Wait = MQ_WAIT_NONE;
    }
    if (waitForMessages) {
        ovrMessageQueue_SleepUntilMessage(messageQueue);
    }
    pthread_mutex_lock(&messageQueue->Mutex);
    if (messageQueue->Tail <= messageQueue->Head) {
        pthread_mutex_unlock(&messageQueue->Mutex);
        return false;
    }
    *message = messageQueue->Messages[messageQueue->Head & (MAX_MESSAGES - 1)];
    messageQueue->Head++;
    pthread_mutex_unlock(&messageQueue->Mutex);
    if (message->Wait == MQ_WAIT_RECEIVED) {
        messageQueue->ReceivedFlag = true;
        pthread_cond_broadcast(&messageQueue->ReceivedCondition);
    } else if (message->Wait == MQ_WAIT_PROCESSED) {
        messageQueue->Wait = MQ_WAIT_PROCESSED;
    }
    return true;
}

/*
================================================================================

ovrAppThread

================================================================================
*/

enum {
    MESSAGE_ON_CREATE,
    MESSAGE_ON_START,
    MESSAGE_ON_RESUME,
    MESSAGE_ON_PAUSE,
    MESSAGE_ON_STOP,
    MESSAGE_ON_DESTROY,
    MESSAGE_ON_SURFACE_CREATED,
    MESSAGE_ON_SURFACE_DESTROYED,
    MESSAGE_ON_KEY_EVENT,
    MESSAGE_ON_TOUCH_EVENT
};

typedef struct {
    JavaVM* JavaVm;
    jobject ActivityObject;
    pthread_t Thread;
    ovrMessageQueue MessageQueue;
    ANativeWindow* NativeWindow;
} ovrAppThread;

ovrApp appState;
void* AppThreadFunction(void* parm) {
    ovrAppThread* appThread = (ovrAppThread*)parm;

    ovrJava java;
    java.Vm = appThread->JavaVm;
    java.Vm->AttachCurrentThread(&java.Env,NULL);
    java.ActivityObject = appThread->ActivityObject;

    // Note that AttachCurrentThread will reset the thread name.
    prctl(PR_SET_NAME, (long)"OVR::Main", 0, 0, 0);

    const ovrInitParms initParms = vrapi_DefaultInitParms(&java);
    int32_t initResult = vrapi_Initialize(&initParms);
    if (initResult != VRAPI_INITIALIZE_SUCCESS) {
        // If intialization failed, vrapi_* function calls will not be available.
        exit(0);
    }

    ovrApp_Clear(&appState);
    appState.Java = java;

    // This app will handle android gamepad events itself.
    vrapi_SetPropertyInt(&appState.Java, VRAPI_EAT_NATIVE_GAMEPAD_EVENTS, 0);

    ovrEgl_CreateContext(&appState.Egl, NULL);

    EglInitExtensions();

    appState.UseMultiview &= glExtensions.multi_view;

    ALOGV("AppState UseMultiview : %d", appState.UseMultiview);

    appState.CpuLevel = CPU_LEVEL;
    appState.GpuLevel = GPU_LEVEL;
    appState.MainThreadTid = gettid();
    appState.DisplayTime=0;

#if MULTI_THREADED
    ovrRenderThread_Create(
        &appState.RenderThread, &appState.Java, &appState.Egl, appState.UseMultiview);
    // Also set the renderer thread to SCHED_FIFO.
    appState.RenderThreadTid = ovrRenderThread_GetTid(&appState.RenderThread);
#else
    ovrRenderer_Create(&appState.Renderer, &java, appState.UseMultiview);
#endif

    const double startTime = GetTimeInSeconds();

    for (bool destroyed = false; destroyed == false;) {
        for (;;) {
            ovrMessage message;
            const bool waitForMessages = (appState.Ovr == NULL && destroyed == false);
            if (!ovrMessageQueue_GetNextMessage(
                    &appThread->MessageQueue, &message, waitForMessages)) {
                break;
            }

            switch (message.Id) {
                case MESSAGE_ON_CREATE: {
                    break;
                }
                case MESSAGE_ON_START: {
                    break;
                }
                case MESSAGE_ON_RESUME: {
                    appState.Resumed = true;
                    break;
                }
                case MESSAGE_ON_PAUSE: {
                    appState.Resumed = false;
                    break;
                }
                case MESSAGE_ON_STOP: {
                    break;
                }
                case MESSAGE_ON_DESTROY: {
                    appState.NativeWindow = NULL;
                    destroyed = true;
                    break;
                }
                case MESSAGE_ON_SURFACE_CREATED: {
                    appState.NativeWindow = (ANativeWindow*)ovrMessage_GetPointerParm(&message, 0);
                    roomEnabled=true;
                    roomScreenEnabled=true;
                    _gapp->initialize();
                    setupApi(LuaDebugging::L);
                    break;
                }
                case MESSAGE_ON_SURFACE_DESTROYED: {
            		_gapp->deinitialize();
                    appState.NativeWindow = NULL;
                    break;
                }
                case MESSAGE_ON_KEY_EVENT: {
                    ovrApp_HandleKeyEvent(
                        &appState,
                        ovrMessage_GetIntegerParm(&message, 0),
                        ovrMessage_GetIntegerParm(&message, 1));
                    break;
                }
            }

            ovrApp_HandleVrModeChanges(&appState);
        }

        // We must read from the event queue with regular frequency.
        ovrApp_HandleVrApiEvents(&appState);

        ovrApp_HandleInput(&appState);

        if (appState.Ovr == NULL) {
            continue;
        }

        // Create the scene if not yet created.
        // The scene is created here to be able to show a loading icon.
/*        if (!ovrScene_IsCreated(&appState.Scene)) {
#if MULTI_THREADED
            // Show a loading icon.
            ovrRenderThread_Submit(
                &appState.RenderThread,
                appState.Ovr,
                RENDER_LOADING_ICON,
                appState.FrameIndex,
                appState.DisplayTime,
                appState.SwapInterval,
                NULL,
                NULL,
                NULL);
#else
            // Show a loading icon.
            int frameFlags = 0;
            frameFlags |= VRAPI_FRAME_FLAG_FLUSH;

            ovrLayerProjection2 blackLayer = vrapi_DefaultLayerBlackProjection2();
            blackLayer.Header.Flags |= VRAPI_FRAME_LAYER_FLAG_INHIBIT_SRGB_FRAMEBUFFER;

            ovrLayerLoadingIcon2 iconLayer = vrapi_DefaultLayerLoadingIcon2();
            iconLayer.Header.Flags |= VRAPI_FRAME_LAYER_FLAG_INHIBIT_SRGB_FRAMEBUFFER;

            const ovrLayerHeader2* layers[] = {
                &blackLayer.Header,
                &iconLayer.Header,
            };

            ovrSubmitFrameDescription2 frameDesc = {0};
            frameDesc.Flags = frameFlags;
            frameDesc.SwapInterval = 1;
            frameDesc.FrameIndex = appState.FrameIndex;
            frameDesc.DisplayTime = appState.DisplayTime;
            frameDesc.LayerCount = 2;
            frameDesc.Layers = layers;

            vrapi_SubmitFrame2(appState.Ovr, &frameDesc);
#endif

            // Create the scene.
            ovrScene_Create(&appState.Scene, appState.UseMultiview);
        }*/

        // This is the only place the frame index is incremented, right before
        // calling vrapi_GetPredictedDisplayTime().
        appState.FrameIndex++;

        // Get the HMD pose, predicted for the middle of the time period during which
        // the new eye images will be displayed. The number of frames predicted ahead
        // depends on the pipeline depth of the engine and the synthesis rate.
        // The better the prediction, the less black will be pulled in at the edges.
        const double predictedDisplayTime =
            vrapi_GetPredictedDisplayTime(appState.Ovr, appState.FrameIndex);
        const ovrTracking2 tracking =
            vrapi_GetPredictedTracking2(appState.Ovr, predictedDisplayTime);

        double elapsed=predictedDisplayTime-appState.DisplayTime;
        appState.DisplayTime = predictedDisplayTime;

#if MULTI_THREADED
        // Render the eye images on a separate thread.
        ovrRenderThread_Submit(
            &appState.RenderThread,
            appState.Ovr,
            RENDER_FRAME,
            appState.FrameIndex,
            appState.DisplayTime,
            appState.SwapInterval,
			elapsed,
            &tracking);
#else
        // Render eye images and setup the primary layer using ovrTracking2.
        const ovrLayerProjection2 worldLayer = ovrRenderer_RenderFrame(
            &appState.Renderer,
            &appState.Java,
			elapsed,
            &tracking,
            appState.Ovr);

        const ovrLayerHeader2* layers[] = {&worldLayer.Header};

        ovrSubmitFrameDescription2 frameDesc = {0};
        frameDesc.Flags = 0;
        frameDesc.SwapInterval = appState.SwapInterval;
        frameDesc.FrameIndex = appState.FrameIndex;
        frameDesc.DisplayTime = appState.DisplayTime;
        frameDesc.LayerCount = 1;
        frameDesc.Layers = layers;

        // Hand over the eye images to the time warp.
        vrapi_SubmitFrame2(appState.Ovr, &frameDesc);
#endif
    }

#if MULTI_THREADED
    ovrRenderThread_Destroy(&appState.RenderThread);
#else
    ovrRenderer_Destroy(&appState.Renderer);
#endif

    ovrEgl_DestroyContext(&appState.Egl);

    vrapi_Shutdown();

    java.Vm->DetachCurrentThread();

    return NULL;
}

static void ovrAppThread_Create(ovrAppThread* appThread, JNIEnv* env, jobject activityObject) {
    env->GetJavaVM(&appThread->JavaVm);
    appThread->ActivityObject = env->NewGlobalRef(activityObject);
    appThread->Thread = 0;
    appThread->NativeWindow = NULL;
    ovrMessageQueue_Create(&appThread->MessageQueue);
/*
    const int createErr = pthread_create(&appThread->Thread, NULL, AppThreadFunction, appThread);
    if (createErr != 0) {
        ALOGE("pthread_create returned %i", createErr);
    }*/
}

static void ovrAppThread_Destroy(ovrAppThread* appThread, JNIEnv* env) {
    //pthread_join(appThread->Thread, NULL);
    env->DeleteGlobalRef(appThread->ActivityObject);
    ovrMessageQueue_Destroy(&appThread->MessageQueue);
}

/*
================================================================================

Activity lifecycle

================================================================================
*/

static ovrAppThread *appThread=NULL;
void oculus::onCreate(JNIEnv *env,jobject activity,LuaApplication *app) {
    ALOGV("    GLES3JNILib::onCreate()");
    _gapp=app;

    appThread = (ovrAppThread*)malloc(sizeof(ovrAppThread));
    ovrAppThread_Create(appThread, env, activity);
}

void oculus::runThread() {
	AppThreadFunction(appThread);
}

void oculus::postCreate() {
    ovrMessageQueue_Enable(&appThread->MessageQueue, true);
    ovrMessage message;
    ovrMessage_Init(&message, MESSAGE_ON_CREATE, MQ_WAIT_PROCESSED);
    ovrMessageQueue_PostMessage(&appThread->MessageQueue, &message);
}

void oculus::onStart()
{
    ALOGV("    GLES3JNILib::onStart()");
    ovrMessage message;
    ovrMessage_Init(&message, MESSAGE_ON_START, MQ_WAIT_PROCESSED);
    ovrMessageQueue_PostMessage(&appThread->MessageQueue, &message);
}

void oculus::onResume()
{
    ALOGV("    GLES3JNILib::onResume()");
    ovrMessage message;
    ovrMessage_Init(&message, MESSAGE_ON_RESUME, MQ_WAIT_PROCESSED);
    ovrMessageQueue_PostMessage(&appThread->MessageQueue, &message);
}

void oculus::onPause()
{
    ALOGV("    GLES3JNILib::onPause()");
    ovrMessage message;
    ovrMessage_Init(&message, MESSAGE_ON_PAUSE, MQ_WAIT_PROCESSED);
    ovrMessageQueue_PostMessage(&appThread->MessageQueue, &message);
}

void oculus::onStop()
{
    ALOGV("    GLES3JNILib::onStop()");
    ovrMessage message;
    ovrMessage_Init(&message, MESSAGE_ON_STOP, MQ_WAIT_PROCESSED);
    ovrMessageQueue_PostMessage(&appThread->MessageQueue, &message);
}

void oculus::onDestroy(JNIEnv *env)
{
    ALOGV("    GLES3JNILib::onDestroy()");
    ovrMessage message;
    ovrMessage_Init(&message, MESSAGE_ON_DESTROY, MQ_WAIT_PROCESSED);
    ovrMessageQueue_PostMessage(&appThread->MessageQueue, &message);
    ovrMessageQueue_Enable(&appThread->MessageQueue, false);

    ovrAppThread_Destroy(appThread, env);
    free(appThread);
    appThread=NULL;
}

/*
================================================================================

Surface lifecycle

================================================================================
*/

void oculus::onSurfaceCreated(JNIEnv *env,jobject surface)
{
    ALOGV("    GLES3JNILib::onSurfaceCreated()");

    ANativeWindow* newNativeWindow = ANativeWindow_fromSurface(env, surface);
    if (ANativeWindow_getWidth(newNativeWindow) < ANativeWindow_getHeight(newNativeWindow)) {
        // An app that is relaunched after pressing the home button gets an initial surface with
        // the wrong orientation even though android:screenOrientation="landscape" is set in the
        // manifest. The choreographer callback will also never be called for this surface because
        // the surface is immediately replaced with a new surface with the correct orientation.
        ALOGE("        Surface not in landscape mode!");
    }

    ALOGV("        NativeWindow = ANativeWindow_fromSurface( env, surface )");
    appThread->NativeWindow = newNativeWindow;
    ovrMessage message;
    ovrMessage_Init(&message, MESSAGE_ON_SURFACE_CREATED, MQ_WAIT_PROCESSED);
    ovrMessage_SetPointerParm(&message, 0, appThread->NativeWindow);
    ovrMessageQueue_PostMessage(&appThread->MessageQueue, &message);
}

void oculus::onSurfaceChanged(JNIEnv *env,jobject surface)
{
    ALOGV("    GLES3JNILib::onSurfaceChanged()");

    ANativeWindow* newNativeWindow = ANativeWindow_fromSurface(env, surface);
    if (ANativeWindow_getWidth(newNativeWindow) < ANativeWindow_getHeight(newNativeWindow)) {
        // An app that is relaunched after pressing the home button gets an initial surface with
        // the wrong orientation even though android:screenOrientation="landscape" is set in the
        // manifest. The choreographer callback will also never be called for this surface because
        // the surface is immediately replaced with a new surface with the correct orientation.
        ALOGE("        Surface not in landscape mode!");
    }

    if (newNativeWindow != appThread->NativeWindow) {
/*        if (appThread->NativeWindow != NULL) {
            ovrMessage message;
            ovrMessage_Init(&message, MESSAGE_ON_SURFACE_DESTROYED, MQ_WAIT_PROCESSED);
            ovrMessageQueue_PostMessage(&appThread->MessageQueue, &message);
            ALOGV("        ANativeWindow_release( NativeWindow )");
            ANativeWindow_release(appThread->NativeWindow);
            appThread->NativeWindow = NULL;
        }
        if (newNativeWindow != NULL) {
            ALOGV("        NativeWindow = ANativeWindow_fromSurface( env, surface )");
            appThread->NativeWindow = newNativeWindow;
            ovrMessage message;
            ovrMessage_Init(&message, MESSAGE_ON_SURFACE_CREATED, MQ_WAIT_PROCESSED);
            ovrMessage_SetPointerParm(&message, 0, appThread->NativeWindow);
            ovrMessageQueue_PostMessage(&appThread->MessageQueue, &message);
        }*/
    } else if (newNativeWindow != NULL) {
        ANativeWindow_release(newNativeWindow);
    }
}

void oculus::onSurfaceDestroyed()
{
	ALOGV("    GLES3JNILib::onSurfaceDestroyed()");
    ovrMessage message;
    ovrMessage_Init(&message, MESSAGE_ON_SURFACE_DESTROYED, MQ_WAIT_PROCESSED);
    ovrMessageQueue_PostMessage(&appThread->MessageQueue, &message);
    ALOGV("        ANativeWindow_release( NativeWindow )");
    ANativeWindow_release(appThread->NativeWindow);
    appThread->NativeWindow = NULL;
}

void oculus::onLuaReinit() {
	setupApi(LuaDebugging::L);
}

static int enableRoom(lua_State *L) {
	roomEnabled=lua_toboolean(L,1);
	roomScreenEnabled=lua_toboolean(L,2);
	return 0;
}

static int setTrackingSpace(lua_State *L) {
	ovrTrackingSpace s=(ovrTrackingSpace)luaL_optinteger(L,1,VRAPI_TRACKING_SPACE_LOCAL);
	vrapi_SetTrackingSpace(appState.Ovr,s);
	roomFloor=(s==VRAPI_TRACKING_SPACE_STAGE);
	return 0;
}

static void pushVector(lua_State *L,ovrVector3f v) {
	lua_newtable(L);
	lua_pushnumber(L,v.x); lua_rawseti(L,-2,1);
	lua_pushnumber(L,v.y); lua_rawseti(L,-2,2);
	lua_pushnumber(L,v.z); lua_rawseti(L,-2,3);
}

static void pushVector4(lua_State *L,ovrQuatf v) {
	lua_newtable(L);
	lua_pushnumber(L,v.x); lua_rawseti(L,-2,1);
	lua_pushnumber(L,v.y); lua_rawseti(L,-2,2);
	lua_pushnumber(L,v.z); lua_rawseti(L,-2,3);
	lua_pushnumber(L,v.w); lua_rawseti(L,-2,4);
}

static int getHeadPose(lua_State *L) {
	lua_newtable(L);
	//Pose
	lua_pushinteger(L,last_Head.Status);
	lua_setfield(L, -2,	"poseStatus");
	pushVector(L,last_Head.HeadPose.Pose.Position);
	lua_setfield(L, -2, "position");
	pushVector4(L,last_Head.HeadPose.Pose.Orientation);
	lua_setfield(L, -2, "rotation");
	pushVector(L,last_Head.HeadPose.LinearVelocity);
	lua_setfield(L, -2, "linearVelocity");
	pushVector(L,last_Head.HeadPose.AngularVelocity);
	lua_setfield(L, -2, "angularVelocity");
	pushVector(L,last_Head.HeadPose.LinearAcceleration);
	lua_setfield(L, -2, "linearAcceleration");
	pushVector(L,last_Head.HeadPose.AngularAcceleration);
	lua_setfield(L, -2, "angularAcceleration");
	return 1;
}

static int getHandMesh(lua_State *L) {
	if (appState.Ovr) {
		ovrHandMesh *mesh=new ovrHandMesh;
		mesh->Header.Version=ovrHandVersion_1;
		if (vrapi_GetHandMesh(appState.Ovr, (ovrHandedness) luaL_checkinteger(L,1), &mesh->Header)>=0)
		{
			lua_newtable(L);
			lua_newtable(L);
			for (int k=0;k<mesh->NumIndices;k++)
			{
				lua_pushinteger(L,mesh->Indices[k]+1);
				lua_rawseti(L,-2,k+1);
			}
			lua_setfield(L,-2,"indices");

			lua_newtable(L);
			for (int k=0;k<mesh->NumVertices;k++)
			{
				lua_pushnumber(L,mesh->VertexPositions[k].x);
				lua_rawseti(L,-2,k*3+1);
				lua_pushnumber(L,mesh->VertexPositions[k].y);
				lua_rawseti(L,-2,k*3+2);
				lua_pushnumber(L,mesh->VertexPositions[k].z);
				lua_rawseti(L,-2,k*3+3);
			}
			lua_setfield(L,-2,"vertices");

			lua_newtable(L);
			for (int k=0;k<mesh->NumVertices;k++)
			{
				lua_pushnumber(L,mesh->VertexNormals[k].x);
				lua_rawseti(L,-2,k*3+1);
				lua_pushnumber(L,mesh->VertexNormals[k].y);
				lua_rawseti(L,-2,k*3+2);
				lua_pushnumber(L,mesh->VertexNormals[k].z);
				lua_rawseti(L,-2,k*3+3);
			}
			lua_setfield(L,-2,"normals");

			lua_newtable(L);
			for (int k=0;k<mesh->NumVertices;k++)
			{
				lua_pushnumber(L,mesh->VertexUV0[k].x);
				lua_rawseti(L,-2,k*2+1);
				lua_pushnumber(L,mesh->VertexUV0[k].y);
				lua_rawseti(L,-2,k*2+2);
			}
			lua_setfield(L,-2,"texcoords");

			lua_newtable(L);
			lua_newtable(L);
			for (int k=0;k<mesh->NumVertices;k++)
			{
				lua_pushinteger(L,mesh->BlendIndices[k].x);
				lua_rawseti(L,-2,k*4+1);
				lua_pushinteger(L,mesh->BlendIndices[k].y);
				lua_rawseti(L,-2,k*4+2);
				lua_pushinteger(L,mesh->BlendIndices[k].z);
				lua_rawseti(L,-2,k*4+3);
				lua_pushinteger(L,mesh->BlendIndices[k].w);
				lua_rawseti(L,-2,k*4+4);
			}
			lua_setfield(L,-2,"bi");

			lua_newtable(L);
			for (int k=0;k<mesh->NumVertices;k++)
			{
				lua_pushnumber(L,mesh->BlendWeights[k].x);
				lua_rawseti(L,-2,k*4+1);
				lua_pushnumber(L,mesh->BlendWeights[k].y);
				lua_rawseti(L,-2,k*4+2);
				lua_pushnumber(L,mesh->BlendWeights[k].z);
				lua_rawseti(L,-2,k*4+3);
				lua_pushnumber(L,mesh->BlendWeights[k].w);
				lua_rawseti(L,-2,k*4+4);
			}
			lua_setfield(L,-2,"bw");
			lua_setfield(L,-2,"animdata");
			delete mesh;
			return 1;
		}
		delete mesh;
	}
	return 0;
}

static int getHandSkeleton(lua_State *L) {
	if (appState.Ovr) {
		ovrHandSkeleton *mesh=new ovrHandSkeleton;
		mesh->Header.Version=ovrHandVersion_1;
		if (vrapi_GetHandSkeleton(appState.Ovr, (ovrHandedness) luaL_checkinteger(L,1), &mesh->Header)>=0) {
			lua_newtable(L);
			lua_newtable(L);
			for (int k=0;k<mesh->NumCapsules;k++)
			{
				lua_newtable(L);
				lua_pushinteger(L,mesh->Capsules[k].BoneIndex);
				lua_setfield(L, -2, "boneIndex");
				pushVector(L,mesh->Capsules[k].Points[0]);
				lua_setfield(L, -2, "pointA");
				pushVector(L,mesh->Capsules[k].Points[1]);
				lua_setfield(L, -2, "pointB");
				lua_pushnumber(L,mesh->Capsules[k].Radius);
				lua_setfield(L, -2, "radius");
				lua_rawseti(L,-2,k+1);
			}
			lua_setfield(L,-2,"capsules");

			lua_newtable(L);
			for (int k=0;k<mesh->NumBones;k++)
			{
				lua_pushinteger(L,mesh->BoneParentIndices[k]);
				lua_rawseti(L,-2,k+1);
			}
			lua_setfield(L,-2,"boneParents");

			lua_newtable(L);
			for (int k=0;k<mesh->NumBones;k++)
			{
				lua_newtable(L);
				pushVector(L,mesh->BonePoses[k].Position);
				lua_setfield(L, -2, "position");
				pushVector4(L,mesh->BonePoses[k].Orientation);
				lua_setfield(L, -2, "rotation");
				lua_rawseti(L,-2,k+1);
			}
			lua_setfield(L,-2,"bones");
			delete mesh;

			return 1;
		}
		delete mesh;
	}
	return 0;
}

void setupApi(lua_State *L)
{
	roomEnabled=true;
	roomScreenEnabled=true;
	roomFloor=false;
	if (appState.Ovr)
		vrapi_SetTrackingSpace(appState.Ovr,VRAPI_TRACKING_SPACE_LOCAL);
    static const luaL_Reg functionList[] = {
		{"enableRoom", enableRoom},
		{"setTrackingSpace", setTrackingSpace},
		{"getHeadPose", getHeadPose},
		{"getHandMesh", getHandMesh},
		{"getHandSkeleton", getHandSkeleton},
        {NULL, NULL},
    };

    lua_newtable(L);
    luaL_register(L, NULL, functionList);;
    lua_setglobal(L, "Oculus");
	ALOGV("=== API registered");
}
