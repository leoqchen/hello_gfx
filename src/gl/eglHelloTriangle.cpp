#if IS_GlEs
#include <GLES3/gl32.h>
#else
#include <GL/gl.h>
#endif

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include "glutils.h" //TODO
#include "myutils.h"
#include "x11utils.h"
#include "eglutils.h"

//----------------------- //TODO
#define glErrorCheck() \
    {\
      GLenum error;\
      if( (error = glGetError()) != GL_NO_ERROR ){\
        printf("glGetError: %s, %s:%s:%d\n", glErrorName(error), __func__, __FILE__, __LINE__);\
        exit(EXIT_FAILURE);\
      }\
    }

const char* glErrorName( GLenum error )
{
    switch( error ){
        case GL_NO_ERROR:
            return "GL_NO_ERROR";
        case GL_INVALID_ENUM:
            return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE:
            return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION:
            return "GL_INVALID_OPERATION";
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            return "GL_INVALID_FRAMEBUFFER_OPERATION";
        case GL_OUT_OF_MEMORY:
            return "GL_OUT_OF_MEMORY";
        case GL_STACK_UNDERFLOW:
            return "GL_STACK_UNDERFLOW";
        case GL_STACK_OVERFLOW:
            return "GL_STACK_OVERFLOW";
        default:
            return "";
    }
}

static const char* glShaderTypeName( GLenum shaderType )
{
    switch( shaderType ){
        case GL_COMPUTE_SHADER:
            return "GL_COMPUTE_SHADER";
        case GL_VERTEX_SHADER:
            return "GL_VERTEX_SHADER";
        case GL_TESS_CONTROL_SHADER:
            return "GL_TESS_CONTROL_SHADER";
        case GL_TESS_EVALUATION_SHADER:
            return "GL_TESS_EVALUATION_SHADER";
        case GL_GEOMETRY_SHADER:
            return "GL_GEOMETRY_SHADER";
        case GL_FRAGMENT_SHADER:
            return "GL_FRAGMENT_SHADER";
        default:
            return "";
    }
}

static GLuint CreateShaderFromSource( GLenum type, const char *shaderSource )
{
    GLuint shader = glCreateShader( type );
    glShaderSource( shader, 1, &shaderSource, NULL );
    glCompileShader( shader );

    // Check the compile status
    GLint success;
    glGetShaderiv( shader, GL_COMPILE_STATUS, &success );
    if( !success ){
        GLint infoLen = 0;
        glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &infoLen );
        if( infoLen > 0 ){
            char *infoLog = (char*)malloc( sizeof(char) * infoLen );
            glGetShaderInfoLog ( shader, infoLen, NULL, infoLog );
            printf("ERROR: shader compile failed, shaderType = %s\n%s\n", glShaderTypeName(type), infoLog);
            free( infoLog );
        }

        glDeleteShader( shader );
        exit(EXIT_FAILURE);
        //return 0;
    }

    return shader;
}

static GLuint CreateProgramFromShader( GLuint vertShader, GLuint fragShader )
{
    GLuint program = glCreateProgram();
    glAttachShader( program, vertShader );
    glAttachShader( program, fragShader );
    glLinkProgram ( program );

    // Check the link status
    GLint success;
    glGetProgramiv ( program, GL_LINK_STATUS, &success );
    if( !success ){
        GLint infoLen = 0;
        glGetProgramiv ( program, GL_INFO_LOG_LENGTH, &infoLen );
        if( infoLen > 0 ){
            char *infoLog = (char*)malloc ( sizeof ( char ) * infoLen );
            glGetProgramInfoLog ( program, infoLen, NULL, infoLog );
            printf("ERROR: programe link failed\n%s\n", infoLog);
            free ( infoLog );
        }

        glDeleteProgram ( program );
        exit(EXIT_FAILURE);
        //return 0;
    }

    return program;
}

static GLuint CreateProgramFromSource( const char *vertShaderSource, const char *fragShaderSource )
{
    GLuint vertShader = CreateShaderFromSource( GL_VERTEX_SHADER, vertShaderSource );
    GLuint fragShader = CreateShaderFromSource( GL_FRAGMENT_SHADER, fragShaderSource );
    GLuint program = CreateProgramFromShader( vertShader, fragShader );

    glDeleteShader( vertShader );
    glDeleteShader( fragShader );

    return program;
}
//-------------------

