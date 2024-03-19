#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/time.h>

#include  <X11/Xlib.h>
#include  <X11/Xatom.h>
#include  <X11/Xutil.h>

#include "x11Utils.h"

// X11 related local variables
static Display *x_display = NULL;
static Window x_win = NULL;
static Atom s_wmDeleteMessage;
static xWindowResizeFunc resizeCallback = NULL;

// This function initialized the native X11 display and window
int xWindowCreate( void** nativeDisplayPtr, void** nativeWindowPtr, const char *title, int width, int height )
{
    Window root;
    XSetWindowAttributes swa;
    XSetWindowAttributes  xattr;
    Atom wm_state;
    XWMHints hints;
    XEvent xev;
    Window win;

    /*
     * X11 native display initialization
     */

    x_display = XOpenDisplay(NULL);
    if ( x_display == NULL )
    {
        *nativeWindowPtr = NULL;
        *nativeDisplayPtr = NULL;
        printf("%s: XOpenDisplay() fail\n", __func__);
        return 0;
    }

    root = DefaultRootWindow(x_display);

    swa.event_mask  =  ExposureMask | PointerMotionMask | KeyPressMask | StructureNotifyMask;
    x_win = XCreateWindow(
               x_display, root,
               0, 0, width, height, 0,
               CopyFromParent, InputOutput,
               CopyFromParent, CWEventMask,
               &swa );
    s_wmDeleteMessage = XInternAtom(x_display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(x_display, x_win, &s_wmDeleteMessage, 1);

    xattr.override_redirect = False;
    XChangeWindowAttributes ( x_display, x_win, CWOverrideRedirect, &xattr );

    hints.input = True;
    hints.flags = InputHint;
    XSetWMHints(x_display, x_win, &hints);

    // make the window visible on the screen
    XMapWindow (x_display, x_win);
    XStoreName (x_display, x_win, title);

    // get identifiers for the provided atom name strings
    wm_state = XInternAtom (x_display, "_NET_WM_STATE", False);

    memset ( &xev, 0, sizeof(xev) );
    xev.type                 = ClientMessage;
    xev.xclient.window       = x_win;
    xev.xclient.message_type = wm_state;
    xev.xclient.format       = 32;
    xev.xclient.data.l[0]    = 1;
    xev.xclient.data.l[1]    = False;
    XSendEvent (
       x_display,
       DefaultRootWindow ( x_display ),
       False,
       SubstructureNotifyMask,
       &xev );

    *nativeWindowPtr = (void*)x_win;
    *nativeDisplayPtr = (void*)x_display;
    return 1;
}

void xWindowSetTitle( const char *name )
{
    if( x_display )
        XStoreName( x_display, x_win, name );
}

// Reads from X11 event loop and interrupt program if there is a keypress, or window close action.
int xWindowPoolEvents()
{
    if( x_display == NULL )
        return 0;

    // Pump all messages from X server. Keypresses are directed to keyfunc (if defined)
    int userinterrupt = 0;
    while( XPending( x_display ))
    {
        XEvent xev;
        XNextEvent( x_display, &xev );

        if( xev.type == KeyPress )
        {
            //KeySym key;
            //char text;
            //if( XLookupString( &xev.xkey, &text, 1, &key, 0 ) == 1 )
            //{
                //if (esContext->keyFunc != NULL)
                //    esContext->keyFunc(esContext, text, 0, 0);
            //}

            //printf("keycode = %u\n", xev.xkey.keycode);
            if( xev.xkey.keycode == 9 ) //Escape
                userinterrupt = 1;
        }
        else if( xev.type == ClientMessage ){
            if( xev.xclient.data.l[0] == s_wmDeleteMessage ){
                userinterrupt = 1;
            }
        }
        else if( xev.type == DestroyNotify ) {
            userinterrupt = 1;
        }
        else if( xev.type == ConfigureNotify ){
            XConfigureEvent xce = xev.xconfigure;
            if( resizeCallback != NULL )
                resizeCallback( xce.width, xce.height );
        }
    }
    return userinterrupt;
}

void xWindowDestroy()
{
    if( x_display && x_win )
        XDestroyWindow( x_display, x_win );

    if( x_display )
        XCloseDisplay( x_display );
}

void xWindowSetWindowResizeCallback( xWindowResizeFunc func )
{
    resizeCallback = func;
}
