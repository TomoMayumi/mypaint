#include<stdio.h>
#include<png.h>
#include"inc3.h"
#include <X11/extensions/shape.h>  /* Shape Extension    */

#define MAX_HISTORY 20 //����������¸��


/**********************************/
/* �ץ�ȥ��������               */
/* (�ե����볰�������Ѥ���ؿ�) */
/**********************************/
//void swapHistory(int num1,int num2);

/**************************************/
/* �ץ�ȥ��������                   */
/* (���Υե�������ǤΤ߻��Ѥ���ؿ�) */
/**************************************/

void save();
void remapLayer(Window *layer_expose,Window *layer,Pixmap *mask,int num);
void remapLayerLast(Window *layer_expose,Window *layer,Pixmap *mask,int num);


/************/
/* ����ѿ� */
/************/

Pixmap history_pix[MAX_HISTORY];           //�����ѥԥå����ޥå�ID
Pixmap history_mask_pix[MAX_HISTORY];      //�����ѥޥ����ԥå����ޥå�ID
Pixmap last_pix[MAX_LAYER];           //�ǽ������ѥԥå����ޥå�ID
Pixmap last_mask_pix[MAX_LAYER];      //�ǽ������ѥޥ����ԥå����ޥå�ID
int history_num[MAX_HISTORY];              //�����ѥ쥤�䡼�ֹ�
GC hcopy_gc;                        //���ԡ��ѥ���ե��å�����ƥ����� ID
GC hmask_gc;                        //�ޥ����ѥ���ե��å�����ƥ����� ID


int bottom,top,next;
int width,height;
Display *dis;
Window win;

void initHistory(Display *display, Window window, int canvas_width, int canvas_height){

  int i;

  bottom=0;
  top=0;
  next=0;
  
  width=canvas_width;
  height=canvas_height;
  dis=display;
  win=window;
  
  // �����ѥԥå����ޥå׺���
  for(i=0;i<MAX_HISTORY;i++){
    history_pix[i] = XCreatePixmap( dis, win, width, height, DefaultDepth(dis,0) );
    history_mask_pix[i] = XCreatePixmap( dis, win, width, height, 1 );
  }
  hcopy_gc = XCreateGC( dis, DefaultRootWindow(dis), 0, 0 );    // GC��ɸ������
  XSetForeground( dis, hcopy_gc, GetColor( dis, "white")  );
  hmask_gc = XCreateGC( dis, history_mask_pix[0], 0, 0 );    // GC��ɸ������
  //XSetForeground( dis, hmask_gc, 1 );

  for(i=0;i<MAX_HISTORY;i++){
    XFillRectangle( dis, history_pix[i], hcopy_gc, 0, 0, width, height );
    XFillRectangle( dis, history_mask_pix[i], hmask_gc, 0, 0, width, height );
  }

  XSetGraphicsExposures( dis, hcopy_gc, False );   //���줬�ʤ��ȥ��ԡ���˥��٥�ȥޥ�����
                                                  //�ط��ʤ�NoExpose���٥�Ȥ������Ƥ��ޤ���
                                                  //����ʥ��٥�Ƚ�����˸���롣
  XSetGraphicsExposures( dis, hmask_gc, False );
  
  // �ǽ������ѥԥå����ޥå׺���
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
  if(next==top){
    top=(top+1)%MAX_HISTORY;
  }
  next=(next+1)%MAX_HISTORY;
  if( next == bottom){
    bottom = (bottom+1)%MAX_HISTORY;
    printf("bottom up\n");
    XCopyArea( dis, history_pix[next], last_pix[history_num[next]], hcopy_gc,
	       0, 0, width, height, 0, 0 );
    XCopyArea( dis, history_mask_pix[next], last_mask_pix[history_num[next]], hmask_gc,
	       0, 0, width, height, 0, 0 );
  }
}


int backHistory(Window *layer_expose,Window *layer,Pixmap *mask){
  int ret,change_num;
  if(next==bottom){
    printf("no history\n");//����ʾ����ʤ�
    return(-1);
  }else{
    next=(next+MAX_HISTORY-1)%MAX_HISTORY;
    ret=history_num[next];
    for(change_num=(next+MAX_HISTORY-1)%MAX_HISTORY;
	change_num!=((bottom+MAX_HISTORY-1)%MAX_HISTORY) && history_num[change_num]!=ret;
	change_num=(change_num+MAX_HISTORY-1)%MAX_HISTORY);
    if(change_num==((bottom+MAX_HISTORY-1)%MAX_HISTORY)||next==bottom){
      printf("remapLast in backHistory\n");
      remapLayerLast(layer_expose,layer,mask,ret);      
    }else{
      printf("remap %d in backHistory\n",change_num);
      remapLayer(layer_expose,layer,mask,change_num);
    }
    printf("%d\n",ret);
    return(ret);
  }
}

void forwardHistory(Window *layer_expose,Window *layer,Pixmap *mask){
  if(next == top){
    ;//����ʾ�ʤ�ʤ�
  }else{
    remapLayer(layer_expose,layer,mask,next);
    next=(next+1)%MAX_HISTORY;
  }
}

