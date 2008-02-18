#include "inc.h"

#define MAX_COLOR 6  /* ���� */
#define MAX_PEN 6 /* �ڥ�������� */
#define MAX_FUNC 5 /* ��ǽ�� */
#define MAX_TYPE 9 /* ����ο� */

void freeHand(XEvent ev);
void line(XEvent ev);
void square(XEvent ev);
void circle(XEvent ev);
void fill(XEvent ev);

void fillCheck(int x,int y,unsigned long pixel,
	       unsigned long **pixelmap,
	       Bool check[CANV_WIDTH][CANV_HEIGHT]);
Display *dis;
Window canvas;

Window color_win[MAX_COLOR];                 //�������Window  ID
Window func_win[MAX_FUNC];                   //��ǽ�����Window  ID
Window func_sub_win[MAX_FUNC];               //�����ǽ��Window  ID
Window pen_win[MAX_PEN];                     //���������Window  ID
Window line_type_win[MAX_TYPE];              //���������Window  ID
Window debug_win;                            //�ǥХå��Ѥ�Window  ID

GC gc;                                       //����ե��å�����ƥ����� ID
GC copy_gc;                                  //���ԡ��ѥ���ե��å�����ƥ����� ID
Pixmap pix;                                  //���ԡ��ѥԥ����ޥå�

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

unsigned long current_color=0; // ��������
unsigned long current_pen=2; // �������Ϥ����Ĥ�
unsigned long current_line=LineSolid; // �������Ϥ����Ĥ�
unsigned long current_cap=CapRound; // �������Ϥ����Ĥ�
unsigned long current_join=JoinMiter; // �������Ϥ����Ĥ�

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
    // ���ԡ��ѥ����ƥ�ν������
    /***************************/
    
    pix = XCreatePixmap( dis, canvas, 600, 600, DefaultDepth(dis,0) );
    copy_gc = XCreateGC( dis, DefaultRootWindow(dis), 0, 0 );    // GC��ɸ������


    /***************************/
    // �ܥ��󥦥���ɥ������ 
    /***************************/
    for ( i=0 ; i<MAX_COLOR ; i++ ){  //������
      color_win[i] = XCreateSimpleWindow(dis, function,
					 2+(i%2)*(button_size+2),
					 100+(i-i%2)/2*(button_size+2),
					 button_size, button_size, 1,
					 GetColor( dis, "rgb:aa/aa/aa"),
					 GetColor( dis, color_name[i]));
      XSelectInput(dis, color_win[i],
		   ButtonPressMask |
		   EnterWindowMask |
		   LeaveWindowMask );
    }

    for ( i=0 ; i<MAX_FUNC ; i++ ){  // ��ǽ����
      func_win[i] = XCreateSimpleWindow(dis, function,
					2+(i%2)*(button_size+2),
					5+(i-i%2)/2*(button_size+2),
					button_size, button_size, 1,
					GetColor( dis, "rgb:aa/aa/aa"),
					GetColor( dis, "rgb:ee/ee/ee"));
      XSelectInput(dis, func_win[i],
		   ButtonPressMask |
		   EnterWindowMask |
		   LeaveWindowMask );
      func_sub_win[i] = XCreateSimpleWindow(dis, function,
					    0, 450,
					    FUNC_WIDTH, FUNC_HEIGHT-200-2, 0,
					    GetColor( dis, "rgb:aa/aa/aa"),
					    GetColor( dis, "gray"));
    }

    for ( i=0 ; i<MAX_PEN ; i++ ){  //��������
      pen_win[i] = XCreateSimpleWindow(dis, function,
				       2+(i%2)*(button_size+2), 
				       200+(i-i%2)/2*(button_size+2),
				       button_size, button_size, 1,
				       GetColor(dis, "rgb:aa/aa/aa"),
				       GetColor(dis, "rgb:ee/ee/ee"));
      //GetColor(dis, "rgb:ee/ee/ee"));
      XSelectInput(dis, pen_win[i],
		   ButtonPressMask |
		   EnterWindowMask |
		   LeaveWindowMask );
    }

    for ( i=0 ; i<MAX_TYPE ; i++ ){  //��������
      line_type_win[i] = XCreateSimpleWindow(dis, function,
					     2, 300+i*(button_size+2),
					     button_size*2, button_size, 1,
					     GetColor(dis, "rgb:aa/aa/aa"),
					     GetColor(dis, "rgb:66/66/66"));
      XSelectInput(dis, line_type_win[i],
		   ButtonPressMask |
		   EnterWindowMask |
		   LeaveWindowMask );
    }

    /**************** �ǥХå��ѥ�����ɥ� *****************/
    debug_win = XCreateSimpleWindow(dis, function, 5, MAIN_HEIGHT-(button_size+5),
				    2*button_size, button_size, 1,
				    GetColor( dis, "rgb:aa/aa/aa"),
				    GetColor( dis, "white"));
    XSelectInput(dis, debug_win,
		 ButtonPressMask |
		 EnterWindowMask |
		 LeaveWindowMask );
    /**************** �ǥХå��ѥ�����ɥ������ޤ� *****************/
    
}

