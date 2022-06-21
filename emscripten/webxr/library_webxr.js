var LibraryWebXR = {

$WebXR: {
    refSpaces: {},
    _curRAF: null,

    _nativize_vec3: function(offset, vec) {
        setValue(offset + 0, vec.x, 'float');
        setValue(offset + 4, vec.y, 'float');
        setValue(offset + 8, vec.z, 'float');

        return offset + 12;
    },

    _nativize_vec4: function(offset, vec) {
        WebXR._nativize_vec3(offset, vec);
        setValue(offset + 12, vec.w, 'float');

        return offset + 16;
    },

    _nativize_matrix: function(offset, mat) {
        for (var i = 0; i < 16; ++i) {
            setValue(offset + i*4, mat[i], 'float');
        }

        return offset + 16*4;
    },

    _nativize_rigid_transform: function(offset, t) {
        offset = WebXR._nativize_matrix(offset, t.matrix);
        offset = WebXR._nativize_vec3(offset, t.position);
        offset = WebXR._nativize_vec4(offset, t.orientation);

        return offset;
    },

    /* Sets input source values to offset and returns pointer after struct */
    _nativize_input_source: function(offset, inputSource, id) {
        var handedness = -1;
        if(inputSource.handedness == "left") handedness = 0;
        else if(inputSource.handedness == "right") handedness = 1;

        var targetRayMode = 0;
        if(inputSource.targetRayMode == "tracked-pointer") targetRayMode = 1;
        else if(inputSource.targetRayMode == "screen") targetRayMode = 2;

        setValue(offset, id, 'i32');
        offset +=4;
        setValue(offset, handedness, 'i32');
        offset +=4;
        setValue(offset, targetRayMode, 'i32');
        offset +=4;

        return offset;
    },

    _set_input_callback__deps: ['$dynCall'],
    _set_input_callback: function(event, callback, userData) {
        var s = Module['webxr_session'];
        if(!s) return;
        if(!callback) return;

        s.addEventListener(event, function(e) {
            /* Nativize input source */
            var inputSource = Module._malloc(8); /* 2*sizeof(int32) */
            WebXR._nativize_input_source(inputSource, e.inputSource, i);

            /* Call native callback */
            dynCall('vii', callback, [inputSource, userData]);

            _free(inputSource);
        });
    },

    _set_session_callback__deps: ['$dynCall'],
    _set_session_callback: function(event, callback, userData) {
        var s = Module['webxr_session'];
        if(!s) return;
        if(!callback) return;

        s.addEventListener(event, function() {
            dynCall('vi', callback, [userData]);
        });
    }
},

webxr_init__deps: ['$dynCall'],
webxr_init: function(frameCallback, startSessionCallback, endSessionCallback, errorCallback, userData) {
    function onError(errorCode) {
        if(!errorCallback) return;
        dynCall('vii', errorCallback, [userData, errorCode]);
    };

    function onSessionEnd(mode) {
        if(!endSessionCallback) return;
        mode = {'inline': 0, 'immersive-vr': 1, 'immersive-ar': 2}[mode];
        dynCall('vii', endSessionCallback, [userData, mode]);
    };

    function onSessionStart(mode) {
        if(!startSessionCallback) return;
        mode = {'inline': 0, 'immersive-vr': 1, 'immersive-ar': 2}[mode];
        dynCall('vii', startSessionCallback, [userData, mode]);
    };

    const SIZE_OF_WEBXR_VIEW = (16 + 3 + 4 + 16 + 4)*4;
    const views = Module._malloc(SIZE_OF_WEBXR_VIEW*2 + (16 + 4 + 3)*4);

    function onFrame(time, frame) {
        if(!frameCallback) return;
        /* Request next frame */
        const session = frame.session;
        /* RAF is set to null on session end to avoid rendering */
        if(Module['webxr_session'] != null) session.requestAnimationFrame(onFrame);

        const pose = frame.getViewerPose(WebXR.refSpaces[WebXR.refSpace]);
        if(!pose) return;

        const glLayer = session.renderState.baseLayer;
        pose.views.forEach(function(view) {
            const viewport = glLayer.getViewport(view);
            let offset = views + SIZE_OF_WEBXR_VIEW*(view.eye == 'right' ? 1 : 0);
            offset = WebXR._nativize_rigid_transform(offset, view.transform);
            offset = WebXR._nativize_matrix(offset, view.projectionMatrix);

            setValue(offset + 0, viewport.x, 'i32');
            setValue(offset + 4, viewport.y, 'i32');
            setValue(offset + 8, viewport.width, 'i32');
            setValue(offset + 12, viewport.height, 'i32');
        });

        /* Model matrix */
        const modelMatrix = views + SIZE_OF_WEBXR_VIEW*2;
        WebXR._nativize_matrix(modelMatrix, pose.transform.matrix);

        /* If framebuffer is non-null, compositor is enabled and we bind it.
         * If it's null, we need to avoid this call otherwise the canvas FBO is bound */
        if(glLayer.framebuffer) {
            /* Make sure that FRAMEBUFFER_BINDING returns a valid value.
             * For that we create an id in the emscripten object tables
             * and add the frambuffer */
            const id = Module.webxr_fbo || GL.getNewId(GL.framebuffers);
            glLayer.framebuffer.name = id;
            GL.framebuffers[id] = glLayer.framebuffer;
            Module.webxr_fbo = id;
            Module.ctx.bindFramebuffer(Module.ctx.FRAMEBUFFER, glLayer.framebuffer);
        }

        /* Set and reset environment for webxr_get_input_pose calls */
        Module['webxr_frame'] = frame;
        dynCall('viiiii', frameCallback, [userData, time, modelMatrix, views, pose.views.length]);
        Module['webxr_frame'] = null;
    };

    function onSessionStarted(session, mode) {
        Module['webxr_session'] = session;

        // React to session ending
        session.addEventListener('end', function() {
            Module['webxr_session'].cancelAnimationFrame(WebXR._curRAF);
            WebXR._curRAF = null;
            Module['webxr_session'] = null;
            onSessionEnd(mode);
        });

        // Ensure our context can handle WebXR rendering
        Module.ctx.makeXRCompatible().then(function() {
            // Create the base layer
            const layer = Module['webxr_baseLayer'] = new window.XRWebGLLayer(session, Module.ctx, {
                framebufferScaleFactor: Module['webxr_framebuffer_scale_factor'],
            });
            session.updateRenderState({ baseLayer: layer });

            /* 'viewer' reference space is always available. */
            session.requestReferenceSpace('viewer').then(refSpace => {
                WebXR.refSpaces['viewer'] = refSpace;

                WebXR.refSpace = 'viewer';

                // Give application a chance to react to session starting
                // e.g. finish current desktop frame.
                onSessionStart(mode);

                // Start rendering
                session.requestAnimationFrame(onFrame);
            });

            /* Request and cache other available spaces, which may not be available */
            for(const s of ['local', 'local-floor', 'bounded-floor', 'unbounded']) {
                session.requestReferenceSpace(s).then(refSpace => {
                    /* We prefer the reference space automatically in above order */
                    WebXR.refSpace = s;

                    WebXR.refSpaces[s] = refSpace;
                }, function() { /* Leave refSpaces[s] unset. */ })
            }
        }, function() {
            onError(-3);
        });
    };

    if(navigator.xr) {
        Module['webxr_request_session_func'] = function(mode, requiredFeatures, optionalFeatures) {
            if(typeof(mode) !== 'string') {
                mode = (['inline', 'immersive-vr', 'immersive-ar'])[mode];
            }

            let toFeatureList = function(bitMask) {
                const f = [];
                const features = ['local', 'local-floor', 'bounded-floor', 'unbounded', 'hit-test'];
                for(let i = 0; i < features.length; ++i) {
                    if((bitMask & (1 << i)) != 0) {
                        f.push(features[i]);
                    }
                }
                return features;
            };
            if(typeof(requiredFeatures) === 'number') {
                requiredFeatures = toFeatureList(requiredFeatures);
            }
            if(typeof(optionalFeatures) === 'number') {
                optionalFeatures = toFeatureList(optionalFeatures);
            }
            navigator.xr.requestSession(mode, {
                requiredFeatures: requiredFeatures,
                optionalFeatures: optionalFeatures
            }).then(function(s) {
                onSessionStarted(s, mode);
            }).catch(console.error);
        };
    } else {
        /* Call error callback with "WebXR not supported" */
        onError(-2);
    }
},

webxr_is_session_supported__deps: ['$dynCall'],
webxr_is_session_supported: function(mode, callback) {
    if(!navigator.xr) {
        /* WebXR not supported at all */
        dynCall('vii', callback, [mode, 0]);
        return;
    }
    navigator.xr.isSessionSupported((['inline', 'immersive-vr', 'immersive-ar'])[mode]).then(function() {
        dynCall('vii', callback, [mode, 1]);
    }, function() {
        dynCall('vii', callback, [mode, 0]);
    });
},

webxr_request_session: function(mode) {
    var s = Module['webxr_request_session_func'];
    if(s) s(mode);
},

webxr_request_exit: function() {
    var s = Module['webxr_session'];
    if(s) Module['webxr_session'].end();
},

webxr_set_projection_params: function(near, far) {
    var s = Module['webxr_session'];
    if(!s) return;

    s.depthNear = near;
    s.depthFar = far;
},

webxr_set_session_blur_callback: function(callback, userData) {
    WebXR._set_session_callback("blur", callback, userData);
},

webxr_set_session_focus_callback: function(callback, userData) {
    WebXR._set_session_callback("focus", callback, userData);
},

webxr_set_select_callback: function(callback, userData) {
    WebXR._set_input_callback("select", callback, userData);
},
webxr_set_select_start_callback: function(callback, userData) {
    WebXR._set_input_callback("selectstart", callback, userData);
},
webxr_set_select_end_callback: function(callback, userData) {
    WebXR._set_input_callback("selectend", callback, userData);
},

webxr_get_input_sources: function(outArrayPtr, max, outCountPtr) {
    let s = Module['webxr_session'];
    if(!s) return; // TODO(squareys) warning or return error

    let i = 0;
    for (let inputSource of s.inputSources) {
        if(i >= max) break;
        outArrayPtr = WebXR._nativize_input_source(outArrayPtr, inputSource, i);
        ++i;
    }
    setValue(outCountPtr, i, 'i32');
},

webxr_get_input_pose: function(source, outPosePtr, space) {
    let f = Module['webxr_frame'];
    if(!f) {
        console.warn("Cannot call webxr_get_input_pose outside of frame callback");
        return false;
    }

    const id = getValue(source, 'i32');
    const input = Module['webxr_session'].inputSources[id];

    const s = space == 0 ? input.gripSpace : input.targetRaySpace;
    if(!s) return false;
    const pose = f.getPose(s, WebXR.refSpaces[WebXR.refSpace]);

    if(!pose || Number.isNaN(pose.transform.matrix[0])) return false;

    WebXR._nativize_rigid_transform(outPosePtr, pose.transform);

    return true;
},

};

autoAddDeps(LibraryWebXR, '$WebXR');
mergeInto(LibraryManager.library, LibraryWebXR);
