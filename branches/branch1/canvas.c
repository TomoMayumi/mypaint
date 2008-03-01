#include "inc3.h"
#include <X11/extensions/shape.h>  /* Shape Extension    */

#define SELECTED_COLOR GetColor( dis, "rgb:66/66/66")
#define UNSELECTED_COLOR1 GetColor( dis, "rgb:ee/ee/ee")
#define UNSELECTED_COLOR2 GetColor( dis, "rgb:99/99/99")
#define BORDERLINE_COLOR GetColor( dis, "rgb:aa/aa/aa")
#define OVERED_BORDERLINE_COLOR GetColor( dis, "black")

#define MAX_NAME 7 //�쥤�䡼��̾������ʸ����

typedef struct _Layer{
  Window win;
  Pixmap mask;
  Pixmap exact;
  int visible;
  int writable;
  char name[MAX_NAME]
}Layer;


/**********************************/
/* �ץ�ȥ��������               */
/* (�ե����볰�������Ѥ���ؿ�) */
/**********************************/
//void initLayer(Display *disp,Window win,Window canvas);
//int eventLayerMenu(XEvent ev,Window canvas);
//void remapCanvas();
//void save_png(Window canvas);

/**************************************/
/* �ץ�ȥ��������                   */
/* (���Υե�������ǤΤ߻��Ѥ���ؿ�) */
/**************************************/


void remapLayerMenu();
void addLayer(Window canvas);
void swapLayer(int num1,int num2);
void deleteLayer(int num);
void changeLayerName(int num);

/************/
/* ����ѿ� */
/************/

Display *dis;

Window canvas;
Layer layer[MAX_LAYER];            //�쥤�䡼
int layer_num;                     //�쥤�䡼�λ��ѿ�
int stack[MAX_LAYER];           //�쥤�䡼�ξ岼��

/**********************************/
/* initCanvas()                   */
/* �����Х�(�쥤�䡼)������ؿ� */
/**********************************/

