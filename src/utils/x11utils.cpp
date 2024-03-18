#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/time.h>

#include  <X11/Xlib.h>
#include  <X11/Xatom.h>
#include  <X11/Xutil.h>

#include "x11utils.h"

// X11 related local variables
static Display *x_display = NULL;
static Atom s_wmDeleteMessage;


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

    swa.event_mask  =  ExposureMask | PointerMotionMask | KeyPressMask;
    win = XCreateWindow(
               x_display, root,
               0, 0, width, height, 0,
               CopyFromParent, InputOutput,
               CopyFromParent, CWEventMask,
               &swa );
    s_wmDeleteMessage = XInternAtom(x_display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(x_display, win, &s_wmDeleteMessage, 1);

    xattr.override_redirect = False;
    XChangeWindowAttributes ( x_display, win, CWOverrideRedirect, &xattr );

    hints.input = True;
    hints.flags = InputHint;
    XSetWMHints(x_display, win, &hints);

    // make the window visible on the screen
    XMapWindow (x_display, win);
    XStoreName (x_display, win, title);

    // get identifiers for the provided atom name strings
    wm_state = XInternAtom (x_display, "_NET_WM_STATE", False);

    memset ( &xev, 0, sizeof(xev) );
    xev.type                 = ClientMessage;
    xev.xclient.window       = win;
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

    *nativeWindowPtr = (void*)win;
    *nativeDisplayPtr = (void*)x_display;
    return 1;
}

// Reads from X11 event loop and interrupt program if there is a keypress, or window close action.
int xWindowShouldClose()
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
            KeySym key;
            char text;
            if( XLookupString( &xev.xkey, &text, 1, &key, 0 ) == 1 )
            {
                //if (esContext->keyFunc != NULL)
                //    esContext->keyFunc(esContext, text, 0, 0);
            }
        }

        if( xev.type == ClientMessage ){
            if( xev.xclient.data.l[0] == s_wmDeleteMessage ){
                userinterrupt = 1;
            }
        }

        if( xev.type == DestroyNotify ) {
            userinterrupt = 1;
        }
    }
    return userinterrupt;
}
