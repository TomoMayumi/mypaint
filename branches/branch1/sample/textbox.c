#include<X11/Xlib.h>
#include<X11/Xutil.h>

void print(Display *d,Window w,GC gc,char*str){
	XClearWindow(d,w);//ウィンドウのクリア
	int width=2;
	int height=10;
	XDrawString( d, w, gc, width, height, str, strlen(str));
	XFlush( d );
}

void addStr(char*buf,char*str){
	if(str[0]==8){//BS文字の判定
		if(strlen(buf)) buf[strlen(buf)-1]='\0';//文字列の長さが0以上なら一文字削除
	}else{
		//文字列の連結
		char strpos[128];
		strcpy(strpos,buf);
		sprintf(buf,"%s%s",strpos,str);
	}
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

	//テキストボックス作成----------------------------------------------------------------------
	//ウィンドウをウインドウの中に作成してテキストボックスにする
	char text1[128]="";//対応する文字のバッファ
	Window edit1=XCreateSimpleWindow( d, w,
							10, 10, 80, 15,//ウインドウのサイズ
							1,BlackPixel(d,screen),//枠の設定
							WhitePixel(d,screen));//背景色の色番号
	//マップして表示
	XMapWindow( d,edit1 );
	//イベントマスクを登録
	XSelectInput( d, edit1,KeyPressMask | ButtonPressMask); 

	//----------------------------------------------------------------------

	//テキストボックス2を作成----------------------------------------------------------------------
	//ウィンドウをウインドウの中に作成してテキストボックスにする
	char text2[128]="";//対応する文字のバッファ
	Window edit2=XCreateSimpleWindow( d, w,
							10, 50, 80, 15,//ウインドウのサイズ
							1,BlackPixel(d,screen),//枠の設定
							WhitePixel(d,screen));//背景色の色番号
	//マップして表示
	XMapWindow( d,edit2 );
	//イベントマスクを登録
	XSelectInput( d, edit2,KeyPressMask | ButtonPressMask); 
	//----------------------------------------------------------------------

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
	XSelectInput( d, w,ExposureMask | KeyPressMask | ButtonPressMask); 

	//マップして表示
	XMapWindow( d, w );
	XFlush( d );
	KeySym key;
	char strbuf[10];

	//クリックされたWindowを保存しておく変数
	//キーボード入力時、このWindowに対応したバッファに文字が追加される
	Window wpos;

	while(1){
		XNextEvent( d, &event );
		switch(event.type){
			case Expose:
				//全てのテキストボックスの再描画
				print(d,edit1,gc,text1);
				print(d,edit2,gc,text2);
				break;
			case ButtonPress://画面上でマウスのボタンが押された時のWindowを確保
				wpos=event.xexpose.window;
				break;
			case KeyPress:
				XLookupString((XKeyEvent *)&event, strbuf, sizeof(strbuf), &key, NULL);
				if(wpos==edit1){//確保されたWindowに対応する文字バッファに文字を追加
					addStr(text1,strbuf);
				}else if(wpos==edit2){
					addStr(text2,strbuf);
				}
				//全てのテキストボックスの再描画
				print(d,edit1,gc,text1);
				print(d,edit2,gc,text2);
				break;
			default:
				break;
		}
	}

}
