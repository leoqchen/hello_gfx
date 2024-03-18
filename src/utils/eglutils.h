#pragma once

int egl_CreateContext( void* nativeDisplayPtr, void* nativeWindowPtr );
void egl_SwapBuffers();
void egl_Terminate();