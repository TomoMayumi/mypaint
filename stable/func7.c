#include <stdio.h>
#include <stdlib.h>
#include "inc3.h"
#include <X11/extensions/shape.h>  /* Shape Extension    */

#define MAX_COLOR 6  /* ���� */
#define MAX_PEN 6 /* �ڥ�������� */
#define MAX_FUNC 5 /* ��ǽ�� */
#define MAX_FUNC_SUB 2 /* �����ǽ�� */
#define MAX_TYPE 9 /* ����ο� */

#define SELECTED_COLOR GetColor( dis, "rgb:66/66/66")
#define UNSELECTED_COLOR1 GetColor( dis, "rgb:ee/ee/ee")
#define UNSELECTED_COLOR2 GetColor( dis, "rgb:99/99/99")
#define BORDERLINE_COLOR GetColor( dis, "rgb:aa/aa/aa")
#define OVERED_BORDERLINE_COLOR GetColor( dis, "black")

void freeHand(XEvent ev,Window *layer_expose,Window *layer,Window *mask,int num);
void line(XEvent ev,Window *layer_expose,Window *layer,Window *mask,int num);
void square(XEvent ev,Window *layer_expose,Window *layer,Window *mask,int num);
void circle(XEvent ev,Window *layer_expose,Window *layer,Window *mask,int num);
void fill(XEvent ev,Window *layer_expose,Window *layer,Window *mask,int num);

void fillCheck(int x,int y,unsigned long pixel,XImage *ximage[],XImage *xmaskimage);

static Display *dis;
Window canvas;

Window color_win[MAX_COLOR];                 //�������Window  ID
Window colors_win;                           //�����������Window  ID
Window func_win[MAX_FUNC];                   //��ǽ�����Window  ID
Window func_sub_win[MAX_FUNC];               //�����ǽ��Window  ID
Window func_type_win[MAX_FUNC][MAX_FUNC_SUB];               //�����ǽ��Window  ID
Window pen_win[MAX_PEN];                     //���������Window  ID
Window line_type_win[MAX_TYPE];              //���������Window  ID
Window debug_win;                            //�ǥХå��Ѥ�Window  ID

GC gc;                                       //����ե��å�����ƥ����� ID
GC mask_gc;                                       //����ե��å�����ƥ����� ID
GC copy_gc;                                  //���ԡ��ѥ���ե��å�����ƥ����� ID
Pixmap copy_pix[2];                          //���ԡ��ѥԥ����ޥå�
Pixmap copymask_pix;                          //���ԡ��ޥ����ѥԥ����ޥå�
Pixmap tilepix;                              //�طʥ������ѥԥ����ޥå�

char *color_name[]={"black",
		    "red",
		    "green",
		    "blue",
		    "yellow",
		    "white"};//���֥�����ɥ�������Ǥ��뿧

unsigned long background; //�ᥤ�󥦥���ɥ����طʿ�
unsigned long foreground; //�ᥤ�󥦥���ɥ����طʿ�

int button_size=20;    //���֥�����ɥ��Υ�����

int state=FREEHAND; // �����ε�ǽ�����򤵤�Ƥ��뤫��
int sub_state=0; // �����������ǽ�����򤵤�Ƥ��뤫��

unsigned long current_color=0; // ��������
unsigned long current_pen=2; // �������Ϥ����Ĥ�
unsigned long current_line=LineSolid; // ���μ���
unsigned long current_cap=CapRound; // ���ν�ü
unsigned long current_join=JoinMiter; // Ϣ³ľ���ΤĤʤ���

