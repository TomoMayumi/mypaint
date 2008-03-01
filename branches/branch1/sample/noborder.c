#include<X11/Xlib.h>
#include<X11/Xutil.h>


//描画メソッド
void paint(Display*d,Window w,int screen,GC gc){
	Colormap    colormap; 
	XColor      near_color, true_color;
	//色の変更
	colormap = DefaultColormap( d, screen ); 
	XAllocNamedColor( d, colormap, "red", &near_color, &true_color ); 
	XSetForeground( d, gc, near_color.pixel );
	//塗りつぶした四角を描画
	XFillRectangle(d,w,gc,10,10,180,180);
	XFlush( d );
}

main(){
	Display	*d;
	Window	w;
	int screen;
	XSetWindowAttributes attr;
	GC gc;
	
	d = XOpenDisplay( NULL );//Xサーバーに接続する
	screen=DefaultScreen(d);
	//ウインドウの作成
	w = XCreateSimpleWindow( d, RootWindow(d,screen),
						0, 0, 200, 200,//ウインドウのサイズ
						1,BlackPixel(d,screen),//枠の設定
						WhitePixel(d,screen));//背景色の色番号

	//ウィンドウの内容を記憶
	attr.backing_store = WhenMapped;
	XChangeWindowAttributes( d, w, CWBackingStore, &attr); 

	//ウインドウ名の設定
	XStoreName(d, w, "test Window");
	XSetIconName(d, w, "test Window");
	
	//グラフィックコンテキストを取得
	gc = XCreateGC( d, RootWindow( d,screen ), 0, 0); 


	XEvent event;
	//イベントマスクを登録
	XSelectInput( d, w,ButtonPressMask | ExposureMask); 

	//ウィンドウの属性を変更
	XSetWindowAttributes wa;
	wa.override_redirect=True;
	XChangeWindowAttributes (d, w, CWOverrideRedirect, &wa);

	//マップして表示
	XMapWindow( d, w );
	XFlush( d );

	while(1){
		XNextEvent( d, &event );
		switch(event.type){
			case ButtonPress://画面上でマウスのボタンが押された時
				XDestroyWindow( d, w);//終了処理
				XCloseDisplay( d );
				exit(0);
			case Expose://再描画要求
				paint(d,w,screen,gc);//再描画
				break;
			default:
				break;
		}
	}
}