void remapLayerLast(Window *layer_expose,Window *layer,Pixmap *mask,int num){
  XCopyArea( dis, last_pix[num], layer_expose[num],
	     hcopy_gc, 0, 0, width, height, 0, 0 );
  XCopyArea( dis, last_pix[num], layer[num],
	     hcopy_gc, 0, 0, width, height, 0, 0 );
  XCopyArea( dis, last_mask_pix[num], mask[num],
	     hmask_gc, 0, 0, width, height, 0, 0 );

  XShapeCombineMask( dis, layer[num], ShapeClip, 0, 0,
		     mask[num], ShapeSet);
  XShapeCombineMask( dis, layer[num], ShapeBounding, 0, 0,
		     mask[num], ShapeSet);
}


void remapLayer(Window *layer_expose,Window *layer,Pixmap *mask,int num){
  int history_num_tmp=history_num[num];

  XCopyArea( dis, history_pix[num], layer_expose[history_num_tmp],
	     hcopy_gc, 0, 0, width, height, 0, 0 );
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
  unsigned char   **image;         // image[HEIGHT][WIDTH]�η����Ǥ�

  int             i, j, pixeldata;
  XImage *ximage;
  
  image = (unsigned char**)malloc(height * sizeof(unsigned char*)); // �ʲ����Ԥϣ������������ݤ��ޤ�
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
  write_png("test2.png", image, width, height);                           // PNG�ե������������ޤ�
  for (j = 0; j < height; j++) free(image[j]);            // �ʲ����Ԥϣ����������������ޤ�
  free(image);
  
}

void swapHistory(int num1,int num2){
  int i;

  Pixmap tmplayer;
  Pixmap tmpmask;
  GC tmpgc;
  GC tmpmaskgc;

  tmplayer = XCreatePixmap(dis, last_pix[num1], CANV_WIDTH, CANV_HEIGHT, DefaultDepth(dis,0));
  tmpmask = XCreatePixmap(dis, last_mask_pix[num1], CANV_WIDTH, CANV_HEIGHT, 1);

  tmpgc = XCreateGC( dis, last_pix[num1], 0, 0 );
  tmpmaskgc = XCreateGC( dis, last_mask_pix[num1], 0, 0 );


  //�����������ؤ�
  XCopyArea(dis, last_pix[num1], tmplayer, tmpgc, 0, 0, CANV_WIDTH, CANV_HEIGHT, 0, 0);
  XCopyArea(dis, last_pix[num2], last_pix[num1], tmpgc, 0, 0, CANV_WIDTH, CANV_HEIGHT, 0, 0);
  XCopyArea(dis, tmplayer, last_pix[num2], tmpgc, 0, 0, CANV_WIDTH, CANV_HEIGHT, 0, 0);
  //�ޥ����������ؤ�
  XCopyArea(dis, last_mask_pix[num1], tmpmask, tmpmaskgc, 0, 0, CANV_WIDTH, CANV_HEIGHT, 0, 0);
  XCopyArea(dis, last_mask_pix[num2], last_mask_pix[num1], tmpmaskgc, 0, 0, CANV_WIDTH, CANV_HEIGHT, 0, 0);
  XCopyArea(dis, tmpmask, last_mask_pix[num2], tmpmaskgc, 0, 0, CANV_WIDTH, CANV_HEIGHT, 0, 0);
  //������Υ쥤�䡼�ֹ������ؤ�
  for(i=bottom;i!=next;i=(i+1)%MAX_HISTORY){
    if(history_num[i]==num1){
      history_num[i]=num2;
    }else if(history_num[i]==num2){
      history_num[i]=num1;
    }
  }
}

void deleteHistory(int num){
  int i,j;

  GC tmpgc;
  GC tmpmaskgc;

  tmpgc = XCreateGC( dis, last_pix[num], 0, 0 );
  tmpmaskgc = XCreateGC( dis, last_mask_pix[num], 0, 0 );

  XSetForeground( dis, tmpgc, GetColor( dis, "white")  );

  //�ǽ���������ˤ���
  XFillRectangle( dis, last_pix[num], tmpgc, 0, 0, width, height );
  XFillRectangle( dis, last_mask_pix[num], tmpmaskgc, 0, 0, width, height );
  //����γ����ս����
  for(i=bottom;i!=next;i=(i+1)%MAX_HISTORY){
      printf("test history_num[i]=%d in deleteHistory\n",history_num[i]);
    if(history_num[i]==num){
      for(j=i;j!=(next+MAX_HISTORY-1)%MAX_HISTORY;j=(j+1)%MAX_HISTORY){
	XCopyArea(dis, history_pix[(j+1)%MAX_HISTORY], history_pix[j], tmpgc,
		  0, 0, CANV_WIDTH, CANV_HEIGHT, 0, 0);
	XCopyArea(dis, history_mask_pix[(j+1)%MAX_HISTORY], history_mask_pix[j], tmpmaskgc,
		  0, 0, CANV_WIDTH, CANV_HEIGHT, 0, 0);
	history_num[j]=history_num[(j+1)%MAX_HISTORY];
      }
      next=(next+MAX_HISTORY-1)%MAX_HISTORY;
      top=(top+MAX_HISTORY-1)%MAX_HISTORY;
      printf("test delete %d in deleteHistory\n",num);
      i=(i+MAX_HISTORY-1)%MAX_HISTORY;
    }
  }

}
