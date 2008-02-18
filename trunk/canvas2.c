// memo

// 削除時のレイヤー選択未解決


#include "inc4.h"
#include <X11/extensions/shape.h>  /* Shape Extension    */

#define SELECTED_COLOR GetColor( dis, "rgb:66/66/66")
#define UNSELECTED_COLOR1 GetColor( dis, "rgb:ee/ee/ee")
#define UNSELECTED_COLOR2 GetColor( dis, "rgb:99/99/99")
#define BORDERLINE_COLOR GetColor( dis, "rgb:aa/aa/aa")
#define OVERED_BORDERLINE_COLOR GetColor( dis, "black")

/**********************************/
/* プロトタイプ宣言               */
/* (ファイル外からも使用する関数) */
/**********************************/
//Window* initCanvas(Display *disp,Window root);
//int eventLayerMenu(XEvent ev);
//void remapCanvas();
//void save_png();

/**************************************/
/* プロトタイプ宣言                   */
/* (このファイル内でのみ使用する関数) */
/**************************************/

void addLayer();
void deleteLayer(Layer *layer);
void upLayer(Layer *layer);
void downLayer(Layer *layer);

/******************/
/* 大域変数(外部) */
/******************/

extern Display *dis;       //mainから参照

/************/
/* 大域変数 */
/************/

Layer *layerlist=NULL;            //レイヤーリスト(layer.cからも参照)
static Window canvas;
static int layer_num;             //レイヤーの使用数

/**********************************/
/* initCanvas()                   */
/* キャンバス(レイヤー)初期化関数 */
/**********************************/

