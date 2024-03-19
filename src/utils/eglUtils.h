/*
 * glad + EGL + X11 glue codes
 */
#pragma once

#include "myUtils.h"

// EGL
int egl_CreateContext( api_t api, void* nativeDisplayPtr, void* nativeWindowPtr );
void egl_SwapBuffers();
void egl_Terminate();

// EGL + X11
void eglx_CreateWindow(api_t api, int width, int height );
void eglx_Terminate();
void eglx_SwapBuffers();
int eglx_ShouldClose();
void eglx_PollEvents();
void eglx_GetWindowSize( int *width, int *height );
void eglx_SetWindowSize( int width, int height );
