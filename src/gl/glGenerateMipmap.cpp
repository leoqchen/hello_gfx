/*
 * 等价性测试：gluBuild2DMipmaps, glGenerateMipmap
 */

#if IS_GlEs
#include <glad/gles2.h>
#else
#include <glad/gl.h>
#include <GL/glu.h>
#endif

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <ctype.h>
#include "glutils.h"
#include "glfwutils.h"
#include "myutils.h"

// settings
static const int WinWidth = 320*2;
static const int WinHeight = 240*2;


const char *vertexShaderSource =
#if IS_GlEs
    "#version 320 es\n"
#else
    "#version 400\n"
#endif
    "uniform float u_offset;                    \n"
    "layout(location = 0) in vec4 a_position;   \n"
    "layout(location = 1) in vec2 a_texCoord;   \n"
    "out vec2 v_texCoord;                       \n"
    "void main()                                \n"
    "{                                          \n"
    "   gl_Position = a_position;               \n"
    "   gl_Position.x += u_offset;              \n"
    "   v_texCoord = a_texCoord;                \n"
    "}                                          \n";

const char *fragmentShaderSource =
#if IS_GlEs
    "#version 320 es\n"
    "precision mediump float;\n"
#else
    "#version 400\n"
#endif
    "in vec2 v_texCoord;                                 \n"
    "layout(location = 0) out vec4 outColor;             \n"
    "uniform sampler2D s_texture;                        \n"
    "void main()                                         \n"
    "{                                                   \n"
    "   outColor = texture( s_texture, v_texCoord );     \n"
    "}                                                   \n";


// Create a mipmapped 2D texture image
// ----------------------------------------
GLuint CreateMipMappedTexture2D()
{
    // Texture object handle
    GLuint textureId;
    int width = 256*4;
    int height = 256*4;
    int level;
    GLubyte *pixels;
    GLubyte *prevImage;
    GLubyte *newImage;

    pixels = GenerateCheckboard_RGB( width, height, 8 );
    if( pixels == NULL ){
        return 0;
    }

    // Generate a texture object
    glGenTextures( 1, &textureId );

    // Bind the texture object
    glBindTexture( GL_TEXTURE_2D, textureId );

    // Load mipmap level 0
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, width, height,
                  0, GL_RGB, GL_UNSIGNED_BYTE, pixels );
    printf("level=%d, width=%d, height=%d\n", 0, width, height);//XXX

#if IS_Cpu
    level = 1;
    prevImage = &pixels[0];
    while ( width > 1 && height > 1 )
    {
        int newWidth,
            newHeight;

        // Generate the next mipmap level
        GenerateNextLevelMipmap_RGB( prevImage, &newImage,
                                     width, height,
                                     &newWidth, &newHeight );

        // Load the mipmap level
        glTexImage2D( GL_TEXTURE_2D, level, GL_RGB,
                      newWidth, newHeight, 0, GL_RGB,
                      GL_UNSIGNED_BYTE, newImage );
        printf("level=%d, width=%d, height=%d\n", level, width, height);//XXX

        // Free the previous image
        free ( prevImage );

        // Set the previous image for the next iteration
        prevImage = newImage;
        level++;

        // Half the width and height
        width = newWidth;
        height = newHeight;
    }
    free ( newImage );
#elif IS_GlLegacy
    gluBuild2DMipmaps(GL_TEXTURE_2D, 4, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
#else
    glGenerateMipmap( GL_TEXTURE_2D );
#endif

    // Set the filtering mode
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    return textureId;
}


int main( int argc, const char* argv[] )
{
#if IS_GlEs
    api_t api = {.api = API_GLES, .major = 3, .minor = 2};
#elif IS_GlLegacy
    api_t api = {.api = API_GLLegacy, .major = 3, .minor = 0};
#else
    api_t api = {.api = API_GL, .major = 3, .minor = 3};
#endif
    if( argc >= 2 && isdigit(argv[1][0]) )
        api.major = argv[1][0] - '0';
    if( argc >= 3 && isdigit(argv[2][0]) )
        api.minor = argv[2][0] - '0';
    printf("command line: major = %d, minor = %d\n", api.major, api.minor);

    // glfw: initialize and configure
    // ------------------------------
    GLFWwindow* window = glfwInit_CreateWindow( api, WinWidth, WinHeight );

    // build and compile our shader program
    // ------------------------------------
    const GLuint program = CreateProgramFromSource( vertexShaderSource, fragmentShaderSource );

    // set up uniform data
    // ------------------------------------
    glUseProgram( program );
    GLint samplerLoc = glGetUniformLocation( program, "s_texture" );
    GLint offsetLoc = glGetUniformLocation( program, "u_offset" );
    glUniform1i( samplerLoc, 0 );

    // set up texture data and configure texture attributes
    // ------------------------------------------------------------------
    GLuint textureId = CreateMipMappedTexture2D();
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, textureId );

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    GLfloat vVertices[] = {
        -0.5f,  0.5f, 0.0f, 1.5f,     // Position 0
        0.0f,  0.0f,                          // TexCoord 0
        -0.5f, -0.5f, 0.0f, 0.75f,    // Position 1
        0.0f,  1.0f,                        // TexCoord 1
        0.5f, -0.5f, 0.0f, 0.75f, // Position 2
        1.0f,  1.0f,                        // TexCoord 2
        0.5f,  0.5f, 0.0f, 1.5f,  // Position 3
        1.0f,  0.0f,                       // TexCoord 3
    };
    GLushort indices[] = { 0, 1, 2, 0, 2, 3 };
    GLuint vertex_array;
    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);

    GLuint vertex_buffer;
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vVertices), vVertices, GL_STATIC_DRAW);

    const GLint vPos_location = glGetAttribLocation(program, "a_position");
    const GLint vTexCoord_location = glGetAttribLocation(program, "a_texCoord");
    printf("Attrib location: vPos=%d\n", vPos_location);
    printf("Attrib location: vTexCoord=%d\n", vTexCoord_location);
    glVertexAttribPointer(vPos_location, 4, GL_FLOAT, GL_FALSE,
                          sizeof(GLfloat) * 6, (void*)0);
    glVertexAttribPointer(vTexCoord_location, 2, GL_FLOAT, GL_FALSE,
                          sizeof(GLfloat) * 6, (void *) (sizeof(GLfloat) * 4));
    glEnableVertexAttribArray(vPos_location);
    glEnableVertexAttribArray(vTexCoord_location);

    // render loop
    // -----------
    glClearColor( 1.0f, 1.0f, 1.0f, 0.0f );
    while (!glfwWindowShouldClose(window))
    {
        // render
        // ------
        glClear(GL_COLOR_BUFFER_BIT );

        // Draw quad with nearest sampling
        glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        glUniform1f ( offsetLoc, -0.6f );
        glDrawElements ( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices );

        // Draw quad with trilinear filtering
        glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
        glUniform1f ( offsetLoc, 0.6f );
        glDrawElements ( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices );

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteTextures( 1, &textureId );
    glDeleteProgram(program);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
