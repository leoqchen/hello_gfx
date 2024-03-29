#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SGI_rgb.h"


/*
** RGB Image Structure
*/

typedef struct _TK_RGBImageRec {
    GLint sizeX, sizeY;
    GLint components;
    unsigned char *data;
} TK_RGBImageRec;



/******************************************************************************/

typedef struct _rawImageRec {
    unsigned short imagic;
    unsigned short type;
    unsigned short dim;
    unsigned short sizeX, sizeY, sizeZ;
    unsigned long min, max;
    unsigned long wasteBytes;
    char name[80];
    unsigned long colorMap;
    FILE *file;
    unsigned char *tmp, *tmpR, *tmpG, *tmpB, *tmpA;
    unsigned long rleEnd;
    GLuint *rowStart;
    GLint *rowSize;
} rawImageRec;

/******************************************************************************/

static void ConvertShort(unsigned short *array, long length)
{
    unsigned long b1, b2;
    unsigned char *ptr;

    ptr = (unsigned char *)array;
    while (length--) {
        b1 = *ptr++;
        b2 = *ptr++;
        *array++ = (unsigned short) ((b1 << 8) | (b2));
    }
}

static void ConvertLong(GLuint *array, long length)
{
    unsigned long b1, b2, b3, b4;
    unsigned char *ptr;

    ptr = (unsigned char *)array;
    while (length--) {
        b1 = *ptr++;
        b2 = *ptr++;
        b3 = *ptr++;
        b4 = *ptr++;
        *array++ = (b1 << 24) | (b2 << 16) | (b3 << 8) | (b4);
    }
}

static rawImageRec *RawImageOpen(const char *fileName)
{
    union {
        int testWord;
        char testByte[4];
    } endianTest;
    rawImageRec *raw;
    GLenum swapFlag;
    int x;
    size_t result;

    endianTest.testWord = 1;
    if (endianTest.testByte[0] == 1) {
        swapFlag = GL_TRUE;
    } else {
        swapFlag = GL_FALSE;
    }

    raw = (rawImageRec *)calloc(1, sizeof(rawImageRec));
    if (raw == NULL) {
        fprintf(stderr, "%s: Out of memory!\n", __func__);
        return NULL;
    }
    raw->file = fopen(fileName, "rb");
    if (raw->file == NULL) {
        const char *baseName = strrchr(fileName, '/');
        if(baseName)
            raw->file = fopen(baseName + 1, "rb");
        if(raw->file == NULL) {
            perror(fileName);
            free(raw);
            return NULL;
        }
    }

    result = fread(raw, 1, 12, raw->file);
    assert(result == 12);

    if (swapFlag) {
        ConvertShort(&raw->imagic, 1);
        ConvertShort(&raw->type, 1);
        ConvertShort(&raw->dim, 1);
        ConvertShort(&raw->sizeX, 1);
        ConvertShort(&raw->sizeY, 1);
        ConvertShort(&raw->sizeZ, 1);
    }

    raw->tmp = (unsigned char *)malloc(raw->sizeX*256);
    raw->tmpR = (unsigned char *)malloc(raw->sizeX*256);
    raw->tmpG = (unsigned char *)malloc(raw->sizeX*256);
    raw->tmpB = (unsigned char *)malloc(raw->sizeX*256);
    if (raw->sizeZ==4) {
        raw->tmpA = (unsigned char *)malloc(raw->sizeX*256);
    }
    if (raw->tmp == NULL || raw->tmpR == NULL || raw->tmpG == NULL ||
        raw->tmpB == NULL) {
        fprintf(stderr, "%s: Out of memory!\n", __func__);
        free(raw->tmp);
        free(raw->tmpR);
        free(raw->tmpG);
        free(raw->tmpB);
        free(raw->tmpA);
        free(raw);
        return NULL;
    }

    if ((raw->type & 0xFF00) == 0x0100) {
        x = raw->sizeY * raw->sizeZ * sizeof(GLuint);
        raw->rowStart = (GLuint *)malloc(x);
        raw->rowSize = (GLint *)malloc(x);
        if (raw->rowStart == NULL || raw->rowSize == NULL) {
            fprintf(stderr, "%s: Out of memory!\n", __func__);
            free(raw->tmp);
            free(raw->tmpR);
            free(raw->tmpG);
            free(raw->tmpB);
            free(raw->tmpA);
            free(raw->rowStart);
            free(raw->rowSize);
            free(raw);
            return NULL;
        }
        raw->rleEnd = 512 + (2 * x);
        fseek(raw->file, 512, SEEK_SET);
        result = fread(raw->rowStart, 1, x, raw->file);
        assert(result == x);
        result = fread(raw->rowSize, 1, x, raw->file);
        assert(result == x);
        if (swapFlag) {
            ConvertLong(raw->rowStart, (long) (x/sizeof(GLuint)));
            ConvertLong((GLuint *)raw->rowSize, (long) (x/sizeof(GLint)));
        }
    }
    return raw;
}

