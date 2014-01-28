// transparent blit routine for tinyPtc | Gdl²
// (C) 2k4/2k5 r043v

//#define VERBOSE
//#define DEBUG
//#define ALL_CHECK
//#define USE_PCX
//#define ALLOW_SAVE
//#define convertTools

#include <windows.h>
#include <alloc.h>
#include <stdio.h>
#include <conio_mingw.h>

extern int WIDTH, HEIGHT, fSIZE, *pixel ;
extern unsigned char key[256] ;

#ifdef ALLOW_SAVE
int getSize(FILE *f)
{ int last = ftell(f) ;  fseek(f,0,SEEK_END) ;
  int size = ftell(f) ;
  fseek(f,last,SEEK_SET) ; return size ;
}

void toHeader(const char *file, const char * name, unsigned char *data=0, int sz=0)
{ printf("\n** entering in toheader with %s | %s | %i | 0x%x\n",file,name,sz,data) ;
  FILE *f = fopen(file,"a") ;
  if(!f) printf("\nfile open error.") ;
  fprintf(f,"unsigned char %s[%i] = {",name,sz) ;
  if(!data) printf("data error.") ;
  for(int c=0;c<sz-1;c++){ fprintf(f,"\t%i,",data[c]) ; if(c>1) if(!((c+1)%8)) fprintf(f,"\n") ; }
  fprintf(f,"\t%i \n};\n",data[sz-1]) ; fclose(f) ;
}

void bin2h(const char *file, const char * name)
{ FILE *f = fopen(file,"rb") ;
  int sz = getSize(f) ;
  char * fl = (char*)malloc(sz) ;
  fread(fl,sz,1,f) ;    fclose(f) ;
  char path[42] ;       sprintf(path,"%s.h",file) ;
  f = fopen(path,"wb") ; fprintf(f,"unsigned char %s[%i] = {\n",name,sz) ;
  for(int c=0;c<sz-1;c++) { fprintf(f,"\t%i,",(unsigned char)fl[c]) ; if(c) if(!(c%8)) fprintf(f,"\n") ; }
  fprintf(f,"\t%i \n};\n",(unsigned char)fl[sz-1]) ;  fclose(f) ;
}

