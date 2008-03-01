#include "inc3.h"
#include <X11/extensions/shape.h>  /* Shape Extension    */

#define SELECTED_COLOR GetColor( dis, "rgb:66/66/66")
#define UNSELECTED_COLOR1 GetColor( dis, "rgb:ee/ee/ee")
#define UNSELECTED_COLOR2 GetColor( dis, "rgb:99/99/99")
#define BORDERLINE_COLOR GetColor( dis, "rgb:aa/aa/aa")
#define OVERED_BORDERLINE_COLOR GetColor( dis, "black")

#define MAX_NAME 7 //レイヤーの名前最大文字数

typedef struct _Layer{
  Window win;
  Pixmap mask;
  Pixmap exact;
  int visible;
  int writable;
  char name[MAX_NAME]
}Layer;


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

Display *dis;

Window canvas;
Layer layer[MAX_LAYER];            //レイヤー
int layer_num;                     //レイヤーの使用数
int stack[MAX_LAYER];           //レイヤーの上下順

/**********************************/
/* initCanvas()                   */
/* キャンバス(レイヤー)初期化関数 */
/**********************************/

Window* initCanvas(Display *disp,Window root){
  int i,j;
  GC gc;
  //標準ディスプレイの位置を保存
  dis=disp;
  //キャンバスウィンドウの作成
  canvas = XCreateSimpleWindow( dis, root,
				0, 0,
				CANV_WIDTH, CANV_HEIGHT,
				0, 0, background );
  XSelectInput( dis, canvas,
		Button1MotionMask
		|ButtonPressMask
		|ButtonReleaseMask
		|ExposureMask ); 
  //使用レイヤー数の初期化
  layer_num = 0;

  //初期レイヤーの作成
  addLayer();

  return(&canvas);
}

/**********************************/
/* eventCanvas()               */
/* レイヤー関係のイベント処理関数 */
/**********************************/

int eventCanvas(XEvent ev){
  int i,j;
  int selected=getSelectedNumber();//現在選択されているレイヤー
  int back_num;
  switch(ev.type){
  case ButtonPress:
    //左クリックの時
    if( ev.xbutton.button == 1 ){
      // キャンバス上
      if ( ev.xany.window == canvas ){
	if(layer[selected].writable!=0){//書き込み設定の確認
	  execFunc(ev,layer,selected);//描画関数の実行
	}
	return(0);
      }
      for(i=0;i<layer_num;i++){	  
	// レイヤー上  
	if ( ev.xany.window == layer[i].win ){
	  if(layer[selected].writable!=0){//書き込み設定の確認
	    execFunc(ev,layer,selected);//描画関数の実行
	  }
	  return(0);
	}
      }
    }
    // ホイールクリックの時
    if( ev.xbutton.button == 2 ){
      back_num = backHistory(layer); // 元に戻る
      return(0);
    }
    
    break;
    
  case Expose:
    if ( ev.xany.window == canvas ){
      remapCanvas();// レイヤー再描画
      return(0);
    }
    for(i=0;i<layer_num;i++){	    
      if ( ev.xany.window == layer[i].win ){
	remapCanvas();// レイヤー再描画
	return(0);
      }
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
  GC gc = XCreateGC( dis, layer[0].win, 0, 0 ); // 標準GCの取得
  for(i=0;i<layer_num;i++){
    XCopyArea( dis, layer[i].exact, layer[i].win,
	       gc, 0, 0, CANV_WIDTH , CANV_HEIGHT, 0, 0 );
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
  int i;

  unsigned long background = WhitePixel(dis,0);

  if(layer_num>=MAX_LAYER)return;

  //レイヤーの作成
  layer[layer_num].win = XCreateSimpleWindow( dis, canvas,
					      0, 0,
					      CANV_WIDTH, CANV_HEIGHT,
					      0, 0, background );
  XSetWindowBackgroundPixmap( dis, layer[layer_num].win, None );//背景を透明に
    
  XSelectInput( dis, layer[layer_num].win,
		Button1MotionMask
		|ButtonPressMask
		|ButtonReleaseMask
		|ExposureMask );
  //レイヤーマスクの作成
  layer[layer_num].mask = XCreatePixmap(dis, canvas, CANV_WIDTH, CANV_HEIGHT, 1);

  //再描画のための画面保存用ピクスマップの作成
  layer[layer_num].exact = XCreatePixmap(dis, canvas, CANV_WIDTH, CANV_HEIGHT,
					 DefaultDepth(dis,0));

  //マスク用１ビットGC取得＆標準設定
  mask_gc = XCreateGC( dis, layer[layer_num].mask, 0, 0 );
  XFillRectangle(dis,layer[layer_num].mask,mask_gc,0,0,CANV_WIDTH,CANV_HEIGHT);//マスクの初期化(すべて0にする)
  XShapeCombineMask( dis, layer[layer_num].win, ShapeBounding, 0, 0,//レイヤーをマスクの
		     layer[layer_num].mask, ShapeSet);              // １の部分のみの形にする
  XShapeCombineMask( dis, layer[layer_num].win, ShapeClip, 0, 0,    // ShapeBounding :ボーダーの外枠設定
		     layer[layer_num].mask, ShapeSet);              // ShapeClip     :ボーダーの内枠設定
  //通常GC取得＆標準設定
  gc = XCreateGC( dis, layer[layer_num].win, 0, 0 );
  XSetForeground( dis, gc, GetColor( dis, "white")  );
  XFillRectangle(dis,layer[layer_num].exact,gc,0,0,CANV_WIDTH,CANV_HEIGHT);//再描画用ピクスマップの初期化


  //名前の設定
  sprintf(layer[layer_num].name,"Layer%d",layer_num);

  //可視、書き込み設定の初期化
  layer[layer_num].visible=1;
  layer[layer_num].writable=1;

  //レイヤーの表示
  XMapWindow( dis, layer[layer_num].win);
  //スタック順をずらして追加
  for(i=layer_num;i>0;i++){
    stack[i+1]=stack[i];
  }
  stack[0]=layer_num;

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

void save_png(){
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
  int i;
  Window windows[MAX_LAYER];

  for(i=0;i<layer_num;i++){
    if(stack[i]==num1){
      stack[i]=num2;
    }else if(stack[i]==num2){
      stack[i]=num1;
    }
  }


  for(i=0;i<layer_num;i++){
    windows[i]=layer[stack[i]].win;
  }
  XRestackWindows(dis, windows, layer_num); 

  //remapCanvas();
  remapLayerMenu();

}

/*******************************/
/* upLayer()                   */
/* レイヤーを前面に持ってくる  */
/* num:持ってくるレイヤー番号  */
/*******************************/
void upLayer(int num){
  int i;

  for(i=0;i<layer_num && stack[i]!=num ;i++);

  if(i>0)swapLayer(stack[i],stack[i-1]);

}
/*******************************/
/* downLayer()                 */
/* レイヤーを背面に持っていく  */
/* num:持っていくレイヤー番号  */
/*******************************/
void downLayer(int num){
  int i;

  for(i=0;i<layer_num && stack[i]!=num ;i++);

  if(i<layer_num-1)swapLayer(stack[i],stack[i+1]);

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
	  printf("%d %d %d %s in changeLayerName\n",strlen(layer_name[num]),strlen(strbuf),MAX_NAME,layer_name[num]);
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
