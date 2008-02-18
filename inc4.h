#include <X11/Xlib.h>
#include <X11/Xutil.h>

#define FREEHAND 0
#define LINE 1
#define SQUARE 2
#define CIRCLE 3
#define FILL 4
#define CHANGECOLOR 22
#define CHANGECOLORPEN 23
#define CHANGEPEN 33

#define FUNC_WIDTH 50 /* Functionウィンドウの幅 */
#define CANV_WIDTH 600 /* Canvasウィンドウの幅 */
#define LAYER_WIDTH 50 /* Layer設定ウィンドウの幅 */
#define MAIN_WIDTH (CANV_WIDTH+FUNC_WIDTH+LAYER_WIDTH) 
#define MAIN_HEIGHT 600
#define FUNC_HEIGHT MAIN_HEIGHT
#define CANV_HEIGHT MAIN_HEIGHT
#define LAYER_HEIGHT MAIN_HEIGHT

#define MAX_NAME 7 //レイヤーの名前最大文字数

#define MAX_LAYER 10

/****************************/
/* 構造体宣言               */
/****************************/
typedef struct _LayerItem{
  Window win;
  Window name;
  Window visible;
  Window writable;
} LayerItem;

typedef struct _Layer{
  Window win;
  Pixmap mask;
  Pixmap exact;
  int visible;
  int writable;
  char name[MAX_NAME];
  LayerItem item;
  struct _Layer *next;
} Layer;

/**********************************/
/* プロトタイプ宣言               */
/* (ファイル外からも使用する関数) */
/**********************************/

/* canvas.c の関数 */
Window* initCanvas(Window root);
int eventLayerMenu(XEvent ev);
void remapCanvas();
void save_png();

/* layer.c の関数 */
Layer* getSelectedLayer();

/* function.c の関数 */
void execFunc(XEvent ev,Layer *layer);


/* history.c の関数 */
int backHistory(Layer *layer);
void deleteHistory(Layer *layer);












/* colorselect.c の関数 */

void initColorSelect(Display *dis,Window main);
unsigned long callColorSelect( Display *dis, unsigned long oldcolor,Window canvas);

/* func.c の関数 */
void initFuncMenu(Display *disp,Window func,Window canv);
void setFuncSubWin();
void remapFuncMenu();
int eventFuncMenu(XEvent ev);

/* history.c の関数 */
void addHistory(Window *layer,Pixmap *mask,int num);
void view();
void swapHistory(int num1,int num2);

/* png.c の関数 */
void write_png(char *file_name, unsigned char **image, int width, int height);

/* メインの関数 */

unsigned long GetColor( Display *dis, char* color_name );
