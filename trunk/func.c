#include "inc.h"

#define MAX_COLOR 6  /* 色数 */
#define MAX_PEN 6 /* ペンの太さ数 */
#define MAX_FUNC 5 /* 機能数 */
#define MAX_TYPE 9 /* 線種の数 */

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

Window color_win[MAX_COLOR];                 //色選択のWindow  ID
Window func_win[MAX_FUNC];                   //機能選択のWindow  ID
Window func_sub_win[MAX_FUNC];               //補助機能のWindow  ID
Window pen_win[MAX_PEN];                     //太さ選択のWindow  ID
Window line_type_win[MAX_TYPE];              //線種選択のWindow  ID
Window debug_win;                            //デバッグ用のWindow  ID

GC gc;                                       //グラフィックコンテキスト ID
GC copy_gc;                                  //コピー用グラフィックコンテキスト ID
Pixmap pix;                                  //コピー用ピクスマップ

char *color_name[]={"black",
		    "red",
		    "green",
		    "blue",
		    "yellow",
		    "white"};//サブウィンドウで選択できる色

unsigned long background; //メインウィンドウの背景色
unsigned long foreground; //メインウィンドウの背景色

int button_size=20;    //サブウィンドウのサイズ

int state=FREEHAND; // 今何の機能が選択されているか。

unsigned long current_color=0; // 今何色か
unsigned long current_pen=2; // 今太さはいくつか
unsigned long current_line=LineSolid; // 今太さはいくつか
unsigned long current_cap=CapRound; // 今太さはいくつか
unsigned long current_join=JoinMiter; // 今太さはいくつか

