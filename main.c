
/* lapinou's Adventure reborn */
// (c) 2k5 r043v

// amor de mis amores

#include <windows.h>
#include "tinyptc.h"

#include <alloc.h>
#include <stdio.h>
#include <conio_mingw.h>

#include "minifmod.h"

#include "./gfx/lapinou/tiles.h"
#include "./gfx/lapinou/stance.h"
#include "./gfx/lapinou/up.h"
#include "./gfx/lapinou/fall.h"
#include "./gfx/lapinou/walk.h"
#include "./gfx/lapinou/salto.h"

#include "./gfx/lapinou/font.h"
#include "./gfx/lapinou/chfont.h"
#include "./gfx/lapinou/green.h"

#include "./lapin.xm.h"

#include "Gdl.h"

void animSwitch(struct anim **b);
void animFlip(struct anim **b);

struct anim *anim[] = { setAnim(up,2,200,0,animSwitch),
                        setAnim(fall,2,150,0,animSwitch),
                        setAnim(walk,4,120),
                        setAnim(stance,5,200,0,animSwitch),
                        setAnim(salto,10,110,1,animSwitch,0,animFlip)
                      };

#define animAr anim
#define animNb 5

#define Up     animAr[0]
#define Fall   animAr[1]
#define Walk   animAr[2]
#define Stance animAr[3]
#define Salto  animAr[4]

int lapinouWay = 0  ; // 0 right 1 left
int lapinouPx = 142 ;
int lapinouPy = -32 ;
int lapinouDwnSpeed=2 ;
int mapScrollx= 0   ;
int mapScrolly= 0   ;
int tick=0 ;

int canDown(void) ;

inline void switch2anim(struct anim **a, struct anim *d)
{  *a=d ; resetAnim(a) ; if(d == Fall) lapinouDwnSpeed=2 ; }

/*******/

struct Debris
{ int *Gfm ;
  int enable ;
  int lastTime ;
  int px, py ;
  int count ;
};

struct Debris *debris ;
int    debrisNb=0     ;

void drawDebris(void)
{ int nb=debrisNb ;
  if(nb)
   for(int c=0;c<42;c++)
    if(debris[c].enable) {
       if(debris[c].lastTime + 32 < tick){ debris[c].py -= debris[c].enable ;
                                           if(debris[c].py <= mapScrolly - 180)
                                           {      debris[c].enable = 0 ; --debrisNb ; }
                                           else { debris[c].lastTime = tick ;
                                                  if( ++(debris[c].count) > 3)
                                                  { debris[c].count = 0 ; ++(debris[c].enable) ; }
                                                }
                                         }
       drawGfm(debris[c].Gfm, debris[c].px - mapScrollx, debris[c].py - mapScrolly);

       if(--nb <= 0) return ;
    }
}

void addDebris(int x, int y, int *Gfm)
{ x>>=5 ; x<<=5 ;
  for(int c=0;c<42;c++)
    if(!debris[c].enable)
    { debris[c].Gfm = Gfm ; debris[c].lastTime=0 ;
      debris[c].px=x ;      debris[c].py=y ; debris[c].count=0 ;
      debris[c].enable=3 ;  ++debrisNb ; return ;
    }
}

/*********/

void animFlip(struct anim **b)
{  if(*b == Salto){ static int last=0 ;
                    if(last + 42 < tick)
                    { int v =((9-(Salto->curentFrm))-4) ;
                      if(v<0) { for(int c=0;c>v;c--) if(canDown()) --lapinouPy ; }
                      else lapinouPy += v ;
                      last = tick ;
                    }
                  }
}

void animSwitch(struct anim **b)
{  if(*b == Salto) switch2anim(b,Fall); // salto 2 fall
   else if(*b == Up) switch2anim(b,Salto);
        else if(*b == Fall) { static int upTime=tick ;
                              if(upTime + 42 < tick) { upTime=tick ;
                                                       if(++lapinouDwnSpeed > 8) lapinouDwnSpeed=8 ;
                                                     }

                            }
        //else if(*b == Walk){ if(!(key[kleft]||key[kright])) switch2anim(b,Stance); }
}

void showInt(int value, int px, int py, int **fnt=chfont)
{    char str[8], *ptr=str ;
     if(value < 10) sprintf(str, "0%i", value) ;
     else sprintf(str, "%i", value) ;
     int sx = ((*fnt)[2])>>16 ; // get font sx
         while(*ptr)
         {  drawGfm(fnt[(*ptr++)-'0'],px,py) ; px+=sx ;
         };
}