void initFuncMenu(Display *disp,Window function,Window canv){

    int i,j,k;

    dis = disp;
    canvas = canv;
    background = WhitePixel( dis, 0);
    foreground = BlackPixel( dis, 0);

    /***************************/
    // GC�ν������
    /***************************/
    
    gc = XCreateGC( dis, DefaultRootWindow(dis), 0, 0 );    // GC��ɸ������
    XSetForeground( dis, gc, current_color  );    // �޷��ο�������
    XSetBackground( dis, gc, background);
    XSetLineAttributes( dis, gc, current_pen, current_line, current_cap, current_join);


    /***************************/
    // �ޥ����ѥ����ƥ�ν������
    /***************************/
    copymask_pix = XCreatePixmap( dis, canvas, CANV_WIDTH, CANV_HEIGHT, 1 );

    mask_gc = XCreateGC( dis, copymask_pix, 0, 0 );    // GC��ɸ������
    XSetForeground( dis, mask_gc, 1  );    // �޷��ο�������
    XSetLineAttributes( dis, mask_gc, current_pen, current_line, current_cap, current_join);

    /***************************/
    // ���ԡ��ѥ����ƥ�ν������
    /***************************/
    for(i=0;i<2;i++){
      copy_pix[i] = XCreatePixmap( dis, canvas, CANV_WIDTH, CANV_HEIGHT, DefaultDepth(dis,0) );
    }
    copy_gc = XCreateGC( dis, DefaultRootWindow(dis), 0, 0 );    // GC��ɸ������


    /***************************/
    // �طʥ������ѥ����ƥ�ν������
    /***************************/
    
    tilepix = XCreatePixmap( dis, function, 4, 4, DefaultDepth(dis,0) );
    XSetForeground( dis, copy_gc, 0  );    // �޷��ο�������
    XFillRectangle(dis,tilepix,copy_gc,0,0,4,4);
    XSetForeground( dis, copy_gc, 0xffffff  );    // �޷��ο�������
    XFillRectangle(dis,tilepix,copy_gc,0,0,2,2);
    XFillRectangle(dis,tilepix,copy_gc,2,2,4,4);

    /***************************/
    // �ܥ��󥦥���ɥ������ 
    /***************************/
    for ( i=0 ; i<MAX_COLOR ; i++ ){  //������
      color_win[i] = XCreateSimpleWindow(dis, function,
					 2+(i%2)*(button_size+2),
					 100+(i-i%2)/2*(button_size+2),
					 button_size, button_size, 1,
					 BORDERLINE_COLOR,
					 GetColor( dis, color_name[i]));
      XSelectInput(dis, color_win[i],
		   ButtonPressMask |
		   EnterWindowMask |
		   LeaveWindowMask );
    }
    //����������(�ܥ�����֤Ͽ�����μ��ΰ���)
    colors_win = XCreateSimpleWindow(dis, function,
				     2+(i%2)*(button_size+2),
				     100+(i-i%2)/2*(button_size+2),
				     button_size, button_size, 1,
				     BORDERLINE_COLOR,
				     0);
    XSetWindowBackgroundPixmap(dis,colors_win,tilepix);

    XSelectInput(dis, colors_win,
		 ButtonPressMask |
		 EnterWindowMask |
		 LeaveWindowMask );
    
    for ( i=0 ; i<MAX_FUNC ; i++ ){  // ��ǽ����
      func_win[i] = XCreateSimpleWindow(dis, function,
					2+(i%2)*(button_size+2),
					5+(i-i%2)/2*(button_size+2),
					button_size, button_size, 1,
					BORDERLINE_COLOR,
					UNSELECTED_COLOR1);
      XSelectInput(dis, func_win[i],
		   ButtonPressMask |
		   EnterWindowMask |
		   LeaveWindowMask );
      func_sub_win[i] = XCreateSimpleWindow(dis, function,
					    0, 500,
					    FUNC_WIDTH, FUNC_HEIGHT-200-2, 0,
					    BORDERLINE_COLOR,
					    GetColor( dis, "gray"));
      for(j=0;j<MAX_FUNC_SUB;j++){
	func_type_win[i][j] = XCreateSimpleWindow(dis, func_sub_win[i],
						  2, j*(button_size+2),
						  button_size*2, button_size, 1,
						  BORDERLINE_COLOR,
						  UNSELECTED_COLOR1);
	XSelectInput(dis, func_type_win[i][j],
		     ButtonPressMask |
		     EnterWindowMask |
		     LeaveWindowMask );
      }
    }

    for ( i=0 ; i<MAX_PEN ; i++ ){  //��������
      pen_win[i] = XCreateSimpleWindow(dis, function,
				       2+(i%2)*(button_size+2), 
				       200+(i-i%2)/2*(button_size+2),
				       button_size, button_size, 1,
				       BORDERLINE_COLOR,
				       UNSELECTED_COLOR1);
      XSelectInput(dis, pen_win[i],
		   ButtonPressMask |
		   EnterWindowMask |
		   LeaveWindowMask );
    }

    for ( i=0 ; i<MAX_TYPE ; i++ ){  //��������
      line_type_win[i] = XCreateSimpleWindow(dis, function,
					     2, 300+i*(button_size+2),
					     button_size*2, button_size, 1,
					     BORDERLINE_COLOR,
					     UNSELECTED_COLOR2);
      XSelectInput(dis, line_type_win[i],
		   ButtonPressMask |
		   EnterWindowMask |
		   LeaveWindowMask );
    }

    /**************** �ǥХå��ѥ�����ɥ� *****************/
    debug_win = XCreateSimpleWindow(dis, function, 5, MAIN_HEIGHT-(button_size+5),
				    2*button_size, button_size/2, 1,
				    BORDERLINE_COLOR,
				    GetColor( dis, "white"));
    XSelectInput(dis, debug_win,
		 ButtonPressMask |
		 EnterWindowMask |
		 LeaveWindowMask );
    /**************** �ǥХå��ѥ�����ɥ������ޤ� *****************/
    

    XSetWindowBackground(dis, func_win[0], SELECTED_COLOR);
    for(i=0;i<MAX_FUNC;i++){
      XSetWindowBackground(dis, func_type_win[i][0], SELECTED_COLOR);
    }
    XSetWindowBackground(dis, func_win[0], SELECTED_COLOR);
    XSetWindowBackground(dis, pen_win[0], SELECTED_COLOR);
    XSetWindowBackground(dis, line_type_win[0], SELECTED_COLOR);
    XSetWindowBackground(dis, line_type_win[3], SELECTED_COLOR);
    XSetWindowBackground(dis, line_type_win[6], SELECTED_COLOR);

}

void setFuncSubWin(){

  int i;

  for(i=0;i<MAX_FUNC;i++){
    XUnmapWindow(dis,func_sub_win[i]);
  }
  XMapWindow(dis,func_sub_win[state]);
  if(state==SQUARE||state==CIRCLE){
    XMapSubwindows( dis, func_sub_win[state]);
  }

}

