// memo

// ������Υ쥤�䡼����̤���


#include "inc4.h"
#include <X11/extensions/shape.h>  /* Shape Extension    */

#define SELECTED_COLOR GetColor( dis, "rgb:66/66/66")
#define UNSELECTED_COLOR1 GetColor( dis, "rgb:ee/ee/ee")
#define UNSELECTED_COLOR2 GetColor( dis, "rgb:99/99/99")
#define BORDERLINE_COLOR GetColor( dis, "rgb:aa/aa/aa")
#define OVERED_BORDERLINE_COLOR GetColor( dis, "black")

/**********************************/
/* �ץ�ȥ��������               */
/* (�ե����볰�������Ѥ���ؿ�) */
/**********************************/
//Window* initCanvas(Display *disp,Window root);
//int eventLayerMenu(XEvent ev);
//void remapCanvas();
//void save_png();

/**************************************/
/* �ץ�ȥ��������                   */
/* (���Υե�������ǤΤ߻��Ѥ���ؿ�) */
/**************************************/

void addLayer();
void deleteLayer(Layer *layer);
void upLayer(Layer *layer);
void downLayer(Layer *layer);

/******************/
/* ����ѿ�(����) */
/******************/

extern Display *dis;       //main���黲��

/************/
/* ����ѿ� */
/************/

Layer *layerlist=NULL;            //�쥤�䡼�ꥹ��(layer.c����⻲��)
static Window canvas;
static int layer_num;             //�쥤�䡼�λ��ѿ�

/**********************************/
/* initCanvas()                   */
/* �����Х�(�쥤�䡼)������ؿ� */
/**********************************/

Window* initCanvas(Window root){
  int i,j;
  GC gc;
  //�����Х�������ɥ��κ���
  canvas = XCreateSimpleWindow( dis, root,
				0, 0,
				CANV_WIDTH, CANV_HEIGHT,
				0, 0, 0xffffff );
  XSelectInput( dis, canvas,
		Button1MotionMask
		|ButtonPressMask
		|ButtonReleaseMask
		|ExposureMask ); 
  //���ѥ쥤�䡼���ν����
  layer_num = 0;

  //����쥤�䡼�κ���
  addLayer();

  return(&canvas);
}

/**********************************/
/* eventCanvas()               */
/* �쥤�䡼�ط��Υ��٥�Ƚ����ؿ� */
/**********************************/

int eventCanvas(XEvent ev){
  int i,j;
  Layer *selectedlayer = getSelectedLayer();//�������򤵤�Ƥ���쥤�䡼
  Layer *layertmp;
  int back_num;

  switch(ev.type){
  case ButtonPress:
    //������å��λ�
    if( ev.xbutton.button == 1 ){
      // �����Х���
      if ( ev.xany.window == canvas ){
	if(selectedlayer->writable!=0){//�񤭹�������γ�ǧ
	  execFunc(ev,selectedlayer);//����ؿ��μ¹�
	}
	return(0);
      }
      layertmp=layerlist;
      for(i=0;i<layer_num;i++){	  
	// �쥤�䡼��  
	if ( ev.xany.window == layertmp->win ){
	  if(layertmp->writable!=0){//�񤭹�������γ�ǧ
	    execFunc(ev,selectedlayer);//����ؿ��μ¹�
	  }
	  return(0);
	}
	layertmp=layertmp->next;
      }
    }
    // �ۥ����륯��å��λ�
    if( ev.xbutton.button == 2 ){
      back_num = backHistory(selectedlayer); // �������
      return(0);
    }
    
    break;
    
  case Expose:
    if ( ev.xany.window == canvas ){
      remapCanvas();// �쥤�䡼������
      return(0);
    }
    layertmp=layerlist;
    for(i=0;i<layer_num;i++){
      if ( ev.xany.window == layertmp->win ){
	remapCanvas();// �쥤�䡼������
	return(0);
      }
      layertmp=layertmp->next;
    }
    break;
  default:
    break;
  }
  return(1);
}

/******************************/
/* remapCanvas()              */
/* ���٤ƤΥ쥤�䡼������ؿ� */
/******************************/

