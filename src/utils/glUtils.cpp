#include "glUtils.h"

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

const char* framebufferStatusName( GLenum status )
{
    switch( status ){
        case GL_FRAMEBUFFER_UNDEFINED:
            return "GL_FRAMEBUFFER_UNDEFINED";
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            return "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            return "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
        //case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
        //    return "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
        //case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
        //    return "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
        case GL_FRAMEBUFFER_UNSUPPORTED:
            return "GL_FRAMEBUFFER_UNSUPPORTED";
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
            return "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
        case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
            return "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS";
        default:
            return "";
    }
}

const char* glContextProfileBitName( GLint profileBit )
{
    switch( profileBit ){
#if !IS_GlEs
        case GL_CONTEXT_CORE_PROFILE_BIT:
            return "GL_CONTEXT_CORE_PROFILE_BIT";
        case GL_CONTEXT_COMPATIBILITY_PROFILE_BIT:
            return "GL_CONTEXT_COMPATIBILITY_PROFILE_BIT";
#endif
        default:
            return "";
    }
}

const char* glContextFlagName( GLint flag )
{
    switch( flag ){
#if !IS_GlEs
        case GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT:
            return "GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT";
#endif
        case GL_CONTEXT_FLAG_DEBUG_BIT:
            return "GL_CONTEXT_FLAG_DEBUG_BIT";
        case GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT:
            return "GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT";
        case GL_CONTEXT_FLAG_NO_ERROR_BIT:
            return "GL_CONTEXT_FLAG_NO_ERROR_BIT";
        default:
            return "";
    }
}