void remapFuncMenu(){
  int i;
  XSetForeground(dis, gc, foreground);
  XSetLineAttributes(dis, gc, 0,
		     LineSolid, CapRound, JoinMiter);
  switch(state){
  case SQUARE:
    XDrawRectangle(dis, func_type_win[SQUARE][0], gc, 6, 6, 28, 8 ); 
    XFillRectangle(dis, func_type_win[SQUARE][1], gc, 6, 6, 28, 8 ); 
    break;
  case CIRCLE:
    XDrawArc(dis, func_type_win[CIRCLE][0], gc, 6, 3, 28, 14, 0, 360*64 );
    XFillArc(dis, func_type_win[CIRCLE][1], gc, 6, 3, 28, 14, 0, 360*64 );
    break;
  default:
    break;
  }
  for (i=0 ; i<MAX_PEN ; i++ ){//�������������
    int pen_size = i*3+2;
    XFillArc(dis, pen_win[i], gc,
	     button_size/2-pen_size/2, button_size/2-pen_size/2,
	     pen_size, pen_size, 0, 360*64);
    
  }
  {//��ͳ�����ʱ�ɮ�ˤκ�����
    XPoint points[7];
    points[0].x=4;points[0].y=11;
    points[1].x=13;points[1].y=2;
    points[2].x=17;points[2].y=6;
    points[3].x=8;points[3].y=15;
    points[4].x=4;points[4].y=15;
    points[5].x=4;points[5].y=11;
    points[6].x=8;points[6].y=15;
    XDrawLines(dis, func_win[FREEHAND], gc, points, 7, CoordModeOrigin ); 
  }
  {
    XDrawLine(dis, func_win[LINE], gc, 3, 16, 16, 3 ); 
  }
  {
    XDrawRectangle(dis, func_win[SQUARE], gc, 3, 6, 14, 8 ); 
  }
  {
    XDrawArc(dis, func_win[CIRCLE], gc, 3, 3, 14, 14, 0, 360*64 ); 
  }
  {//�ɤ�Ĥ֤��ʡˤκ�����
    XPoint points[7];
    points[0].x=12;points[0].y=3;
    points[1].x=18;points[1].y=12;
    points[2].x=10;points[2].y=16;
    points[3].x=6;points[3].y=7;
    points[4].x=15;points[4].y=7;
    points[5].x=6;points[5].y=7;
    points[6].x=6;points[6].y=14;
    XDrawLines(dis, func_win[FILL], gc, points, 7, CoordModeOrigin ); 
    XDrawArc(dis, func_win[FILL], gc, 4, 7, 4, 7, 90*64, 180*64 );
  }
  {//�������������

    XSetLineAttributes(dis, gc, 7, LineSolid, CapButt, JoinMiter);
    XDrawLine(dis, line_type_win[0], gc,
	      0, button_size/2,
	      button_size*2, button_size/2);
    
    XSetLineAttributes(dis, gc, 7, LineOnOffDash, CapButt, JoinMiter);
    XDrawLine(dis, line_type_win[1], gc,
	      0, button_size/2,
	      button_size*2, button_size/2);
    
    XSetLineAttributes(dis, gc, 7, LineDoubleDash, CapButt, JoinMiter);
    XDrawLine(dis, line_type_win[2], gc,
	      0, button_size/2,
	      button_size*2, button_size/2);
    
    XSetLineAttributes(dis, gc, 7, LineSolid, CapRound, JoinMiter);
    XDrawLine(dis, line_type_win[3], gc,
	      button_size/2, button_size/2,
	      button_size*2, button_size/2);
    
    XSetLineAttributes(dis, gc, 7, LineSolid, CapButt, JoinMiter);
    XDrawLine(dis, line_type_win[4], gc,
	      button_size/2, button_size/2,
	      button_size*2, button_size/2);
    
    XSetLineAttributes(dis, gc, 7, LineSolid, CapProjecting, JoinMiter);
    XDrawLine(dis, line_type_win[5], gc,
	      button_size/2, button_size/2,
	      button_size*2, button_size/2);
    
    XPoint points[3];
    points[0].x=button_size;  points[0].y=0;
    points[1].x=button_size;  points[1].y=button_size/2;
    points[2].x=button_size*2;points[2].y=button_size/2;

    XSetLineAttributes(dis, gc, 7, LineSolid, CapButt, JoinMiter);
    XDrawLines(dis, line_type_win[6], gc, points, 3, CoordModeOrigin );
    
    XSetLineAttributes(dis, gc, 7, LineSolid, CapButt, JoinRound);
    XDrawLines(dis, line_type_win[7], gc, points, 3, CoordModeOrigin );
    
    XSetLineAttributes(dis, gc, 7, LineSolid, CapButt, JoinBevel);
    XDrawLines(dis, line_type_win[8], gc, points, 3, CoordModeOrigin );
    
  }

  XSetForeground(dis, gc, current_color);
  XSetLineAttributes(dis, gc, current_pen,
		     current_line, current_cap, current_join);
}