void remapCanvas(){
  int i;
  Layer *layertmp=layerlist;
  GC gc = XCreateGC( dis, layertmp->win, 0, 0 ); // ɸ��GC�μ���
  for(i=0;i<layer_num;i++){
    XCopyArea( dis, layertmp->exact, layertmp->win,
	       gc, 0, 0, CANV_WIDTH , CANV_HEIGHT, 0, 0 );
    layertmp=layertmp->next;
  }
  XFreeGC(dis,gc);
}

/********************/
/* addLayer()       */
/* �쥤�䡼�ɲôؿ� */
/********************/

void addLayer(){
  GC mask_gc;
  GC gc;
  Layer *new;
  int i;

  unsigned long background = WhitePixel(dis,0);

  if(layer_num>=MAX_LAYER)return;

  if( (new=(Layer *)malloc(sizeof(Layer))) == NULL ){
    printf("*malloc error (addLayer)*\n");
  }

  //�쥤�䡼�κ���
  new->win = XCreateSimpleWindow( dis, canvas,
				  0, 0,
				  CANV_WIDTH, CANV_HEIGHT,
				  0, 0, background );
  XSetWindowBackgroundPixmap( dis, new->win, None );//�طʤ�Ʃ����
  
  XSelectInput( dis, new->win,
		Button1MotionMask
		|ButtonPressMask
		|ButtonReleaseMask
		|ExposureMask );
  //�쥤�䡼�ޥ����κ���
  new->mask = XCreatePixmap(dis, canvas, CANV_WIDTH, CANV_HEIGHT, 1);

  //������Τ���β�����¸�ѥԥ����ޥåפκ���
  new->exact = XCreatePixmap(dis, canvas, CANV_WIDTH, CANV_HEIGHT,
			     DefaultDepth(dis,0));

  //�ޥ����ѣ��ӥå�GC������ɸ������
  mask_gc = XCreateGC( dis, new->mask, 0, 0 );
  XFillRectangle(dis,new->mask,mask_gc,0,0,CANV_WIDTH,CANV_HEIGHT);//�ޥ����ν����(���٤�0�ˤ���)
  XShapeCombineMask( dis, new->win, ShapeBounding, 0, 0,//�쥤�䡼��ޥ�����
		     new->mask, ShapeSet);              // ������ʬ�Τߤη��ˤ���
  XShapeCombineMask( dis, new->win, ShapeClip, 0, 0,    // ShapeBounding :�ܡ������γ�������
		     new->mask, ShapeSet);              // ShapeClip     :�ܡ���������������
  //�̾�GC������ɸ������
  gc = XCreateGC( dis, new->win, 0, 0 );
  XSetForeground( dis, gc, GetColor( dis, "white")  );
  XFillRectangle(dis,new->exact,gc,0,0,CANV_WIDTH,CANV_HEIGHT);//�������ѥԥ����ޥåפν����


  //̾��������
  sprintf(new->name,"Layer%d",layer_num);

  //�Ļ롢�񤭹�������ν����
  new->visible=1;
  new->writable=1;

  //�ꥹ�Ȥ��ɲ�
  new->next=layerlist;
  layerlist=new;

  //�쥤�䡼��ɽ��
  XMapWindow( dis, new->win);

  //���ѥ쥤�䡼�������䤹
  layer_num++;
  //���Ѥ���GC�β���
  XFreeGC(dis,gc);
  XFreeGC(dis,mask_gc);
}

/*********************/
/* save_png()        */
/* PNG��������¸���� */
/*********************/