const char* glFormatName( GLenum format )
{
    switch( format ){
        case GL_RED: return "GL_RED";
        case GL_RED_INTEGER: return "GL_RED_INTEGER";
        case GL_RG: return "GL_RG";
        case GL_RG_INTEGER: return "GL_RG_INTEGER";
        case GL_RGB: return "GL_RGB";
        case GL_RGB_INTEGER: return "GL_RGB_INTEGER";
        case GL_RGBA: return "GL_RGBA";
        case GL_RGBA_INTEGER: return "GL_RGBA_INTEGER";
        case GL_DEPTH_COMPONENT: return "GL_DEPTH_COMPONENT";
        case GL_DEPTH_STENCIL: return "GL_DEPTH_STENCIL";
        case GL_LUMINANCE_ALPHA: return "GL_LUMINANCE_ALPHA";
        case GL_LUMINANCE: return "GL_LUMINANCE";
        case GL_ALPHA: return "GL_ALPHA";
        case GL_R8: return "GL_R8";
        case GL_R8_SNORM: return "GL_R8_SNORM";
        case GL_R16F: return "GL_R16F";
        case GL_R32F: return "GL_R32F";
        case GL_R8UI: return "GL_R8UI";
        case GL_R8I: return "GL_R8I";
        case GL_R16UI: return "GL_R16UI";
        case GL_R16I: return "GL_R16I";
        case GL_R32UI: return "GL_R32UI";
        case GL_R32I: return "GL_R32I";
        case GL_RG8: return "GL_RG8";
        case GL_RG8_SNORM: return "GL_RG8_SNORM";
        case GL_RG16F: return "GL_RG16F";
        case GL_RG32F: return "GL_RG32F";
        case GL_RG8UI: return "GL_RG8UI";
        case GL_RG8I: return "GL_RG8I";
        case GL_RG16UI: return "GL_RG16UI";
        case GL_RG16I: return "GL_RG16I";
        case GL_RG32UI: return "GL_RG32UI";
        case GL_RG32I: return "GL_RG32I";
        case GL_RGB8: return "GL_RGB8";
        case GL_SRGB8: return "GL_SRGB8";
        case GL_RGB565: return "GL_RGB565";
        case GL_RGB8_SNORM: return "GL_RGB8_SNORM";
        case GL_R11F_G11F_B10F: return "GL_R11F_G11F_B10F";
        case GL_RGB9_E5: return "GL_RGB9_E5";
        case GL_RGB16F: return "GL_RGB16F";
        case GL_RGB32F: return "GL_RGB32F";
        case GL_RGB8UI: return "GL_RGB8UI";
        case GL_RGB8I: return "GL_RGB8I";
        case GL_RGB16UI: return "GL_RGB16UI";
        case GL_RGB16I: return "GL_RGB16I";
        case GL_RGB32UI: return "GL_RGB32UI";
        case GL_RGB32I: return "GL_RGB32I";
        case GL_RGBA8: return "GL_RGBA8";
        case GL_SRGB8_ALPHA8: return "GL_SRGB8_ALPHA8";
        case GL_RGBA8_SNORM: return "GL_RGBA8_SNORM";
        case GL_RGB5_A1: return "GL_RGB5_A1";
        case GL_RGBA4: return "GL_RGBA4";
        case GL_RGB10_A2: return "GL_RGB10_A2";
        case GL_RGBA16F: return "GL_RGBA16F";
        case GL_RGBA32F: return "GL_RGBA32F";
        case GL_RGBA8UI: return "GL_RGBA8UI";
        case GL_RGBA8I: return "GL_RGBA8I";
        case GL_RGB10_A2UI: return "GL_RGB10_A2UI";
        case GL_RGBA16UI: return "GL_RGBA16UI";
        case GL_RGBA16I: return "GL_RGBA16I";
        case GL_RGBA32I: return "GL_RGBA32I";
        case GL_RGBA32UI: return "GL_RGBA32UI";
        case GL_DEPTH_COMPONENT16: return "GL_DEPTH_COMPONENT16";
        case GL_DEPTH_COMPONENT24: return "GL_DEPTH_COMPONENT24";
        case GL_DEPTH_COMPONENT32F: return "GL_DEPTH_COMPONENT32F";
        case GL_DEPTH24_STENCIL8: return "GL_DEPTH24_STENCIL8";
        case GL_DEPTH32F_STENCIL8: return "GL_DEPTH32F_STENCIL8";
        default: return "";
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

GLuint CreateTexture_FillWithCheckboard( GLsizei width, GLsizei height )
{
    GLubyte *img = GenerateCheckboard_RGBA( width, height, 8 );

    const GLenum filter = GL_NEAREST;
    GLuint obj;
    glGenTextures(1, &obj);
    glBindTexture(GL_TEXTURE_2D, obj);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, img);
    free(img);

    return obj;
}

void MatrixPrint( GLenum pname, const char *file, int line )
{
    GLfloat matrix[16];
    glGetFloatv (pname, matrix);

    printf("%s: %s:%d\n", __func__, file, line);
    for( int i=0; i < 16; i+=4 ){
        printf("%f %f %f %f\n", matrix[i+0], matrix[i+1], matrix[i+2], matrix[i+3]);
    }
    printf("\n");
}

void ReadPixels_FromFboColorAttachment( void *dstData, GLuint texture, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type )
{
    GLint originalFBO = 0;
    glGetIntegerv( GL_FRAMEBUFFER_BINDING, &originalFBO );

    // create FBO, and its color attachment
    GLuint fbo;
    glGenFramebuffers( 1, &fbo );
    glBindFramebuffer( GL_FRAMEBUFFER, fbo );
    glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0 );
    GLenum stat = glCheckFramebufferStatus( GL_FRAMEBUFFER );
    if( stat != GL_FRAMEBUFFER_COMPLETE ){
        printf("%s: Error: incomplete FBO!: 0x%X, %s\n", __func__, stat, framebufferStatusName(stat));
        exit(1);
    }

    // read pixels from FBO color attachment
    glReadBuffer( GL_COLOR_ATTACHMENT0 );
    glReadPixels(0, 0, width, height, format, type, dstData );

    // Restore the original framebuffer
    glDeleteFramebuffers( 1, &fbo );
    glBindFramebuffer ( GL_FRAMEBUFFER, originalFBO );
}