void gfm2h(int *Gfm, char * path, char * name)
{ FILE *f = fopen(path,"a") ; // open in add
  fprintf(f,"int %s[%i]={",name,Gfm[1]/4) ;
  for(int c=0;c<(Gfm[1]/4)-1;c++) { fprintf(f,"\t0x%X,",Gfm[c]) ; if(Gfm[c] <= 0xFFFFF) fprintf(f,"\t"); if(c>1) if(!((c+1)%8)) fprintf(f,"\n\t") ; }
  fprintf(f,"\t0x%X \n};\n",Gfm[(Gfm[1]/4)-1]) ; fclose(f) ;
}
#endif
/*
int getSize(FILE *f)
{ int last = ftell(f) ;  fseek(f,0,SEEK_END) ;
  int size = ftell(f) ;
  fseek(f,last,SEEK_SET) ; return size ;
}

int countFps(void)
{  	static LARGE_INTEGER tm=0, freq=0 ;
	static int fps=0, fps_count=0 ;
    static double fq ;
    if(!freq.QuadPart) { QueryPerformanceFrequency(&freq); fq = ((double)freq.QuadPart) ; }
    LARGE_INTEGER t ;  QueryPerformanceCounter(&t) ;
  	fps++ ;  if(((t.QuadPart - tm.QuadPart)/fq)*1000 > 999)
             { fps_count = fps ; fps=0 ; QueryPerformanceCounter(&tm) ; }
    return fps_count ;
}

void countAndDrawFps(void)
{ static int lastFps=1 ;
  int fps = countFps() ;
  if(fps - lastFps > 5 || fps - lastFps < -5){ printf("\n%i fps",fps); lastFps = fps ; }
}
*/
int* scanImg(int *img, int sx, int sy, int *trClr)
{   int *start, *lend;
    int tfull=0, ttr=0 ; int line=0 ;
    int *dta  = (int*)malloc((sx*sy+sy*42*6)*4) ;        int size ;
    int * jmp = dta+sx*sy ; int * sze = jmp + sy*42*2 ; int * cln = sze + sy*42*2 ;
    int * jmpPtr = jmp ;    int * szePtr = sze ;    int * dtaPtr = dta ; int * clnPtr = cln ;
    int h=sy ;
    //*trClr = 0xC0C0BC02 ;
    img = img + sx*sy ; img -= sx ;
    lend = img + sx ;
    while(h--)
    {  *clnPtr = 0 ;
     while(img < lend)
     { (*clnPtr)++ ;
       start=img ;
       while(img < lend && *img == *trClr) ++img ;

       size = img-start ;
       ttr+=size ; start=img ; *jmpPtr++ = size ;

       while(img < lend && *img != *trClr) ++img ;

       size = img-start ; *szePtr++ = size ;
       if(size) { memcpy(dtaPtr,start,size*4) ; line++ ; dtaPtr+=size ; }
       else {  (*clnPtr)-- ; szePtr-- ; jmpPtr-- ; }
     }; clnPtr++ ;
         lend -= sx ; img = lend-sx ;
    };

    tfull = dtaPtr-dta ;
    int objectSize = 16 + (sy + line*2 + tfull)*4 ;
    int *out = (int *)malloc(objectSize) ; // alloc size for the Gfm data

    #ifdef VERBOSE
           printf("\nalloc %i bytes for the object.",objectSize) ;
    #endif

    int *o = out ;

    *o++ = 0x6d6647 ;            // put signature "Gfm\0"
    *o++ = objectSize ;            // put Gfm object size
    *o++ = sx<<16 | (sy & 65535) ; // put frame size x and y

    #ifdef VERBOSE
        printf("\n\nscan result\n") ;
    #endif

    int c=0 ; jmpPtr=jmp ; szePtr=sze ; dtaPtr=dta ; int cn ;
    while(c < sy)
    {   *o++ = cln[c] ;
        #ifdef VERBOSE
            printf("\nline %i\t %i full lines\t{ ",sy-c,cln[c]) ;
        #endif
        for(cn=0;cn < cln[c];cn++)
        {
          #ifdef VERBOSE
            if(*jmpPtr) printf("+%i ",*jmpPtr) ;
            printf("w%i ",*szePtr) ;
          #endif
          *o++ = *jmpPtr ; *o++ = *szePtr ;
          memcpy(o,dtaPtr,(*szePtr)*4) ; dtaPtr += *szePtr ; o += *szePtr ;
          jmpPtr++ ; szePtr++ ;
        };
        c++ ;
        #ifdef VERBOSE
           printf("}") ;
        #endif
    };      *o++ = 0x2a2a2a2a ; // end check with "****"
        #ifdef VERBOSE
               printf("\n\nobject size : %i bytes", (o-out)*4) ;
               printf("\n\n* total : \ntr   %i\nfull %i\nsum  %i\n%i lines\nsx * sy = %i * %i = %i\n",ttr,tfull,ttr+tfull,line,sx,sy,sx*sy) ;
        #endif
    free(dta) ; return out ;
}

int* Gfm2array(int *Gfm)
{ int sx = Gfm[2]>>16, sy = Gfm[2]&0xffff ;
  //printf("\n convert Gfm into an array | sx %i .. sy %i",sx,sy) ;
  int *array=(int*)malloc(sx*sy*4) ; int *o=array ;
  memset(array,0xFF,sx*sy*4) ;
  int cnt=0, c ; int *gPtr = Gfm + 3 ;
  int jump, size ;
  while(cnt<sy)
  {    c = *gPtr++ ;
       while(c--) { array += *gPtr++ ; size = *gPtr++ ;
                    memcpy(array,gPtr,size*4) ;
                    gPtr+=size ; array+=size ;
                  }; array = o + sx*(++cnt) ;
  }; return o ;
}

void flipSprite(int *img, int sx, int sy)
{ int *tmp = (int*)malloc(sx*sy*4) ;
  int ssx = sx-1, ssy = sy-1 ;
  for(int x=0;x<sx;x++)
    for(int y=0;y<sy;y++)
      tmp[(ssy-y)*sx + (ssx-x)] = img[y*sx + x] ;
  memcpy(img,tmp,sx*sy*4) ; free(tmp) ;
}

int** flipGfm(int **Gfm, int nb)
{  int ** iGfm = (int**)malloc(sizeof(int*)*nb) ;
   int sx, sy, tmp ; int *itmp ;
   for(int c=0; c<nb; c++)
   { tmp = (Gfm[c])[2] ; sx = tmp>>16 ; sy = tmp&65535 ;
     itmp = Gfm2array(Gfm[c]) ;
     flipSprite(itmp,sx,sy) ;
     iGfm[c] = scanImg(itmp,sx,sy,itmp) ;
     free(itmp) ;
   };
   return iGfm ;
}

void addFreeEntry(void * add) ;

