/* mtest.cc */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>

#include "inc.h"

#define MAX_COLOR 6  /* ���� */
#define MAX_PEN 6 /* �ڥ�������� */
#define WIDTH 600
#define HEIGHT 600
// �ᥤ��ΤߤǻȤ��ؿ�
void freeHand( Display *dis, Window win, GC gc, XEvent ev);


unsigned long GetColor( Display* dis, char* color_name )
{
    Colormap cmap;
    XColor near_color, true_color;

    cmap = DefaultColormap( dis, 0 );
    XAllocNamedColor( dis, cmap, color_name, &near_color, &true_color );
    return( near_color.pixel );
}


unsigned long current_color=0; // ��������
unsigned long current_pen=1; // �������Ϥ����Ĥ�


int main( void )
{
    Display* dis;                                //Display pointer
    Window win;                                  //Window  ID
    Window color_win[MAX_COLOR];                         //�������Window  ID
    //Window type_win[5];                        //ư�������Window  ID
    Window pen_win[MAX_PEN];                           //���������Window  ID
    Window debug_win;                            //�ǥХå��Ѥ�Window  ID
    XSetWindowAttributes att;                    //��°�����ѿ�
    GC gc;                                       //����ե��å�����ƥ����� ID
    XEvent ev;                                   //���٥�ȼ������ѿ�

    char title[]      = "Paint";
    char icon_title[] = "Painta";

    int window_width=WIDTH; //�ᥤ�󥦥���ɥ��β��Υ�����
    int window_height=HEIGHT; //�ᥤ�󥦥���ɥ��νĤΥ�����
    int button_size=40;    //���֥�����ɥ��Υ�����
    unsigned long background; //�ᥤ�󥦥���ɥ����طʿ�
    unsigned long foreground; //�ᥤ�󥦥���ɥ���ʸ����

    char *color_name[]={"black","red","green","blue","yellow","white"};//���֥�����ɥ�������Ǥ��뿧

    int stopEvent=0; //1 �λ������Υ��٥�Ȥ��ɤޤ��˥롼�פ��롣 
    int state=1; // ������ư����򤵤�Ƥ��뤫��

    int remap_flag=0;//���ٺ����褷���顢������ֺ����褷�ʤ������Υ�����ȡ�
    int exit_flag=0;//��λ����Ȥ�1�ˤʤ�
    int i;

    if((dis = XOpenDisplay( NULL ))<=0){                    //Xserver�Ȥ���³
      printf("can't connect Xserver\n");
      exit(0);
    }

    background = WhitePixel( dis, 0);
    foreground = BlackPixel( dis, 0);

    win = XCreateSimpleWindow( dis, RootWindow(dis,0),
			       100, 100,
			       window_width, window_height,
			       0, 0, background );  //�ᥤ�󥦥���ɥ�������

    XSetStandardProperties(dis, win, title, icon_title,
			   None, NULL, 0, NULL);
    //�ᥤ�󥦥���ɥ��ϡ���򳫤����ɥ�å�������å��������Ȥ������٥�Ȥ����������
    XSelectInput( dis, win, ExposureMask|Button1MotionMask|ButtonPressMask|ButtonReleaseMask ); 
    
    gc = XCreateGC( dis, DefaultRootWindow(dis), 0, 0 );    // GC��ɸ������
    XSetForeground( dis, gc, current_color  );    // �޷��ο�������
    XSetBackground( dis, gc, background);
    XSetLineAttributes( dis, gc, current_pen, LineSolid, CapRound, JoinMiter);

    // ư�����򡦿������������򥦥���ɥ������ 
    for ( i=0 ; i<MAX_COLOR ; i++ ){
      color_win[i] = XCreateSimpleWindow(dis, win, 10, (button_size+10)*i+30,
					 button_size, button_size, 1,
					 GetColor( dis, "rgb:aa/aa/aa"),
					 GetColor( dis, color_name[i]));
      XSelectInput(dis, color_win[i],
		   ButtonPressMask |
		   EnterWindowMask |
		   LeaveWindowMask );
    }
    for ( i=0 ; i<MAX_PEN ; i++ ){
      pen_win[i] = XCreateSimpleWindow(dis, win,
				       window_width-10-button_size, (button_size+10)*i+30,
				       button_size, button_size, 1,
				       GetColor(dis, "rgb:aa/aa/aa"),
				       GetColor(dis, "rgb:ee/ee/ee"));
      XSelectInput(dis, pen_win[i],
		   ButtonPressMask |
		   EnterWindowMask |
		   LeaveWindowMask );
    }

    /**************** �ǥХå��ѥ�����ɥ� *****************/
    debug_win = XCreateSimpleWindow(dis, win, 10, window_height-(button_size+10),
				    window_width-20, button_size, 1,
				    GetColor( dis, "rgb:aa/aa/aa"),
				    GetColor( dis, "white"));
    XSelectInput(dis, debug_win,
		 ButtonPressMask |
		 EnterWindowMask |
		 LeaveWindowMask );
    /**************** �ǥХå��ѥ�����ɥ������ޤ� *****************/
    

    /*    backing store �ϵ�ǽ���ʤ����Ȥ⤢��Τ����ʤ��� 
     *
     *    att.backing_store = Always;                       //������¸��������򤹤�
     *    XChangeWindowAttributes( dis, win, CWBackingStore, &att );
     */

    initHistory(dis, win, window_width, window_height);//����ν����

    // ���ƤΥ�����ɥ���ޥå�
    XMapWindow( dis, win);
    XMapSubwindows( dis, win);


    do{                                                     //�뤬�������Ԥĥ롼��
      XNextEvent( dis, &ev);
    }while( ev.type != Expose );                            // Expose���٥�Ȥ��Ϥ��ޤǤ����򷫤��֤�
    
    // �����ޤ��褿�鿿�ù����뤬�о줷�Ƥ���Ϥ���
    remap();
    for ( i=0 ; i<MAX_PEN ; i++ ){
      int pen_size = i*3+2;
      XSetForeground(dis, gc, foreground);
      XFillArc(dis, pen_win[i], gc,
	       button_size/2-pen_size/2, button_size/2-pen_size/2,
	       pen_size, pen_size, 0, 360*64);
      
    }

    while(exit_flag==0){


      //�ޥ�����ư���ƻ�
      if(stopEvent) 
	stopEvent = 0;
      else
	XNextEvent( dis, &ev );
      
      switch(ev.type){
      case ButtonPress:

	if( ev.xbutton.button == 1 ){//������å��λ�
	  
	  for ( i=0 ; i<MAX_PEN ; i++ ){	  /* �����������򥦥���ɥ���ǲ����줿? */

	    if ( ev.xany.window == pen_win[i] ){      /* �ڥ󥵥������ѹ� */
	      current_pen = i*3+2;
	      XSetLineAttributes(dis, gc, current_pen,
				 LineSolid, CapRound, JoinMiter);

	      break;
	    }
	  }
	  for ( i=0 ; i<MAX_COLOR ; i++ ){
	    if ( ev.xany.window == color_win[i] ){   /* �����ѹ� */
	      current_color = GetColor(dis, color_name[i]);
	      XSetForeground(dis, gc, current_color);

	      break;
	    }
	  }
	  

	  if ( ev.xany.window == win ){  /* �����Х���ǲ����줿? */
	    switch(state){ // ���򤵤�Ƥ���ư��μ¹�
	    case FREEHAND://��ͳ����
	      freeHand(dis, win, gc, ev);
	      break;
	    default:
	      break;
	    }
	  }

	  /***************************** �ǥХå��� ********************************/
	  if ( ev.xany.window == debug_win ){ 
	    //printf("color=%d size=%d\n",current_color,current_pen);
	    for(i=0;i<MAX_COLOR;i++)
	      printf("%s %x\n",color_name[i],GetColor( dis, color_name[i]));
	    printf("%x\n",GetColor(dis,"yellow")>>8&0xff);
	    view();

	  }
	  /***************************** �ǥХå��� *********************************/
	  break;
	}
	if( ev.xbutton.button == 2 ){// �������
	  backHistory();
	  //XSetForeground(dis, gc, current_color);
	  //XSetLineAttributes(dis, gc, current_pen,
	  //		     LineSolid, CapRound, JoinMiter);
	  break;
	}
	if( ev.xbutton.button == 3 ){
	  exit_flag=1;
	  XFlush(dis);
	  break;
	}
	break;
      case Expose:

	//if(remap_flag==0){		/* ������ */
	  remap();		/* ������ */
	  //  remap_flag=1;
	  //}else{
	  //remap_flag--;
	  //}


	  /* �ڥ󥵥���������ɥ�������� */
	for ( i=0 ; i<MAX_PEN ; i++ ){
	  int pen_size = i*3+2;
	  XSetForeground(dis, gc, foreground);
	  XFillArc(dis, pen_win[i], gc,
		   button_size/2-pen_size/2, button_size/2-pen_size/2,
		   pen_size, pen_size, 0, 360*64);
	  
	}
	break;
	
      case EnterNotify:	/* ������ɥ��˥ݥ��󥿤����ä� */
	XSetWindowBorder(dis, ev.xany.window, GetColor( dis, "black"));
	break;
	
      case LeaveNotify:	/* ������ɥ�����ݥ��󥿤��Ф� */
	XSetWindowBorder(dis, ev.xany.window, GetColor(dis, "rgb:aa/aa/aa"));
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

/* ��ͳ���� */
void freeHand(Display *dis, Window win, GC gc, XEvent ev){
  int x,y;
  int i;

  x=ev.xbutton.x;
  y=ev.xbutton.y;

  XSetForeground( dis, gc, current_color  );    // �޷��ο�������
  
  XNextEvent( dis, &ev );
  while( !(ev.type == ButtonRelease || ev.xbutton.button == 1) ){
    XDrawLine( dis, win, gc, x, y, ev.xbutton.x, ev.xbutton.y );//�Ȥꤢ����DrawLine��ɽ����
    XFlush( dis );

    x=ev.xbutton.x;
    y=ev.xbutton.y;
    
    XNextEvent( dis, &ev );
    
  }

  addHistory();//������ɲ�

}

