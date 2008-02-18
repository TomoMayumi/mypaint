#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>

#include "inc4.h"
#define TITLEBAR_HEIGHT 5

/****************************/
/* ��¤�����               */
/****************************/
typedef struct _Windata{
  Window win;
  Window titlebar;
  Window *main;
  unsigned int width;
  unsigned int height;
  int visible;
} Windata;

/****************************/
/* ����ѿ����             */
/****************************/
Display* dis;                                //Display pointer


unsigned long GetColor( Display* dis, char* color_name )
{
    Colormap cmap;
    XColor near_color, true_color;

    cmap = DefaultColormap( dis, 0 );
    XAllocNamedColor( dis, cmap, color_name, &near_color, &true_color );
    return( near_color.pixel );
}

int main( void )
{
  Window win;                                  //Window  ID

  Windata canvas;
  Windata function;
  Windata color_pallet;
  Windata layer_select;

  XSetWindowAttributes att;                    //��°�����ѿ�
  XWindowAttributes root_info;                      //�롼����ξ�������뤿����ѿ�
  //XWindowAttributes win_info;                      //�������줿��ξ�������뤿����ѿ�
  XEvent ev;                                   //���٥�ȼ������ѿ�
  
  char title[]      = "Paint";
  char icon_title[] = "Paint";
  
  int window_width=MAIN_WIDTH; //�ᥤ�󥦥���ɥ��β��Υ�����
  int window_height=MAIN_HEIGHT; //�ᥤ�󥦥���ɥ��νĤΥ�����
  unsigned long background; //�ᥤ�󥦥���ɥ����طʿ�
  unsigned long foreground; //�ᥤ�󥦥���ɥ���ʸ����
  
  //int stopEvent=0; //1 �λ������Υ��٥�Ȥ��ɤޤ��˥롼�פ��롣 
  
  int exit_flag=0;//��λ����Ȥ�1�ˤʤ�
  
  int i,j,k;
  
  //Xserver�Ȥ���³
  if((dis = XOpenDisplay( NULL ))<=0){
    printf("can't connect Xserver\n");
    exit(0);
  }
  
  background = WhitePixel( dis, 0);
  foreground = BlackPixel( dis, 0);
  
  //�롼�������μ���
  XGetWindowAttributes(dis, RootWindow(dis,0), &root_info);
  
  /***************************/
  // ������ɥ��κ���
  /***************************/
  
  /* �ᥤ�󥦥���ɥ� */
  win = XCreateSimpleWindow( dis, RootWindow(dis,0),
			     0, 0,
			     root_info.width, root_info.height,
			     0, 0, background );
  
  XSetStandardProperties(dis, win, title, icon_title,
			 None, NULL, 0, NULL);
  
  XSelectInput( dis, win, ExposureMask|KeyPressMask ); 
  
  
  /* Canvas������ɥ� */
  canvas.win = XCreateSimpleWindow( dis, win,
				    0, 0,
				    root_info.width, root_info.height,
				    0, 0, background );
  XSelectInput( dis, canvas.win, 0);
  
  canvas.main = initCanvas( canvas.win);
  
  initSubWindow(dis,canvas);
  
  /* ��ǽ��˥塼������ɥ� */
  function.win = XCreateSimpleWindow( dis, win,
				    0, 0,
				    root_info.width, root_info.height,
				    0, 0, background );
  XSelectInput( dis, function.win, 0);
  
  function.main = initFuncMenu( dis, function.win);
  
  initSubWindow(dis,function);
  


  initHistory(dis, canvas, CANV_WIDTH, CANV_HEIGHT);//����ν����
  initLayer(dis, win, canvas);//�쥤�䡼��˥塼�ν����
  
  // ���ƤΥ�����ɥ���ޥå�
  XMapWindow( dis, win);
  XMapSubwindows( dis, win);
  XMapSubwindows( dis, function);
  XMapSubwindows( dis, canvas);
  setFuncSubWin();
  
  initColorSelect(dis,win);//���顼���쥯�Ȥν����
  
  do{                                                     //�뤬�������Ԥĥ롼��
    XNextEvent( dis, &ev);
  }while( ev.type != Expose );                            // Expose���٥�Ȥ��Ϥ��ޤǤ����򷫤��֤�
  
  // �����ޤ��褿�鿿�ù����뤬�о줷�Ƥ���Ϥ���
  remapCanvas();
  remapFuncMenu();
  
  while(exit_flag==0){
    
    
    //�ޥ�����ư���ƻ�
    //if(stopEvent) 
    //stopEvent = 0;
    //else
    XNextEvent( dis, &ev );
    
    if(!eventFuncMenu(ev))continue;
    if(!eventLayerMenu(ev,canvas))continue;
    
    switch(ev.type){
    case ButtonPress:
      if( ev.xbutton.button == 3 ){
	exit_flag=1;
	XFlush(dis);
	break;
      }
      break;
    case Expose:
      if ( ev.xany.window == function ){
	remapFuncMenu();
      }
      break;
      
    case MotionNotify:	/* �ܥ���򲡤��ʤ���ޥ�����ư���� */
      break;
      
    default:
      break;
      
    }
    
  }
  
  XDestroyWindow( dis , win );
  XCloseDisplay( dis );
  
  return(0);
}

void initSubWindow(Display *dis, Windata windata){
  unsigned int dummy;
  
  //������ɥ��Υ����������
  XGetGeometry( dis, windata.*main, (Window *)&dummy, (int *)&dummy, (int *)&dummy,
		&(windata.width), &(windata.height), &dummy, &dummy);
  
  windata.visible=1;
  
  XResizeWindow(dis,windata.main,windata.width,windata.height+TITLEBAR_HEIGHT);
  XMoveWindow(dis,windata.main,0,TITLEBAR_HEIGHT);
  
  windata.titlebar = XCreateSimpleWindow( dis, windata.win,
					  0, 0,
					  windata.width, TITLEBAR_HEIGHT,
					  0, 0, background );
  XSelectInput( dis, windata.titlebar,
		Button1MotionMask
		|ButtonPressMask
		|ButtonReleaseMask
		|ExposureMask ); 
  


}
