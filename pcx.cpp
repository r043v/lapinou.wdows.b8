#include "stdio.h"
#include "stdlib.h"

#define ushort unsigned short
#define uchar unsigned char
#define uint unsigned int
#define ulong unsigned long
#define byte char

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define error printf

#define getColor(r,g,b)    (r<<16)|(g<<8)|b
#define m_getw(p)          (*p)+(*(p+1))*256

void taille_pcx(const char * fichier, unsigned short * largeur, unsigned short * hauteur)
{
    FILE * flot;
    unsigned width = 0, height = 0;

    flot = fopen(fichier,"rb");
    if(!flot) { printf("\n* file not found %s",fichier) ; return ; }

    fseek(flot,8,SEEK_SET);
    fread(&width,2,1,flot);
    fread(&height,2,1,flot);
    width++;
    height++;

    *largeur = width;
    *hauteur = height;

    fclose(flot);
}

void charge_pcx(const char * fichier, unsigned char * buffer, unsigned taille)
{
    FILE * flot;

    unsigned long position;
    unsigned num;
    unsigned char ch;
    //printf("\n* opening %s",fichier) ;
    flot = fopen(fichier,"rb");
    if(!flot) { printf("\n* file not found %s",fichier) ; return ; }
    fseek(flot,128,SEEK_SET);

    position=0;
    do
    {
        ch = fgetc(flot);
        if ((ch & 0xC0) == 0xC0)
        {
            num = ch & 0x3F;
            ch = fgetc(flot);
            while(num--)
                buffer[position++]=ch;
        }
        else
            buffer[position++]=ch;
    } while(position < (taille));
    fclose(flot);
    //printf("\n   closing %s",fichier) ;
}

uint * getPal(const char * fileName)
{
	FILE * file;
	uint	 i,r,g,b ;
	uint	*pal, *palPtr ;
	ushort	colorRead=256 ;
	uchar	*buf, colorSize=3, vgaInverted=0 ;
	char	 ext[5] ;

	strcpy(ext,fileName+strlen(fileName)-4) ;

	file = fopen(fileName, "rb");
      if(!file) { error("\n* file not found %s",fileName); return 0 ; }
	pal = palPtr = (uint*)malloc(1024) ;
	memset(pal,0xFF,1024);

	if((!strcmp(ext,".BMP"))||(!strcmp(ext,".bmp"))) // case a bmp file
	{
		fseek(file,54,SEEK_SET);
		vgaInverted=1 ;
		colorSize=4 ;
	}
		else
		
			if((!strcmp(ext,".PCX"))||(!strcmp(ext,".pcx"))) // case a pcx file
			{
				uchar palType ;
				fseek(file, -769,SEEK_END);
				fread(&palType,1,1,file); // palette type
				if(palType!=12) 	// 16 color palette
				{	colorRead = 16 ;
					fseek(file,16, SEEK_SET);
				}
			}	else	colorRead=0 ;

	ushort size = colorSize*colorRead ;
	buf = (uchar*)malloc(size) ;
	fread(buf,size,1,file);
	fclose(file) ;
	
	for(i=0;i<colorRead;i++)
	{	
		r = (buf[colorSize*i])  	;
		g = (buf[colorSize*i+1])	;
		b = (buf[colorSize*i+2])	;
		
		if(!vgaInverted)	*palPtr++ = (r<<16)|(g<<8)|b ;
		else				*palPtr++ = (b<<16)|(g<<8)|r ;
	};

	free(buf) ;
	return pal ;
}
