#pragma once

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

typedef unsigned char Pixel1;	/* 1-byte pixel */
typedef struct {Pixel1 r, g, b;}	Pixel1_rgb;
typedef short                           Pixel2;
typedef struct {Pixel2 r, g, b;}        Pixel2_rgb;
typedef Pixel2_rgb Rgbcolor;


class Pic{		/* PICTURE */
public:
    int m_width, m_height;			/* width & height, in pixels */
    int m_channels;			/* bytes per pixel = 1, 3, or 4 */
    Pixel1 *pix;		/* array of pixels */
				/* data is in row-major order,
				    i.e. it has the same memory layout as:
				    if 1 byte per pixel,  then array[ny][nx]
				    if 3 bytes per pixel, then array[ny][nx][3]
				    if 4 bytes per pixel, then array[ny][nx][4] */


    float Pixel( const unsigned int& i_row, const unsigned int& i_col, const unsigned int& i_channel )
    {
        if( 
            m_height   <= static_cast< int >( i_row ) || 
            m_width    <= static_cast< int >( i_col ) ||
            m_channels <= static_cast< int >( i_channel )
            )
        {
            return 0.0f;
        }

        return static_cast<float>( pix[ ( i_row * m_width + i_col ) * m_channels + i_channel ] );
    }


};



/*----------------------Allocation routines--------------------------*/
Pic *pic_alloc( int nx, int ny, int bytes_per_pixel, Pic *opic );
void pic_free( Pic *p );
Pic* ReadJPEG( const char* i_path );
bool WriteJPEG( const char* i_path, const Pic* i_pic );