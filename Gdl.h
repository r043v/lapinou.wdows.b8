/*************
  Gdl ² reborn - pc release
                 ************/

#include "windows.h"

#define kspace 32
#define kenter 13
#define kfin   35
#define kshift 17
#define kleft  37
#define kup    38
#define kright 39
#define kdown  40

void drawGfm(int *Gfm, int x, int y);//, int way=0);
		
struct	anim
{	int	lastTime  ;
	int	frmTime   ;
	int	curentFrm ;
	int	frmNumber ;
    int animType  ;
    void (*onfinish)(struct anim **) ;
    void (*onflip)(struct anim **) ;
    void (*onplay)(struct anim **) ;
  	int	**Gfm, **iGfm ;
};

void	playAnim(struct anim **b, int x, int y, int way=0)
{   int tick = GetTickCount() ;
    struct anim *a = *b ;
	if(a->lastTime + a->frmTime < tick)
	{	if(++(a->curentFrm) >= a->frmNumber)
        {    switch(a->animType)
             { case 0 : a->curentFrm = 0 ; break ;
               case 1 : a->curentFrm = a->frmNumber-1 ; break ;
             };
             if(a->onfinish) (a->onfinish)(b) ;
        } else if(a->onflip) (a->onflip)(b) ;

		a->lastTime = tick ;
	} else if(a->onplay) a->onplay(b) ;
	
	if(!way) drawGfm((a->Gfm)[a->curentFrm],x,y) ;
    else drawGfm((a->iGfm)[a->curentFrm],x,y) ;
}

void ifreeAnim(struct anim *a)
{ if(!(a->iGfm)) return ;
  for(int c=0;c<a->frmNumber;c++) free((a->iGfm)[c]) ;
}

void mifreeAnim(struct anim **a, int nb)
{ for(int c=0;c<nb;c++) ifreeAnim(a[c]) ; }

int** flipGfm(int **Gfm, int nb);

struct anim * setAnim( int **Gfm, int nb,
                       int frmTime, int animType=0,
                       void (*onfinish)(struct anim**)=0,
                       void (*onflip)(struct anim**)=0,
                       void (*onplay)(struct anim**)=0
                     )
{	struct anim *a = (struct anim *)malloc(sizeof(struct anim)) ;
	a->Gfm = Gfm ;	a->frmNumber = nb ;  a->iGfm = flipGfm(Gfm,nb) ;
	a->curentFrm = a->lastTime = 0 ;
    a->onplay=onplay ; a->onflip=onflip ; a->onfinish=onfinish ;
	a->frmTime = frmTime ; a->animType = animType ;
    return a ;
}

int WIDTH=320, HEIGHT=240, fSIZE=WIDTH*HEIGHT, *pixel ;
extern unsigned char key[256] ;
void mPcx2h(const char *pcx, int nb=0, int revertFrame=0);

void resetAnim(struct anim **a)
{ (*a)->curentFrm=0 ; (*a)->lastTime = GetTickCount() ; }

//struct anim * setAnim(int **Gfm, int nb, int frmTime, void (*onfinish)(struct anim*)=0, int animType=1)
//{ return setAnim(Gfm,nb,frmTime,animType,onfinish); }

void addFreeEntry(void * add)
{  static void **freeList = (void**)malloc(512*4) ; // 512 entry max
   static int    entryNum = 0 ;

   if(add) freeList[entryNum++] = add ; // add an entry
   else {  // free all
           //printf("\nfree %i ...",entryNum) ; system("pause") ;
           while(entryNum--) free(freeList[entryNum]) ; free(freeList) ;
   }
}
