#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "inc3.h"
#include <X11/extensions/shape.h>  /* Shape Extension    */

#define SELECTED_COLOR GetColor( dis, "rgb:66/66/66")
#define UNSELECTED_COLOR1 GetColor( dis, "rgb:ee/ee/ee")
#define UNSELECTED_COLOR2 GetColor( dis, "rgb:99/99/99")
#define BORDERLINE_COLOR GetColor( dis, "rgb:aa/aa/aa")
#define OVERED_BORDERLINE_COLOR GetColor( dis, "black")

#define MAX_NAME 7 //�쥤�䡼��̾������ʸ����


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

static Display *dis;


Window layer_menu;                  //�쥤�䡼�����ѥ�˥塼������ɥ�

Window add_layer;                   //�쥤�䡼�ɲåܥ���
Window delete_layer;                //�쥤�䡼����ܥ���
Window up_layer;                    //�쥤�䡼�����ؤ��ܥ���
Window down_layer;                  //�쥤�䡼�����ؤ��ܥ���

Window layer_item[MAX_LAYER];       //�ƥ쥤�䡼�ѥ�����ɥ�
Window layer_namew[MAX_LAYER];      //�쥤�䡼��̾��������ɥ�
char layer_name[MAX_LAYER][MAX_NAME]; //�쥤�䡼��̾��
Window layer_viewable[MAX_LAYER];   //�Ļ��ԲĻ������ܥ���
int view_state[MAX_LAYER];          //�Ļ��ԲĻ��������� 1:�� 0:�Բ�
Window layer_writeable[MAX_LAYER];  //�񤭹��߲�ǽ������ܥ���
int write_state[MAX_LAYER];         //�񤭹��߲�ǽ��������� 1:�� 0:�Բ�

Window layer[MAX_LAYER];            //ɽ������쥤�䡼
Pixmap layer_mask[MAX_LAYER];       //Ʃ����ʬ�Υޥ���
Pixmap layer_expose[MAX_LAYER];     //�쥤�䡼�μºݤβ���

int layer_num;                      //�쥤�䡼�λ��ѿ�

int selected_layer;              //�������򤵤�Ƥ���쥤�䡼�ֹ�

/**********************/
/* initLayer()        */
/* �쥤�䡼������ؿ� */
/**********************/

void initLayer(Display *disp,Window win,Window canvas){
  int i,j;
  GC mask_gc;
  GC gc;

  //ɸ��ǥ����ץ쥤�ΰ��֤���¸
  dis=disp;
  //���ѥ쥤�䡼���ν����
  layer_num = 0;

  //��˥塼������ɥ��κ���  
  layer_menu = XCreateSimpleWindow( dis, win,
				    MAIN_WIDTH-LAYER_WIDTH, 0,
				    LAYER_WIDTH-2, LAYER_HEIGHT-2,
				    1, 0, GetColor(dis,"gray") );
  XSelectInput( dis, layer_menu,ExposureMask);

  //�쥤�䡼�ɲåܥ���κ���
  add_layer = XCreateSimpleWindow( dis, layer_menu,
				   2, 2,
				   LAYER_WIDTH/2-2-4-2, 20-2,
				   1, 0, GetColor(dis,"gray") );
  XSelectInput( dis, add_layer,ButtonPressMask);
  XMapWindow( dis, add_layer);

  //�쥤�䡼����ܥ���κ���
  delete_layer = XCreateSimpleWindow( dis, layer_menu,
				   2, 2+20+2,
				   (LAYER_WIDTH-2)/2-4-2, 20-2,
				   1, 0, GetColor(dis,"gray") );
  XSelectInput( dis, delete_layer,ButtonPressMask);
  XMapWindow( dis, delete_layer);

  //�쥤�䡼�����ؤ��ܥ���κ���
  up_layer = XCreateSimpleWindow( dis, layer_menu,
				   (LAYER_WIDTH-2)/2+2, 2,
				   (LAYER_WIDTH-2)/2-4-2, 21-2,
				   1, 0, GetColor(dis,"gray") );
  XSelectInput( dis, up_layer,ButtonPressMask);
  XMapWindow( dis, up_layer);
  down_layer = XCreateSimpleWindow( dis, layer_menu,
				   (LAYER_WIDTH-2)/2+2, 23,
				   (LAYER_WIDTH-2)/2-4-2, 21-2,
				   1, 0, GetColor(dis,"gray") );
  XSelectInput( dis, down_layer,ButtonPressMask);
  XMapWindow( dis, down_layer);

  //����쥤�䡼�κ���
  addLayer(canvas);

  //�쥤�䡼����ν����
  selected_layer=0;
  XSetWindowBackground( dis, layer_namew[0], SELECTED_COLOR);

}

