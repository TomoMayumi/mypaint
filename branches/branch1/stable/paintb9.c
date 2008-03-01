/* mtest.cc */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>

#include "inc3.h"

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
    Display* dis;                                //Display pointer
    Window win;                                  //Window  ID
    Window canvas;
    Window function;
    XSetWindowAttributes att;                    //��°�����ѿ�
    XEvent ev;                                   //���٥�ȼ������ѿ�

    char title[]      = "Paint";
    char icon_title[] = "Painta";

    int window_width=MAIN_WIDTH; //�ᥤ�󥦥���ɥ��β��Υ�����
    int window_height=MAIN_HEIGHT; //�ᥤ�󥦥���ɥ��νĤΥ�����
    unsigned long background; //�ᥤ�󥦥���ɥ����طʿ�
    unsigned long foreground; //�ᥤ�󥦥���ɥ���ʸ����


    int stopEvent=0; //1 �λ������Υ��٥�Ȥ��ɤޤ��˥롼�פ��롣 

    //int remap_flag=0;//���ٺ����褷���顢������ֺ����褷�ʤ������Υ�����ȡ�
    int exit_flag=0;//��λ����Ȥ�1�ˤʤ�
    int i,j,k;

    if((dis = XOpenDisplay( NULL ))<=0){                    //Xserver�Ȥ���³
      printf("can't connect Xserver\n");
      exit(0);
    }

    background = WhitePixel( dis, 0);
    foreground = BlackPixel( dis, 0);

    /***************************/
    // ������ɥ��κ���
    /***************************/

    /* �ᥤ�󥦥���ɥ� */
    win = XCreateSimpleWindow( dis, RootWindow(dis,0),
			       600, 100,
			       window_width, window_height,
			       0, 0, background );

    XSetStandardProperties(dis, win, title, icon_title,
			   None, NULL, 0, NULL);

    XSelectInput( dis, win, ExposureMask|KeyPressMask ); 
    

    /* Canvas������ɥ� */
    canvas = XCreateSimpleWindow( dis, win,
				  FUNC_WIDTH, 0,
				  CANV_WIDTH, CANV_HEIGHT,
				  0, 0, background );
      XSelectInput( dis, canvas,
		    Button1MotionMask
		    |ButtonPressMask
		    |ButtonReleaseMask
		    |ExposureMask ); 
    /* ��ǽ��˥塼������ɥ� */
    function = XCreateSimpleWindow( dis, win,
				    0, 0,
				    FUNC_WIDTH-2, FUNC_HEIGHT-2, 1,
				    GetColor(dis,"black"),
				    GetColor(dis,"gray") );

    XSelectInput( dis, function, ExposureMask|EnterWindowMask|LeaveWindowMask ); 

    initFuncMenu(dis, function, canvas);//��ǽ��˥塼������ɥ��ν����
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
      if(stopEvent) 
	stopEvent = 0;
      else
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