void initFuncMenu(Display *disp,Window function,Window canv){

    int i,j,k;

    dis = disp;
    canvas = canv;
    background = WhitePixel( dis, 0);
    foreground = BlackPixel( dis, 0);

    /***************************/
    // GCの初期設定
    /***************************/
    
    gc = XCreateGC( dis, DefaultRootWindow(dis), 0, 0 );    // GCの標準設定
    XSetForeground( dis, gc, current_color  );    // 図形の色を設定
    XSetBackground( dis, gc, background);
    XSetLineAttributes( dis, gc, current_pen, current_line, current_cap, current_join);


    /***************************/
    // コピー用アイテムの初期設定
    /***************************/
    
    pix = XCreatePixmap( dis, canvas, 600, 600, DefaultDepth(dis,0) );
    copy_gc = XCreateGC( dis, DefaultRootWindow(dis), 0, 0 );    // GCの標準設定


    /***************************/
    // ボタンウィンドウを作成 
    /***************************/
    for ( i=0 ; i<MAX_COLOR ; i++ ){  //色選択
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

    for ( i=0 ; i<MAX_FUNC ; i++ ){  // 機能選択
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

    for ( i=0 ; i<MAX_PEN ; i++ ){  //太さ選択
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

    for ( i=0 ; i<MAX_TYPE ; i++ ){  //線種選択
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

    /**************** デバッグ用ウィンドウ *****************/
    debug_win = XCreateSimpleWindow(dis, function, 5, MAIN_HEIGHT-(button_size+5),
				    2*button_size, button_size, 1,
				    GetColor( dis, "rgb:aa/aa/aa"),
				    GetColor( dis, "white"));
    XSelectInput(dis, debug_win,
		 ButtonPressMask |
		 EnterWindowMask |
		 LeaveWindowMask );
    /**************** デバッグ用ウィンドウここまで *****************/
    
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
  for (i=0 ; i<MAX_PEN ; i++ ){//太さ選択再描画
    int pen_size = i*3+2;
    XFillArc(dis, pen_win[i], gc,
	     button_size/2-pen_size/2, button_size/2-pen_size/2,
	     pen_size, pen_size, 0, 360*64);
    
  }
  {//自由曲線（鉛筆）の再描画
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
  {//塗りつぶし（）の再描画
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
  {//線種選択再描画

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
    
    for ( i=0 ; i<MAX_PEN ; i++ ){	  /* 太さ・色選択ウィンドウ上で押された? */
      
      if ( ev.xany.window == pen_win[i] ){      /* ペンサイズを変更 */
	current_pen = i*3+2;
	XSetLineAttributes(dis, gc, current_pen,
			   current_line, current_cap, current_join);
			   //LineSolid, CapRound, JoinMiter);
	
	return(0);
      }
    }
    for ( i=0 ; i<MAX_COLOR ; i++ ){
      if ( ev.xany.window == color_win[i] ){   /* 色を変更 */
	current_color = GetColor(dis, color_name[i]);
	XSetForeground(dis, gc, current_color);
	
	return(0);
      }
    }
    for ( i=0 ; i<MAX_FUNC ; i++ ){
      if ( ev.xany.window == func_win[i] ){   /* 機能を変更 */
	state=i;
	setFuncSubWin();
	return(0);
      }
    }
    for ( i=0 ; i<MAX_TYPE ; i++ ){
      if ( ev.xany.window == line_type_win[i] ){   /* 線種を変更 */
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
    
    /***************************** デバッグ用 ********************************/
    if ( ev.xany.window == debug_win ){ 
      //printf("color=%d size=%d\n",current_color,current_pen);
      for(i=0;i<MAX_COLOR;i++)
	printf("%s %x\n",color_name[i],GetColor( dis, color_name[i]));
      printf("%x\n",GetColor(dis,"yellow")>>8&0xff);
      view();
      return(0);    
    }
    /***************************** デバッグ用 *********************************/
    break;
  case EnterNotify:	/* ウィンドウにポインタが入った */

    for ( i=0 ; i<MAX_PEN ; i++ ){
      if ( ev.xany.window == pen_win[i] ){      /* ペンサイズ */
	XSetWindowBorder(dis, ev.xany.window, GetColor( dis, "black"));
	return(0);
      }
    }
    for ( i=0 ; i<MAX_TYPE ; i++ ){
      if ( ev.xany.window == line_type_win[i] ){      /* 線種 */
	XSetWindowBorder(dis, ev.xany.window, GetColor( dis, "black"));
	return(0);
      }
    }
    for ( i=0 ; i<MAX_COLOR ; i++ ){
      if ( ev.xany.window == color_win[i] ){   /* 色 */
	XSetWindowBorder(dis, ev.xany.window, GetColor( dis, "black"));
	return(0);
      }
    }
    for ( i=0 ; i<MAX_FUNC ; i++ ){
      if ( ev.xany.window == func_win[i] ){   /* 機能 */
	XSetWindowBorder(dis, ev.xany.window, GetColor( dis, "black"));
	return(0);
      }
    }
    if ( ev.xany.window == debug_win ){ /* デバッグ */
      XSetWindowBorder(dis, ev.xany.window, GetColor( dis, "black"));
      return(0);
    }
    
    break;
    
  case LeaveNotify:	/* ウィンドウからポインタが出た */
    for ( i=0 ; i<MAX_PEN ; i++ ){
      if ( ev.xany.window == pen_win[i] ){      /* ペンサイズ */
	XSetWindowBorder(dis, ev.xany.window, GetColor( dis, "rgb:aa/aa/aa"));
	return(0);
      }
    }
    for ( i=0 ; i<MAX_TYPE ; i++ ){
      if ( ev.xany.window == line_type_win[i] ){      /* 線種 */
	XSetWindowBorder(dis, ev.xany.window, GetColor( dis, "rgb:aa/aa/aa"));
	return(0);
      }
    }
    for ( i=0 ; i<MAX_COLOR ; i++ ){
      if ( ev.xany.window == color_win[i] ){   /* 色 */
	XSetWindowBorder(dis, ev.xany.window, GetColor( dis, "rgb:aa/aa/aa"));
	return(0);
      }
    }
    for ( i=0 ; i<MAX_FUNC ; i++ ){
      if ( ev.xany.window == func_win[i] ){   /* 機能 */
	XSetWindowBorder(dis, ev.xany.window, GetColor( dis, "rgb:aa/aa/aa"));
	return(0);
      }
    }
    if ( ev.xany.window == debug_win ){ /* デバッグ */
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

  switch(state){ // 選択されている動作の実行
  case FREEHAND://自由曲線
    freeHand(ev);
    break;
  case LINE://直線
    line(ev);
    break;
  case SQUARE://四角
    square(ev);
    break;
  case CIRCLE://丸
    circle(ev);
    break;
  case FILL://塗りつぶし
    fill(ev);
    break;
  default:
    break;
  }

}

/* 自由曲線 */
void freeHand(XEvent ev){
  int x,y;
  int i;

  x=ev.xbutton.x;
  y=ev.xbutton.y;

  XSetForeground( dis, gc, current_color  );    // 図形の色を設定
  
  XNextEvent( dis, &ev );
  while( !(ev.type == ButtonRelease && ev.xbutton.button == 1) ){
    XDrawLine( dis, canvas, gc, x, y, ev.xbutton.x, ev.xbutton.y );//とりあえずDrawLineで表示。
    XFlush( dis );

    x=ev.xbutton.x;
    y=ev.xbutton.y;
    
    XNextEvent( dis, &ev );
    
  }

  addHistory();//履歴に追加

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

  addHistory();//履歴に追加

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

  addHistory();//履歴に追加

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

  addHistory();//履歴に追加

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

  addHistory();//履歴に追加

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
  addHistory();//履歴に追加

}
*/
