/*******************************************************************************
 * Software Name :safe
 *
 * Copyright (C) 2014 HanVideo.
 *
 * author : 梁智游
 ******************************************************************************/

#include "tool.h"
#include<math.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>



int hex2int(int hexValue) {
	int sum = 0;
	int i;
	for (i = 0; hexValue != 0; i++) {
		sum = (hexValue % 16) * pow(16, i) + sum;
		hexValue = hexValue / 16;
	}
	return sum;
}

unsigned long long GetCurSysTime()
{
	struct timeval tv;
	if(gettimeofday(&tv,0) < 0)
	{
		return -1;
	}

	return (((unsigned long long)tv.tv_sec) * 1000 + ((unsigned long long)tv.tv_usec) / 1000);
}

void mysleep(unsigned int milliseconds)
{
#if 1
	unsigned int micro_seconds = milliseconds*1000;
	usleep(micro_seconds);
#else
	struct timeval timeval;
    timeval.tv_sec  = milliseconds / 1000;
    timeval.tv_usec = ( milliseconds % 1000 ) * 1000;

    select( 0, NULL, NULL, NULL, &timeval );
#endif

}

int find_nal_units(const unsigned char* buf,int buf_size,int* nal_start, int* nal_end)
{
    if(!buf || buf_size <= 0)
        return 0;

    int i;
    // find start
    *nal_start = 0;
    *nal_end = 0;
    int size;
    size = buf_size;

    i = 0;
    while (   //( next_bits( 24 ) != 0x000001 && next_bits( 32 ) != 0x00000001 )
        (buf[i] != 0 || buf[i+1] != 0 || buf[i+2] != 0x01) &&
        (buf[i] != 0 || buf[i+1] != 0 || buf[i+2] != 0 || buf[i+3] != 0x01)
        )
    {
        i++; // skip leading zero
        if (i+4 >= size) { return 0; } // did not find nal start
    }
    *nal_start = i;
    if  (buf[i] != 0 || buf[i+1] != 0 || buf[i+2] != 0x01) // ( next_bits( 24 ) != 0x000001 )
    {
        i++;
    }

    if  (buf[i] != 0 || buf[i+1] != 0 || buf[i+2] != 0x01) { /* error, should never happen */ return 0; }
    i+= 3;


    while (   //( next_bits( 24 ) != 0x000000 && next_bits( 24 ) != 0x000001 )
        (buf[i] != 0 || buf[i+1] != 0 || buf[i+2] != 0) &&
        (buf[i] != 0 || buf[i+1] != 0 || buf[i+2] != 0x01)
        )
    {
        i++;
        //  the next line fails when reading a nal that ends exactly at the end of the data
        if (i+3 >= size) { *nal_end = size; return (*nal_end - *nal_start); } // did not find nal end, stream ended first
    }

    *nal_end = i;
    return (*nal_end - *nal_start);
}


int get_nal_type( void *p, int len )
{
    if ( !p || 5 >= len )
        return -1;

    unsigned char *b = (unsigned char*)p;

    // Verify NAL marker
    if ( b[ 0 ] || b[ 1 ] || 0x01 != b[ 2 ] )
    {   b++;
        if ( b[ 0 ] || b[ 1 ] || 0x01 != b[ 2 ] )
            return -1;
    } // end if

    b += 3;

    return (*b & 0x1F);
}

int get_hevc_nal_type( void *p, int len ){
    if ( !p || 7 >= len )
        return -1;

    unsigned char *b = (unsigned char*)p;

    // Verify NAL marker
    if ( b[ 0 ] || b[ 1 ] || 0x01 != b[ 2 ] )
    {   b++;
        if ( b[ 0 ] || b[ 1 ] || 0x01 != b[ 2 ] )
            return -1;
    } // end if

    b += 3;

    return (*b & 0x7E)>>1;
}

int is_h264_keyframe(void *data,int len) {
    if (!data)
        return -1;

    int ret = -1;

    if (5 >= len)
        return -1;


    int nal_start = 0;
    int nal_end = 0;
    int frame_len = 0, offset = 0;

    while ((frame_len = find_nal_units(data + offset, len - offset, &nal_start, &nal_end)) >
           0) {
        int iNALType = get_nal_type((data + offset + nal_start), frame_len);
        if (iNALType == NAL_TYPE_I) {
            ret = 0;
            break;
        }

        offset += (frame_len + nal_start);
        nal_start = 0;
        nal_end = 0;
    }


    return ret;
}