void setFuncSubWin(){

  int i;

  for(i=0;i<MAX_FUNC;i++){
    XUnmapWindow(dis,func_sub_win[i]);
    //XSetWindowBackground(dis, func_win[i], GetColor( dis, "rgb:ee/ee/ee"));
  }
  XMapWindow(dis,func_sub_win[state]);
  XMapSubwindows( dis, func_sub_win[state]);
  //XSetWindowBackground(dis, func_win[state], GetColor( dis, "blue"));

}

void remapFuncMenu(){
  int i;
  XSetForeground(dis, gc, foreground);
  XSetLineAttributes(dis, gc, 0,
		     LineSolid, CapRound, JoinMiter);
  switch(state){
  case FREEHAND:
    break;
  case LINE:
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
    points[0].x=12;points[0].y=11;
    points[1].x=13;points[1].y=2;
    points[2].x=17;points[2].y=6;
    points[3].x=8;points[3].y=15;
    points[4].x=4;points[4].y=15;
    points[5].x=4;points[5].y=11;
    points[6].x=8;points[6].y=15;
    XDrawLines(dis, func_win[FILL], gc, points, 7, CoordModeOrigin ); 
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
  int i;

  switch(ev.type){
  case ButtonPress:
    
    for ( i=0 ; i<MAX_PEN ; i++ ){	  /* �����������򥦥���ɥ���ǲ����줿? */
      
      if ( ev.xany.window == pen_win[i] ){      /* �ڥ󥵥������ѹ� */
	current_pen = i*3+2;
	XSetLineAttributes(dis, gc, current_pen,
			   current_line, current_cap, current_join);
			   //LineSolid, CapRound, JoinMiter);
	
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
    for ( i=0 ; i<MAX_FUNC ; i++ ){
      if ( ev.xany.window == func_win[i] ){   /* ��ǽ���ѹ� */
	state=i;
	setFuncSubWin();
	return(0);
      }
    }
    for ( i=0 ; i<MAX_TYPE ; i++ ){
      if ( ev.xany.window == line_type_win[i] ){   /* ������ѹ� */
	switch(i){
	case 0:
	  current_line=LineSolid;
	  break;
	case 1:
	  current_line=LineOnOffDash;
	  break;
	case 2:
	  current_line=LineDoubleDash;
	  break;
	case 3:
	  current_cap=CapRound;
	  break;
	case 4:
	  current_cap=CapButt;
	  break;
	case 5:
	  current_cap=CapProjecting;
	  break;
	case 6:
	  current_join=JoinMiter;
	  break;
	case 7:
	  current_join=JoinRound;
	  break;
	case 8:
	  current_join=JoinBevel;
	  break;
	default:
	  break;
	}
	XSetLineAttributes(dis, gc, current_pen,
			   current_line, current_cap, current_join);
      }
    }
    
    /***************************** �ǥХå��� ********************************/
    if ( ev.xany.window == debug_win ){ 
      //printf("color=%d size=%d\n",current_color,current_pen);
      for(i=0;i<MAX_COLOR;i++)
	printf("%s %x\n",color_name[i],GetColor( dis, color_name[i]));
      printf("%x\n",GetColor(dis,"yellow")>>8&0xff);
      view();
      return(0);    
    }
    /***************************** �ǥХå��� *********************************/
    break;
  case EnterNotify:	/* ������ɥ��˥ݥ��󥿤����ä� */

    for ( i=0 ; i<MAX_PEN ; i++ ){
      if ( ev.xany.window == pen_win[i] ){      /* �ڥ󥵥��� */
	XSetWindowBorder(dis, ev.xany.window, GetColor( dis, "black"));
	return(0);
      }
    }
    for ( i=0 ; i<MAX_TYPE ; i++ ){
      if ( ev.xany.window == line_type_win[i] ){      /* ���� */
	XSetWindowBorder(dis, ev.xany.window, GetColor( dis, "black"));
	return(0);
      }
    }
    for ( i=0 ; i<MAX_COLOR ; i++ ){
      if ( ev.xany.window == color_win[i] ){   /* �� */
	XSetWindowBorder(dis, ev.xany.window, GetColor( dis, "black"));
	return(0);
      }
    }
    for ( i=0 ; i<MAX_FUNC ; i++ ){
      if ( ev.xany.window == func_win[i] ){   /* ��ǽ */
	XSetWindowBorder(dis, ev.xany.window, GetColor( dis, "black"));
	return(0);
      }
    }
    if ( ev.xany.window == debug_win ){ /* �ǥХå� */
      XSetWindowBorder(dis, ev.xany.window, GetColor( dis, "black"));
      return(0);
    }
    
    break;
    
  case LeaveNotify:	/* ������ɥ�����ݥ��󥿤��Ф� */
    for ( i=0 ; i<MAX_PEN ; i++ ){
      if ( ev.xany.window == pen_win[i] ){      /* �ڥ󥵥��� */
	XSetWindowBorder(dis, ev.xany.window, GetColor( dis, "rgb:aa/aa/aa"));
	return(0);
      }
    }
    for ( i=0 ; i<MAX_TYPE ; i++ ){
      if ( ev.xany.window == line_type_win[i] ){      /* ���� */
	XSetWindowBorder(dis, ev.xany.window, GetColor( dis, "rgb:aa/aa/aa"));
	return(0);
      }
    }
    for ( i=0 ; i<MAX_COLOR ; i++ ){
      if ( ev.xany.window == color_win[i] ){   /* �� */
	XSetWindowBorder(dis, ev.xany.window, GetColor( dis, "rgb:aa/aa/aa"));
	return(0);
      }
    }
    for ( i=0 ; i<MAX_FUNC ; i++ ){
      if ( ev.xany.window == func_win[i] ){   /* ��ǽ */
	XSetWindowBorder(dis, ev.xany.window, GetColor( dis, "rgb:aa/aa/aa"));
	return(0);
      }
    }
    if ( ev.xany.window == debug_win ){ /* �ǥХå� */
      XSetWindowBorder(dis, ev.xany.window, GetColor( dis, "rgb:aa/aa/aa"));
      return(0);
    }
    
    break;
    
  default:
    break;
    
  }

  return(1);
}

void execFunc(XEvent ev){

  switch(state){ // ���򤵤�Ƥ���ư��μ¹�
  case FREEHAND://��ͳ����
    freeHand(ev);
    break;
  case LINE://ľ��
    line(ev);
    break;
  case SQUARE://�ͳ�
    square(ev);
    break;
  case CIRCLE://��
    circle(ev);
    break;
  case FILL://�ɤ�Ĥ֤�
    fill(ev);
    break;
  default:
    break;
  }

}

/* ��ͳ���� */
void freeHand(XEvent ev){
  int x,y;
  int i;

  x=ev.xbutton.x;
  y=ev.xbutton.y;

  XSetForeground( dis, gc, current_color  );    // �޷��ο�������
  
  XNextEvent( dis, &ev );
  while( !(ev.type == ButtonRelease && ev.xbutton.button == 1) ){
    XDrawLine( dis, canvas, gc, x, y, ev.xbutton.x, ev.xbutton.y );//�Ȥꤢ����DrawLine��ɽ����
    XFlush( dis );

    x=ev.xbutton.x;
    y=ev.xbutton.y;
    
    XNextEvent( dis, &ev );
    
  }

  addHistory();//������ɲ�

}

void line(XEvent ev){
  int xs,ys,xe,ye;

  XCopyArea( dis, canvas, pix, gc, 0, 0, CANV_WIDTH, CANV_HEIGHT, 0, 0 );

  xs=ev.xbutton.x;
  ys=ev.xbutton.y;

  while( !(ev.type == ButtonRelease && ev.xbutton.button == 1) ){
    if(ev.type == MotionNotify){
      XCopyArea( dis, pix, canvas, gc, 0, 0, CANV_WIDTH, CANV_HEIGHT, 0, 0 );
      xe=ev.xbutton.x;
      ye=ev.xbutton.y;
      
      XDrawLine(dis, canvas, gc, xs, ys, xe, ye);
      XFlush(dis);
    }
    XNextEvent( dis, &ev );
  }

  addHistory();//������ɲ�

}

void square(XEvent ev){
  int xs,ys,xe,ye;

  XCopyArea( dis, canvas, pix, gc, 0, 0, CANV_WIDTH, CANV_HEIGHT, 0, 0 );

  xs=ev.xbutton.x;
  ys=ev.xbutton.y;

  while( !(ev.type == ButtonRelease && ev.xbutton.button == 1) ){
    if(ev.type == MotionNotify){
      XCopyArea( dis, pix, canvas, gc, 0, 0, CANV_WIDTH, CANV_HEIGHT, 0, 0 );
      xe=ev.xbutton.x;
      ye=ev.xbutton.y;
      
      XDrawRectangle(dis, canvas, gc,
		     (xs<xe)?xs:xe, (ys<ye)?ys:ye,
		     abs(xe-xs), abs(ye-ys));
      XFlush(dis);
    }
    XNextEvent( dis, &ev );
  }

  addHistory();//������ɲ�

}

void circle(XEvent ev){
  int xs,ys,xe,ye;

  XCopyArea( dis, canvas, pix, gc, 0, 0, CANV_WIDTH, CANV_HEIGHT, 0, 0 );

  xs=ev.xbutton.x;
  ys=ev.xbutton.y;

  while( !(ev.type == ButtonRelease && ev.xbutton.button == 1) ){
    if(ev.type == MotionNotify){
      XCopyArea( dis, pix, canvas, gc, 0, 0, CANV_WIDTH, CANV_HEIGHT, 0, 0 );
      xe=ev.xbutton.x;
      ye=ev.xbutton.y;
      
      XDrawArc(dis, canvas, gc,
	       (xs<xe)?xs:xe, (ys<ye)?ys:ye,
	       abs(xe-xs), abs(ye-ys),
	       0,360*64);
      
      XFlush(dis);
    }
    XNextEvent( dis, &ev );
  }

  addHistory();//������ɲ�

}

void fill(XEvent ev){

  int i,j;
  int x,y;
  unsigned long pixel;
  unsigned long **pixelmap;
  //unsigned long pixelmap[CANV_WIDTH][CANV_HEIGHT];
  XImage *ximage;
  Bool check[CANV_WIDTH][CANV_HEIGHT];

  if((pixelmap=(unsigned long **)malloc(sizeof(unsigned int *)*CANV_WIDTH))==NULL)printf("malloc fault1\n");
  for(i=0;i<CANV_WIDTH;i++)
    if((pixelmap[i]=(unsigned long *)malloc(sizeof(unsigned int)*CANV_HEIGHT))==NULL)printf("malloc fault2\n");

  for(i=0;i<CANV_WIDTH;i++)
    for(j=0;j<CANV_HEIGHT;j++)
      check[i][j]=0;
  
  x=ev.xbutton.x;
  y=ev.xbutton.y;

  XCopyArea( dis, canvas, pix, gc, 0, 0, CANV_WIDTH, CANV_HEIGHT, 0, 0 );
  
  for (j = 0; j < CANV_HEIGHT; j++) {
    ximage = XGetImage(dis,pix,0,j,CANV_WIDTH,1,AllPlanes,ZPixmap);
  
    for(i=0;i<CANV_WIDTH;i++){
      pixelmap[i][j] = XGetPixel(ximage,i,0);
    }
    //XPutImage(dis,win,copy_gc,ximage,0,j,0,j,width,1);
  }
  pixel=pixelmap[x][y];
  pixelmap[x][y]=current_color;
  check[x][y]=1;
  fillCheck(x,y,pixel,pixelmap,check);


  ximage = XGetImage(dis,pix,0,0,CANV_WIDTH,1,AllPlanes,ZPixmap);
  for (j = 0; j < CANV_HEIGHT; j++) {
    for(i=0;i<CANV_WIDTH;i++){
      XPutPixel(ximage,i,0,pixelmap[i][j]);
    }
    XPutImage(dis,pix,gc,ximage,0,0,0,j,CANV_WIDTH,1);
  }

  XCopyArea( dis, pix, canvas, gc, 0, 0, CANV_WIDTH, CANV_HEIGHT, 0, 0 );

  addHistory();//������ɲ�

}

void fillCheck(int x,int y,unsigned long pixel,
	       unsigned long **pixelmap,
	       Bool check[CANV_WIDTH][CANV_HEIGHT]){
  if(x-1>=0 && check[x-1][y]==0 && pixel==pixelmap[x-1][y]){
    pixelmap[x-1][y]=current_color;
    check[x-1][y]=1;
    fillCheck(x-1,y,pixel,pixelmap,check);
  }
  if(y-1>=0 && check[x][y-1]==0 && pixel==pixelmap[x][y-1]){
    pixelmap[x][y-1]=current_color;
    check[x][y-1]=1;
    fillCheck(x,y-1,pixel,pixelmap,check);
  }
  if(x+1<CANV_WIDTH && check[x+1][y]==0 && pixel==pixelmap[x+1][y]){
    pixelmap[x+1][y]=current_color;
    check[x+1][y]=1;
    fillCheck(x+1,y,pixel,pixelmap,check);
  }
  if(y+1<CANV_HEIGHT && check[x][y+1]==0 && pixel==pixelmap[x][y+1]){
    pixelmap[x][y+1]=current_color;
    check[x][y+1]=1;
    fillCheck(x,y+1,pixel,pixelmap,check);
  }
}


/*
void fill(XEvent ev){

  void fillCheck(int x,int y,XImage *ximage,unsigned long pixel,Bool check[CANV_WIDTH][CANV_HEIGHT]){
    unsigned long getpixel;
    if(x-1>=0 && check[x-1][y]==False && pixel==XGetPixel(ximage,x-1,y)){
      XPutPixel(ximage,x-1,y,current_color);
      check[x-1][y]=True;
      fillCheck(x-1,y,ximage,pixel,check);
    }
    if(y-1>=0 && check[x][y-1]==False && pixel==XGetPixel(ximage,x,y-1)){
      XPutPixel(ximage,x,y-1,current_color);
      check[x][y-1]=True;
      fillCheck(x,y-1,ximage,pixel,check);
    }
    if(x+1<CANV_WIDTH && check[x+1][y]==False && pixel==XGetPixel(ximage,x+1,y)){
      XPutPixel(ximage,x+1,y,current_color);
      check[x+1][y]=True;
      fillCheck(x+1,y,ximage,pixel,check);
    }
    if(y+1<CANV_HEIGHT && check[x][y+1]==False && pixel==XGetPixel(ximage,x,y+1)){
      XPutPixel(ximage,x,y+1,current_color);
      check[x][y+1]=True;
      fillCheck(x,y+1,ximage,pixel,check);
    }
  }

  int i,j;
  int x,y;
  unsigned long pixel;
  XImage *ximage;
  Bool check[CANV_WIDTH][CANV_HEIGHT];

  int testtest(int a){
    return x+y;
  }

  printf("%d\n",testtest(1));

  for(i=0;i<CANV_WIDTH;i++)
    for(j=0;j<CANV_HEIGHT;j++)
      check[i][j]=False;
  
  x=ev.xbutton.x;
  y=ev.xbutton.y;

  XCopyArea( dis, canvas, pix, gc, 0, 0, CANV_WIDTH, CANV_HEIGHT, 0, 0 );
  
  ximage = XGetImage(dis, pix, 0, 0, CANV_WIDTH, CANV_HEIGHT, AllPlanes, ZPixmap);

  pixel=XGetPixel(ximage,x,y);
  XPutPixel(ximage,x,y,current_color);
  check[x][y]=True;
  fillCheck(x,y,ximage,pixel,check);
  addHistory();//������ɲ�

}
*/
