#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


int main( int argc, const char *argv[] )
{
    if( argc != 2 ){
        printf("Usage: %s file\n", argv[0]);
        return 0;
    }
    const char *infile = argv[1];

    int width, height, channels;
    unsigned char *data = stbi_load( infile, &width, &height, &channels, 0);
    if( data == NULL ){
        printf("fail to read file: %s\n", infile);
        return 1;
    }
    printf("width = %d, height = %d, channels = %d\n", width, height, channels);

    const char *outfile = "/tmp/1.jpg";
    stbi_write_jpg( outfile, width, height, channels, data, 90 );
    printf("write to %s\n", outfile);

    return 0;
}