int * data2Gfm(unsigned char *data)
{ unsigned char *d = data ;
  data+=3 ; int clNum=*data++ ;
  int sx   = *(short*)data ; data+=2 ;
  int sy   = *(short*)data ; data+=2 ;
  int outSize = *(int*)data ; data+=4 ;
  int *Gfm =  (int*)malloc(outSize) ;
  int *pal =  (int*)malloc(4*clNum) ;
  memcpy(pal,data,4*clNum) ; data+=(4*clNum) ;
  int *o = Gfm, cnt=0, c, jump, size, p1, p2 ;

  //printf("\n\nout size : %i\n%i colors { %x",outSize,clNum,*pal) ;
  //for(int c=1;c<clNum;c++) printf(",%x",pal[c]) ; printf(" }\nsize : %i*%i",sx,sy) ;

  *o++ = 0x6d6647 ;              // put signature "Gfm\0"
  *o++ = outSize ;               // put Gfm object size
  *o++ = sx<<16 | (sy & 65535) ; // put frame size x and y

  while(cnt++ < sy)
  {    *o++ = c = *data++ ; //printf("\n* line %i, %i sublines",cnt,c) ;
       while(c--) { jump = *data++ ; size = *data++ ; *o++ = jump ; *o++ = size ;
                    //printf("\n jmp %i sze %i | ",jump,size) ;
                    if(jump > sx || size > sx) {
                      //printf("\njump or size error ... pos %i",data-d) ;
                      return 0 ;
                    }

                    while(size > 1) { p1 = (*data)>>4 ; p2 = (*data)&15 ;
                                      //printf(",%x,%x",p1,p2) ;
                                      size-=2 ; *o++ = pal[p1] ; *o++ = pal[p2] ;

                                      if(p1 >= clNum || p2 >= clNum) {
                                       //printf("\ndata error, out of pal ! ... pos %i ... data %i | %x | %c",data-d,*data,*data,*data) ;
                                        return 0 ;
                                      }

                                      ++data ;
                                    };
                    if(size!=0) { /*printf(" + %x",*data);*/ *o++ = pal[(*data++)] ; }
                  };
  };
  *o = 0x2a2a2a2a ;
  addFreeEntry(Gfm);
  return Gfm ;
}