Window* initCanvas(Display *disp,Window root){
  int i,j;
  GC gc;
  //ɸ��ǥ����ץ쥤�ΰ��֤���¸
  dis=disp;
  //�����Х�������ɥ��κ���
  canvas = XCreateSimpleWindow( dis, root,
				0, 0,
				CANV_WIDTH, CANV_HEIGHT,
				0, 0, background );
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
  int selected=getSelectedNumber();//�������򤵤�Ƥ���쥤�䡼
  int back_num;
  switch(ev.type){
  case ButtonPress:
    //������å��λ�
    if( ev.xbutton.button == 1 ){
      // �����Х���
      if ( ev.xany.window == canvas ){
	if(layer[selected].writable!=0){//�񤭹�������γ�ǧ
	  execFunc(ev,layer,selected);//����ؿ��μ¹�
	}
	return(0);
      }
      for(i=0;i<layer_num;i++){	  
	// �쥤�䡼��  
	if ( ev.xany.window == layer[i].win ){
	  if(layer[selected].writable!=0){//�񤭹�������γ�ǧ
	    execFunc(ev,layer,selected);//����ؿ��μ¹�
	  }
	  return(0);
	}
      }
    }
    // �ۥ����륯��å��λ�
    if( ev.xbutton.button == 2 ){
      back_num = backHistory(layer); // �������
      return(0);
    }
    
    break;
    
  case Expose:
    if ( ev.xany.window == canvas ){
      remapCanvas();// �쥤�䡼������
      return(0);
    }
    for(i=0;i<layer_num;i++){	    
      if ( ev.xany.window == layer[i].win ){
	remapCanvas();// �쥤�䡼������
	return(0);
      }
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
  GC gc = XCreateGC( dis, layer[0].win, 0, 0 ); // ɸ��GC�μ���
  for(i=0;i<layer_num;i++){
    XCopyArea( dis, layer[i].exact, layer[i].win,
	       gc, 0, 0, CANV_WIDTH , CANV_HEIGHT, 0, 0 );
  }
  XFreeGC(dis,gc);
}

/********************/
/* addLayer()       */
/* �쥤�䡼�ɲôؿ� */
/********************/

void addLayer(Window canvas){
  GC mask_gc;
  GC gc;
  int i;

  unsigned long background = WhitePixel(dis,0);

  if(layer_num>=MAX_LAYER)return;

  //�쥤�䡼�κ���
  layer[layer_num].win = XCreateSimpleWindow( dis, canvas,
					      0, 0,
					      CANV_WIDTH, CANV_HEIGHT,
					      0, 0, background );
  XSetWindowBackgroundPixmap( dis, layer[layer_num].win, None );//�طʤ�Ʃ����
    
  XSelectInput( dis, layer[layer_num].win,
		Button1MotionMask
		|ButtonPressMask
		|ButtonReleaseMask
		|ExposureMask );
  //�쥤�䡼�ޥ����κ���
  layer[layer_num].mask = XCreatePixmap(dis, canvas, CANV_WIDTH, CANV_HEIGHT, 1);

  //������Τ���β�����¸�ѥԥ����ޥåפκ���
  layer[layer_num].exact = XCreatePixmap(dis, canvas, CANV_WIDTH, CANV_HEIGHT,
					 DefaultDepth(dis,0));

  //�ޥ����ѣ��ӥå�GC������ɸ������
  mask_gc = XCreateGC( dis, layer[layer_num].mask, 0, 0 );
  XFillRectangle(dis,layer[layer_num].mask,mask_gc,0,0,CANV_WIDTH,CANV_HEIGHT);//�ޥ����ν����(���٤�0�ˤ���)
  XShapeCombineMask( dis, layer[layer_num].win, ShapeBounding, 0, 0,//�쥤�䡼��ޥ�����
		     layer[layer_num].mask, ShapeSet);              // ������ʬ�Τߤη��ˤ���
  XShapeCombineMask( dis, layer[layer_num].win, ShapeClip, 0, 0,    // ShapeBounding :�ܡ������γ�������
		     layer[layer_num].mask, ShapeSet);              // ShapeClip     :�ܡ���������������
  //�̾�GC������ɸ������
  gc = XCreateGC( dis, layer[layer_num].win, 0, 0 );
  XSetForeground( dis, gc, GetColor( dis, "white")  );
  XFillRectangle(dis,layer[layer_num].exact,gc,0,0,CANV_WIDTH,CANV_HEIGHT);//�������ѥԥ����ޥåפν����


  //̾��������
  sprintf(layer[layer_num].name,"Layer%d",layer_num);

  //�Ļ롢�񤭹�������ν����
  layer[layer_num].visible=1;
  layer[layer_num].writable=1;

  //�쥤�䡼��ɽ��
  XMapWindow( dis, layer[layer_num].win);
  //�����å���򤺤餷���ɲ�
  for(i=layer_num;i>0;i++){
    stack[i+1]=stack[i];
  }
  stack[0]=layer_num;

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


/************************************/
/* swapLayer()                      */
/* �쥤�䡼�������ؤ���             */
/* num1,num2:�����ؤ���쥤�䡼�ֹ� */
/************************************/

void swapLayer(int num1,int num2){
  int i;
  Window windows[MAX_LAYER];

  for(i=0;i<layer_num;i++){
    if(stack[i]==num1){
      stack[i]=num2;
    }else if(stack[i]==num2){
      stack[i]=num1;
    }
  }


  for(i=0;i<layer_num;i++){
    windows[i]=layer[stack[i]].win;
  }
  XRestackWindows(dis, windows, layer_num); 

  //remapCanvas();
  remapLayerMenu();

}

/*******************************/
/* upLayer()                   */
/* �쥤�䡼�����̤˻��äƤ���  */
/* num:���äƤ���쥤�䡼�ֹ�  */
/*******************************/
void upLayer(int num){
  int i;

  for(i=0;i<layer_num && stack[i]!=num ;i++);

  if(i>0)swapLayer(stack[i],stack[i-1]);

}
/*******************************/
/* downLayer()                 */
/* �쥤�䡼�����̤˻��äƤ���  */
/* num:���äƤ����쥤�䡼�ֹ�  */
/*******************************/
void downLayer(int num){
  int i;

  for(i=0;i<layer_num && stack[i]!=num ;i++);

  if(i<layer_num-1)swapLayer(stack[i],stack[i+1]);

}

/****************************/
/* deleteLayer()            */
/* �쥤�䡼��������       */
/* num:�������쥤�䡼�ֹ� */
/****************************/

void deleteLayer(int num){
  int i;
  //�쥤�䡼����Ĥ����ʤ�����ư��ʤ�
  if(layer_num==1)return;
  //�������
  deleteHistory(num);
  printf("test in deleteLayer\n");
  //����������쥤�䡼����ֺǸ�ػ��äƤ���
  for(i=num;i<(layer_num-1);i++){
    swapLayer(i,i+1);
  }
  //���
  XDestroyWindow(dis,layer[i]);
  XFreePixmap(dis,layer_mask[i]);
  XFreePixmap(dis,layer_expose[i]);
  XDestroyWindow(dis,layer_item[i]);

  layer_num--;
  //�Ǹ�Υ쥤�䡼�����򤵤�Ƥ������ϡ����μ����Υ쥤�䡼��������֤ˤ��롣
  if(selected_layer>layer_num-1){
    selected_layer=layer_num-1;
  }
}

/****************************/
/* changeLayerName()        */
/* �쥤�䡼��̾�����ѹ����� */
/* num:�ѹ�����쥤�䡼�ֹ� */
/****************************/

void changeLayerName(int num){
  GC gc;
  XEvent ev;    //���٥�ȼ������ѿ�
  KeySym key;
  char strbuf[10];
  int exit_flag=0;
  
  //�̾�GC������ɸ������
  gc = XCreateGC( dis, layer[num], 0, 0 );
  XSetForeground( dis, gc, GetColor( dis, "black")  );
  //̾��������ɥ����طʤ����
  XSetWindowBackground( dis, layer_namew[num], 0xffffff);
  XClearWindow(dis,layer_namew[num]);
  XDrawString( dis, layer_namew[num], gc, 2, 12, layer_name[num], strlen(layer_name[num]));

  //ʸ�����ϼ���
  while(exit_flag==0){
    XNextEvent( dis, &ev );
    switch(ev.type){
    case ButtonPress:
      //�㤦������ɥ�������å����줿���ѹ���λ
      if(ev.xany.window!=layer_namew[num]){
	exit_flag=1;
      }
      break;
    case KeyPress:
      XLookupString((XKeyEvent *)&ev, strbuf, sizeof(strbuf), &key, NULL);
      //BSʸ����Ƚ��
      if(strbuf[0]==8){
	//ʸ�����Ĺ����0�ʾ�ʤ��ʸ�����
	if(strlen(layer_name[num])) layer_name[num][strlen(layer_name[num])-1]='\0';
      }else{
	if( (strlen(layer_name[num]) + strlen(strbuf)) < MAX_NAME){
	  printf("%d %d %d %s in changeLayerName\n",strlen(layer_name[num]),strlen(strbuf),MAX_NAME,layer_name[num]);
	  //ʸ�����Ϣ��
	  char strpos[MAX_NAME];
	  strcpy(strpos,layer_name[num]);
	  sprintf(layer_name[num],"%s%s",strpos,strbuf);
	}
      }
      XClearWindow(dis,layer_namew[num]);
      XDrawString( dis, layer_namew[num], gc, 2, 12, layer_name[num], strlen(layer_name[num]));
      XFlush( dis );
      break;
    case Expose:
      remapLayerMenu();
      break;
    default:
      break;
    }
  }

  //̾��������ɥ����طʤ򸵤��᤹
  XSetWindowBackground( dis, layer_namew[num], SELECTED_COLOR);
  XClearWindow(dis,layer_namew[num]);
  XDrawString( dis, layer_namew[num], gc, 2, 12, layer_name[num], strlen(layer_name[num]));

  //���Ѥ���GC�β���
  XFreeGC(dis,gc);
}
