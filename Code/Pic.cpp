#include "pic.h"


extern "C"
{
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stdio.h>
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"
}

/*----------------------Allocation routines--------------------------*/
Pic *pic_alloc( int nx, int ny, int bytes_per_pixel, Pic *opic )
{
    opic = new Pic();

    opic->m_width = nx;
    opic->m_height = ny;
    opic->m_channels = bytes_per_pixel;

    return opic;
}


extern void pic_free( Pic *p )
{
    if( p )
    {
        stbi_image_free( p->pix );
    }
}


extern Pic* ReadJPEG( const char* i_path )
{
    Pic* pic = new Pic();
    pic->pix = stbi_load( i_path, &pic->m_width, &pic->m_height, &pic->m_channels, 0 );

    return pic;
}


extern bool WriteJPEG( const char* i_path, const Pic* i_pic )
{
    return stbi_write_jpg( i_path, i_pic->m_width, i_pic->m_height, i_pic->m_channels, i_pic->pix, 100 );
}





