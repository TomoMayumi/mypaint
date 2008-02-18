/* mtest.cc */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>

#include "inc.h"

#define MAX_COLOR 6  /* 色数 */
#define MAX_PEN 6 /* ペンの太さ数 */
#define WIDTH 600
#define HEIGHT 600
// メインのみで使う関数
void freeHand( Display *dis, Window win, GC gc, XEvent ev);


unsigned long GetColor( Display* dis, char* color_name )
{
    Colormap cmap;
    XColor near_color, true_color;

    cmap = DefaultColormap( dis, 0 );
    XAllocNamedColor( dis, cmap, color_name, &near_color, &true_color );
    return( near_color.pixel );
}


unsigned long current_color=0; // 今何色か
unsigned long current_pen=1; // 今太さはいくつか


int main( void )
{
    Display* dis;                                //Display pointer
    Window win;                                  //Window  ID
    Window color_win[MAX_COLOR];                         //色選択のWindow  ID
    //Window type_win[5];                        //動作選択のWindow  ID
    Window pen_win[MAX_PEN];                           //太さ選択のWindow  ID
    Window debug_win;                            //デバッグ用のWindow  ID
    XSetWindowAttributes att;                    //窓属性の変数
    GC gc;                                       //グラフィックコンテキスト ID
    XEvent ev;                                   //イベント取り込み変数

    char title[]      = "Paint";
    char icon_title[] = "Painta";

    int window_width=WIDTH; //メインウィンドウの横のサイズ
    int window_height=HEIGHT; //メインウィンドウの縦のサイズ
    int button_size=40;    //サブウィンドウのサイズ
    unsigned long background; //メインウィンドウの背景色
    unsigned long foreground; //メインウィンドウの文字色

    char *color_name[]={"black","red","green","blue","yellow","white"};//サブウィンドウで選択できる色

    int stopEvent=0; //1 の時、次のイベントを読まずにループする。 
    int state=1; // 今何の動作が選択されているか。

    int remap_flag=0;//一度再描画したら、一定時間再描画しない。そのカウント。
    int exit_flag=0;//終了するとき1になる
    int i;

    if((dis = XOpenDisplay( NULL ))<=0){                    //Xserverとの接続
      printf("can't connect Xserver\n");
      exit(0);
    }

    background = WhitePixel( dis, 0);
    foreground = BlackPixel( dis, 0);

    win = XCreateSimpleWindow( dis, RootWindow(dis,0),
			       100, 100,
			       window_width, window_height,
			       0, 0, background );  //メインウィンドウの生成

    XSetStandardProperties(dis, win, title, icon_title,
			   None, NULL, 0, NULL);
    //メインウィンドウは、窓を開く、ドラッグ、クリック、放すというイベントだけ受け取る
    XSelectInput( dis, win, ExposureMask|Button1MotionMask|ButtonPressMask|ButtonReleaseMask ); 
    
    gc = XCreateGC( dis, DefaultRootWindow(dis), 0, 0 );    // GCの標準設定
    XSetForeground( dis, gc, current_color  );    // 図形の色を設定
    XSetBackground( dis, gc, background);
    XSetLineAttributes( dis, gc, current_pen, LineSolid, CapRound, JoinMiter);

    // 動作選択・色選択・太さ選択ウィンドウを作成 
    for ( i=0 ; i<MAX_COLOR ; i++ ){
      color_win[i] = XCreateSimpleWindow(dis, win, 10, (button_size+10)*i+30,
					 button_size, button_size, 1,
					 GetColor( dis, "rgb:aa/aa/aa"),
					 GetColor( dis, color_name[i]));
      XSelectInput(dis, color_win[i],
		   ButtonPressMask |
		   EnterWindowMask |
		   LeaveWindowMask );
    }
    for ( i=0 ; i<MAX_PEN ; i++ ){
      pen_win[i] = XCreateSimpleWindow(dis, win,
				       window_width-10-button_size, (button_size+10)*i+30,
				       button_size, button_size, 1,
				       GetColor(dis, "rgb:aa/aa/aa"),
				       GetColor(dis, "rgb:ee/ee/ee"));
      XSelectInput(dis, pen_win[i],
		   ButtonPressMask |
		   EnterWindowMask |
		   LeaveWindowMask );
    }

    /**************** デバッグ用ウィンドウ *****************/
    debug_win = XCreateSimpleWindow(dis, win, 10, window_height-(button_size+10),
				    window_width-20, button_size, 1,
				    GetColor( dis, "rgb:aa/aa/aa"),
				    GetColor( dis, "white"));
    XSelectInput(dis, debug_win,
		 ButtonPressMask |
		 EnterWindowMask |
		 LeaveWindowMask );
    /**************** デバッグ用ウィンドウここまで *****************/
    

    /*    backing store は機能しないこともあるので頼らない。 
     *
     *    att.backing_store = Always;                       //絵を保存する設定をする
     *    XChangeWindowAttributes( dis, win, CWBackingStore, &att );
     */

    initHistory(dis, win, window_width, window_height);//履歴の初期化

    // 全てのウィンドウをマップ
    XMapWindow( dis, win);
    XMapSubwindows( dis, win);


    do{                                                     //窓が開くの待つループ
      XNextEvent( dis, &ev);
    }while( ev.type != Expose );                            // Exposeイベントが届くまでここを繰り返す
    
    // ここまで来たら真っ黒な窓が登場しているはず。
    remap();
    for ( i=0 ; i<MAX_PEN ; i++ ){
      int pen_size = i*3+2;
      XSetForeground(dis, gc, foreground);
      XFillArc(dis, pen_win[i], gc,
	       button_size/2-pen_size/2, button_size/2-pen_size/2,
	       pen_size, pen_size, 0, 360*64);
      
    }

    while(exit_flag==0){


      //マウスの動き監視
      if(stopEvent) 
	stopEvent = 0;
      else
	XNextEvent( dis, &ev );
      
      switch(ev.type){
      case ButtonPress:

	if( ev.xbutton.button == 1 ){//左クリックの時
	  
	  for ( i=0 ; i<MAX_PEN ; i++ ){	  /* 太さ・色選択ウィンドウ上で押された? */

	    if ( ev.xany.window == pen_win[i] ){      /* ペンサイズを変更 */
	      current_pen = i*3+2;
	      XSetLineAttributes(dis, gc, current_pen,
				 LineSolid, CapRound, JoinMiter);

	      break;
	    }
	  }
	  for ( i=0 ; i<MAX_COLOR ; i++ ){
	    if ( ev.xany.window == color_win[i] ){   /* 色を変更 */
	      current_color = GetColor(dis, color_name[i]);
	      XSetForeground(dis, gc, current_color);

	      break;
	    }
	  }
	  

	  if ( ev.xany.window == win ){  /* キャンバス上で押された? */
	    switch(state){ // 選択されている動作の実行
	    case FREEHAND://自由曲線
	      freeHand(dis, win, gc, ev);
	      break;
	    default:
	      break;
	    }
	  }

	  /***************************** デバッグ用 ********************************/
	  if ( ev.xany.window == debug_win ){ 
	    //printf("color=%d size=%d\n",current_color,current_pen);
	    for(i=0;i<MAX_COLOR;i++)
	      printf("%s %x\n",color_name[i],GetColor( dis, color_name[i]));
	    printf("%x\n",GetColor(dis,"yellow")>>8&0xff);
	    view();

	  }
	  /***************************** デバッグ用 *********************************/
	  break;
	}
	if( ev.xbutton.button == 2 ){// 元に戻る
	  backHistory();
	  //XSetForeground(dis, gc, current_color);
	  //XSetLineAttributes(dis, gc, current_pen,
	  //		     LineSolid, CapRound, JoinMiter);
	  break;
	}
	if( ev.xbutton.button == 3 ){
	  exit_flag=1;
	  XFlush(dis);
	  break;
	}
	break;
      case Expose:

	//if(remap_flag==0){		/* 再描画 */
	  remap();		/* 再描画 */
	  //  remap_flag=1;
	  //}else{
	  //remap_flag--;
	  //}


	  /* ペンサイズウィンドウを再描画 */
	for ( i=0 ; i<MAX_PEN ; i++ ){
	  int pen_size = i*3+2;
	  XSetForeground(dis, gc, foreground);
	  XFillArc(dis, pen_win[i], gc,
		   button_size/2-pen_size/2, button_size/2-pen_size/2,
		   pen_size, pen_size, 0, 360*64);
	  
	}
	break;
	
      case EnterNotify:	/* ウィンドウにポインタが入った */
	XSetWindowBorder(dis, ev.xany.window, GetColor( dis, "black"));
	break;
	
      case LeaveNotify:	/* ウィンドウからポインタが出た */
	XSetWindowBorder(dis, ev.xany.window, GetColor(dis, "rgb:aa/aa/aa"));
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

/* 自由曲線 */
void freeHand(Display *dis, Window win, GC gc, XEvent ev){
  int x,y;
  int i;

  x=ev.xbutton.x;
  y=ev.xbutton.y;

  XSetForeground( dis, gc, current_color  );    // 図形の色を設定
  
  XNextEvent( dis, &ev );
  while( !(ev.type == ButtonRelease || ev.xbutton.button == 1) ){
    XDrawLine( dis, win, gc, x, y, ev.xbutton.x, ev.xbutton.y );//とりあえずDrawLineで表示。
    XFlush( dis );

    x=ev.xbutton.x;
    y=ev.xbutton.y;
    
    XNextEvent( dis, &ev );
    
  }

  addHistory();//履歴に追加

}

