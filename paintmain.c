#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>

#include "inc4.h"
#define TITLEBAR_HEIGHT 5

/****************************/
/* 構造体宣言               */
/****************************/
typedef struct _Windata{
  Window win;
  Window titlebar;
  Window *main;
  unsigned int width;
  unsigned int height;
  int visible;
} Windata;

/****************************/
/* 大域変数宣言             */
/****************************/
Display* dis;                                //Display pointer


unsigned long GetColor( Display* dis, char* color_name )
{
    Colormap cmap;
    XColor near_color, true_color;

    cmap = DefaultColormap( dis, 0 );
    XAllocNamedColor( dis, cmap, color_name, &near_color, &true_color );
    return( near_color.pixel );
}

int main( void )
{
  Window win;                                  //Window  ID

  Windata canvas;
  Windata function;
  Windata color_pallet;
  Windata layer_select;

  XSetWindowAttributes att;                    //窓属性の変数
  XWindowAttributes root_info;                      //ルート窓の情報を得るための変数
  //XWindowAttributes win_info;                      //作成された窓の情報を得るための変数
  XEvent ev;                                   //イベント取り込み変数
  
  char title[]      = "Paint";
  char icon_title[] = "Paint";
  
  int window_width=MAIN_WIDTH; //メインウィンドウの横のサイズ
  int window_height=MAIN_HEIGHT; //メインウィンドウの縦のサイズ
  unsigned long background; //メインウィンドウの背景色
  unsigned long foreground; //メインウィンドウの文字色
  
  //int stopEvent=0; //1 の時、次のイベントを読まずにループする。 
  
  int exit_flag=0;//終了するとき1になる
  
  int i,j,k;
  
  //Xserverとの接続
  if((dis = XOpenDisplay( NULL ))<=0){
    printf("can't connect Xserver\n");
    exit(0);
  }
  
  background = WhitePixel( dis, 0);
  foreground = BlackPixel( dis, 0);
  
  //ルート窓情報の取得
  XGetWindowAttributes(dis, RootWindow(dis,0), &root_info);
  
  /***************************/
  // ウィンドウの作成
  /***************************/
  
  /* メインウィンドウ */
  win = XCreateSimpleWindow( dis, RootWindow(dis,0),
			     0, 0,
			     root_info.width, root_info.height,
			     0, 0, background );
  
  XSetStandardProperties(dis, win, title, icon_title,
			 None, NULL, 0, NULL);
  
  XSelectInput( dis, win, ExposureMask|KeyPressMask ); 
  
  
  /* Canvasウィンドウ */
  canvas.win = XCreateSimpleWindow( dis, win,
				    0, 0,
				    root_info.width, root_info.height,
				    0, 0, background );
  XSelectInput( dis, canvas.win, 0);
  
  canvas.main = initCanvas( canvas.win);
  
  initSubWindow(dis,canvas);
  
  /* 機能メニューウィンドウ */
  function.win = XCreateSimpleWindow( dis, win,
				    0, 0,
				    root_info.width, root_info.height,
				    0, 0, background );
  XSelectInput( dis, function.win, 0);
  
  function.main = initFuncMenu( dis, function.win);
  
  initSubWindow(dis,function);
  


  initHistory(dis, canvas, CANV_WIDTH, CANV_HEIGHT);//履歴の初期化
  initLayer(dis, win, canvas);//レイヤーメニューの初期化
  
  // 全てのウィンドウをマップ
  XMapWindow( dis, win);
  XMapSubwindows( dis, win);
  XMapSubwindows( dis, function);
  XMapSubwindows( dis, canvas);
  setFuncSubWin();
  
  initColorSelect(dis,win);//カラーセレクトの初期化
  
  do{                                                     //窓が開くの待つループ
    XNextEvent( dis, &ev);
  }while( ev.type != Expose );                            // Exposeイベントが届くまでここを繰り返す
  
  // ここまで来たら真っ黒な窓が登場しているはず。
  remapCanvas();
  remapFuncMenu();
  
  while(exit_flag==0){
    
    
    //マウスの動き監視
    //if(stopEvent) 
    //stopEvent = 0;
    //else
    XNextEvent( dis, &ev );
    
    if(!eventFuncMenu(ev))continue;
    if(!eventLayerMenu(ev,canvas))continue;
    
    switch(ev.type){
    case ButtonPress:
      if( ev.xbutton.button == 3 ){
	exit_flag=1;
	XFlush(dis);
	break;
      }
      break;
    case Expose:
      if ( ev.xany.window == function ){
	remapFuncMenu();
      }
      break;
      
    case MotionNotify:	/* ボタンを押しながらマウスが動いた */
      break;
      
    default:
      break;
      
    }
    
  }
  
  XDestroyWindow( dis , win );
  XCloseDisplay( dis );
  
  return(0);
}

void initSubWindow(Display *dis, Windata windata){
  unsigned int dummy;
  
  //ウィンドウのサイズを取得
  XGetGeometry( dis, windata.*main, (Window *)&dummy, (int *)&dummy, (int *)&dummy,
		&(windata.width), &(windata.height), &dummy, &dummy);
  
  windata.visible=1;
  
  XResizeWindow(dis,windata.main,windata.width,windata.height+TITLEBAR_HEIGHT);
  XMoveWindow(dis,windata.main,0,TITLEBAR_HEIGHT);
  
  windata.titlebar = XCreateSimpleWindow( dis, windata.win,
					  0, 0,
					  windata.width, TITLEBAR_HEIGHT,
					  0, 0, background );
  XSelectInput( dis, windata.titlebar,
		Button1MotionMask
		|ButtonPressMask
		|ButtonReleaseMask
		|ExposureMask ); 
  


}