/*
void saveGfm(int *Gfm, const char *path)
{ if(*Gfm != 0x6d6647) return ;
  FILE *f = fopen(path,"w") ;
  fwrite(Gfm,Gfm[1],1,f) ;
  fclose(f) ;
}

int *loadGfm(const char *path)
{ FILE * f = fopen(path,"r") ;
  int  sz[2] ;  fread(sz,8,1,f) ;  rewind(f) ;
  if(sz[0] != 0x6d6647) { fclose(f) ; return NULL ; }
  int *out = (int*)malloc(sz[1]) ;
  fread(out,sz[1],1,f) ;
  fclose(f) ; return out ;
}


*/
/*
void reverseGfmColor(int *Gfm)
{ int sy = Gfm[2]&0xffff ;
  int cnt=0, c ; int *gPtr = Gfm + 3 ;
  int jump, size ;
  while(cnt<sy)
  {    c = *gPtr++ ;  ++cnt ;
       while(c--) { gPtr++ ; size = *gPtr++ ;
                    for(int i=0;i<size;i++)
                    { int r,g,b,color=*gPtr ;
                      r = 255-((color>>16)&255) ;
                      g = 255-((color>>8)&255) ;
                      b = 255-((color)&255) ;
                      *gPtr++ = (r<<16)|(g<<8)|b ;
                    };
       };
  };
}

void gfm16_2h(short *Gfm, char * path, char * name)
{ FILE *f = fopen(path,"a") ; // open in add
  int size = ((Gfm[1]<<16)|Gfm[2]) ;
  fprintf(f,"unsigned short %s[%i] = {",name,size) ;
  for(int c=0;c<size-1;c++) { fprintf(f,"\t0x%X,",Gfm[c]&65535) ; if(c+1>1) if(!((c+1)%8)) fprintf(f,"\n") ; }
  fprintf(f,"\t0x%X \n};\n\n",Gfm[size-1]&65535) ; fclose(f) ;
}

void rotate90r(int *s, int *d, int sx, int sy)
{	int cptx=sx ; int * dd = d ; d+=(sx-1) ;
	while(cptx--)
	{	for(int cpty=0;cpty<sy;cpty++)
		{	*d = *s++ ; d+=sx ;	};
		d = dd+(cptx-1) ;
	};
}

inline short	getGpColor(int r, int g, int b)
{
	r>>=3 ;	g>>=3; b>>=3 ;
	return	(r<<11) | (g<<6) | (b<<1) | 1 ;
}

short * Gfm32_2_16(int *Gfm)
{ short *Gfm16 ;
  short *tmp = (short*)malloc(1024*100) ;
  Gfm16=tmp ; *tmp++ = 0x6d47 ; // "Gm"
  tmp+=2 ; // skip size, put after convertion
  int sx = Gfm[2]>>16, sy = Gfm[2]&0xffff ;
  *tmp++ = sy ;  *tmp++ = sx ;
  int cnt=0, c ; int *gPtr = Gfm + 3 ;
  int jump, size ;
  while(cnt<sy)
  {    c = *tmp++ = *gPtr++ ;  ++cnt ;
       while(c--) { *tmp++ = *gPtr++ ; size = *tmp++ = *gPtr++ ;
                    for(int i=0;i<size;i++)
                    { int r,g,b,color=*gPtr++ ;
                      r = (color>>16)&255 ;
                      g = (color>>8)&255 ;
                      b = (color)&255 ;
                      *tmp++  = getGpColor(r,g,b) ;
                    };
       };
  };
  tmp-- ; int s = tmp-Gfm16 ;
  tmp=Gfm16 + 1 ;
  *tmp++ = s>>16 ; *tmp++ = s&65535 ;
  printf("gfm 32 2 16\nsx %i sy %i\nbyte : %i\n",sy,sx,(Gfm16[1]<<16)|Gfm16[2]) ;
  tmp = (short*)malloc(s*2) ;
  memcpy(tmp,Gfm16,s*2) ;
  free(Gfm16) ; return tmp ;
}
*/
void udrawGfm(int *Gfm, int * scr)  // draw a Gfm unclipped on x
{   Gfm+=2 ;   int sx = *Gfm>>16 , sy = (*Gfm++)&65535 ;
    static int * scrEnd = pixel + WIDTH*HEIGHT - 1 ;
    int lnb, sze ;

    if(scr > scrEnd) return ; // out of screen at up

    int upClip = (scr + sy*WIDTH > scrEnd) ; // is clipped at up ?

    if(scr < pixel) // is clipped at down ?
    {  if(scr + sy*WIDTH < pixel) return ; // out of screen at down
       do{    lnb = *Gfm++ ;
              while(lnb--) { Gfm++ ; Gfm += *Gfm++ ; };
              scr += WIDTH ; sy-- ;
       } while(scr < pixel) ;
    }

    int *screen = scr ; int c = 0 ;
    while(c < sy)
    {  lnb = *Gfm++ ;
       while(lnb--) { scr += *Gfm++ ; sze = *Gfm++ ;
                      memcpy(scr,Gfm,sze*4) ;
                      Gfm += sze ;    scr+=sze ;
       };  scr = screen + WIDTH*(++c) ;
       if(upClip) if(scr > scrEnd) return ;
    };
}
/*
void udrawIGfm(int *Gfm, int * scr)  // draw an inverted Gfm without x clipping
{   Gfm+=2 ;   int sx = *Gfm>>16 , sy = (*Gfm++)&65535 ;
    static int * scrEnd = pixel + WIDTH*HEIGHT - 1 ;
    int lnb, sze ; scr+=sx ;

    if(scr > scrEnd) return ; // out of screen at up

    int upClip = (scr + sy*WIDTH > scrEnd) ; // is clipped at up ?

    if(scr < pixel) // is clipped at down ?
    {  if(scr + sy*WIDTH < pixel) return ; // out of screen at down
       do{    lnb = *Gfm++ ;
              while(lnb--) { Gfm++ ; Gfm += *Gfm++ ; };
              scr += WIDTH ; sy-- ;
       } while(scr < pixel) ;
    }

    int *screen = scr ; int c = 0 ;
    while(c < sy)
    {  lnb = *Gfm++ ;
       while(lnb--) { scr -= *Gfm++ ; sze = *Gfm++ ;
                      for(int n=0;n<sze;n++) *scr-- = *Gfm++ ;
                      //memcpy(scr,Gfm,sze*4) ;
                      //Gfm += sze ;    scr-=sze ;
       };  scr = screen + WIDTH*(++c) ;
       if(upClip) if(scr > scrEnd) return ;
    };
}*/
// fuck !!! :|
/*inline void imemcpy(int *d, int *s, int z)
{   for(int n=0;n<z;n++) *d++ = s[z-n] ;
       //{ d+=z ; while(z--) *--d = *s++ ; }
}
*/
void drawGfm(int *Gfm, int x, int y)//, int way=0)
{    int sx = Gfm[2]>>16 ; int sy = Gfm[2]&65535 ; //way^=1 ;
     static int * scrEnd = pixel + WIDTH*HEIGHT - 1 ;
     if(x >= 0 && x + sx < WIDTH) udrawGfm(Gfm,pixel + y*WIDTH + x) ;
                                 /*{  if(!way)  udrawGfm(Gfm,pixel + y*WIDTH + x) ;
                                    else     udrawIGfm(Gfm,pixel + y*WIDTH + x) ;
                                   return ;
                                 }*/

     if(x < -sx || x > WIDTH) return ;// out of screen on x
     if(x >= 0) // clip right only
     {   int max = WIDTH-x ;  int lnb, sze ;
         int * scr = pixel + y*WIDTH + x ;
         if(scr > scrEnd) return ; // out of screen at up
         int upClip = (scr + sy*WIDTH > scrEnd) ; // is clipped at up ?
         Gfm += 3 ;
         if(scr < pixel) // is clipped at down ?
         {  if(scr + sy*WIDTH < pixel) return ; // out of screen at down
            do{ lnb = *Gfm++ ;
                while(lnb--) { Gfm++ ; Gfm += *Gfm++ ; };
                scr += WIDTH ; sy-- ;
            } while(scr < pixel) ;
         }

         int *screen = scr ; int c = 0 ; int * lend ;
         while(c < sy)
         {  lnb = *Gfm++ ; lend = scr + max ;
            while(lnb--) { scr += *Gfm++ ; sze = *Gfm++ ;
                           if(scr + sze < lend) memcpy(scr,Gfm,sze*4) ;
                           else if(scr < lend)  memcpy(scr,Gfm,(lend-scr)*4) ;
                           Gfm += sze ;    scr+=sze ;
            };  scr = screen + WIDTH*(++c) ;
            if(upClip) if(scr > scrEnd) return ;
         };
     } else if(x+sx < WIDTH) { // clip left only
         int lnb, sze ; int *s = pixel + y*WIDTH ; int *scr = s+x ;
         if(s > scrEnd) return ; // out of screen at up
         int upClip = (s + sy*WIDTH > scrEnd) ; // is clipped at up ?
         Gfm += 3 ;
         if(s < pixel) // is clipped at down ?
         {  if(s + sy*WIDTH < pixel) return ; // out of screen at down
            do{ lnb = *Gfm++ ;
                while(lnb--) { Gfm++ ; Gfm += *Gfm++ ; };
                s += WIDTH ; sy-- ;
            } while(s < pixel) ;
         }
         scr = s+x ; int *screen = scr ; int c=0 ; int size ;
         while(c < sy)
         {  lnb = *Gfm++ ;
            while(lnb--) { scr += *Gfm++ ; sze = *Gfm++ ;
                           if(scr >= s)   memcpy(scr,Gfm,sze*4) ;
                           else if(scr + sze > s) { size = (scr + sze)-s ;
                                                    memcpy(s,Gfm+(sze-size),size*4) ;
                                                  }
                           Gfm += sze ;    scr+=sze ;
            };  scr = screen + WIDTH*(++c) ; s = scr-x ;
            if(upClip) if(s > scrEnd) return ;
         };
     } else return ;
}