void drawText(const char *txt, int x, int y, int **fnt=font, int limit=42)
{int n=0, px=x, sx=(((fnt[0])[2])>>16) - 4 ;
 while(txt[n]!='\0' && n < limit)
 { if(txt[n] >= 'a' && txt[n] <= 'z')
        drawGfm(fnt[txt[n++]-'a'],px,y) ;
   else n++ ;
   px+=sx ; if(px > WIDTH) return ;
 };
}

#define Map(x,y) (map[(x) + (y)*mapSx])
#define tmpArray(x,y) (tempArray[(x) + (y)*mapSx])
#define WalkArray(x,y) (walkArray[(x) + (y)*mapSx])

#include "map.h"

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

void showMap(short *Array, int **tileset, int x, int y)
{   int debut_case_aff_x = x>>5 ; // /32
	int debut_case_aff_y = y>>5 ;
	int decalage_case_x = ((debut_case_aff_x+1)<<5) - x ;
	int decalage_case_y = ((debut_case_aff_y+1)<<5) - y ;
	int cpt_x,cpt_y,id ;
    int start = -1 ; if(y<32)start=0 ;

		for(cpt_x=0;cpt_x<12;cpt_x++)
			for(cpt_y=start;cpt_y<9;cpt_y++)
			{   id=Array[(63-(cpt_y+debut_case_aff_y))*24 + cpt_x + debut_case_aff_x] ;
				if(id)// >0 && id < 29)
				{ 	drawGfm(tileset[id-1],((cpt_x-1)<<5) + decalage_case_x,((cpt_y-1)<<5) + decalage_case_y) ;
				}
			} ;
}
/*
void autoScroll(int *scx, int *scy, int msx, int msy, int px, int py, int szx, int szy, int limit_up, int limit_down, int limit_right, int limit_left)
{  	// test gauche
	if((px - *scx) < limit_left)
	{   *scx -= limit_left - (px - *scx) ;
		if(*scx < 0) *scx=0 ;
	}	else // test droite
        {   if(((px+szx) - *scx) > (320-limit_right))
		     {  	*scx += ((px+szx) - *scx) - (320-limit_right) ;
			        if(*scx >= msx*32 - 320) *scx =  msx*32 - 321 ;
             }
        }

	// test haut
	if((py - *scy) < limit_up)
	{   *scy -= limit_up - (py - *scy) ;
		if(*scy < 0) *scy=0 ;
	}
	else // test bas
	{   if(((py+szy) - *scy) > (240-limit_down))
		{  	*scy += ((py+szy) - *scy) - (240-limit_down) ;
			if(*scy > msy*32 - 240) *scy = msy*32 - 240 ;
		}
	}
}

short getTileS(short *Array, int x, int y, int scx, int scy)
{
	int map_x = (scx + x)>>5 ;
	int map_y = (scy + y)>>5 ;
	
	return Array[map_y*24 + map_x] ;
}*/

void getGfmRealSize(int *Gfm, int *up, int *down);

int **tset=0 ;
int score=0, dscore=0 ;

short getTileM(short *Array, int x, int y, int **tileset=tset)
{   if(x<=0 || y<=0 || x>63*32 || y>63*32) return 1 ;
   	int map_x = x>>5 ;
	int map_y = (64*32-y)>>5 ;
	int v =  Array[map_y*24 + map_x] ;
    if(tileset){ // see real tile up and down value, for a semi pixel colide..
                 static int *sze = 0 ;
                 if(!sze){ sze = (int*)malloc(100*4) ;
                           for(int c=0;c<100;c++) sze[c] = -1 ;
                           addFreeEntry(sze) ;
                         }
                 int dy = (64*32-y) - (map_y<<5) ;
                 int up, down ;   int val = v ; if(val) --val ;

                 if(sze[val] == -1){ getGfmRealSize(tileset[val], &up, &down) ;
                                     sze[val] = (up<<16) | (down&0xFFFF) ;
                 }           else  { up = (sze[val])>>16 ; down = (sze[val])&0xFFFF ; }

                 dy = 31-dy ;
                 //printf("\n getTileM(%i,%i) -> %i [%i,%i] | dy %i",x,y,val,up,down,dy) ;
                 if(dy < down || dy > up) return 0 ;
    }

     if(v==15) // carots
     {    static int lastTime=0, points=0 ;
          Array[map_y*24 + map_x]=0 ;
          if(lastTime + 424 > tick){ if(points < 2) ++points ; }
          else                      points=0 ;

          switch(points)
          { case 0 : score+=100  ; break ;
            case 1 : score+=500  ; break ;
            case 2 : score+=1000 ; break ;
          };

          lastTime=tick ;         addDebris(map_x<<5,(63-map_y)<<5,tiles[25+points]) ;
          return 0 ;
     }
     if(v) return v-1 ;
     else  return 0 ;
	//return Array[map_y*24 + map_x] ;
}

