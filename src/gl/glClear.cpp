#include <stdio.h>
#include <ctype.h>
#include "glad.h"
#include "eglUtils.h"
#include "glUtils.h"
#include "myUtils.h"


// settings
static const int WinWidth = 800;
static const int WinHeight = 600;


int main( int argc, const char* argv[] )
{
    api_t api = apiInitial( API_Current, argc, argv );
    printf("%s: %s\n", argv[0], apiName(api));

    int __frame = integerFromArgs("--f", argc, argv, NULL );

    // initialize and configure
    // ------------------------------
    eglx_CreateWindow( api, WinWidth, WinHeight );

    // render loop
    // -----------
    int i = 0;
    int frame = 0;
    while (!eglx_ShouldClose())
    {
        // render
        // ------
        frame++;
        if( frame % 6 == 0 ){
            if( ++i >= 1000 ){
                i = 0;
            }
        }
        float blue = (float)((i % 10) + 1) * 0.1f;
        float green = (float)(((i/10) % 10) + 1) * 0.1f;
        float red = (float)(((i/100) % 10) + 1) * 0.1f;
        glClearColor(red, green, blue, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        eglx_SwapBuffers();
        eglx_PollEvents();

        if( __frame != -1 && frame >= __frame ){
            printf("frame count: %d\n", frame);
            break;
        }
    }
    glErrorCheck();

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------

    // terminate, clearing all previously allocated resources.
    // ------------------------------------------------------------------
    eglx_Terminate();
    return 0;
}
