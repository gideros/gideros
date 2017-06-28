package com.giderosmobile.android.player;

    import javax.microedition.khronos.egl.EGL10;
    import javax.microedition.khronos.egl.EGLConfig;
    import javax.microedition.khronos.egl.EGLDisplay;

    import android.opengl.GLSurfaceView;
    import android.util.Log;


    public class GiderosConfigChooser implements GLSurfaceView.EGLConfigChooser {
        static private final String kTag = "Gideros";
        private int[] mValue;
        int numConfigs = 64;

        @Override
        public EGLConfig chooseConfig(EGL10 egl, EGLDisplay display) {
            mValue = new int[1];

            int[] configSpec = {
                    EGL10.EGL_RED_SIZE, 5,
                    EGL10.EGL_GREEN_SIZE, 6,
                    EGL10.EGL_BLUE_SIZE, 5,
                    EGL10.EGL_ALPHA_SIZE, 0,
                    EGL10.EGL_DEPTH_SIZE, 0,
                    EGL10.EGL_STENCIL_SIZE, 0,
                    EGL10.EGL_NONE
            };

            // Get all matching configurations.
            EGLConfig[] configs = new EGLConfig[numConfigs];
            if (!egl.eglChooseConfig(display, configSpec, configs, numConfigs,
                    mValue)) {
                throw new IllegalArgumentException("data eglChooseConfig failed");
            }
            numConfigs = mValue[0];
            Log.d(kTag, "eglChooseConfig returned " + numConfigs + "configuraions");



            // CAUTION! eglChooseConfigs returns configs with higher bit depth
            // first: Even though we asked for rgb565 configurations, rgb888
            // configurations are considered to be "better" and returned first.
            // You need to explicitly filter the data returned by eglChooseConfig!
            int index = -1;
            for (int i = 0; i < numConfigs; ++i) {
                int cr = findConfigAttrib(egl, display, configs[i], EGL10.EGL_RED_SIZE, 0);
                int cg = findConfigAttrib(egl, display, configs[i], EGL10.EGL_GREEN_SIZE, 0);
                int cb = findConfigAttrib(egl, display, configs[i], EGL10.EGL_BLUE_SIZE, 0);
                int ca = findConfigAttrib(egl, display, configs[i], EGL10.EGL_ALPHA_SIZE, 0);
                int cd = findConfigAttrib(egl, display, configs[i], EGL10.EGL_DEPTH_SIZE, 0);
                int cs = findConfigAttrib(egl, display, configs[i], EGL10.EGL_STENCIL_SIZE, 0);
                Log.i(kTag, "config " + i+ " R:"+cr+" G:"+cg+" B:"+cb+" A:"+ca+" D:"+cd+" S:"+cs);
                
                if ((cs>0)&&(cd>0)&&(index<0))
                {
                    Log.i(kTag, "Choosing config "+i);
                	index=i;
                }
            }
            if (index == -1) {
                Log.w(kTag, "Did not find sane config, using first (possibly 3D and Path2D won't work)");
            }

            EGLConfig config = numConfigs > 0 ? configs[index] : null;
            if (config == null) {
                throw new IllegalArgumentException("No config chosen");
            }
            return config;
        }

        private int findConfigAttrib(EGL10 egl, EGLDisplay display,
                                     EGLConfig config, int attribute, int defaultValue) {
            if (egl.eglGetConfigAttrib(display, config, attribute, mValue)) {
                return mValue[0];
            }
            return defaultValue;
        }



    }