int eventFuncMenu(XEvent ev){
  int i,j,k;

  switch(ev.type){
  case ButtonPress:
    
    for ( i=0 ; i<MAX_PEN ; i++ ){ /* �����������򥦥���ɥ���ǲ����줿? */

      if ( ev.xany.window == pen_win[i] ){      /* �ڥ󥵥������ѹ� */

	for ( j=0 ; j<MAX_PEN ; j++ ){
	  XSetWindowBackground( dis, pen_win[j], UNSELECTED_COLOR1 );
	  XClearWindow(dis,pen_win[j]);
	}
	XSetWindowBackground( dis, pen_win[i], SELECTED_COLOR);
	XClearWindow(dis,pen_win[i]);
	remapFuncMenu();

	current_pen = i*3+2;
	XSetLineAttributes(dis, gc, current_pen,
			   current_line, current_cap, current_join);
	XSetLineAttributes(dis, mask_gc, current_pen,
			   current_line, current_cap, current_join);
	return(0);
      }
    }
    for ( i=0 ; i<MAX_COLOR ; i++ ){

      if ( ev.xany.window == color_win[i] ){   /* �����ѹ� */
	current_color = GetColor(dis, color_name[i]);
	XSetForeground(dis, gc, current_color);
	
	return(0);
      }
    }
    if ( ev.xany.window == colors_win ){   /* ������ꤷ���ѹ� */
      current_color = callColorSelect(dis, current_color,canvas);
      XSetForeground(dis, gc, current_color);
      
      remapCanvas();
      remapFuncMenu();
      return(0);
    }
    for ( i=0 ; i<MAX_FUNC ; i++ ){
      if ( ev.xany.window == func_win[i] ){   /* ��ǽ���ѹ� */

	for ( j=0 ; j<MAX_FUNC ; j++ ){
	  XSetWindowBackground( dis, func_win[j], UNSELECTED_COLOR1 );
	  XClearWindow(dis,func_win[j]);
	}
	XSetWindowBackground( dis, func_win[i], SELECTED_COLOR);
	XClearWindow(dis,func_win[i]);
	remapFuncMenu();

	state=i;
	setFuncSubWin();
	return(0);
      }
      for ( j=0 ; j<MAX_FUNC_SUB ; j++ ){
	if ( ev.xany.window == func_type_win[i][j] ){   /* �����ǽ���ѹ� */
	  
	  for ( k=0 ; k<MAX_FUNC_SUB ; k++ ){
	    XSetWindowBackground( dis, func_type_win[i][k], UNSELECTED_COLOR1 );
	    XClearWindow(dis,func_type_win[i][k]);
	  }
	  XSetWindowBackground( dis, func_type_win[i][j], SELECTED_COLOR);
	  XClearWindow(dis,func_type_win[i][j]);
	  remapFuncMenu();
	  
	  sub_state=j;
	  setFuncSubWin();
	  return(0);
	}
      }
    }
    for ( i=0 ; i<MAX_TYPE ; i++ ){
      if ( ev.xany.window == line_type_win[i] ){   /* ������ѹ� */
	switch(i){
	case 0:
	  XSetWindowBackground( dis, line_type_win[0], SELECTED_COLOR);
	  XClearWindow(dis,line_type_win[0]);
	  XSetWindowBackground( dis, line_type_win[1], UNSELECTED_COLOR2);
	  XClearWindow(dis,line_type_win[1]);
	  XSetWindowBackground( dis, line_type_win[2], UNSELECTED_COLOR2);
	  XClearWindow(dis,line_type_win[2]);

	  current_line=LineSolid;
	  break;
	case 1:
	  XSetWindowBackground( dis, line_type_win[0], UNSELECTED_COLOR2);
	  XClearWindow(dis,line_type_win[0]);
	  XSetWindowBackground( dis, line_type_win[1], SELECTED_COLOR);
	  XClearWindow(dis,line_type_win[1]);
	  XSetWindowBackground( dis, line_type_win[2], UNSELECTED_COLOR2);
	  XClearWindow(dis,line_type_win[2]);

	  current_line=LineOnOffDash;
	  break;
	case 2:
	  XSetWindowBackground( dis, line_type_win[0], UNSELECTED_COLOR2);
	  XClearWindow(dis,line_type_win[0]);
	  XSetWindowBackground( dis, line_type_win[1], UNSELECTED_COLOR2);
	  XClearWindow(dis,line_type_win[1]);
	  XSetWindowBackground( dis, line_type_win[2], SELECTED_COLOR);
	  XClearWindow(dis,line_type_win[2]);

	  current_line=LineDoubleDash;
	  break;
	case 3:
	  XSetWindowBackground( dis, line_type_win[3], SELECTED_COLOR);
	  XClearWindow(dis,line_type_win[3]);
	  XSetWindowBackground( dis, line_type_win[4], UNSELECTED_COLOR2);
	  XClearWindow(dis,line_type_win[4]);
	  XSetWindowBackground( dis, line_type_win[5], UNSELECTED_COLOR2);
	  XClearWindow(dis,line_type_win[5]);

	  current_cap=CapRound;
	  break;
	case 4:
	  XSetWindowBackground( dis, line_type_win[3], UNSELECTED_COLOR2);
	  XClearWindow(dis,line_type_win[3]);
	  XSetWindowBackground( dis, line_type_win[4], SELECTED_COLOR);
	  XClearWindow(dis,line_type_win[4]);
	  XSetWindowBackground( dis, line_type_win[5], UNSELECTED_COLOR2);
	  XClearWindow(dis,line_type_win[5]);

	  current_cap=CapButt;
	  break;
	case 5:
	  XSetWindowBackground( dis, line_type_win[3], UNSELECTED_COLOR2);
	  XClearWindow(dis,line_type_win[3]);
	  XSetWindowBackground( dis, line_type_win[4], UNSELECTED_COLOR2);
	  XClearWindow(dis,line_type_win[4]);
	  XSetWindowBackground( dis, line_type_win[5], SELECTED_COLOR);
	  XClearWindow(dis,line_type_win[5]);

	  current_cap=CapProjecting;
	  break;
	case 6:
	  XSetWindowBackground( dis, line_type_win[6], SELECTED_COLOR);
	  XClearWindow(dis,line_type_win[6]);
	  XSetWindowBackground( dis, line_type_win[7], UNSELECTED_COLOR2);
	  XClearWindow(dis,line_type_win[7]);
	  XSetWindowBackground( dis, line_type_win[8], UNSELECTED_COLOR2);
	  XClearWindow(dis,line_type_win[8]);

	  current_join=JoinMiter;
	  break;
	case 7:
	  XSetWindowBackground( dis, line_type_win[6], UNSELECTED_COLOR2);
	  XClearWindow(dis,line_type_win[6]);
	  XSetWindowBackground( dis, line_type_win[7], SELECTED_COLOR);
	  XClearWindow(dis,line_type_win[7]);
	  XSetWindowBackground( dis, line_type_win[8], UNSELECTED_COLOR2);
	  XClearWindow(dis,line_type_win[8]);

	  current_join=JoinRound;
	  break;
	case 8:
	  XSetWindowBackground( dis, line_type_win[6], UNSELECTED_COLOR2);
	  XClearWindow(dis,line_type_win[6]);
	  XSetWindowBackground( dis, line_type_win[7], UNSELECTED_COLOR2);
	  XClearWindow(dis,line_type_win[7]);
	  XSetWindowBackground( dis, line_type_win[8], SELECTED_COLOR);
	  XClearWindow(dis,line_type_win[8]);

	  current_join=JoinBevel;
	  break;
	default:
	  break;
	}
	remapFuncMenu();
	XSetLineAttributes(dis, gc, current_pen,
			   current_line, current_cap, current_join);
	XSetLineAttributes(dis, mask_gc, current_pen,
			   current_line, current_cap, current_join);
      }
    }
    
    /***************************** �ǥХå��� ********************************/
    if ( ev.xany.window == debug_win ){ 
      //printf("color=%d size=%d\n",current_color,current_pen);
      for(i=0;i<MAX_COLOR;i++)
	printf("%s %lx\n",color_name[i],GetColor( dis, color_name[i]));
      printf("%lx\n",GetColor(dis,"yellow")>>8&0xff);
      save_png(canvas);
      return(0);    
    }
    /***************************** �ǥХå��� *********************************/
    break;
  case EnterNotify:	/* ������ɥ��˥ݥ��󥿤����ä� */

    for ( i=0 ; i<MAX_PEN ; i++ ){
      if ( ev.xany.window == pen_win[i] ){      /* �ڥ󥵥��� */
	XSetWindowBorder(dis, ev.xany.window, OVERED_BORDERLINE_COLOR);
	return(0);
      }
    }
    for ( i=0 ; i<MAX_TYPE ; i++ ){
      if ( ev.xany.window == line_type_win[i] ){      /* ���� */
	XSetWindowBorder(dis, ev.xany.window, OVERED_BORDERLINE_COLOR);
	return(0);
      }
    }
    for ( i=0 ; i<MAX_COLOR ; i++ ){
      if ( ev.xany.window == color_win[i] ){   /* �� */
	XSetWindowBorder(dis, ev.xany.window, OVERED_BORDERLINE_COLOR);
	return(0);
      }
    }
    if ( ev.xany.window == colors_win ){   /* ������ */
      XSetWindowBorder(dis, ev.xany.window, OVERED_BORDERLINE_COLOR);
      return(0);
    }
    for ( i=0 ; i<MAX_FUNC ; i++ ){
      if ( ev.xany.window == func_win[i] ){   /* ��ǽ */
	XSetWindowBorder(dis, ev.xany.window, OVERED_BORDERLINE_COLOR);
	return(0);
      }
      for ( j=0 ; j<MAX_FUNC_SUB ; j++ ){
	if ( ev.xany.window == func_type_win[i][j] ){   /* �����ǽ */
	  XSetWindowBorder(dis, ev.xany.window, OVERED_BORDERLINE_COLOR);
	  return(0);
	}
      }
    }
    if ( ev.xany.window == debug_win ){ /* �ǥХå� */
      XSetWindowBorder(dis, ev.xany.window, OVERED_BORDERLINE_COLOR);
      return(0);
    }
    
    break;
    
  case LeaveNotify:	/* ������ɥ�����ݥ��󥿤��Ф� */
    for ( i=0 ; i<MAX_PEN ; i++ ){
      if ( ev.xany.window == pen_win[i] ){      /* �ڥ󥵥��� */
	XSetWindowBorder(dis, ev.xany.window, BORDERLINE_COLOR);
	return(0);
      }
    }
    for ( i=0 ; i<MAX_TYPE ; i++ ){
      if ( ev.xany.window == line_type_win[i] ){      /* ���� */
	XSetWindowBorder(dis, ev.xany.window, BORDERLINE_COLOR);
	return(0);
      }
    }
    for ( i=0 ; i<MAX_COLOR ; i++ ){
      if ( ev.xany.window == color_win[i] ){   /* �� */
	XSetWindowBorder(dis, ev.xany.window, BORDERLINE_COLOR);
	return(0);
      }
    }
    if ( ev.xany.window == colors_win ){   /* ������ */
      XSetWindowBorder(dis, ev.xany.window, BORDERLINE_COLOR);
      return(0);
    }
    for ( i=0 ; i<MAX_FUNC ; i++ ){
      if ( ev.xany.window == func_win[i] ){   /* ��ǽ */
	XSetWindowBorder(dis, ev.xany.window, BORDERLINE_COLOR);
	return(0);
      }
      for ( j=0 ; j<MAX_FUNC_SUB ; j++ ){
	if ( ev.xany.window == func_type_win[i][j] ){   /* �����ǽ */
	  XSetWindowBorder(dis, ev.xany.window, BORDERLINE_COLOR);
	  return(0);
	}
      }
    }
    if ( ev.xany.window == debug_win ){ /* �ǥХå� */
      XSetWindowBorder(dis, ev.xany.window, BORDERLINE_COLOR);
      return(0);
    }
    
    break;
    
  default:
    break;
    
  }

  return(1);
}