#ifdef USE_PCX
void taille_pcx(const char * fichier, unsigned short * largeur, unsigned short * hauteur) ;
void charge_pcx(const char * fichier, unsigned char * buffer, unsigned taille) ;
unsigned int * getPal(const char * fileName) ;

int *loadPcx(const char *path, int *sx=0, int *sy=0, int color=0)
{    unsigned short l, h ;
     taille_pcx(path, &l, &h) ; int size = l * h ;
     if(sx) *sx = l ; if(sy) *sy = h ;
     unsigned char * b8 = (unsigned char*)malloc(size) ;
     int  *img = (int*)calloc(size,4) ;
     unsigned int * pal = getPal(path) ;
     charge_pcx(path, b8, size) ;
     if(!color) for(int x=0;x<size;x++) img[x] = pal[b8[x]] ;
     else       for(int x=0;x<size;x++) {
                 if(*img != img[x]) img[x] = color ;
                 else               img[x] = pal[*img] ;
                };
     free(b8) ; free(pal) ; return img ;
}
/*
void rotate90r(int *s, int *d, int sx, int sy)
{	int cptx=sx ; int * dd = d ; d+=(sx-1) ;
	while(cptx--)
	{	for(int cpty=0;cpty<sy;cpty++)
		{	*d = *s++ ; d+=sx ;	};
		d = dd+(cptx-1) ;
	};
}

int *rldPcx(const char *path, int nb)
{    unsigned short l, h ;
     taille_pcx(path, &l, &h) ; int size = l * h ;
     int tsize = l/nb ;
     unsigned char * b8 = (unsigned char*)malloc(size) ;
     int  *img = (int*)calloc(size,4) ;
     unsigned int * pal = getPal(path) ;
     charge_pcx(path, b8, size) ;
     for(int x=0;x<size;x++) img[x] = pal[b8[x]] ;
     free(b8) ; free(pal) ;
     int *out = (int*)calloc(size,4) ;
     int *o, *ptr ;
     for(int c=0;c<nb;c++)
     {  o = out + (h*tsize)*c ;
        ptr = img + c*tsize ;
        for(int n=0;n<h;n++)
        { memcpy(o,ptr,tsize) ; ptr+=l ; o+=tsize ;
        };
     } free(img) ; return out ;
}

int *rloadPcx(const char *path, int *sx=0, int *sy=0)
{    unsigned short l, h ;
     taille_pcx(path, &l, &h) ; int size = l * h ;
     if(sx) *sx = h ; if(sy) *sy = l ;
     unsigned char * b8 = (unsigned char*)malloc(size) ;
     int  *img = (int*)calloc(size,4) ;
     unsigned int * pal = getPal(path) ;
     charge_pcx(path, b8, size) ;
     for(int x=0;x<size;x++) img[x] = pal[b8[x]] ;
     free(b8) ; free(pal) ;
     int *rImg = (int*)calloc(size,4) ;
     rotate90r(img, rImg, h, l) ;
     free(img) ;    return rImg ;
}
*/
int * pcx2Gfm(const char *path, int trColor=0, int fitColor=0)
{   int l, h ;  int *img = loadPcx(path,&l,&h,fitColor) ;
    if(!trColor) trColor = *img ;
    int *Gfm = scanImg(img, l, h, &trColor) ;
    free(img) ; return Gfm ;
}
#endif
/*
void vDrawLine(int x, int y, int size, int color)
{ if(x < 0 || x > WIDTH) return ;
  if(y < 0 || y+size > HEIGHT) return ;
  int *s = pixel + WIDTH*y+x ;
  while(size--) { *s = color ; s+=WIDTH ; }
}

void hDrawLine(int x, int y, int size, int color)
{ if(x < 0 || x+size > WIDTH) return ;
  if(y < 0 || y > HEIGHT) return ;
  int *s = pixel + WIDTH*y+x ;
  while(size--) *s++ = color ;
}



*/

