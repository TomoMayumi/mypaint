#include<stdio.h>
#include<png.h>
#include"inc.h"


#define MAX_HISTORY 20 //最大履歴保存数


void remap();
void save();


Pixmap history_pix[MAX_HISTORY];         //履歴用ピックスマップID
GC copy_gc;                        //コピー用グラフィックコンテキスト ID


int bottom,next;
int width,height;
Display *dis;
Window win;

void initHistory(Display *display, Window window, int window_width, int window_height){

  int i;

  bottom=0;
  next=0;
  
  width=window_width;
  height=window_height;
  dis=display;
  win=window;
  
  // 履歴用ピックスマップ作成
  for(i=0;i<MAX_HISTORY;i++){
    history_pix[i] = XCreatePixmap( dis, win, width, height, DefaultDepth(dis,0) );
  }
  copy_gc = XCreateGC( dis, DefaultRootWindow(dis), 0, 0 );    // GCの標準設定
  XSetForeground( dis, copy_gc, GetColor( dis, "white")  );

  for(i=0;i<MAX_HISTORY;i++){
    XFillRectangle( dis, history_pix[i], copy_gc, 0, 0, width, height );
  }

  XSetGraphicsExposures( dis, copy_gc, False );
  
}

void addHistory(){
  XCopyArea( dis, win, history_pix[next], copy_gc, 0, 0, width, height, 0, 0 );
  if( (next=(next+1)%MAX_HISTORY) == bottom){
    bottom = (bottom+1)%MAX_HISTORY;
    printf("bottom up\n");
  }
}


void backHistory(){
  if(next==bottom){
    ;//これ以上戻れない
  }else{
    next=(next+MAX_HISTORY-1)%MAX_HISTORY;
    remap();
  }
}

void forwardHistory(){
  if((next+1)%MAX_HISTORY == bottom){
    ;//これ以上進めない
  }else{
    next=next=(next+1)%MAX_HISTORY;
    remap();
  }
}




void remap(){
    XClearWindow(dis,win);
    XCopyArea( dis, history_pix[(next+MAX_HISTORY-1)%MAX_HISTORY], win, copy_gc, 0, 0, width, height, 0, 0 );
}

void view(){
  int w,h,x,y;
  //printf("bottom = %d  next = %d\n",bottom,next);
  //XWriteBitmapFile(dis,"test.xbm",history_pix[(next+MAX_HISTORY-1)%MAX_HISTORY],width,height,-1,-1);

  //XReadBitmapFile(dis,history_pix[(next+MAX_HISTORY-1)%MAX_HISTORY],"test",&w,&h,&history_pix[(next+MAX_HISTORY-2)%MAX_HISTORY],&x,&y);
  //printf("success %d\n",DefaultDepth(dis,0));
  //remap();
  save();
}

void save(){
  unsigned char   **image;                                // image[HEIGHT][WIDTH]の形式です

  int             i, j, pixeldata;
  XImage *ximage;

  printf("%d %d\n",sizeof(png_byte),sizeof(unsigned char));
  
  image = (unsigned char**)malloc(height * sizeof(unsigned char*)); // 以下３行は２次元配列を確保します
  for (j = 0; j < height; j++)
    image[j] = (unsigned char*)malloc(width * sizeof(unsigned char) * 3);

  for (j = 0; j < height; j++) {
    ximage = XGetImage(dis,history_pix[(next+MAX_HISTORY-1)%MAX_HISTORY],0,j,width,1,AllPlanes,ZPixmap);
  
    for(i=0;i<width;i++){
      pixeldata = XGetPixel(ximage,i,0);
      image[j][3*i] = pixeldata>>16&0xff;
      image[j][3*i+1] = pixeldata>>8&0xff;
      image[j][3*i+2] = pixeldata&0xff;
    }
    XPutImage(dis,win,copy_gc,ximage,0,j,0,j,width,1);
  }
  write_png("test2.png", image, width, height);                           // PNGファイルを作成します
  XFlush(dis);
  for (j = 0; j < height; j++) free(image[j]);            // 以下２行は２次元配列を解放します
  free(image);
  
}