void execFunc(XEvent ev,Window *layer_expose,Window *layer,Window *mask,int num){

  switch(state){ // ���򤵤�Ƥ���ư��μ¹�
  case FREEHAND://��ͳ����
    freeHand(ev,layer_expose,layer,mask,num);
    break;
  case LINE://ľ��
    line(ev,layer_expose,layer,mask,num);
    break;
  case SQUARE://�ͳ�
    square(ev,layer_expose,layer,mask,num);
    break;
  case CIRCLE://��
    circle(ev,layer_expose,layer,mask,num);
    break;
  case FILL://�ɤ�Ĥ֤�
    fill(ev,layer_expose,layer,mask,num);
    break;
  default:
    break;
  }

}

/**********************************/
// ��ͳ����
/**********************************/
void freeHand(XEvent ev,Window *layer_expose,Window *layer,Window *mask,int num){
  int x,y;
  int i;

  x=ev.xbutton.x;
  y=ev.xbutton.y;

  XSetForeground( dis, gc, current_color  );    // �޷��ο�������

  XShapeCombineMask( dis, layer[num], ShapeClip, 0, 0, None, ShapeSet);//���ä���ޥ�������
  XShapeCombineMask( dis, layer[num], ShapeBounding, 0, 0, None, ShapeSet);
  
  XNextEvent( dis, &ev );
  while( !(ev.type == ButtonRelease && ev.xbutton.button == 1) ){
    if(ev.type == MotionNotify){
      XDrawLine( dis, layer_expose[num], gc, x, y, ev.xbutton.x, ev.xbutton.y );
      XDrawLine( dis, layer[num], gc, x, y, ev.xbutton.x, ev.xbutton.y );
      XDrawLine( dis, mask[num], mask_gc, x, y, ev.xbutton.x, ev.xbutton.y );
      XFlush( dis );
      
      x=ev.xbutton.x;
      y=ev.xbutton.y;
    }
    XNextEvent( dis, &ev );
    
  }
  XShapeCombineMask( dis, layer[num], ShapeBounding, 0, 0, mask[num], ShapeSet);
  XShapeCombineMask( dis, layer[num], ShapeClip, 0, 0, mask[num], ShapeSet);

  addHistory(layer_expose,mask,num);//������ɲ�

}

