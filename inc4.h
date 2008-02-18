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

#define FUNC_WIDTH 50 /* Function������ɥ����� */
#define CANV_WIDTH 600 /* Canvas������ɥ����� */
#define LAYER_WIDTH 50 /* Layer���ꥦ����ɥ����� */
#define MAIN_WIDTH (CANV_WIDTH+FUNC_WIDTH+LAYER_WIDTH) 
#define MAIN_HEIGHT 600
#define FUNC_HEIGHT MAIN_HEIGHT
#define CANV_HEIGHT MAIN_HEIGHT
#define LAYER_HEIGHT MAIN_HEIGHT

#define MAX_NAME 7 //�쥤�䡼��̾������ʸ����

#define MAX_LAYER 10

/****************************/
/* ��¤�����               */
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
/* �ץ�ȥ��������               */
/* (�ե����볰�������Ѥ���ؿ�) */
/**********************************/

/* canvas.c �δؿ� */
Window* initCanvas(Window root);
int eventLayerMenu(XEvent ev);
void remapCanvas();
void save_png();

/* layer.c �δؿ� */
Layer* getSelectedLayer();

/* function.c �δؿ� */
void execFunc(XEvent ev,Layer *layer);


/* history.c �δؿ� */
int backHistory(Layer *layer);
void deleteHistory(Layer *layer);












/* colorselect.c �δؿ� */

void initColorSelect(Display *dis,Window main);
unsigned long callColorSelect( Display *dis, unsigned long oldcolor,Window canvas);

/* func.c �δؿ� */
void initFuncMenu(Display *disp,Window func,Window canv);
void setFuncSubWin();
void remapFuncMenu();
int eventFuncMenu(XEvent ev);

/* history.c �δؿ� */
void addHistory(Window *layer,Pixmap *mask,int num);
void view();
void swapHistory(int num1,int num2);

/* png.c �δؿ� */
void write_png(char *file_name, unsigned char **image, int width, int height);

/* �ᥤ��δؿ� */

unsigned long GetColor( Display *dis, char* color_name );
