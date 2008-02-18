#include "inc3.h"
#include <X11/extensions/shape.h>  /* Shape Extension    */

#define SELECTED_COLOR GetColor( dis, "rgb:66/66/66")
#define UNSELECTED_COLOR1 GetColor( dis, "rgb:ee/ee/ee")
#define UNSELECTED_COLOR2 GetColor( dis, "rgb:99/99/99")
#define BORDERLINE_COLOR GetColor( dis, "rgb:aa/aa/aa")
#define OVERED_BORDERLINE_COLOR GetColor( dis, "black")

#define MAX_NAME 16 //レイヤーの名前最大文字数


/**********************************/
/* プロトタイプ宣言               */
/* (ファイル外からも使用する関数) */
/**********************************/
//void initLayer(Display *disp,Window win,Window canvas);
//int eventLayerMenu(XEvent ev,Window canvas);
//void remapCanvas();
//void save_png(Window canvas);

/**************************************/
/* プロトタイプ宣言                   */
/* (このファイル内でのみ使用する関数) */
/**************************************/


void remapLayerMenu();
void addLayer(Window canvas);

/************/
/* 大域変数 */
/************/

Display *dis;


Window layer_menu;                  //レイヤー管理用メニューウィンドウ

Window add_layer;                   //レイヤー追加ボタン
Window delete_layer;                   //レイヤー削除ボタン

Window layer_item[MAX_LAYER];       //各レイヤー用ウィンドウ
Window layer_namew[MAX_LAYER];      //レイヤーの名前ウィンドウ
char layer_name[MAX_LAYER][MAX_NAME]; //レイヤーの名前
Window layer_viewable[MAX_LAYER];   //可視不可視の設定ボタン
int view_state[MAX_LAYER];          //可視不可視の設定状態 1:可 0:不可
Window layer_writeable[MAX_LAYER];  //書き込み可能の設定ボタン
int write_state[MAX_LAYER];         //書き込み可能の設定状態 1:可 0:不可

Window layer[MAX_LAYER];            //表示するレイヤー
Pixmap layer_mask[MAX_LAYER];       //透過部分のマスク
Pixmap layer_expose[MAX_LAYER];     //レイヤーの実際の画像

int layer_num;                      //レイヤーの使用数

int selected_layer;              //現在選択されているレイヤー番号

/**********************/
/* initLayer()        */
/* レイヤー初期化関数 */
/**********************/

void initLayer(Display *disp,Window win,Window canvas){
  int i,j;
  GC mask_gc;
  GC gc;

  //標準ディスプレイの位置を保存
  dis=disp;
  //使用レイヤー数の初期化
  layer_num = 0;

  //メニューウィンドウの作成  
  layer_menu = XCreateSimpleWindow( dis, win,
				    MAIN_WIDTH-LAYER_WIDTH, 0,
				    LAYER_WIDTH-2, LAYER_HEIGHT-2,
				    1, 0, GetColor(dis,"gray") );
  XSelectInput( dis, layer_menu,ExposureMask);

  //レイヤー追加ボタンの作成
  add_layer = XCreateSimpleWindow( dis, layer_menu,
				   2, 2,
				   LAYER_WIDTH/2-2-4-2, 20-2,
				   1, 0, GetColor(dis,"gray") );
  XSelectInput( dis, add_layer,ButtonPressMask);
  XMapWindow( dis, add_layer);

  //レイヤー削除ボタンの作成
  delete_layer = XCreateSimpleWindow( dis, layer_menu,
				   (LAYER_WIDTH-2)/2+2, 2,
				   (LAYER_WIDTH-2)/2-4-2, 20-2,
				   1, 0, GetColor(dis,"gray") );
  XSelectInput( dis, delete_layer,ButtonPressMask);
  XMapWindow( dis, delete_layer);

  //初期レイヤーの作成
  addLayer(canvas);
  addLayer(canvas);
  addLayer(canvas);

  //レイヤー選択の初期化
  selected_layer=0;
  XSetWindowBackground( dis, layer_namew[0], SELECTED_COLOR);

}

/**********************************/
/* eventLayerMenu()               */
/* レイヤー関係のイベント処理関数 */
/**********************************/