/**********************************/
// ľ��
/**********************************/
void line(XEvent ev,Window *layer_expose,Window *layer,Window *mask,int num){
  int xs,ys,xe,ye;

  XShapeCombineMask( dis, layer[num], ShapeClip, 0, 0, None, ShapeSet);//���ä���ޥ�������
  XShapeCombineMask( dis, layer[num], ShapeBounding, 0, 0, None, ShapeSet);

  XCopyArea( dis, layer[num], copy_pix[0], gc, 0, 0, CANV_WIDTH, CANV_HEIGHT, 0, 0 );

  xs=ev.xbutton.x;
  ys=ev.xbutton.y;

  while( !(ev.type == ButtonRelease && ev.xbutton.button == 1) ){
    if(ev.type == MotionNotify){
      XCopyArea( dis, copy_pix[0], copy_pix[1], gc, 0, 0, CANV_WIDTH, CANV_HEIGHT, 0, 0 );
      xe=ev.xbutton.x;
      ye=ev.xbutton.y;
      
      XDrawLine(dis, copy_pix[1], gc, xs, ys, xe, ye);
      XCopyArea( dis, copy_pix[1], layer[num], gc, 0, 0, CANV_WIDTH, CANV_HEIGHT, 0, 0 );
      XFlush(dis);
    }
    XNextEvent( dis, &ev );
  }
  XDrawLine(dis, mask[num], mask_gc, xs, ys, xe, ye);
  XDrawLine(dis, layer_expose[num], gc, xs, ys, xe, ye);
  XShapeCombineMask( dis, layer[num], ShapeBounding, 0, 0, mask[num], ShapeSet);
  XShapeCombineMask( dis, layer[num], ShapeClip, 0, 0, mask[num], ShapeSet);

  addHistory(layer_expose,mask,num);//������ɲ�

}

/**********************************/
// �ͳ�
/**********************************/
void square(XEvent ev,Window *layer_expose,Window *layer,Window *mask,int num){
  int xs,ys,xe,ye;

  XShapeCombineMask( dis, layer[num], ShapeClip, 0, 0, None, ShapeSet);//���ä���ޥ�������
  XShapeCombineMask( dis, layer[num], ShapeBounding, 0, 0, None, ShapeSet);

  XCopyArea( dis, layer[num], copy_pix[0], gc, 0, 0, CANV_WIDTH, CANV_HEIGHT, 0, 0 );

  xs=ev.xbutton.x;
  ys=ev.xbutton.y;

  while( !(ev.type == ButtonRelease && ev.xbutton.button == 1) ){
    if(ev.type == MotionNotify){
      XCopyArea( dis, copy_pix[0], copy_pix[1], gc, 0, 0, CANV_WIDTH, CANV_HEIGHT, 0, 0 );
      xe=ev.xbutton.x;
      ye=ev.xbutton.y;

      if(sub_state==0){
	XDrawRectangle(dis, copy_pix[1], gc,
		       (xs<xe)?xs:xe, (ys<ye)?ys:ye,
		       abs(xe-xs), abs(ye-ys));
      }else{
	XFillRectangle(dis, copy_pix[1], gc,
		       (xs<xe)?xs:xe, (ys<ye)?ys:ye,
		       abs(xe-xs), abs(ye-ys));
      }
      XCopyArea( dis, copy_pix[1], layer[num], gc, 0, 0, CANV_WIDTH, CANV_HEIGHT, 0, 0 );
      XFlush(dis);
    }
    XNextEvent( dis, &ev );
  }

  if(sub_state==0){
    XDrawRectangle(dis, mask[num], mask_gc,
		   (xs<xe)?xs:xe, (ys<ye)?ys:ye,
		   abs(xe-xs), abs(ye-ys));
    XDrawRectangle(dis, layer_expose[num], gc,
		   (xs<xe)?xs:xe, (ys<ye)?ys:ye,
		   abs(xe-xs), abs(ye-ys));
  }else{
    XFillRectangle(dis, mask[num], mask_gc,
		   (xs<xe)?xs:xe, (ys<ye)?ys:ye,
		   abs(xe-xs), abs(ye-ys));
    XFillRectangle(dis, layer_expose[num], gc,
		   (xs<xe)?xs:xe, (ys<ye)?ys:ye,
		   abs(xe-xs), abs(ye-ys));
  }
  XShapeCombineMask( dis, layer[num], ShapeBounding, 0, 0, mask[num], ShapeSet);
  XShapeCombineMask( dis, layer[num], ShapeClip, 0, 0, mask[num], ShapeSet);
  addHistory(layer_expose,mask,num);//������ɲ�

}