void save_png(){
  Window savemap;
  unsigned char   **image;         // image[HEIGHT][WIDTH]�η����Ǥ�
  int             i, j, pixeldata;
  XImage *ximage;
  
  savemap = XCreateSimpleWindow( dis, canvas,
				 0, 0,
				 CANV_WIDTH, CANV_HEIGHT,
				 0, 0, 0 );
  XSetWindowBackgroundPixmap( dis, savemap, None );//�طʤ�Ʃ����
  XMapWindow(dis,savemap);

  image = (unsigned char**)malloc(CANV_HEIGHT * sizeof(unsigned char*)); // �ʲ����Ԥϣ������������ݤ��ޤ�
  for (j = 0; j < CANV_HEIGHT; j++)
    image[j] = (unsigned char*)malloc(CANV_WIDTH * sizeof(unsigned char) * 3);

  for (j = 0; j < CANV_HEIGHT; j++) {
    ximage = XGetImage(dis,savemap,0,j,CANV_WIDTH,1,AllPlanes,ZPixmap);
  
    for(i=0;i<CANV_WIDTH;i++){
      pixeldata = XGetPixel(ximage,i,0);
      image[j][3*i] = pixeldata>>16&0xff;
      image[j][3*i+1] = pixeldata>>8&0xff;
      image[j][3*i+2] = pixeldata&0xff;
    }
    //XPutImage(dis,canvas,hcopy_gc,ximage,0,j,0,j,CANV_WIDTH,1);
  }
  write_png("test2.png", image, CANV_WIDTH, CANV_HEIGHT);            // PNG�ե������������ޤ�
  for (j = 0; j < CANV_HEIGHT; j++) free(image[j]);            // �ʲ����Ԥϣ����������������ޤ�
  free(image);

  XUnmapWindow(dis,savemap);

  XDestroyWindow(dis,savemap);
  
}


/*******************************/
/* upLayer()                   */
/* �쥤�䡼�����̤˻��äƤ���  */
/* layer:���äƤ���쥤�䡼    */
/*******************************/
void upLayer(Layer *layer){
  int i;
  Layer *layertmp=layerlist;
  Layer *layertmp2;

  if(layertmp==layer)return;

  if(layertmp->next==layer){
    layertmp->next=layer->next;
    layer->next=layertmp;
    layerlist=layer;
  }else{
    for(i=0;i<(layer_num-2);i++){
      if(layertmp->next->next==layer){
	layertmp2=layertmp->next;
	layertmp2->next=layer->next;
	layer->next=layertmp2;
	layertmp->next=layer;
	break;
      }
    }
  }
  resetLayer();
}
/*******************************/
/* downLayer()                 */
/* �쥤�䡼�����̤˻��äƤ���  */
/* num:���äƤ����쥤�䡼�ֹ�  */
/*******************************/
void downLayer(Layer *layer){
  int i;
  Layer *layertmp=layerlist;

  if(layertmp==layer){
    layertmp=layertmp->next;
    layer->next=layertmp->next;
    layertmp->next=layer;
    layerlist=layertmp;
  }else{ 
    for(i=0;i<(layer_num-1);i++){
      if(layertmp->next==layer){
	layertmp->next=layer->next;
	layer->next=layer->next->next;
	layertmp->next->next=layer;
	break;
      }
    }
  }
  resetLayer();
}

/****************************/
/* deleteLayer()            */
/* �쥤�䡼��������       */
/* num:�������쥤�䡼�ֹ� */
/****************************/

void deleteLayer(Layer *layer){
  int i;
  Layer *layertmp=layerlist;
  //�쥤�䡼����Ĥ����ʤ�����ư��ʤ�
  if(layer_num==1)return;
  //�������
  deleteHistory(layer);
  printf("test in deleteLayer\n");
  //����������쥤�䡼��ꥹ�Ȥ������
  if(layertmp==layer)
    layerlist=layerlist->next;
  else{
    for(i=0;i<(layer_num-1);i++){
      if(layertmp->next==layer){
	layertmp->next=layer->next;
	break;
      }
    }
  }
  //���
  XDestroyWindow(dis,layer->win);
  XFreePixmap(dis,layer->mask);
  XFreePixmap(dis,layer->exact);
  DeleteLayerItem(layer);
  free(layer);

  layer_num--;
  //�Ǹ�Υ쥤�䡼�����򤵤�Ƥ������ϡ����μ����Υ쥤�䡼��������֤ˤ��롣
  //if(selected_layer>layer_num-1){
  //  selected_layer=layer_num-1;
  //}
}
