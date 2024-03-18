#include <stdio.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "eglutils.h"

static EGLNativeDisplayType eglNativeDisplay = NULL;
static EGLNativeWindowType  eglNativeWindow = NULL;
static EGLDisplay eglDisplay = EGL_NO_DISPLAY;
static EGLContext eglContext = EGL_NO_CONTEXT;
static EGLSurface eglSurface = EGL_NO_SURFACE;

int egl_CreateContext( void* nativeDisplayPtr, void* nativeWindowPtr )
{
    eglNativeWindow = (EGLNativeWindowType) nativeWindowPtr;
    eglNativeDisplay = (EGLNativeDisplayType) nativeDisplayPtr;

    eglDisplay = eglGetDisplay( eglNativeDisplay );
    if( eglDisplay == EGL_NO_DISPLAY ){
        printf("%s: eglGetDisplay() fail\n", __func__);
        return 0;
    }

    // Initialize EGL
    EGLint majorVersion;
    EGLint minorVersion;
    if( !eglInitialize ( eglDisplay, &majorVersion, &minorVersion )){
        printf("%s: eglInitialize() fail\n", __func__);
        return 0;
    }

    // Choose config
    EGLint numConfigs = 0;
    EGLint attribList[] = {
        EGL_RED_SIZE,       8,
        EGL_GREEN_SIZE,     8,
        EGL_BLUE_SIZE,      8,
        EGL_ALPHA_SIZE,     8,
        EGL_DEPTH_SIZE,     24,
        EGL_STENCIL_SIZE,   8,
        //EGL_SAMPLE_BUFFERS, 1,
        EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR, //TODO EGL_OPENGL_BIT
        EGL_NONE
    };
    EGLConfig config;
    if( !eglChooseConfig( eglDisplay, attribList, &config, 1, &numConfigs )
        || numConfigs < 1 )
    {
        printf("%s: eglChooseConfig() fail\n", __func__);
        return 0;
    }

    // Create a surface
    eglSurface = eglCreateWindowSurface ( eglDisplay, config, eglNativeWindow, NULL );
    if( eglSurface == EGL_NO_SURFACE ){
        printf("%s: eglCreateWindowSurface() fail\n", __func__);
        return 0;
    }

    // Create a GL context
    eglBindAPI( EGL_OPENGL_ES_API ); //TODO: EGL_OPENGL_API
    EGLint contextAttribs[] = {
        EGL_CONTEXT_MAJOR_VERSION, 3,
        EGL_CONTEXT_MINOR_VERSION, 2, //TODO: OpenGL 3.3
        EGL_NONE
    };
    eglContext = eglCreateContext ( eglDisplay, config, EGL_NO_CONTEXT, contextAttribs );
    if( eglContext == EGL_NO_CONTEXT ){
        printf("%s: eglCreateContext() fail\n", __func__);
        return 0;
    }

    // Make the context current
    if( !eglMakeCurrent ( eglDisplay, eglSurface,eglSurface, eglContext )){
        printf("%s: eglMakeCurrent() fail\n", __func__);
        return 0;
    }

    return 1;
}

void egl_SwapBuffers()
{
    eglSwapBuffers( eglDisplay, eglSurface );
}