/**********************************/
// ��
/**********************************/
void circle(XEvent ev,Window *layer_expose,Window *layer,Window *mask,int num){
  int xs,ys,xe,ye;

  XShapeCombineMask( dis, layer[num], ShapeClip, 0, 0, None, ShapeSet);//���ä���ޥ�������
  XShapeCombineMask( dis, layer[num], ShapeBounding, 0, 0, None, ShapeSet);

  XCopyArea( dis, layer[num], copy_pix[0], gc, 0, 0, CANV_WIDTH, CANV_HEIGHT, 0, 0 );

  xs=ev.xbutton.x;
  ys=ev.xbutton.y;

  while( !(ev.type == ButtonRelease && ev.xbutton.button == 1) ){
    if(ev.type == MotionNotify){
      XCopyArea( dis, copy_pix[0], copy_pix[1], gc, 0, 0, CANV_WIDTH, CANV_HEIGHT, 0, 0 );
      xe=ev.xbutton.x;
      ye=ev.xbutton.y;

      if(sub_state==0){
	XDrawArc(dis, copy_pix[1], gc,
		 (xs<xe)?xs:xe, (ys<ye)?ys:ye,
		 abs(xe-xs), abs(ye-ys),
		 0,360*64);
      }else{
	XFillArc(dis, copy_pix[1], gc,
		 (xs<xe)?xs:xe, (ys<ye)?ys:ye,
		 abs(xe-xs), abs(ye-ys),
		 0,360*64);
      }

      XCopyArea( dis, copy_pix[1], layer[num], gc, 0, 0, CANV_WIDTH, CANV_HEIGHT, 0, 0 );
      
      XFlush(dis);
    }
    XNextEvent( dis, &ev );
  }


  if(sub_state==0){
    XDrawArc(dis, mask[num], mask_gc,
	     (xs<xe)?xs:xe, (ys<ye)?ys:ye,
	     abs(xe-xs), abs(ye-ys),
	     0,360*64);
    XDrawArc(dis, layer_expose[num], gc,
	     (xs<xe)?xs:xe, (ys<ye)?ys:ye,
	     abs(xe-xs), abs(ye-ys),
	     0,360*64);
  }else{
    XFillArc(dis, mask[num], mask_gc,
	     (xs<xe)?xs:xe, (ys<ye)?ys:ye,
	     abs(xe-xs), abs(ye-ys),
	     0,360*64);
    XFillArc(dis, layer_expose[num], gc,
	     (xs<xe)?xs:xe, (ys<ye)?ys:ye,
	     abs(xe-xs), abs(ye-ys),
	     0,360*64);
  }
  XShapeCombineMask( dis, layer[num], ShapeBounding, 0, 0, mask[num], ShapeSet);
  XShapeCombineMask( dis, layer[num], ShapeClip, 0, 0, mask[num], ShapeSet);
  addHistory(layer_expose,mask,num);//������ɲ�

}