static void RawImageClose(rawImageRec *raw)
{
    fclose(raw->file);
    free(raw->tmp);
    free(raw->tmpR);
    free(raw->tmpG);
    free(raw->tmpB);
    if (raw->rowStart)
        free(raw->rowStart);
    if (raw->rowSize)
        free(raw->rowSize);
    if (raw->sizeZ>3) {
        free(raw->tmpA);
    }
    free(raw);
}

static void RawImageGetRow(rawImageRec *raw, unsigned char *buf, int y, int z)
{
    unsigned char *iPtr, *oPtr, pixel;
    int count, done = 0;
    size_t result;

    if ((raw->type & 0xFF00) == 0x0100) {
        fseek(raw->file, (long) raw->rowStart[y+z*raw->sizeY], SEEK_SET);
        result = fread(raw->tmp, 1, (unsigned int)raw->rowSize[y+z*raw->sizeY],
                       raw->file);
        assert(result == (unsigned int)raw->rowSize[y+z*raw->sizeY]);

        iPtr = raw->tmp;
        oPtr = buf;
        while (!done) {
            pixel = *iPtr++;
            count = (int)(pixel & 0x7F);
            if (!count) {
                done = 1;
                return;
            }
            if (pixel & 0x80) {
                while (count--) {
                    *oPtr++ = *iPtr++;
                }
            } else {
                pixel = *iPtr++;
                while (count--) {
                    *oPtr++ = pixel;
                }
            }
        }
    } else {
        fseek(raw->file, 512+(y*raw->sizeX)+(z*raw->sizeX*raw->sizeY),
              SEEK_SET);
        result = fread(buf, 1, raw->sizeX, raw->file);
        assert(result == raw->sizeX);
    }
}


static void RawImageGetData(rawImageRec *raw, TK_RGBImageRec *final)
{
    unsigned char *ptr;
    int i, j;

    final->data = (unsigned char *)malloc((raw->sizeX+1)*(raw->sizeY+1)*4);
    if (final->data == NULL) {
        fprintf(stderr, "%s: Out of memory!\n", __func__);
        return;
    }

    ptr = final->data;
    for (i = 0; i < (int)(raw->sizeY); i++) {
        RawImageGetRow(raw, raw->tmpR, i, 0);
        RawImageGetRow(raw, raw->tmpG, i, 1);
        RawImageGetRow(raw, raw->tmpB, i, 2);
        if (raw->sizeZ>3) {
            RawImageGetRow(raw, raw->tmpA, i, 3);
        }
        for (j = 0; j < (int)(raw->sizeX); j++) {
            *ptr++ = *(raw->tmpR + j);
            *ptr++ = *(raw->tmpG + j);
            *ptr++ = *(raw->tmpB + j);
            if (raw->sizeZ>3) {
                *ptr++ = *(raw->tmpA + j);
            }
        }
    }
}


static TK_RGBImageRec *tkRGBImageLoad(const char *fileName)
{
    rawImageRec *raw;
    TK_RGBImageRec *final;

    raw = RawImageOpen(fileName);
    if (!raw) {
        fprintf(stderr, "%s: File not found\n", __func__);
        return NULL;
    }
    final = (TK_RGBImageRec *)malloc(sizeof(TK_RGBImageRec));
    if (final == NULL) {
        fprintf(stderr, "%s: Out of memory!\n", __func__);
        RawImageClose(raw);
        return NULL;
    }
    final->sizeX = raw->sizeX;
    final->sizeY = raw->sizeY;
    final->components = raw->sizeZ;
    RawImageGetData(raw, final);
    RawImageClose(raw);
    return final;
}


static void FreeImage( TK_RGBImageRec *image )
{
    free(image->data);
    free(image);
}


#if 0
/*
 * Load an SGI .rgb file and generate a set of 2-D mipmaps from it.
 * Input:  imageFile - name of .rgb to read
 *         intFormat - internal texture format to use, or number of components
 * Return:  GL_TRUE if success, GL_FALSE if error.
 */
GLboolean SGI_LoadRGBMipmaps( const char *imageFile, GLint intFormat )
{
   GLint w, h;
   return SGI_LoadRGBMipmaps2( imageFile, GL_TEXTURE_2D, intFormat, &w, &h );
}