void setTileM(short *Array, int x, int y, int id)
{
	int map_x = x>>5 ;
	int map_y = (64*32-y)>>5 ;

	if(id) Array[map_y*24 + map_x] = id+1 ;
    else   Array[map_y*24 + map_x] = 0 ;
}
/*
void setTileS(short *Array, int x, int y, int scx, int scy, short id)
{
	int map_x = (scx + x)>>5 ;
	int map_y = (scy + y)>>5 ;
	
	Array[map_y*24 + map_x] = id ;
}*/

//-----------------------------------------------------------------------

int canLeft(void)
{ if(getTileM(maparray, lapinouPx+8, lapinouPy+4)||getTileM(maparray, lapinouPx+8, lapinouPy+16)||getTileM(maparray, lapinouPx+8, lapinouPy+28)) return 1 ;
  return !(getTileM(maparray, lapinouPx+2, lapinouPy+4)>1 || getTileM(maparray, lapinouPx+2, lapinouPy+14)>1 || getTileM(maparray, lapinouPx+2, lapinouPy+24)>1) ;
}

int canRight(void)
{ if(getTileM(maparray, lapinouPx+24, lapinouPy+4)||getTileM(maparray, lapinouPx+24, lapinouPy+16)||getTileM(maparray, lapinouPx+24, lapinouPy+28)) return 1 ;
  return !(getTileM(maparray, lapinouPx+29, lapinouPy+4)>1 || getTileM(maparray, lapinouPx+29, lapinouPy+14)>1 || getTileM(maparray, lapinouPx+29, lapinouPy+24)>1) ;
}

int canDown(void)
{ if(getTileM(maparray, lapinouPx+4, lapinouPy) || getTileM(maparray, lapinouPx+27, lapinouPy))
    if(!(getTileM(maparray, lapinouPx+4, lapinouPy+1) || getTileM(maparray, lapinouPx+27, lapinouPy+1)))
     return 0 ;
  return 1 ;
}

int isDestructible(void)
{  int t1,t2 ;
   t1 = getTileM(maparray, lapinouPx+4, lapinouPy)  ; t1 = (t1 > 1 && t1 < 14) ;
   t2 = getTileM(maparray, lapinouPx+27, lapinouPy) ; t2 = (t2 > 1 && t2 < 14) ;
   return (t1||t2) ;
}

int dstTime=0 ;

FMUSIC_MODULE * mod ; MEMFILE zk ;

void addFreeEntry(void * add) ;

void close(void)
{ free(pixel) ; free(debris) ;
  mifreeAnim(animAr,animNb) ;
  FMUSIC_StopSong(mod) ;
  FSOUND_Close() ;
  addFreeEntry(0) ;
}

void bin2h(const char *file, const char * name);
void mGfm2h(int **Gfm, int nb, const char *path, const char *name) ;
void mPcx2h(const char *pcx, int nb=0, int revertFrame=0) ;

