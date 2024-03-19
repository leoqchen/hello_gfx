#include "glad.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <stdio.h>

#include "eglUtils.h"
#include "x11Utils.h"

static EGLNativeDisplayType eglNativeDisplay = NULL;
static EGLNativeWindowType  eglNativeWindow = NULL;
static EGLDisplay eglDisplay = EGL_NO_DISPLAY;
static EGLContext eglContext = EGL_NO_CONTEXT;
static EGLSurface eglSurface = EGL_NO_SURFACE;
static int shouldClose = 0;

int egl_CreateContext( api_t api, void* nativeDisplayPtr, void* nativeWindowPtr )
{
    eglNativeWindow = (EGLNativeWindowType) nativeWindowPtr;
    eglNativeDisplay = (EGLNativeDisplayType) nativeDisplayPtr;

    eglDisplay = eglGetDisplay( eglNativeDisplay );
    if( eglDisplay == EGL_NO_DISPLAY ){
        printf("%s: eglGetDisplay() fail\n", __func__);
        return 0;
    }

    // Initialize EGL
    // --------------------
    EGLint majorVersion;
    EGLint minorVersion;
    if( !eglInitialize ( eglDisplay, &majorVersion, &minorVersion )){
        printf("%s: eglInitialize() fail\n", __func__);
        return 0;
    }

    // Choose config
    // --------------------
    EGLint numConfigs = 0;
    EGLint attribList[] = {
        EGL_RED_SIZE,       8,
        EGL_GREEN_SIZE,     8,
        EGL_BLUE_SIZE,      8,
        EGL_ALPHA_SIZE,     8,
        EGL_DEPTH_SIZE,     24,
        EGL_STENCIL_SIZE,   8,
        EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
        EGL_CONFORMANT, EGL_OPENGL_ES3_BIT_KHR,
        EGL_NONE
    };
    if( api.api == API_GLLegacy || api.api == API_GL ){
        attribList[15] = EGL_OPENGL_BIT;
        attribList[17] = EGL_OPENGL_BIT;
    }
    EGLConfig config;
    if( !eglChooseConfig( eglDisplay, attribList, &config, 1, &numConfigs )
        || numConfigs < 1 )
    {
        printf("%s: eglChooseConfig() fail\n", __func__);
        return 0;
    }

    // Create a surface
    // --------------------
    eglSurface = eglCreateWindowSurface ( eglDisplay, config, eglNativeWindow, NULL );
    if( eglSurface == EGL_NO_SURFACE ){
        printf("%s: eglCreateWindowSurface() fail\n", __func__);
        return 0;
    }

    // Create a GL context
    // --------------------
    if( api.api == API_GLLegacy || api.api == API_GL ){
        eglBindAPI( EGL_OPENGL_API );
    }else{
        eglBindAPI( EGL_OPENGL_ES_API );
    }
    EGLint contextAttribs[8] = {
        EGL_CONTEXT_MAJOR_VERSION, api.major,
        EGL_CONTEXT_MINOR_VERSION, api.minor,
        EGL_NONE
    };
    if( api.api == API_GLLegacy || api.api == API_GL ){
        contextAttribs[4] = EGL_CONTEXT_OPENGL_PROFILE_MASK;
        contextAttribs[5] = EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT;
        contextAttribs[6] = EGL_NONE;
    }
    eglContext = eglCreateContext ( eglDisplay, config, EGL_NO_CONTEXT, contextAttribs );
    if( eglContext == EGL_NO_CONTEXT ){
        printf("%s: eglCreateContext() fail\n", __func__);
        return 0;
    }

    // Make the context current
    // --------------------
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

void egl_Terminate()
{
    eglDestroyContext(eglDisplay, eglContext);
    eglDestroySurface(eglDisplay, eglSurface);
    eglTerminate(eglDisplay);
}


static void window_resize_callback( int width, int height )
{
    glViewport( 0, 0, width, height );
}

void eglx_CreateWindow(api_t api, int width, int height )
{
    // X11 create window
    // --------------------
    void* nativeDisplayPtr;
    void* nativeWindowPtr;
    xWindowCreate( &nativeDisplayPtr, &nativeWindowPtr, "", width, height );

    // EGL create context
    // --------------------
    egl_CreateContext( api, nativeDisplayPtr, nativeWindowPtr );

    // glad loader
    // --------------------
#if IS_GlEs
    int version = gladLoadGLES2(eglGetProcAddress);
#else
    int version = gladLoadGL(eglGetProcAddress);
#endif
    printf("%s: glad load version: %d.%d\n", __func__, GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

    // some queries
    // --------------------
    printf("%s: GL_VENDER = %s\n", __func__, glGetString(GL_VENDOR));
    printf("%s: GL_RENDERER = %s\n", __func__, glGetString(GL_RENDERER));
    printf("%s: GL_VERSION = %s\n", __func__, glGetString(GL_VERSION));
    printf("%s: GL_SHADING_LANGUAGE_VERSION = %s\n", __func__, glGetString(GL_SHADING_LANGUAGE_VERSION));

    GLint contextFlag;
    glGetIntegerv( GL_CONTEXT_FLAGS, &contextFlag );
    printf("%s: GL_CONTEXT_FLAGS = 0x%x\n", __func__, contextFlag);
#if IS_GlLegacy
    GLint profileBit;
    glGetIntegerv( GL_CONTEXT_PROFILE_MASK, &profileBit );
    printf("%s: GL_CONTEXT_PROFILE_MASK = %s\n", __func__, glContextProfileBitName(profileBit));
#endif

    printf("\n");
    xWindowSetTitle( (const char*)glGetString(GL_VERSION) );
    xWindowSetWindowResizeCallback( window_resize_callback );
}

void eglx_Terminate()
{
    egl_Terminate();
    xWindowDestroy();
}

void eglx_SwapBuffers()
{
    egl_SwapBuffers();
}

int eglx_ShouldClose()
{
    eglx_PollEvents();
    return shouldClose;
}

void eglx_PollEvents()
{
    shouldClose |= xWindowPoolEvents();
}