GLboolean SGI_LoadRGBMipmaps2( const char *imageFile, GLenum target,
                           GLint intFormat, GLint *width, GLint *height )
{
   GLint error;
   GLenum format;
   TK_RGBImageRec *image;

   image = tkRGBImageLoad( imageFile );
   if (!image) {
      return GL_FALSE;
   }

   if (image->components==3) {
      format = GL_RGB;
   }
   else if (image->components==4) {
      format = GL_RGBA;
   }
   else {
      /* not implemented */
      fprintf(stderr,
              "%s: Error in SGI_LoadRGBMipmaps %d-component images not implemented\n",
              __func__,
              image->components );
      FreeImage(image);
      return GL_FALSE;
   }

   error = gluBuild2DMipmaps( target,
                              intFormat,
                              image->sizeX, image->sizeY,
                              format,
                              GL_UNSIGNED_BYTE,
                              image->data );

   *width = image->sizeX;
   *height = image->sizeY;

   FreeImage(image);

   return error ? GL_FALSE : GL_TRUE;
}
#endif


/*
 * Load an SGI .rgb file and return a pointer to the image data.
 * Input:  imageFile - name of .rgb to read
 * Output:  width - width of image
 *          height - height of image
 *          format - format of image (GL_RGB or GL_RGBA)
 * Return:  pointer to image data or NULL if error
 */
GLubyte *SGI_LoadRGBImage( const char *imageFile, GLint *width, GLint *height, GLenum *format )
{
    TK_RGBImageRec *image;
    GLint bytes;
    GLubyte *buffer;

    image = tkRGBImageLoad( imageFile );
    if (!image) {
        return NULL;
    }

    if (image->components==3) {
        *format = GL_RGB;
    }
    else if (image->components==4) {
        *format = GL_RGBA;
    }
    else {
        /* not implemented */
        fprintf(stderr,
                "%s: Error in SGI_LoadRGBImage %d-component images not implemented\n",
                __func__,
                image->components );
        FreeImage(image);
        return NULL;
    }

    *width = image->sizeX;
    *height = image->sizeY;

    bytes = image->sizeX * image->sizeY * image->components;
    buffer = (GLubyte *) malloc(bytes);
    if (!buffer) {
        FreeImage(image);
        return NULL;
    }

    memcpy( (void *) buffer, (void *) image->data, bytes );

    FreeImage(image);

    return buffer;
}

#define CLAMP( X, MIN, MAX )  ( (X)<(MIN) ? (MIN) : ((X)>(MAX) ? (MAX) : (X)) )


static void ConvertRGBtoYUV(GLint w, GLint h, GLint texel_bytes,
                            const GLubyte *src,
                            GLushort *dest)
{
    GLint i, j;

    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            const GLfloat r = (src[0]) / 255.0;
            const GLfloat g = (src[1]) / 255.0;
            const GLfloat b = (src[2]) / 255.0;
            GLfloat y, cr, cb;
            GLint iy, icr, icb;

            y  = r * 65.481 + g * 128.553 + b * 24.966 + 16;
            cb = r * -37.797 + g * -74.203 + b * 112.0 + 128;
            cr = r * 112.0 + g * -93.786 + b * -18.214 + 128;
            /*printf("%f %f %f -> %f %f %f\n", r, g, b, y, cb, cr);*/
            iy  = (GLint) CLAMP(y,  0, 254);
            icb = (GLint) CLAMP(cb, 0, 254);
            icr = (GLint) CLAMP(cr, 0, 254);

            if (j & 1) {
                /* odd */
                *dest = (iy << 8) | icr;
            }
            else {
                /* even */
                *dest = (iy << 8) | icb;
            }
            dest++;
            src += texel_bytes;
        }
    }
}


/*
 * Load an SGI .rgb file and return a pointer to the image data, converted
 * to 422 yuv.
 *
 * Input:  imageFile - name of .rgb to read
 * Output:  width - width of image
 *          height - height of image
 * Return:  pointer to image data or NULL if error
 */
GLushort *SGI_LoadYUVImage( const char *imageFile, GLint *width, GLint *height )
{
    TK_RGBImageRec *image;
    GLushort *buffer;

    image = tkRGBImageLoad( imageFile );
    if (!image) {
        return NULL;
    }

    if (image->components != 3 && image->components !=4 ) {
        /* not implemented */
        fprintf(stderr,
                "%s: Error in SGI_LoadYUVImage %d-component images not implemented\n",
                __func__,
                image->components );
        FreeImage(image);
        return NULL;
    }

    *width = image->sizeX;
    *height = image->sizeY;

    buffer = (GLushort *) malloc( image->sizeX * image->sizeY * 2 );

    if (buffer)
        ConvertRGBtoYUV( image->sizeX,
                         image->sizeY,
                         image->components,
                         image->data,
                         buffer );


    FreeImage(image);
    return buffer;
}