int eventLayerMenu(XEvent ev,Window canvas){
  int i,j;
  int back_num;
  GC gc = XCreateGC( dis, layer[selected_layer], 0, 0 ); // GCの標準設定
  switch(ev.type){
  case ButtonPress:
    //左クリックの時
    if( ev.xbutton.button == 1 ){
      // キャンバス上
      if ( ev.xany.window == canvas ){
	if(write_state[selected_layer]!=0){//書き込み設定の確認
	  execFunc(ev,layer_expose,layer,layer_mask,selected_layer);//描画関数の実行
	}
	return(0);
      }
      for(i=0;i<layer_num;i++){	  
	// レイヤー上  
	if ( ev.xany.window == layer[i] ){
	  if(write_state[selected_layer]!=0){//書き込み設定の確認
	    execFunc(ev,layer_expose,layer,layer_mask,selected_layer);//描画関数の実行
	  }
	  return(0);
	}
	// レイヤーの名前部分
	if ( ev.xany.window == layer_namew[i] ){
	  XSetWindowBackground( dis, layer_namew[selected_layer], UNSELECTED_COLOR1);
	  XClearWindow(dis,layer_namew[selected_layer]);
	  selected_layer = i; //レイヤー変更
	  XSetWindowBackground( dis, layer_namew[i], SELECTED_COLOR);
	  XClearWindow(dis,layer_namew[i]);
	  remapLayerMenu();
	  return(0);
	}
	// レイヤーの可視設定ボタン
	if ( ev.xany.window == layer_viewable[i] ){
	  if(view_state[i]==0){//見えるようにする
	    view_state[i]=1;
	    XMapWindow( dis, layer[i]);
	    remapLayerMenu();

	  }else{//見えなくする
	    view_state[i]=0;
	    XUnmapWindow( dis, layer[i]);
	    write_state[i]=0;
	    remapLayerMenu();


	  }
	  return(0);
	}
	// レイヤーの書き込み設定ボタン
	if ( ev.xany.window == layer_writeable[i] ){
	  if(write_state[i]==0){//書き込み可能にする
	    write_state[i]=1;
	    view_state[i]=1;
	    XMapWindow( dis, layer[i]);
	    remapLayerMenu();

	  }else{//書き込み不可にする
	    write_state[i]=0;
	    remapLayerMenu();


	  }
	  return(0);
	}
      }
      //レイヤー追加ボタン
      if( ev.xany.window == add_layer ){
	addLayer(canvas);
	return(0);
      }
      //レイヤー削除ボタン
      if( ev.xany.window == delete_layer ){

	return(0);
      }
      
    }
    // ホイールクリックの時
    if( ev.xbutton.button == 2 ){
      back_num = backHistory(layer_expose,layer,layer_mask); // 元に戻る
      return(0);
    }
    
    break;
    
  case Expose:
    if ( ev.xany.window == canvas ){
      remapCanvas();// レイヤー再描画
      return(0);
    }
    for(i=0;i<layer_num;i++){	    
      if ( ev.xany.window == layer[i] ){
	remapCanvas();// レイヤー再描画
	return(0);
      }
    }
    if ( ev.xany.window == layer_menu ){
      remapLayerMenu();// レイヤーメニュー再描画
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
/* すべてのレイヤー再描画関数 */
/******************************/

void remapCanvas(){
  int i;
  GC gc = XCreateGC( dis, layer[selected_layer], 0, 0 ); // 標準GCの取得
  for(i=0;i<layer_num;i++){
    XCopyArea( dis, layer_expose[i], layer[i],
	       gc, 0, 0, CANV_WIDTH , CANV_HEIGHT, 0, 0 );
  }
  XFreeGC(dis,gc);
}

/******************************/
/* remapLayerMenu()           */
/* レイヤーメニュー再描画関数 */
/******************************/

void remapLayerMenu(){
  int i;
  int button_width=(LAYER_WIDTH-2-4-2)/2-2;
  int button_height=15-2;
  GC gc = XCreateGC( dis, layer[selected_layer], 0, 0 ); // 標準GCの取得
  for(i=0;i<layer_num;i++){
    //ボタン模様の再描画
    XSetForeground(dis,gc,GetColor(dis,"black"));
    {//可視設定ボタン
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
    {//書き込み設定ボタン
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
/* レイヤー追加関数 */
/********************/

void addLayer(Window canvas){
  GC mask_gc;
  GC gc;

  unsigned long background = WhitePixel(dis,0);

  if(layer_num>=MAX_LAYER)return;

  //レイヤーの作成
  layer[layer_num] = XCreateSimpleWindow( dis, canvas,
					  0, 0,
					  CANV_WIDTH, CANV_HEIGHT,
					  0, 0, background );
  XSetWindowBackgroundPixmap( dis, layer[layer_num], None );//背景を透明に
    
  XSelectInput( dis, layer[layer_num],
		Button1MotionMask
		|ButtonPressMask
		|ButtonReleaseMask
		|ExposureMask );
  //レイヤーマスクの作成
  layer_mask[layer_num] = XCreatePixmap(dis, canvas, CANV_WIDTH, CANV_HEIGHT, 1);

  //再描画のための画面保存用ピクスマップの作成
  layer_expose[layer_num] = XCreatePixmap(dis, canvas, CANV_WIDTH, CANV_HEIGHT,
					  DefaultDepth(dis,0));

  //マスク用１ビットGC取得＆標準設定
  mask_gc = XCreateGC( dis, layer_mask[layer_num], 0, 0 );
  XFillRectangle(dis,layer_mask[layer_num],mask_gc,0,0,CANV_WIDTH,CANV_HEIGHT);//マスクの初期化(すべて0にする)
  XShapeCombineMask( dis, layer[layer_num], ShapeBounding, 0, 0,//レイヤーをマスクの
		     layer_mask[layer_num], ShapeSet);          // １の部分のみの形にする
  XShapeCombineMask( dis, layer[layer_num], ShapeClip, 0, 0,    // ShapeBounding :ボーダーの外枠設定
		     layer_mask[layer_num], ShapeSet);          // ShapeClip     :ボーダーの内枠設定
  //通常GC取得＆標準設定
  gc = XCreateGC( dis, layer[layer_num], 0, 0 );
  XSetForeground( dis, gc, GetColor( dis, "white")  );
  XFillRectangle(dis,layer_expose[layer_num],gc,0,0,CANV_WIDTH,CANV_HEIGHT);//再描画用ピクスマップの初期化


  //レイヤー設定用パネルの作成
  layer_item[layer_num] = XCreateSimpleWindow( dis, layer_menu,
					       2, 24+layer_num*34,
					       LAYER_WIDTH-2-4-2, 30,
					       1, 0, GetColor(dis,"gray") );
  
  //レイヤーの名前用ウィンドウの作成
  layer_namew[layer_num] = XCreateSimpleWindow( dis, layer_item[layer_num],
						0, 0,
						LAYER_WIDTH-2-4-2, 15,
						0, 0, UNSELECTED_COLOR1 );
  
  XSelectInput( dis, layer_namew[layer_num],ButtonPressMask);
  //可視、書き込み設定ボタンの大きさ
  int button_width=(LAYER_WIDTH-2-4-2)/2-2;
  int button_height=15-2;
  //レイヤー可視設定ボタンの作成
  layer_viewable[layer_num] = XCreateSimpleWindow( dis, layer_item[layer_num],
						   0, 15,
						   button_width, button_height,
						   1, 0, UNSELECTED_COLOR1 );
  XSelectInput( dis, layer_viewable[layer_num],ButtonPressMask);
  
  //レイヤー書き込み設定ボタンの作成
  layer_writeable[layer_num] = XCreateSimpleWindow( dis, layer_item[layer_num],
						    (LAYER_WIDTH-2-4-2)/2, 15,
						    button_width, button_height,
						    1, 0, UNSELECTED_COLOR1 );
  XSelectInput( dis, layer_writeable[layer_num],ButtonPressMask);

  //可視、書き込み設定の初期化
  view_state[layer_num]=1;
  write_state[layer_num]=1;

  //レイヤーの表示(ただし今は何も表示されない)
  XMapWindow( dis, layer_item[layer_num]);
  //レイヤー設定パネルの表示
  XMapSubwindows( dis, layer_item[layer_num]);
  //ボタンの模様の描画
  XSetForeground(dis,gc,GetColor(dis,"black"));
  {//可視設定ボタン
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
  {//書き込み設定ボタン
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
  
  //使用レイヤー数を増やす
  layer_num++;
  //使用したGCの解放
  XFreeGC(dis,gc);
  XFreeGC(dis,mask_gc);
}

void save_png(Window canvas){
  Window savemap;
  unsigned char   **image;         // image[HEIGHT][WIDTH]の形式です
  int             i, j, pixeldata;
  XImage *ximage;
  
  savemap = XCreateSimpleWindow( dis, canvas,
				 0, 0,
				 CANV_WIDTH, CANV_HEIGHT,
				 0, 0, 0 );
  XSetWindowBackgroundPixmap( dis, savemap, None );//背景を透明に
  XMapWindow(dis,savemap);

  image = (unsigned char**)malloc(CANV_HEIGHT * sizeof(unsigned char*)); // 以下３行は２次元配列を確保します
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
  write_png("test2.png", image, CANV_WIDTH, CANV_HEIGHT);            // PNGファイルを作成します
  for (j = 0; j < CANV_HEIGHT; j++) free(image[j]);            // 以下２行は２次元配列を解放します
  free(image);

  XUnmapWindow(dis,savemap);

  XDestroyWindow(dis,savemap);
  
}
