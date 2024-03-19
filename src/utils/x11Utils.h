#pragma once

typedef void (* xWindowResizeFunc)(int, int);

int xWindowCreate( void** nativeDisplayPtr, void** nativeWindowPtr, const char *title, int width, int height );
void xWindowSetTitle( const char *name );
int xWindowPoolEvents();
void xWindowDestroy();

void xWindowSetWindowResizeCallback( xWindowResizeFunc func );
