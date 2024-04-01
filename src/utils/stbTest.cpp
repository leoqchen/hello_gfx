#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"

int main( int argc, const char *argv[] )
{
    if( argc != 2 ){
        printf("Usage: %s file\n", argv[0]);
        return 0;
    }
    const char *infile = argv[1];

    // load
    int width, height, channels;
    unsigned char *data = stbi_load( infile, &width, &height, &channels, 0);
    if( data == NULL ){
        printf("fail to read file: %s\n", infile);
        return 1;
    }
    printf("width = %d, height = %d, channels = %d\n", width, height, channels);

    const char *outfile = "/tmp/1.jpg";
    stbi_write_jpg( outfile, width, height, channels, data, 90 );
    printf("write to %s\n\n", outfile);

    // resize
    int resizewidth = width * 2;
    int resizeheight = height * 2;
    unsigned char *data2 = stbir_resize_uint8_linear( data, width, height, 0, NULL, resizewidth, resizeheight, 0, (stbir_pixel_layout)channels );
    const char *outfile2 = "/tmp/2.jpg";
    stbi_write_jpg( outfile2, resizewidth, resizeheight, channels, data2, 90 );
    printf("write to %s\n\n", outfile2);

    return 0;
}