void getGfmRealSize(int *Gfm, int *up, int *down)
{   int sy = Gfm[2]&0xffff ;
    int cnt=0, c ; int *gPtr = Gfm + 3 ; int jump, size ; int last=0 ;
    *up=*down=-1 ;

    //printf("\n* getGfmRealSize .. sy %i",sy) ;

  while(cnt<sy)
  {    c = *gPtr++ ; if(*down == -1){ if(c) *down = cnt ; }
                     else if(!c && last) *up = cnt ;
                     last = c ;

       while(c--) { gPtr++ ; gPtr += *gPtr++ ; };
       cnt++ ;
  };
  if(*down == -1) *down=0 ;
  if(*up == -1) *up = sy-1 ;
  //printf("up %i .. down %i\n",*up,*down) ;
}


#ifdef convertTools

char getId(int color, int *pal)
{ for(int c=0;c<16;c++)
   if(pal[c] == color) return c ;//{ printf("{Id%i}",c) ; return c ; }
  printf("\ngetId : color error %i | %x | %c\n",color,color,color) ; system("pause") ;
  return 0xFF ;
}

// convert a 32 bit Gfm into a 16 color Gfm.
unsigned char* Gfm2data(int *Gfm, int *sze=0)
{ // first, count color number and create palette
  unsigned char clNum=0 ; int pal[16] ;
  int sy = Gfm[2]&0xffff ;
  int sx = Gfm[2]>>16 ;
  unsigned char plop = ((4&15)<<4) | (8&15) ;
  //printf("\n plop %x -> %x %x\n",plop,(plop>>4)&15,plop&15) ;

  int cnt=0, c, tmp, tmp2, tmp3 ; int *gPtr = Gfm + 3 ; int jump, size ;
  while(cnt++ < sy)
  {    c = *gPtr++ ;
       while(c--) { *gPtr++ ; size = *gPtr++ ;
                    for(tmp=0;tmp<size;tmp++)
                    { tmp3=0 ;
                      for(tmp2=0;tmp2<clNum;tmp2++)
                       if(*gPtr == pal[tmp2]) break ;
                       else tmp3++ ;
                      if(tmp3 == clNum) pal[clNum++] = *gPtr ; /*printf("\nfound a new color : 0x%x",*gPtr) ;*/
                      gPtr++ ;  if(clNum > 16){ printf("\nmore than 16 color.\n") ; system("pause") ; }
                    };
                  };
  };

  printf("\npalette : %i colors { 0x%x",clNum,pal[0]) ;
  for(int c=1;c<clNum;c++) printf(",%x",pal[c]) ; printf(" }") ;
  gPtr = Gfm+3 ; cnt=0 ;
  unsigned char *out = (unsigned char*)malloc(102400) ; unsigned char *o = out ;
  unsigned char temp ;

  while(cnt++ < sy)
  {    *o++ = c = *gPtr++ ; //printf("\nline %i, %i sublines",cnt,c) ;
       while(c--) { jump = *o++ = *gPtr++ ; size = *o++ = *gPtr++ ; //printf("\n jump %i size %i | ",jump,size) ;
                    while(size > 1) { temp = getId(*gPtr++,pal)<<4 ;
                                      temp = temp | (getId(*gPtr++,pal)&15) ;
                                      size-=2 ; *o = temp ;
                                      //printf(",%x,%x",(*o)>>4,(*o)&15) ;
                                      o++ ;
                                    }
                    if(size){ *o = getId(*gPtr++,pal) ; printf(" + %x",*o++) ; }
                  };
  };

  printf("\nread output agf ...\n") ;
  cnt=0 ; o = out ;
  while(cnt++ < sy)
  {    c = *o++ ; printf("\n* line %i, %i sublines",cnt,c) ;
       while(c--) { jump = *o++ ; size = *o++ ;
                    printf("\n jmp %i sze %i | ",jump,size) ;
                    while(size > 1) { printf(",%x,%x",(*o)>>4,(*o)&15) ;
                                      o++ ; size-=2 ;
                                    };
                    if(size) printf(" + %x",*o++);
                  };
  };
  unsigned char * agf = (unsigned char*)malloc(12 + (o-out) + clNum*4) ; unsigned char *a = agf ;
  if(!agf) { printf("\nalloc error.\n") ; system("pause") ; }
  *(int*)agf = 0x00666761 ; // put "agf"
  agf+=3 ;  *agf++ = clNum ;
  *(short*)agf = sx ; agf+=2 ; *(short*)agf = sy ; agf+=2 ;
  *(int*)agf = Gfm[1] ; agf+=4 ;
  memcpy(agf,pal,clNum*4) ; agf += clNum*4 ;
  memcpy(agf,out,o-out)   ; agf += o-out   ;
  if(sze) *sze = agf-a ;
  free(out) ;
  printf("\nconvert finish ...\noriginal size %i\nnew size %i",(gPtr-Gfm)*4,agf-a) ;
  return a ;
}

