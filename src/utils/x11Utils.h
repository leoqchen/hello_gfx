#pragma once

typedef void (* xWindowResizeFunc)(int, int);

int xWindowCreate( void** nativeDisplayPtr, void** nativeWindowPtr, const char *title, int width, int height );
void xWindowSetTitle( const char *name );
int xWindowPoolEvents();
void xWindowDestroy();
void xWindowGetSize( int *width, int *height );
void xWindowSetSize( int width, int height );

void xWindowSetWindowResizeCallback( xWindowResizeFunc func );