/**********************************/
// �ɤ�Ĥ֤�
/**********************************/
void fill(XEvent ev,Window *layer_expose,Window *layer,Window *mask,int num){

  int i,j;
  int x,y;
  unsigned long pixel;
  //unsigned long pixelmap[CANV_WIDTH][CANV_HEIGHT];
  XImage *ximage[CANV_HEIGHT];
  XImage *xmaskimage;

  XShapeCombineMask( dis, layer[num], ShapeClip, 0, 0, None, ShapeSet);//���ä���ޥ�������
  XShapeCombineMask( dis, layer[num], ShapeBounding, 0, 0, None, ShapeSet);
  
  x=ev.xbutton.x;
  y=ev.xbutton.y;

  XCopyArea( dis, layer_expose[num], copy_pix[0], gc, 0, 0, CANV_WIDTH, CANV_HEIGHT, 0, 0 );
  
  for (j = 0; j < CANV_HEIGHT; j++) {
    ximage[j] = XGetImage(dis,copy_pix[0],0,j,CANV_WIDTH,1,AllPlanes,ZPixmap);
  }
  xmaskimage = XGetImage(dis,mask[num],0,0,CANV_WIDTH,CANV_HEIGHT,AllPlanes,ZPixmap);

  pixel=XGetPixel(ximage[y],x,0);
  if(pixel==current_color){
    XShapeCombineMask( dis, layer[num], ShapeBounding, 0, 0, mask[num], ShapeSet);
    XShapeCombineMask( dis, layer[num], ShapeClip, 0, 0, mask[num], ShapeSet);
    return;
  }
  XPutPixel(ximage[y],x,0,current_color);
  XPutPixel(xmaskimage,x,y,1);
  fillCheck(x,y,pixel,ximage,xmaskimage);
  for (j = 0; j < CANV_HEIGHT; j++) {
    XPutImage(dis,copy_pix[0],gc,ximage[j],0,0,0,j,CANV_WIDTH,1);
  }
  XPutImage(dis,mask[num],mask_gc,xmaskimage,0,0,0,0,CANV_WIDTH,CANV_HEIGHT);

  XCopyArea( dis, copy_pix[0], layer[num], gc, 0, 0, CANV_WIDTH, CANV_HEIGHT, 0, 0 );
  XCopyArea( dis, copy_pix[0], layer_expose[num], gc, 0, 0, CANV_WIDTH, CANV_HEIGHT, 0, 0 );
  

  XShapeCombineMask( dis, layer[num], ShapeBounding, 0, 0, mask[num], ShapeSet);
  XShapeCombineMask( dis, layer[num], ShapeClip, 0, 0, mask[num], ShapeSet);
  addHistory(layer_expose,mask,num);//������ɲ�

}
/**********************************/
// ���ɤ�Ĥ֤�������ؿ���
/**********************************/
void fillCheck(int x,int y,unsigned long pixel,XImage *ximage[],XImage *xmaskimage){
#define Pixel(z,y) (XGetPixel(ximage[(y)],(z),0))
#define SetPixel(z,y,p) (XPutPixel(ximage[(y)],(z),0,p))
#define SetMask(z,y) (XPutPixel(xmaskimage,(z),(y),1))
#define MAX_BUFF 512
  struct scanline{
    int xl,xr,y;
    char type;//���ӥåȻ��ѡ�4:�������褿 2:������ 1:�����ɡ�
  }scanline[MAX_BUFF];
  int top=0,bottom=0,num=0;
  int left,right,type,s,yt;

  left=x;
  right=x;
  SetPixel(x,y,current_color);
  SetMask(x,y);
  while(left>0 && pixel==Pixel(left-1,y)){
    left--;
    SetPixel(left,y,current_color);
    SetMask(left,y);
  }
  while(right<CANV_WIDTH-1 && pixel==Pixel(right+1,y)){
    right++;
    SetPixel(right,y,current_color);
    SetMask(right,y);
  }
  if(y-1>=0){
    s=left;
    while(s<=right && pixel!=Pixel(s,y-1))s++;
    if(s<=right){
      SetPixel(s,y-1,current_color);
      SetMask(s,y-1);
      scanline[top].xl=s;
      scanline[top].type=0;
      scanline[top].y=y-1;
      scanline[top].type+=4;
      if(s-1<0 || pixel!=Pixel(s-1,y-1))scanline[top].type+=2;
      while(1){
	while(s<right && pixel==Pixel(s+1,y-1)){
	  s++;
	  SetPixel(s,y-1,current_color);
	  SetMask(s,y-1);
	}
	if(s+1>=CANV_WIDTH ||pixel!=Pixel(s+1,y-1))scanline[top].type+=1;
	scanline[top].xr=s;
	top=(top+1)%MAX_BUFF;
	num++;
	
	while(s<right && pixel!=Pixel(s+1,y-1))s++;
	if(s>=right)break;
	
	s++;
	SetPixel(s,y-1,current_color);
	SetMask(s,y-1);
	scanline[top].xl=s;
	scanline[top].type=0;
	scanline[top].y=y-1;
	scanline[top].type+=4;
	scanline[top].type+=2;
      }
    }
  }
  if(y+1<CANV_WIDTH){
    s=left;
    while(s<=right && pixel!=Pixel(s,y+1))s++;
    if(s<=right){
      SetPixel(s,y+1,current_color);
      SetMask(s,y+1);
      scanline[top].xl=s;
      scanline[top].type=0;
      scanline[top].y=y+1;
      if(s-1<0 || pixel!=Pixel(s-1,y+1))scanline[top].type+=2;
      while(1){
	while(s<right && pixel==Pixel(s+1,y+1)){
	  s++;
	  SetPixel(s,y+1,current_color);
	  SetMask(s,y+1);
	}
	if(s+1>=CANV_WIDTH ||pixel!=Pixel(s+1,y+1))scanline[top].type+=1;

	scanline[top].xr=s;
	top=(top+1)%MAX_BUFF;
	num++;
	
	while(s<right && pixel!=Pixel(s+1,y+1))s++;
	if(s>=right)break;
	
	s++;
	SetPixel(s,y+1,current_color);
	SetMask(s,y+1);
	scanline[top].xl=s;
	scanline[top].type=0;
	scanline[top].y=y+1;
	scanline[top].type+=2;
      }
    }
  }
  while(num>0){            //�����֤�����
    
    scanline[top].type=0;
    left=scanline[bottom].xl;
    right=scanline[bottom].xr;
    yt=scanline[bottom].y;
    type=scanline[bottom].type;
    if((type&2)==0){
      while(left>0 && pixel==Pixel(left-1,yt)){
	left--;
	SetPixel(left,yt,current_color);
	SetMask(left,yt);
      }
    }
    if((type&1)==0){
      while(right<CANV_WIDTH-1 && pixel==Pixel(right+1,yt)){
	right++;
	SetPixel(right,yt,current_color);
	SetMask(right,yt);
      }
    }
    if(yt-1>=0 && type!=3){
      s=left;
      while(s<=right && pixel!=Pixel(s,yt-1))s++;
      if(s<=right){
	SetPixel(s,yt-1,current_color);
	SetMask(s,yt-1);
	scanline[top].xl=s;
	scanline[top].type=0;
	scanline[top].y=yt-1;
	scanline[top].type+=4;
	if(s-1<0 || pixel!=Pixel(s-1,yt-1))scanline[top].type+=2;
	while(1){
	  while(s<right && pixel==Pixel(s+1,yt-1)){
	    s++;
	    SetPixel(s,yt-1,current_color);
	    SetMask(s,yt-1);
	  }
	  if(s+1>=CANV_WIDTH ||pixel!=Pixel(s+1,yt-1))scanline[top].type+=1;
	  scanline[top].xr=s;
	  top=(top+1)%MAX_BUFF;
	  num++;
	  
	  while(s<right && pixel!=Pixel(s+1,yt-1))s++;
	  if(s>=right)break;
	  
	  s++;
	  SetPixel(s,yt-1,current_color);
	  SetMask(s,yt-1);
	  scanline[top].xl=s;
	  scanline[top].type=0;
	  scanline[top].y=yt-1;
	  scanline[top].type+=4;
	  scanline[top].type+=2;
	}
      }
    }
    if(yt+1<CANV_WIDTH && type!=7){
      s=left;
      while(s<=right && pixel!=Pixel(s,yt+1))s++;
      if(s<=right){
	SetPixel(s,yt+1,current_color);
	SetMask(s,yt+1);
	scanline[top].xl=s;
	scanline[top].type=0;
	scanline[top].y=yt+1;
	if(s-1<0 || pixel!=Pixel(s-1,yt+1))scanline[top].type+=2;
	while(1){
	  while(s<right && pixel==Pixel(s+1,yt+1)){
	    s++;
	    SetPixel(s,yt+1,current_color);
	    SetMask(s,yt+1);
	  }
	  if(s+1>=CANV_WIDTH ||pixel!=Pixel(s+1,yt+1))scanline[top].type+=1;
	  scanline[top].xr=s;
	  top=(top+1)%MAX_BUFF;
	  num++;
	  
	  while(s<right && pixel!=Pixel(s+1,yt+1))s++;
	  if(s>=right)break;
	  
	  s++;
	  SetPixel(s,yt+1,current_color);
	  SetMask(s,yt+1);
	  scanline[top].xl=s;
	  scanline[top].type=0;
	  scanline[top].y=yt+1;
	  scanline[top].type+=2;
	}
      }
    }
    bottom=(bottom+1)%MAX_BUFF;
    num--;

  }
}