void mGfm2h(int **Gfm, int nb, const char *path, const char *name)
{ char nm[42] ; int size ; unsigned char *Gf ;
  FILE *f = fopen(path,"w") ; fclose(f) ;
  for(int c=0;c<nb;c++)
  {  printf("\n\n .. convert frame %i\n",c) ; //system("pause") ;
     sprintf(nm,"%s%i",name,c) ;
     Gf = Gfm2data(Gfm[c],&size) ;
     toHeader(path, nm, Gf, size); free(Gf) ;
  };
  f = fopen(path,"a") ;
  fprintf(f,"\n\nint * data2Gfm(unsigned char *data) ;\n\nint* %s[%i] = { data2Gfm(%s0)",name,nb,name);
  for(int c=1;c<nb;c++) fprintf(f,", data2Gfm(%s%i)",name,c) ; fprintf(f," };") ; fclose(f) ;
}

void readGfm(int *Gfm)
{ printf("\n\n* show Gfm info *\n\n") ;
  printf("signature : %s\n",Gfm) ;
  int sy = Gfm[2]&0xffff ;
  printf("objectSize : %i\n",Gfm[1]) ;
  printf("size : %i * %i\n\n",Gfm[2]>>16,sy) ;
  int cnt=0, c ; int *gPtr = Gfm + 3 ;
  int jump, size ;
  while(cnt<sy)
  {    c = *gPtr++ ;  printf("\nline %i\t %i parts\n",++cnt,c) ;
       while(c--) { jump = *gPtr++ ; size = *gPtr++ ;
                    printf("\t+%i w%i\t{ ",jump,size) ;
                    gPtr+=size ;  printf("}") ;
       };
  };
  printf("\nchend end : %x",*gPtr) ;
}

void revertSprite(int *img, int sx, int sy)
{ int *tmp = (int*)malloc(sx*sy*4) ;
  for(int y=0;y<sy;y++)
    for(int x=0;x<sx;x++)
      tmp[y*sx + (sx-1-x)] = img[y*sx + x] ;
  memcpy(img,tmp,sx*sy*4) ; free(tmp) ;
}

