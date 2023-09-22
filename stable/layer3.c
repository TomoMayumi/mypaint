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

#define MAX_NAME 7 //レイヤーの名前最大文字数


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
void swapLayer(int num1,int num2);
void deleteLayer(int num);
void changeLayerName(int num);

/************/
/* 大域変数 */
/************/

static Display *dis;


Window layer_menu;                  //レイヤー管理用メニューウィンドウ

Window add_layer;                   //レイヤー追加ボタン
Window delete_layer;                //レイヤー削除ボタン
Window up_layer;                    //レイヤー入れ替えボタン
Window down_layer;                  //レイヤー入れ替えボタン２

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
				   2, 2+20+2,
				   (LAYER_WIDTH-2)/2-4-2, 20-2,
				   1, 0, GetColor(dis,"gray") );
  XSelectInput( dis, delete_layer,ButtonPressMask);
  XMapWindow( dis, delete_layer);

  //レイヤー入れ替えボタンの作成
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

  //初期レイヤーの作成
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
	  if(selected_layer == i){
	    changeLayerName(i);
	    return(0);
	  }else{
	    XSetWindowBackground( dis, layer_namew[selected_layer], UNSELECTED_COLOR1);
	    XClearWindow(dis,layer_namew[selected_layer]);
	    selected_layer = i; //レイヤー変更
	    XSetWindowBackground( dis, layer_namew[i], SELECTED_COLOR);
	    XClearWindow(dis,layer_namew[i]);
	    remapLayerMenu();
	    return(0);
	  }
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
	deleteLayer(selected_layer);
	XSetWindowBackground( dis, layer_namew[selected_layer], SELECTED_COLOR);
	XClearWindow(dis,layer_namew[selected_layer]);
	remapLayerMenu();
	return(0);
      }
      //レイヤー入れ替えボタン
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
    //名前の描画
    XDrawString( dis, layer_namew[i], gc, 2, 12, layer_name[i], strlen(layer_name[i]));
    //不可の設定のところに×を付ける
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
					       2, 46+layer_num*34,
					       LAYER_WIDTH-2-4-2, 30,
					       1, 0, GetColor(dis,"gray") );
  
  //レイヤーの名前用ウィンドウの作成
  layer_namew[layer_num] = XCreateSimpleWindow( dis, layer_item[layer_num],
						0, 0,
						LAYER_WIDTH-2-4-2, 15,
						0, 0, UNSELECTED_COLOR1 );  
  XSelectInput( dis, layer_namew[layer_num],ButtonPressMask | KeyPressMask);
  //名前の設定
  sprintf(layer_name[layer_num],"Layer%d",layer_num);

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
  //名前の描画
  XDrawString( dis, layer_namew[layer_num], gc, 2, 12, layer_name[layer_num], strlen(layer_name[layer_num]));

  //レイヤーの表示
  XMapWindow( dis, layer[layer_num]);

  //使用レイヤー数を増やす
  layer_num++;
  //使用したGCの解放
  XFreeGC(dis,gc);
  XFreeGC(dis,mask_gc);
}

/*********************/
/* save_png()        */
/* PNG形式に保存する */
/*********************/

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