Window* initCanvas(Window root){
  int i,j;
  GC gc;
  //キャンバスウィンドウの作成
  canvas = XCreateSimpleWindow( dis, root,
				0, 0,
				CANV_WIDTH, CANV_HEIGHT,
				0, 0, 0xffffff );
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
  Layer *selectedlayer = getSelectedLayer();//現在選択されているレイヤー
  Layer *layertmp;
  int back_num;

  switch(ev.type){
  case ButtonPress:
    //左クリックの時
    if( ev.xbutton.button == 1 ){
      // キャンバス上
      if ( ev.xany.window == canvas ){
	if(selectedlayer->writable!=0){//書き込み設定の確認
	  execFunc(ev,selectedlayer);//描画関数の実行
	}
	return(0);
      }
      layertmp=layerlist;
      for(i=0;i<layer_num;i++){	  
	// レイヤー上  
	if ( ev.xany.window == layertmp->win ){
	  if(layertmp->writable!=0){//書き込み設定の確認
	    execFunc(ev,selectedlayer);//描画関数の実行
	  }
	  return(0);
	}
	layertmp=layertmp->next;
      }
    }
    // ホイールクリックの時
    if( ev.xbutton.button == 2 ){
      back_num = backHistory(selectedlayer); // 元に戻る
      return(0);
    }
    
    break;
    
  case Expose:
    if ( ev.xany.window == canvas ){
      remapCanvas();// レイヤー再描画
      return(0);
    }
    layertmp=layerlist;
    for(i=0;i<layer_num;i++){
      if ( ev.xany.window == layertmp->win ){
	remapCanvas();// レイヤー再描画
	return(0);
      }
      layertmp=layertmp->next;
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
  Layer *layertmp=layerlist;
  GC gc = XCreateGC( dis, layertmp->win, 0, 0 ); // 標準GCの取得
  for(i=0;i<layer_num;i++){
    XCopyArea( dis, layertmp->exact, layertmp->win,
	       gc, 0, 0, CANV_WIDTH , CANV_HEIGHT, 0, 0 );
    layertmp=layertmp->next;
  }
  XFreeGC(dis,gc);
}

/********************/
/* addLayer()       */
/* レイヤー追加関数 */
/********************/

void addLayer(){
  GC mask_gc;
  GC gc;
  Layer *new;
  int i;

  unsigned long background = WhitePixel(dis,0);

  if(layer_num>=MAX_LAYER)return;

  if( (new=(Layer *)malloc(sizeof(Layer))) == NULL ){
    printf("*malloc error (addLayer)*\n");
  }

  //レイヤーの作成
  new->win = XCreateSimpleWindow( dis, canvas,
				  0, 0,
				  CANV_WIDTH, CANV_HEIGHT,
				  0, 0, background );
  XSetWindowBackgroundPixmap( dis, new->win, None );//背景を透明に
  
  XSelectInput( dis, new->win,
		Button1MotionMask
		|ButtonPressMask
		|ButtonReleaseMask
		|ExposureMask );
  //レイヤーマスクの作成
  new->mask = XCreatePixmap(dis, canvas, CANV_WIDTH, CANV_HEIGHT, 1);

  //再描画のための画面保存用ピクスマップの作成
  new->exact = XCreatePixmap(dis, canvas, CANV_WIDTH, CANV_HEIGHT,
			     DefaultDepth(dis,0));

  //マスク用１ビットGC取得＆標準設定
  mask_gc = XCreateGC( dis, new->mask, 0, 0 );
  XFillRectangle(dis,new->mask,mask_gc,0,0,CANV_WIDTH,CANV_HEIGHT);//マスクの初期化(すべて0にする)
  XShapeCombineMask( dis, new->win, ShapeBounding, 0, 0,//レイヤーをマスクの
		     new->mask, ShapeSet);              // １の部分のみの形にする
  XShapeCombineMask( dis, new->win, ShapeClip, 0, 0,    // ShapeBounding :ボーダーの外枠設定
		     new->mask, ShapeSet);              // ShapeClip     :ボーダーの内枠設定
  //通常GC取得＆標準設定
  gc = XCreateGC( dis, new->win, 0, 0 );
  XSetForeground( dis, gc, GetColor( dis, "white")  );
  XFillRectangle(dis,new->exact,gc,0,0,CANV_WIDTH,CANV_HEIGHT);//再描画用ピクスマップの初期化


  //名前の設定
  sprintf(new->name,"Layer%d",layer_num);

  //可視、書き込み設定の初期化
  new->visible=1;
  new->writable=1;

  //リストに追加
  new->next=layerlist;
  layerlist=new;

  //レイヤーの表示
  XMapWindow( dis, new->win);

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


/*******************************/
/* upLayer()                   */
/* レイヤーを前面に持ってくる  */
/* layer:持ってくるレイヤー    */
/*******************************/
void upLayer(Layer *layer){
  int i;
  Layer *layertmp=layerlist;
  Layer *layertmp2;

  if(layertmp==layer)return;

  if(layertmp->next==layer){
    layertmp->next=layer->next;
    layer->next=layertmp;
    layerlist=layer;
  }else{
    for(i=0;i<(layer_num-2);i++){
      if(layertmp->next->next==layer){
	layertmp2=layertmp->next;
	layertmp2->next=layer->next;
	layer->next=layertmp2;
	layertmp->next=layer;
	break;
      }
    }
  }
  resetLayer();
}
/*******************************/
/* downLayer()                 */
/* レイヤーを背面に持っていく  */
/* num:持っていくレイヤー番号  */
/*******************************/
void downLayer(Layer *layer){
  int i;
  Layer *layertmp=layerlist;

  if(layertmp==layer){
    layertmp=layertmp->next;
    layer->next=layertmp->next;
    layertmp->next=layer;
    layerlist=layertmp;
  }else{ 
    for(i=0;i<(layer_num-1);i++){
      if(layertmp->next==layer){
	layertmp->next=layer->next;
	layer->next=layer->next->next;
	layertmp->next->next=layer;
	break;
      }
    }
  }
  resetLayer();
}

/****************************/
/* deleteLayer()            */
/* レイヤーを削除する       */
/* num:削除するレイヤー番号 */
/****************************/

void deleteLayer(Layer *layer){
  int i;
  Layer *layertmp=layerlist;
  //レイヤーが一つしかない時は動作しない
  if(layer_num==1)return;
  //履歴を削除
  deleteHistory(layer);
  printf("test in deleteLayer\n");
  //削除したいレイヤーをリストから除外
  if(layertmp==layer)
    layerlist=layerlist->next;
  else{
    for(i=0;i<(layer_num-1);i++){
      if(layertmp->next==layer){
	layertmp->next=layer->next;
	break;
      }
    }
  }
  //削除
  XDestroyWindow(dis,layer->win);
  XFreePixmap(dis,layer->mask);
  XFreePixmap(dis,layer->exact);
  DeleteLayerItem(layer);
  free(layer);

  layer_num--;
  //最後のレイヤーが選択されていた時は、その手前のレイヤーを選択状態にする。
  //if(selected_layer>layer_num-1){
  //  selected_layer=layer_num-1;
  //}
}
