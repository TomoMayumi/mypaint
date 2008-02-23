/* mtest.cc */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>

#include "inc3.h"

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
    Display* dis;                                //Display pointer
    Window win;                                  //Window  ID
    Window canvas;
    Window function;
    XSetWindowAttributes att;                    //窓属性の変数
    XEvent ev;                                   //イベント取り込み変数

    char title[]      = "Paint";
    char icon_title[] = "Painta";

    int window_width=MAIN_WIDTH; //メインウィンドウの横のサイズ
    int window_height=MAIN_HEIGHT; //メインウィンドウの縦のサイズ
    unsigned long background; //メインウィンドウの背景色
    unsigned long foreground; //メインウィンドウの文字色


    int stopEvent=0; //1 の時、次のイベントを読まずにループする。 

    //int remap_flag=0;//一度再描画したら、一定時間再描画しない。そのカウント。
    int exit_flag=0;//終了するとき1になる
    int i,j,k;

    if((dis = XOpenDisplay( NULL ))<=0){                    //Xserverとの接続
      printf("can't connect Xserver\n");
      exit(0);
    }

    background = WhitePixel( dis, 0);
    foreground = BlackPixel( dis, 0);

    /***************************/
    // ウィンドウの作成
    /***************************/

    /* メインウィンドウ */
    win = XCreateSimpleWindow( dis, RootWindow(dis,0),
			       600, 100,
			       window_width, window_height,
			       0, 0, background );

    XSetStandardProperties(dis, win, title, icon_title,
			   None, NULL, 0, NULL);

    XSelectInput( dis, win, ExposureMask|KeyPressMask ); 
    

    /* Canvasウィンドウ */
    canvas = XCreateSimpleWindow( dis, win,
				  FUNC_WIDTH, 0,
				  CANV_WIDTH, CANV_HEIGHT,
				  0, 0, background );
      XSelectInput( dis, canvas,
		    Button1MotionMask
		    |ButtonPressMask
		    |ButtonReleaseMask
		    |ExposureMask ); 
    /* 機能メニューウィンドウ */
    function = XCreateSimpleWindow( dis, win,
				    0, 0,
				    FUNC_WIDTH-2, FUNC_HEIGHT-2, 1,
				    GetColor(dis,"black"),
				    GetColor(dis,"gray") );

    XSelectInput( dis, function, ExposureMask|EnterWindowMask|LeaveWindowMask ); 

    initFuncMenu(dis, function, canvas);//機能メニューウィンドウの初期化
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
      if(stopEvent) 
	stopEvent = 0;
      else
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