/************************************/
/* swapLayer()                      */
/* レイヤーを入れ替える             */
/* num1,num2:入れ替えるレイヤー番号 */
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

  //レイヤーの名前入れ替え
  strcpy(tmpchar,layer_name[num2]);
  strcpy(layer_name[num2],layer_name[num1]);
  strcpy(layer_name[num1],tmpchar);
  //書き込み設定の入れ替え
  tmpint = write_state[num1];
  write_state[num1] = write_state[num2];
  write_state[num2] = tmpint;
  //可視設定の入れ替え
  tmpint = view_state[num1];
  view_state[num1] = view_state[num2];
  view_state[num2] = tmpint;

  //実際の画像の入れ替え
  XCopyArea(dis, layer_expose[num1], tmplayer, tmpgc, 0, 0, CANV_WIDTH, CANV_HEIGHT, 0, 0);
  XCopyArea(dis, layer_expose[num2], layer_expose[num1], tmpgc, 0, 0, CANV_WIDTH, CANV_HEIGHT, 0, 0);
  XCopyArea(dis, tmplayer, layer_expose[num2], tmpgc, 0, 0, CANV_WIDTH, CANV_HEIGHT, 0, 0);
  //マスクの入れ替え
  XCopyArea(dis, layer_mask[num1], tmpmask, tmpmaskgc, 0, 0, CANV_WIDTH, CANV_HEIGHT, 0, 0);
  XCopyArea(dis, layer_mask[num2], layer_mask[num1], tmpmaskgc, 0, 0, CANV_WIDTH, CANV_HEIGHT, 0, 0);
  XCopyArea(dis, tmpmask, layer_mask[num2], tmpmaskgc, 0, 0, CANV_WIDTH, CANV_HEIGHT, 0, 0);
  //マスクのしなおし
  XShapeCombineMask( dis, layer[num1], ShapeBounding, 0, 0, layer_mask[num1], ShapeSet);
  XShapeCombineMask( dis, layer[num1], ShapeClip, 0, 0, layer_mask[num1], ShapeSet);
  XShapeCombineMask( dis, layer[num2], ShapeBounding, 0, 0, layer_mask[num2], ShapeSet);
  XShapeCombineMask( dis, layer[num2], ShapeClip, 0, 0, layer_mask[num2], ShapeSet);
  //表示のしなおし
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
  //履歴の入れ替え
  swapHistory(num1,num2);

  remapCanvas();
  remapLayerMenu();

}

/****************************/
/* deleteLayer()            */
/* レイヤーを削除する       */
/* num:削除するレイヤー番号 */
/****************************/

void deleteLayer(int num){
  int i;
  //レイヤーが一つしかない時は動作しない
  if(layer_num==1)return;
  //履歴を削除
  deleteHistory(num);
  printf("test in deleteLayer\n");
  //削除したいレイヤーを一番最後へ持っていく
  for(i=num;i<(layer_num-1);i++){
    swapLayer(i,i+1);
  }
  //削除
  XDestroyWindow(dis,layer[i]);
  XFreePixmap(dis,layer_mask[i]);
  XFreePixmap(dis,layer_expose[i]);
  XDestroyWindow(dis,layer_item[i]);

  layer_num--;
  //最後のレイヤーが選択されていた時は、その手前のレイヤーを選択状態にする。
  if(selected_layer>layer_num-1){
    selected_layer=layer_num-1;
  }
}

/****************************/
/* changeLayerName()        */
/* レイヤーの名前を変更する */
/* num:変更するレイヤー番号 */
/****************************/

void changeLayerName(int num){
  GC gc;
  XEvent ev;    //イベント取り込み変数
  KeySym key;
  char strbuf[10];
  int exit_flag=0;
  
  //通常GC取得＆標準設定
  gc = XCreateGC( dis, layer[num], 0, 0 );
  XSetForeground( dis, gc, GetColor( dis, "black")  );
  //名前ウィンドウの背景を白に
  XSetWindowBackground( dis, layer_namew[num], 0xffffff);
  XClearWindow(dis,layer_namew[num]);
  XDrawString( dis, layer_namew[num], gc, 2, 12, layer_name[num], strlen(layer_name[num]));

  //文字入力受付
  while(exit_flag==0){
    XNextEvent( dis, &ev );
    switch(ev.type){
    case ButtonPress:
      //違うウィンドウがクリックされたら変更終了
      if(ev.xany.window!=layer_namew[num]){
	exit_flag=1;
      }
      break;
    case KeyPress:
      XLookupString((XKeyEvent *)&ev, strbuf, sizeof(strbuf), &key, NULL);
      //BS文字の判定
      if(strbuf[0]==8){
	//文字列の長さが0以上なら一文字削除
	if(strlen(layer_name[num])) layer_name[num][strlen(layer_name[num])-1]='\0';
      }else{
	if( (strlen(layer_name[num]) + strlen(strbuf)) < MAX_NAME){
	  printf("%zu %zu %d %s in changeLayerName\n",strlen(layer_name[num]),strlen(strbuf),MAX_NAME,layer_name[num]);
	  //文字列の連結
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

  //名前ウィンドウの背景を元に戻す
  XSetWindowBackground( dis, layer_namew[num], SELECTED_COLOR);
  XClearWindow(dis,layer_namew[num]);
  XDrawString( dis, layer_namew[num], gc, 2, 12, layer_name[num], strlen(layer_name[num]));

  //使用したGCの解放
  XFreeGC(dis,gc);
}
