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

#define MAX_LAYER 10

/* layer.c の関数 */
void initLayer(Display *disp,Window win,Window canvas);
int eventLayerMenu(XEvent ev,Window canvas);
void remapCanvas();
void save_png(Window canvas);
/* colorselect.c の関数 */

void initColorSelect(Display *dis,Window main);
unsigned long callColorSelect( Display *dis, unsigned long oldcolor,Window canvas);

/* func.c の関数 */
void initFuncMenu(Display *disp,Window func,Window canv);
void setFuncSubWin();
void remapFuncMenu();
int eventFuncMenu(XEvent ev);
void execFunc(XEvent ev,Window *layer_expose,Window *layer,Window *mask,int num);

/* history.c の関数 */
void addHistory(Window *layer,Pixmap *mask,int num);
int backHistory(Window *layer_expose,Window *layer,Pixmap *mask);
void view();
void swapHistory(int num1,int num2);
void deleteHistory(int num);

/* png.c の関数 */
void write_png(char *file_name, unsigned char **image, int width, int height);

/* メインの関数 */

unsigned long GetColor( Display *dis, char* color_name );
