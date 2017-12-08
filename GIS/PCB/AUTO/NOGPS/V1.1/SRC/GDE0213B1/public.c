#include "main.h"
const unsigned char ocHex[58]={
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,1,2,3,4,5,6,7,
    8,9
};

const unsigned ASCValue[128] = {
	0,0,0,0,0,0,0,0,0,0,		// 0-9
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,1,
	2,3,4,5,6,7,8,9,0,0,
	0,0,0,0,0,10,11,12,13,14,
	15,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,10,11,12,
	13,14,15,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0
};

void GetDirc(unsigned char *src,unsigned char *dst)
{
	int i;
    for(i=0;i<3;i++)
    {
        if(*(src+i) == '.')
            break;
    }
	switch(i)
	{
	case 1:
		dst[0] = 0;
		dst[1] = ASCValue[src[0]]<<4;
		break;
	case 2:
		dst[0] = ASCValue[src[0]];
		dst[1] = ASCValue[src[1]]<<4;
		break;
	case 3:
		dst[0] = ((ASCValue[src[0]]<<4) | ASCValue[src[1]]);
		dst[1] = ASCValue[src[2]]<<4;
		break;
	default:
		dst[0] = 0xFF;
		dst[1] = 0xF0;
		break;
	}
}

void GetSpeed(unsigned char *src,unsigned char *dst)
{
    int i;
    for(i=0;i<4;i++)
    {
        if(*(src+i) == '.')
            break;
    }
	if(i)
	{	
		switch(i)
		{
		case 1:
			dst[0] = 0;
			dst[1] = (((ASCValue[src[0]])<<4) | ASCValue[src[2]]);
			break;
		case 2:
			dst[0] = ASCValue[src[0]];
			dst[1] = (((ASCValue[src[1]])<<4) | ASCValue[src[3]]);
			break;
		case 3:
			dst[0] = (((ASCValue[src[0]])<<4) | ASCValue[src[1]]);
			dst[1] = (((ASCValue[src[2]])<<4) | ASCValue[src[4]]);
			break;
		default:
			dst[0] = 0xFF;
			dst[1] = 0xFF;
			break;
		}

	}
	else
	{
		dst[0] = 0;
		dst[1] = 0;
	}
}

void StringToHex(const unsigned char *src,unsigned char *dst,unsigned char sLen)	// ACII to hex 	only 0-F
{
	int i,j;
	for(i = 0,j = 0; i < sLen;)
	{
		dst[j] = ASCValue[src[i++]];
		dst[j] <<= 4;
		dst[j++] += ASCValue[src[i++]];
	}
}

void Str2Hex(unsigned char *src,unsigned char *dst)		// for lat and lon
{
    int i;
    for(i=0;i<12;i++)
    {
        if(*(src+i) == '.')
            break;
    }
    if(i==5)
    {
        dst[0] = ocHex[src[0]]*100 + ocHex[src[1]]*10 + ocHex[src[2]];
        dst[1] = (ocHex[src[3]]<<4) + ocHex[src[4]];
        dst[2] = (ocHex[src[6]]<<4) + ocHex[src[7]];
        dst[3] = (ocHex[src[8]]<<4) + ocHex[src[9]];
        dst[4] = (ocHex[src[10]]<<4) + ocHex[src[11]];
    }
    else if(i==4)
    {
        dst[0] = (ocHex[src[0]])*10 + ocHex[src[1]];
        dst[1] = (ocHex[src[2]]<<4) + ocHex[src[3]];
        dst[2] = (ocHex[src[5]]<<4) + ocHex[src[6]];
        dst[3] = (ocHex[src[7]]<<4) + ocHex[src[8]];
        dst[4] = (ocHex[src[9]]<<4) + ocHex[src[10]];
    }
	else
	{
		dst[0] = 0;
		dst[1] = 0;
		dst[2] = 0;
		dst[3] = 0;
		dst[4] = 0;
	}
}
