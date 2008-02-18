#include<stdio.h>
#include<png.h>
#include"inc2.h"
#include <X11/extensions/shape.h>  /* Shape Extension    */

#define MAX_HISTORY 20 //最大履歴保存数


void save();
void remapLayer(Window *layer,Pixmap *mask,int num);
void remapLayerLast(Window *layer,Pixmap *mask,int num);


Pixmap history_pix[MAX_HISTORY];           //履歴用ピックスマップID
Pixmap history_mask_pix[MAX_HISTORY];      //履歴用マスクピックスマップID
Pixmap last_pix[MAX_LAYER];           //最終履歴用ピックスマップID
Pixmap last_mask_pix[MAX_LAYER];      //最終履歴用マスクピックスマップID
int history_num[MAX_HISTORY];              //履歴用レイヤー番号
GC hcopy_gc;                        //コピー用グラフィックコンテキスト ID
GC hmask_gc;                        //マスク用グラフィックコンテキスト ID


int bottom,next;
int width,height;
Display *dis;
Window win;

void initHistory(Display *display, Window window, int canvas_width, int canvas_height){

  int i;

  bottom=0;
  next=0;
  
  width=canvas_width;
  height=canvas_height;
  dis=display;
  win=window;
  
  // 履歴用ピックスマップ作成
  for(i=0;i<MAX_HISTORY;i++){
    history_pix[i] = XCreatePixmap( dis, win, width, height, DefaultDepth(dis,0) );
    history_mask_pix[i] = XCreatePixmap( dis, win, width, height, 1 );
  }
  hcopy_gc = XCreateGC( dis, DefaultRootWindow(dis), 0, 0 );    // GCの標準設定
  XSetForeground( dis, hcopy_gc, GetColor( dis, "white")  );
  hmask_gc = XCreateGC( dis, history_mask_pix[0], 0, 0 );    // GCの標準設定
  //XSetForeground( dis, hmask_gc, 1 );

  for(i=0;i<MAX_HISTORY;i++){
    XFillRectangle( dis, history_pix[i], hcopy_gc, 0, 0, width, height );
    XFillRectangle( dis, history_mask_pix[i], hmask_gc, 0, 0, width, height );
  }

  XSetGraphicsExposures( dis, hcopy_gc, False );   //これがないとコピー先にイベントマスクに
                                                  //関係なくNoExposeイベントが送られてしまい、
                                                  //正常なイベント処理を妨げる。
  XSetGraphicsExposures( dis, hmask_gc, False );
  
  // 最終履歴用ピックスマップ作成
  for(i=0;i<MAX_LAYER;i++){
    last_pix[i] = XCreatePixmap( dis, win, width, height, DefaultDepth(dis,0) );
    last_mask_pix[i] = XCreatePixmap( dis, win, width, height, 1 );
    XFillRectangle( dis, last_pix[i], hcopy_gc, 0, 0, width, height );
    XFillRectangle( dis, last_mask_pix[i], hmask_gc, 0, 0, width, height );
  }
}

void addHistory(Window *layer,Pixmap *mask,int num){
  XCopyArea( dis, layer[num], history_pix[next], hcopy_gc, 0, 0,
	     width, height, 0, 0 );
  XCopyArea( dis, mask[num], history_mask_pix[next], hmask_gc, 0, 0,
	     width, height, 0, 0 );
  history_num[next]=num;
  if( (next=(next+1)%MAX_HISTORY) == bottom){
    bottom = (bottom+1)%MAX_HISTORY;
    printf("bottom up\n");
    XCopyArea( dis, history_pix[next], last_pix[history_num[next]], hcopy_gc,
	       0, 0, width, height, 0, 0 );
    XCopyArea( dis, history_mask_pix[next], last_mask_pix[history_num[next]], hmask_gc,
	       0, 0, width, height, 0, 0 );
  }
}


int backHistory(Window *layer,Pixmap *mask){
  int ret,change_num;
  if(next==bottom){
    printf("no history\n");//これ以上戻れない
    return(-1);
  }else{
    next=(next+MAX_HISTORY-1)%MAX_HISTORY;
    ret=history_num[next];
    for(change_num=(next+MAX_HISTORY-1)%MAX_HISTORY;
	change_num!=bottom && history_num[change_num]!=ret;
	change_num=(change_num+MAX_HISTORY-1)%MAX_HISTORY);
    if(change_num==bottom||next==bottom){
      remapLayerLast(layer,mask,ret);      
    }else{
      remapLayer(layer,mask,change_num);
    }
    printf("%d\n",ret);
    return(ret);
  }
}

void forwardHistory(Window *layer,Pixmap *mask){
  if((next+1)%MAX_HISTORY == bottom){
    ;//これ以上進めない
  }else{
    remapLayer(layer,mask,next);
    next=(next+1)%MAX_HISTORY;
  }
}

void remapLayerLast(Window *layer,Pixmap *mask,int num){
  XClearWindow(dis,layer[num]);
  XCopyArea( dis, last_pix[num], layer[num],
	     hcopy_gc, 0, 0, width, height, 0, 0 );
  XCopyArea( dis, last_mask_pix[num], mask[num],
	     hmask_gc, 0, 0, width, height, 0, 0 );

  XShapeCombineMask( dis, layer[num], ShapeClip, 0, 0,
		     mask[num], ShapeSet);
  XShapeCombineMask( dis, layer[num], ShapeBounding, 0, 0,
		     mask[num], ShapeSet);
}


void remapLayer(Window *layer,Pixmap *mask,int num){
  int history_num_tmp=history_num[num];

  XClearWindow(dis,layer[history_num_tmp]);
  XCopyArea( dis, history_pix[num], layer[history_num_tmp],
	     hcopy_gc, 0, 0, width, height, 0, 0 );
  XCopyArea( dis, history_mask_pix[num], mask[history_num_tmp],
	     hmask_gc, 0, 0, width, height, 0, 0 );

  XShapeCombineMask( dis, layer[history_num_tmp], ShapeClip, 0, 0,
		     mask[history_num_tmp], ShapeSet);
  XShapeCombineMask( dis, layer[history_num_tmp], ShapeBounding, 0, 0,
		     mask[history_num_tmp], ShapeSet);
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
  unsigned char   **image;         // image[HEIGHT][WIDTH]の形式です

  int             i, j, pixeldata;
  XImage *ximage;
  
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
    //XPutImage(dis,canvas,hcopy_gc,ximage,0,j,0,j,width,1);
  }
  write_png("test2.png", image, width, height);                           // PNGファイルを作成します
  for (j = 0; j < height; j++) free(image[j]);            // 以下２行は２次元配列を解放します
  free(image);
  
}