int main(int argc, char *argv[])
{   zk.pos=0 ; zk.length=16079 ; zk.data=muZIk ;
    FSOUND_File_SetCallbacks(memopen, memclose, memread, memseek, memtell);
    FSOUND_Init(44100, 0); mod = FMUSIC_LoadSong((char*)&zk, NULL);   if(mod) FMUSIC_PlaySong(mod);
    if (!ptc_open("lapinou Adventures",WIDTH,HEIGHT)) return 1;
    srand(GetTickCount());
    pixel = (int*)malloc(320*240*4) ;
    debris = (struct Debris *)malloc(sizeof(struct Debris)*42) ;
    struct anim *lapinou = Up ;
    lapinouWay = 0 ;

    //mPcx2h("e:\\green.pcx", 1) ;
    //int ** ag = (int**)malloc(8) ; ag[0] = green ; mGfm2h(ag, 1, "c:\\lapinou\\agreen.h", "green") ;

    for(int c=0;c<42;c++) debris[c].enable=0 ;

    tset=(int**)malloc(4*28) ; addFreeEntry(tset) ;

    for(int c=0;c<28;c++) tset[c] = tiles[c] ;
    tset[15] = tiles[15] = tiles[1] ;
    tiles[15] = green ;

    //mPcx2h("c:\\lapin\\tiles.pcx") ; bin2h("e:\\lapin.xm","muZIk") ;

    while(1){tick = GetTickCount() ;

             if(key[kspace]){ if(!mod) return 0 ;
                              static int pos=0 ;

                              if(!pos){ FMUSIC_StopSong(mod) ;
                                        FSOUND_Close() ;
                                      }
                              else    { FSOUND_Init(44100, 0);
                                        FMUSIC_PlaySong(mod) ;
                                      }

                              pos^=1 ; while(key[kspace]) ptc_update(pixel) ;
                            }

             memset(pixel,0x88,320*240*4) ;
             ultimate_auto_scrool(&mapScrollx, &mapScrolly, 23, 64, lapinouPx, lapinouPy, 32, 32,40,80,110,110,32,40,20,20) ;
             drawDebris();
             showMap(maparray, tiles, mapScrollx, mapScrolly) ;
             if(key[kdown])           ----lapinouPy ;
             if(lapinou == Up)        ++++++lapinouPy ;

             { //if(wlkTime + 16 < tick) //wlkTime=tick ;
               static int flip=0 ; (++flip)%=3  ;
               if(key[kleft]) if(canLeft())  { lapinouPx-- ; if(!flip && canLeft())lapinouPx-- ;  lapinouWay=1 ; }
               if(key[kright])if(canRight()) { lapinouPx++ ; if(!flip && canRight())lapinouPx++ ; lapinouWay=0 ; }
             }

             showInt(countFps(),12,12) ;
             if(dscore) showInt(dscore,260,222) ;
             if(dscore < score) dscore+=30 ;
             if(dscore > score) dscore = score ;
             //showInt(getTileM(maparray,lapinouPx+26,lapinouPy),12,42) ;

             if(lapinou == Fall || (lapinou == Salto && Salto->curentFrm > 5))
             {   if(!canDown()) { switch2anim(&lapinou,Stance) ; dstTime=tick ; }
                 else           if(lapinou == Fall)
                                  for(int c=0;c<lapinouDwnSpeed;c++) { if(canDown()) --lapinouPy ;
                                                                       else break ;
                                                                     }
             } else if(lapinou == Stance || lapinou == Walk) if(canDown()) switch2anim(&lapinou,Fall) ;

             // to catch carots anytime
             getTileM(maparray, lapinouPx+4,  lapinouPy+30);
             getTileM(maparray, lapinouPx+27, lapinouPy+30);
             getTileM(maparray, lapinouPx+16, lapinouPy+16);

             if(lapinou == Stance || lapinou == Walk)
             {   if(lapinou == Stance)
                 {   if(key[kleft]||key[kright])
                     { switch2anim(&lapinou,Walk) ;
                     }
                 } else if(lapinou == Walk)
                        {   if(!(key[kleft]||key[kright]))
                            { switch2anim(&lapinou,Stance) ;
                            }
                        }

                if(dstTime + 242 < tick)
                 if( isDestructible() )
                 {   static int lastDestructTime = 0, dsTime = 424 ;

                     if(lastDestructTime + dsTime < tick)
                     {  static int last=1 ;       if(lastDestructTime + 1024 < tick) last = rand()%2 ;
                        lastDestructTime = tick ; int x = lapinouPx+4 + 22*last, tile=0, deb=0, t ;
                        t = getTileM(maparray,x,lapinouPy) ; last^=1 ;
                        if(t < 2 || t>13) { x = lapinouPx+4 + 22*last ; t = getTileM(maparray,x,lapinouPy) ;}
                        if(t > 1 || t<14) {
                          switch(t)
                          {  case 4  : tile=9 ; deb=16 ; dsTime=224 ; break ; // full red
                             case 9  : tile=10; deb=17 ; dsTime=224 ; break ; // broken1 red
                             case 10 : tile=13; deb=18 ; dsTime=224 ; break ; // broken2 red
                             case 13 :  deb=13;          dsTime=224 ; break ; // broken3 red

                             case 3  : tile=7 ; deb=16 ; dsTime=324 ; break ; // full yellow
                             case 7  : tile=8 ; deb=17 ; dsTime=324 ; break ; // broken1 yellow
                             case 8  : tile=12; deb=18 ; dsTime=324 ; break ; // broken2 yellow
                             case 12 :  deb=12;          dsTime=324 ; break ; // broken3 yellow

                             case 2  : tile=5 ; deb=16 ; dsTime=424 ; break ; // full green
                             case 5  : tile=6 ; deb=17 ; dsTime=424 ; break ; // broken1 green
                             case 6  : tile=11; deb=18 ; dsTime=424 ; break ; // broken2 green
                             case 11 :  deb=11;          dsTime=424 ; break ; // broken3 green

                             case 1  : tile=1 ; deb=0  ; dsTime=424 ; break ; // full tile
                          }; setTileM(maparray,x,lapinouPy,tile) ;
                          if(deb) addDebris(x,lapinouPy-32,tiles[deb]) ;
                        }
                     }
                 }
            }

             if(key[kup])
               if(lapinou != Up || lapinou->curentFrm > 1)
                 switch2anim(&lapinou,Up) ;

            playAnim(&lapinou,lapinouPx-mapScrollx,lapinouPy-mapScrolly,lapinouWay) ;

            ptc_update(pixel) ;
    }
}