/**********************************/
/* eventLayerMenu()               */
/* �쥤�䡼�ط��Υ��٥�Ƚ����ؿ� */
/**********************************/

int eventLayerMenu(XEvent ev,Window canvas){
  int i,j;
  int back_num;
  GC gc = XCreateGC( dis, layer[selected_layer], 0, 0 ); // GC��ɸ������
  switch(ev.type){
  case ButtonPress:
    //������å��λ�
    if( ev.xbutton.button == 1 ){
      // �����Х���
      if ( ev.xany.window == canvas ){
	if(write_state[selected_layer]!=0){//�񤭹�������γ�ǧ
	  execFunc(ev,layer_expose,layer,layer_mask,selected_layer);//����ؿ��μ¹�
	}
	return(0);
      }
      for(i=0;i<layer_num;i++){	  
	// �쥤�䡼��  
	if ( ev.xany.window == layer[i] ){
	  if(write_state[selected_layer]!=0){//�񤭹�������γ�ǧ
	    execFunc(ev,layer_expose,layer,layer_mask,selected_layer);//����ؿ��μ¹�
	  }
	  return(0);
	}
	// �쥤�䡼��̾����ʬ
	if ( ev.xany.window == layer_namew[i] ){
	  if(selected_layer == i){
	    changeLayerName(i);
	    return(0);
	  }else{
	    XSetWindowBackground( dis, layer_namew[selected_layer], UNSELECTED_COLOR1);
	    XClearWindow(dis,layer_namew[selected_layer]);
	    selected_layer = i; //�쥤�䡼�ѹ�
	    XSetWindowBackground( dis, layer_namew[i], SELECTED_COLOR);
	    XClearWindow(dis,layer_namew[i]);
	    remapLayerMenu();
	    return(0);
	  }
	}
	// �쥤�䡼�βĻ�����ܥ���
	if ( ev.xany.window == layer_viewable[i] ){
	  if(view_state[i]==0){//������褦�ˤ���
	    view_state[i]=1;
	    XMapWindow( dis, layer[i]);
	    remapLayerMenu();

	  }else{//�����ʤ�����
	    view_state[i]=0;
	    XUnmapWindow( dis, layer[i]);
	    write_state[i]=0;
	    remapLayerMenu();


	  }
	  return(0);
	}
	// �쥤�䡼�ν񤭹�������ܥ���
	if ( ev.xany.window == layer_writeable[i] ){
	  if(write_state[i]==0){//�񤭹��߲�ǽ�ˤ���
	    write_state[i]=1;
	    view_state[i]=1;
	    XMapWindow( dis, layer[i]);
	    remapLayerMenu();

	  }else{//�񤭹����ԲĤˤ���
	    write_state[i]=0;
	    remapLayerMenu();


	  }
	  return(0);
	}
      }
      //�쥤�䡼�ɲåܥ���
      if( ev.xany.window == add_layer ){
	addLayer(canvas);
	return(0);
      }
      //�쥤�䡼����ܥ���
      if( ev.xany.window == delete_layer ){
	deleteLayer(selected_layer);
	XSetWindowBackground( dis, layer_namew[selected_layer], SELECTED_COLOR);
	XClearWindow(dis,layer_namew[selected_layer]);
	remapLayerMenu();
	return(0);
      }
      //�쥤�䡼�����ؤ��ܥ���
      if( ev.xany.window == up_layer ){
	if(selected_layer>0){

	  swapLayer(selected_layer,selected_layer-1);

	  XSetWindowBackground( dis, layer_namew[selected_layer], UNSELECTED_COLOR1);
	  XClearWindow(dis,layer_namew[selected_layer]);

	  selected_layer--;

	  XSetWindowBackground( dis, layer_namew[selected_layer], SELECTED_COLOR);
	  XClearWindow(dis,layer_namew[selected_layer]);
	  remapLayerMenu();
	}
	return(0);
      }
      if( ev.xany.window == down_layer ){
	if(selected_layer<layer_num-1){
	  swapLayer(selected_layer,selected_layer+1);

	  XSetWindowBackground( dis, layer_namew[selected_layer], UNSELECTED_COLOR1);
	  XClearWindow(dis,layer_namew[selected_layer]);

	  selected_layer++;

	  XSetWindowBackground( dis, layer_namew[selected_layer], SELECTED_COLOR);
	  XClearWindow(dis,layer_namew[selected_layer]);
	  remapLayerMenu();
	}
	return(0);
      }
      
    }
    // �ۥ����륯��å��λ�
    if( ev.xbutton.button == 2 ){
      back_num = backHistory(layer_expose,layer,layer_mask); // �������
      return(0);
    }
    
    break;
    
  case Expose:
    if ( ev.xany.window == canvas ){
      remapCanvas();// �쥤�䡼������
      return(0);
    }
    for(i=0;i<layer_num;i++){	    
      if ( ev.xany.window == layer[i] ){
	remapCanvas();// �쥤�䡼������
	return(0);
      }
    }
    if ( ev.xany.window == layer_menu ){
      remapLayerMenu();// �쥤�䡼��˥塼������
      return(0);
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
  GC gc = XCreateGC( dis, layer[selected_layer], 0, 0 ); // ɸ��GC�μ���
  for(i=0;i<layer_num;i++){
    XCopyArea( dis, layer_expose[i], layer[i],
	       gc, 0, 0, CANV_WIDTH , CANV_HEIGHT, 0, 0 );
  }
  XFreeGC(dis,gc);
}

/******************************/
/* remapLayerMenu()           */
/* �쥤�䡼��˥塼������ؿ� */
/******************************/

void remapLayerMenu(){
  int i;
  int button_width=(LAYER_WIDTH-2-4-2)/2-2;
  int button_height=15-2;
  GC gc = XCreateGC( dis, layer[selected_layer], 0, 0 ); // ɸ��GC�μ���
  for(i=0;i<layer_num;i++){
    //�ܥ������ͤκ�����
    XSetForeground(dis,gc,GetColor(dis,"black"));
    {//�Ļ�����ܥ���
      XClearWindow(dis,layer_viewable[i]);
      XFillArc(dis, layer_viewable[i], gc,
	       button_width/2-(button_height*0.4142)/2,
	       button_height/2-(button_height*0.4142)/2,
	       button_height*0.4142, button_height*0.4142, 0, 360*64);
      XDrawArc(dis, layer_viewable[i], gc,
	       button_width/2-(button_height*0.4142)/2,
	       button_height/2-(button_height*0.4142)/2,
	       button_height*0.4142, button_height*0.4142, 0, 360*64);
      XDrawArc(dis, layer_viewable[i], gc,
	       button_width/2-button_height/2-1, button_height*(1-1.4142/2)-1,
	       button_height, button_height, 30*64, 120*64);
      XDrawArc(dis, layer_viewable[i], gc,
	       button_width/2-button_height/2-1, button_height*(1.4142/2-1)-1,
	       button_height, button_height, 210*64, 120*64);
      XDrawLine(dis, layer_viewable[i], gc, 5,2,6,3);
      XDrawLine(dis, layer_viewable[i], gc, 12,2,11,3);
      XDrawLine(dis, layer_viewable[i], gc, 8,1,8,2);
    }
    {//�񤭹�������ܥ���
      XClearWindow(dis,layer_writeable[i]);
      XPoint points[7];
      points[0].x=3;points[0].y=7;
      points[1].x=11;points[1].y=1;
      points[2].x=15;points[2].y=5;
      points[3].x=7;points[3].y=11;
      points[4].x=3;points[4].y=11;
      points[5].x=3;points[5].y=7;
      points[6].x=7;points[6].y=11;
      XDrawLines(dis, layer_writeable[i], gc, points, 7, CoordModeOrigin ); 
    }
    //̾��������
    XDrawString( dis, layer_namew[i], gc, 2, 12, layer_name[i], strlen(layer_name[i]));
    //�ԲĤ�����ΤȤ���ˡߤ��դ���
    XSetForeground(dis,gc,GetColor(dis,"red"));
    if(view_state[i]==0){
      XDrawLine(dis, layer_viewable[i], gc, 3,1,13,11);
      XDrawLine(dis, layer_viewable[i], gc, 3,11,13,1);
    }
    if(write_state[i]==0){
      XDrawLine(dis, layer_writeable[i], gc, 3,1,13,11);
      XDrawLine(dis, layer_writeable[i], gc, 3,11,13,1);
    }
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

  unsigned long background = WhitePixel(dis,0);

  if(layer_num>=MAX_LAYER)return;

  //�쥤�䡼�κ���
  layer[layer_num] = XCreateSimpleWindow( dis, canvas,
					  0, 0,
					  CANV_WIDTH, CANV_HEIGHT,
					  0, 0, background );
  XSetWindowBackgroundPixmap( dis, layer[layer_num], None );//�طʤ�Ʃ����
    
  XSelectInput( dis, layer[layer_num],
		Button1MotionMask
		|ButtonPressMask
		|ButtonReleaseMask
		|ExposureMask );
  //�쥤�䡼�ޥ����κ���
  layer_mask[layer_num] = XCreatePixmap(dis, canvas, CANV_WIDTH, CANV_HEIGHT, 1);

  //������Τ���β�����¸�ѥԥ����ޥåפκ���
  layer_expose[layer_num] = XCreatePixmap(dis, canvas, CANV_WIDTH, CANV_HEIGHT,
					  DefaultDepth(dis,0));

  //�ޥ����ѣ��ӥå�GC������ɸ������
  mask_gc = XCreateGC( dis, layer_mask[layer_num], 0, 0 );
  XFillRectangle(dis,layer_mask[layer_num],mask_gc,0,0,CANV_WIDTH,CANV_HEIGHT);//�ޥ����ν����(���٤�0�ˤ���)
  XShapeCombineMask( dis, layer[layer_num], ShapeBounding, 0, 0,//�쥤�䡼��ޥ�����
		     layer_mask[layer_num], ShapeSet);          // ������ʬ�Τߤη��ˤ���
  XShapeCombineMask( dis, layer[layer_num], ShapeClip, 0, 0,    // ShapeBounding :�ܡ������γ�������
		     layer_mask[layer_num], ShapeSet);          // ShapeClip     :�ܡ���������������
  //�̾�GC������ɸ������
  gc = XCreateGC( dis, layer[layer_num], 0, 0 );
  XSetForeground( dis, gc, GetColor( dis, "white")  );
  XFillRectangle(dis,layer_expose[layer_num],gc,0,0,CANV_WIDTH,CANV_HEIGHT);//�������ѥԥ����ޥåפν����


  //�쥤�䡼�����ѥѥͥ�κ���
  layer_item[layer_num] = XCreateSimpleWindow( dis, layer_menu,
					       2, 46+layer_num*34,
					       LAYER_WIDTH-2-4-2, 30,
					       1, 0, GetColor(dis,"gray") );
  
  //�쥤�䡼��̾���ѥ�����ɥ��κ���
  layer_namew[layer_num] = XCreateSimpleWindow( dis, layer_item[layer_num],
						0, 0,
						LAYER_WIDTH-2-4-2, 15,
						0, 0, UNSELECTED_COLOR1 );  
  XSelectInput( dis, layer_namew[layer_num],ButtonPressMask | KeyPressMask);
  //̾��������
  sprintf(layer_name[layer_num],"Layer%d",layer_num);

  //�Ļ롢�񤭹�������ܥ�����礭��
  int button_width=(LAYER_WIDTH-2-4-2)/2-2;
  int button_height=15-2;
  //�쥤�䡼�Ļ�����ܥ���κ���
  layer_viewable[layer_num] = XCreateSimpleWindow( dis, layer_item[layer_num],
						   0, 15,
						   button_width, button_height,
						   1, 0, UNSELECTED_COLOR1 );
  XSelectInput( dis, layer_viewable[layer_num],ButtonPressMask);
  
  //�쥤�䡼�񤭹�������ܥ���κ���
  layer_writeable[layer_num] = XCreateSimpleWindow( dis, layer_item[layer_num],
						    (LAYER_WIDTH-2-4-2)/2, 15,
						    button_width, button_height,
						    1, 0, UNSELECTED_COLOR1 );
  XSelectInput( dis, layer_writeable[layer_num],ButtonPressMask);

  //�Ļ롢�񤭹�������ν����
  view_state[layer_num]=1;
  write_state[layer_num]=1;

  //�쥤�䡼��ɽ��(���������ϲ���ɽ������ʤ�)
  XMapWindow( dis, layer_item[layer_num]);
  //�쥤�䡼����ѥͥ��ɽ��
  XMapSubwindows( dis, layer_item[layer_num]);
  //�ܥ�������ͤ�����
  XSetForeground(dis,gc,GetColor(dis,"black"));
  {//�Ļ�����ܥ���
    XClearWindow(dis,layer_viewable[layer_num]);
    XFillArc(dis, layer_viewable[layer_num], gc,
	     button_width/2-(button_height*0.4142)/2,
	     button_height/2-(button_height*0.4142)/2,
	     button_height*0.4142, button_height*0.4142, 0, 360*64);
    XDrawArc(dis, layer_viewable[layer_num], gc,
	     button_width/2-(button_height*0.4142)/2,
	     button_height/2-(button_height*0.4142)/2,
	     button_height*0.4142, button_height*0.4142, 0, 360*64);
    XDrawArc(dis, layer_viewable[layer_num], gc,
	     button_width/2-button_height/2-1, button_height*(1-1.4142/2)-1,
	     button_height, button_height, 30*64, 120*64);
    XDrawArc(dis, layer_viewable[layer_num], gc,
	     button_width/2-button_height/2-1, button_height*(1.4142/2-1)-1,
	     button_height, button_height, 210*64, 120*64);
    XDrawLine(dis, layer_viewable[layer_num], gc, 5,2,6,3);
    XDrawLine(dis, layer_viewable[layer_num], gc, 12,2,11,3);
    XDrawLine(dis, layer_viewable[layer_num], gc, 8,1,8,2);
  }
  {//�񤭹�������ܥ���
    XClearWindow(dis,layer_writeable[layer_num]);
    XPoint points[7];
    points[0].x=3;points[0].y=7;
    points[1].x=11;points[1].y=1;
    points[2].x=15;points[2].y=5;
    points[3].x=7;points[3].y=11;
    points[4].x=3;points[4].y=11;
    points[5].x=3;points[5].y=7;
    points[6].x=7;points[6].y=11;
    XDrawLines(dis, layer_writeable[layer_num], gc, points, 7, CoordModeOrigin ); 
  }
  //̾��������
  XDrawString( dis, layer_namew[layer_num], gc, 2, 12, layer_name[layer_num], strlen(layer_name[layer_num]));

  //�쥤�䡼��ɽ��
  XMapWindow( dis, layer[layer_num]);

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

void save_png(Window canvas){
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

  Pixmap tmplayer;
  Pixmap tmpmask;
  char tmpchar[MAX_NAME];
  int tmpint;
  GC tmpgc;
  GC tmpmaskgc;

  tmplayer = XCreatePixmap(dis, layer[num1], CANV_WIDTH, CANV_HEIGHT, DefaultDepth(dis,0));
  tmpmask = XCreatePixmap(dis, layer_mask[num1], CANV_WIDTH, CANV_HEIGHT, 1);

  tmpgc = XCreateGC( dis, layer[num1], 0, 0 );
  tmpmaskgc = XCreateGC( dis, layer_mask[num1], 0, 0 );

  //�쥤�䡼��̾�������ؤ�
  strcpy(tmpchar,layer_name[num2]);
  strcpy(layer_name[num2],layer_name[num1]);
  strcpy(layer_name[num1],tmpchar);
  //�񤭹�������������ؤ�
  tmpint = write_state[num1];
  write_state[num1] = write_state[num2];
  write_state[num2] = tmpint;
  //�Ļ�����������ؤ�
  tmpint = view_state[num1];
  view_state[num1] = view_state[num2];
  view_state[num2] = tmpint;

  //�ºݤβ����������ؤ�
  XCopyArea(dis, layer_expose[num1], tmplayer, tmpgc, 0, 0, CANV_WIDTH, CANV_HEIGHT, 0, 0);
  XCopyArea(dis, layer_expose[num2], layer_expose[num1], tmpgc, 0, 0, CANV_WIDTH, CANV_HEIGHT, 0, 0);
  XCopyArea(dis, tmplayer, layer_expose[num2], tmpgc, 0, 0, CANV_WIDTH, CANV_HEIGHT, 0, 0);
  //�ޥ����������ؤ�
  XCopyArea(dis, layer_mask[num1], tmpmask, tmpmaskgc, 0, 0, CANV_WIDTH, CANV_HEIGHT, 0, 0);
  XCopyArea(dis, layer_mask[num2], layer_mask[num1], tmpmaskgc, 0, 0, CANV_WIDTH, CANV_HEIGHT, 0, 0);
  XCopyArea(dis, tmpmask, layer_mask[num2], tmpmaskgc, 0, 0, CANV_WIDTH, CANV_HEIGHT, 0, 0);
  //�ޥ����Τ��ʤ���
  XShapeCombineMask( dis, layer[num1], ShapeBounding, 0, 0, layer_mask[num1], ShapeSet);
  XShapeCombineMask( dis, layer[num1], ShapeClip, 0, 0, layer_mask[num1], ShapeSet);
  XShapeCombineMask( dis, layer[num2], ShapeBounding, 0, 0, layer_mask[num2], ShapeSet);
  XShapeCombineMask( dis, layer[num2], ShapeClip, 0, 0, layer_mask[num2], ShapeSet);
  //ɽ���Τ��ʤ���
  if(view_state[num1]!=view_state[num2]){
    switch(view_state[num1]){
    case 0:
      XMapWindow(dis,layer[num2]);
      XUnmapWindow(dis,layer[num1]);
      break;
    case 1:
      XMapWindow(dis,layer[num1]);
      XUnmapWindow(dis,layer[num2]);
      break;
    default:
      printf("*error[view_state] in swapLayer*\n");
      exit(1);
    }
  }
  //����������ؤ�
  swapHistory(num1,num2);

  remapCanvas();
  remapLayerMenu();

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
	  printf("%zu %zu %d %s in changeLayerName\n",strlen(layer_name[num]),strlen(strbuf),MAX_NAME,layer_name[num]);
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