// settings
static const int WinWidth = 800;
static const int WinHeight = 600;

const char *vertexShaderSource =
#if IS_GlEs
    "#version 320 es\n"
#else
    "#version 400\n"
#endif
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\n\0";

const char *fragmentShaderSource =
#if IS_GlEs
    "#version 320 es\n"
    "precision mediump float;\n"
#else
    "#version 400\n"
#endif
    "layout (location = 0) out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    "}\n\0";


int main( int argc, const char* argv[] )
{
    api_t api = apiInitial( API_Current, argc, argv );
    printf("%s: %s\n", argv[0], apiName(api));

    // glfw: initialize and configure
    // ------------------------------
//    GLFWwindow* window = glfwInit_CreateWindow( api, WinWidth, WinHeight );
    void* nativeDisplayPtr;
    void* nativeWindowPtr;
    xWindowCreate( &nativeDisplayPtr, &nativeWindowPtr, "", WinWidth, WinHeight );
    egl_CreateContext( nativeDisplayPtr, nativeWindowPtr );
    printf("%s: GL_VENDER = %s\n", __func__, glGetString(GL_VENDOR));
    printf("%s: GL_RENDERER = %s\n", __func__, glGetString(GL_RENDERER));
    printf("%s: GL_VERSION = %s\n", __func__, glGetString(GL_VERSION));
    printf("%s: GL_SHADING_LANGUAGE_VERSION = %s\n", __func__, glGetString(GL_SHADING_LANGUAGE_VERSION));

    // some query
    int major_version = 0;
    int minor_version = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &major_version);
    glGetIntegerv(GL_MINOR_VERSION, &minor_version);
    printf("GL_MAJOR_VERSION = %d\n", major_version);
    printf("GL_MINOR_VERSION = %d\n", minor_version);

#if IS_GlLegacy
    printf("GL_EXTENSIONS = %s\n", glGetString(GL_EXTENSIONS)); // GL_EXTENSIONS is removed from core OpenGL 3.1 aand above
#endif

    int num_extensions = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);
    printf("GL_NUM_EXTENSIONS = %d\n", num_extensions);
    for( int i=0; i < num_extensions; i++ ){
        printf("%s ", glGetStringi(GL_EXTENSIONS, i));
    }
    printf("\n");

    // build and compile our shader program
    // ------------------------------------
    const GLuint program = CreateProgramFromSource( vertexShaderSource, fragmentShaderSource );

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
        -0.5f, -0.5f, 0.0f, // left  
         0.5f, -0.5f, 0.0f, // right 
         0.0f,  0.5f, 0.0f  // top   
    };

    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    const GLint apos_location = glGetAttribLocation(program, "aPos");
    printf("Uniform location: aPos=%d\n", apos_location);
    glVertexAttribPointer(apos_location, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray( apos_location );

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0);


    // uncomment this call to draw in wireframe polygons.
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
//    while(!glfwWindowShouldClose(window)) //TODO
    while( 1 )
    {
        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // draw our first triangle
        glUseProgram(program);
        glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
        glDrawArrays(GL_TRIANGLES, 0, 3);
        // glBindVertexArray(0); // no need to unbind it every time 
 
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
//        glfwSwapBuffers(window);
//        glfwPollEvents();
        egl_SwapBuffers();

        // dump to disk
        // ------------
        static int i = 0;
        if( i == 0 ){
            i = 1;

            glFinish();
            GLubyte* pixels = (GLubyte*)malloc( WinWidth * WinHeight * 4 );
            glReadPixels(0, 0, WinWidth, WinHeight, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

            // Write image Y-flipped because OpenGL
            stbi_flip_vertically_on_write( 1 );
            stbi_write_png("/tmp/1.png", WinWidth, WinHeight, 4, pixels, WinWidth*4);
            printf("dump to /tmp/1.png\n");

            free( pixels );
        }
    }
    glErrorCheck();

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(program);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
//    glfwDestroyWindow(window);
//    glfwTerminate();
    return 0;
}
