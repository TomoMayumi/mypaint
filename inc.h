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
#define MAIN_WIDTH (CANV_WIDTH+FUNC_WIDTH) 
#define MAIN_HEIGHT 600
#define FUNC_HEIGHT MAIN_HEIGHT
#define CANV_HEIGHT MAIN_HEIGHT

/* colorselect.c �δؿ� */

void initColorSelect(Display *dis,Window main);
unsigned long callColorSelect( Display *dis, unsigned long oldcolor);

/* func.c �δؿ� */
void initFuncMenu(Display *disp,Window func,Window canv);
void setFuncSubWin();
void remapFuncMenu();
int eventFuncMenu(XEvent ev);
void execFunc(XEvent ev);

/* history.c �δؿ� */
void addHistory();

void remapCanvas();
void backHistory();
void view();

/* png.c �δؿ� */
void write_png(char *file_name, unsigned char **image, int width, int height);

/* �ᥤ��δؿ� */

unsigned long GetColor( Display *dis, char* color_name );