// massive pcx 2 header, convert a frameset (horizontal or vertical) into an .h
void mPcx2h(const char *pcx, int nb=0, int revertFrame=0)
{   int ** img = (int**)malloc(nb*sizeof(int*)) ;
    int l, h ;    int *i=loadPcx(pcx,&l,&h) ;
    int sy, sx ;

    if(!nb) // auto compute frame number
    {  int s ; int yyop ; int *v=&h, *vh=&l ;
       if(l > h*2) { v=&l ; vh=&h ; } // horizontal frameset
       //printf("\n%s\n* search for frame size .. v %i vh %i",pcx,*v,*vh) ;
       s = *vh + 3 ;
       do { yyop = (*v)/s ; } while(s && yyop*s-- != *v) ; ++s ;
       if(!s) printf("\nerror on compute frame size in %s",pcx) ;
       //else   printf("\nsize found %i*%i, %i frames",s,*vh,yyop) ;
       nb = yyop ;
    }

    sy = h ; sx = l/nb ;
    int *ptr, *optr, *o ;

    if(l > h*2 && nb>1)   // rotate way of frames if horizontal frameset
    {   o = (int*)malloc(l*h*4) ;
        for(int x=0;x<nb;x++)
        {    ptr = i + sx*x ; optr= o + sx*sy*x ;
             for(int y=0;y<sy;y++)
             { memcpy(optr,ptr,sx*4) ;
               ptr+=l ; optr+=sx ;
             };
        };
    } else { sy = h/nb ; sx = l ; o=i ; }

    char *pth = (char *)malloc(1024) ;
    sprintf(pth ,"%s",pcx) ;
    char *p = pth ; while(*p != '.') p++ ; *p = 0 ;
    sprintf(pth+512 ,"%s.h",pth) ;
    char *np = p ; while(*np != '\\' && np>=pth) np-- ; np++ ;

    FILE *f = fopen(pth+512,"w") ;
    fprintf(f,"\n// file %s (%ix%i) converted into %i %ix%i 32b Gfm ",pcx,l,h,nb,sx,sy) ;
    if(nb>1) fprintf(f,"(%s0 to %s%i) ",np,np,nb-1) ;   fprintf(f,"| converted by Gdl²\n\n") ;
    fclose(f) ;

    for(int n=0;n<nb;n++)   // convert frame and put them into the .h
    { if(revertFrame) revertSprite(o+(sx*sy*n),sx,sy) ;
      img[n] = scanImg(o+(sx*sy*n),sx,sy,i) ;
      sprintf(pth+42,"%s%i",np,n) ;
      gfm2h(img[n],pth+512,pth+42) ;
    };

    if(nb>1) { // include frame array
               f = fopen(pth+512,"a") ;  fprintf(f,"\nint *%s[%i] = {",np,nb) ;
               for(int c=0;c<nb-1;c++)   fprintf(f," %s%i,",np,c) ;
               fprintf(f," %s%i } ;\n",np,nb-1) ;
               fclose(f) ;
             }

    free(pth);  if(o-i) free(o);  free(i) ; for(int n=0;n<nb;n++) free(img[n]) ;  free(img) ;
}
#endif

int isColide(int *Gfm1, int *Gfm2, int px1, int px2, int py1, int py2)
{	int sx1 = Gfm1[2]>>16 ;
	int sy1 = Gfm1[2]&0xffff ;
	int sx2 = Gfm2[2]>>16 ;
	int sy2 = Gfm2[2]&0xffff ;
	
	// first, box test
	if(py1 + sy1 >= sy2)
		if(py1 <= py2 + sy2)
			if(px1 + sx1 >= px2)
				if(px1 <= px2 + sx2)
					return 1 ;

/*		if(colision)
		{
			// detection de la zone colision
			
			short x1 = sprite1->pos_x ;
			short y1 = sprite1->pos_y ;
			short x2 = sprite1->pos_x + sprite1->size_x ;
			short y2 = sprite1->pos_y + sprite1->size_y ;
			
			short xx1 = sprite2->pos_x ;
			short yy1 = sprite2->pos_y ;
			short xx2 = sprite2->pos_x + sprite2->size_x ;
			short yy2 = sprite2->pos_y + sprite2->size_y ;
			
			short x=0,y=0 ;

			// calcul de la position y du sprite 1 par rapport au 2 en colision
			if(y1 < yy1) y=0 ;
			else
			{
				if(y2 > yy2) y=2 ;
				else y=1 ;
			}

			// calcul de la position x du sprite 1 par rapport au 2 en colision
			if(x1 < xx1) x=0 ;
			else
			{
				if(x2 > xx2) x=2 ;
				else x=1 ;
			}

			return((y*3)+(x+1)) ;
		}
	*/
}
