#include "glutils.h"

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

const char* glShaderTypeName( GLenum shaderType )
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

const char* glslVersion( api_t api )
{
    // https://en.wikipedia.org/wiki/OpenGL_Shading_Language

    if( api.api == API_GLLegacy || api.api == API_GL ){
        if( api.major == 2 || api.minor == 0 )
            return "#version 110";
        else if( api.major == 2 || api.minor == 1 )
            return "#version 120";
        else if( api.major == 3 || api.minor == 0 )
            return "#version 130";
        else if( api.major == 3 || api.minor == 1 )
            return "#version 140";
        else if( api.major == 3 || api.minor == 2 )
            return "#version 150";
        else if( api.major == 3 || api.minor == 3 )
            return "#version 330";
        else if( api.major == 4 || api.minor == 0 )
            return "#version 400";
        else if( api.major == 4 || api.minor == 1 )
            return "#version 410";
        else if( api.major == 4 || api.minor == 2 )
            return "#version 420";
        else if( api.major == 4 || api.minor == 3 )
            return "#version 430";
        else if( api.major == 4 || api.minor == 4 )
            return "#version 440";
        else if( api.major == 4 || api.minor == 5 )
            return "#version 450";
        else if( api.major == 4 || api.minor == 6 )
            return "#version 460";
    }
    else if( api.api == API_GLES ){
        if( api.major == 2 || api.minor == 0 )
            return "#version 110";
        else if( api.major == 3 || api.minor == 0 )
            return "#version 300 es";
        else if( api.major == 3 || api.minor == 1 )
            return "#version 310 es";
        else if( api.major == 3 || api.minor == 2 )
            return "#version 320 es";
    }

    printf("%s: invalid api settings: api = %d, major = %d, minor = %d\n", __func__, api.api, api.major, api.minor);
    exit(EXIT_FAILURE);
    return "";
}

GLuint CreateShaderFromSource( GLenum type, const char *shaderSource )
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

GLuint CreateProgramFromShader( GLuint vertShader, GLuint fragShader )
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

GLuint CreateProgramFromSource( const char *vertShaderSource, const char *fragShaderSource )
{
    GLuint vertShader = CreateShaderFromSource( GL_VERTEX_SHADER, vertShaderSource );
    GLuint fragShader = CreateShaderFromSource( GL_FRAGMENT_SHADER, fragShaderSource );
    GLuint program = CreateProgramFromShader( vertShader, fragShader );

    glDeleteShader( vertShader );
    glDeleteShader( fragShader );

    return program;
}
