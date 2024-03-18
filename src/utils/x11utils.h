#pragma once

int xWindowCreate( void** nativeDisplayPtr, void** nativeWindowPtr, const char *title, int width, int height );
int xWindowShouldClose();
