#include<X11/Xlib.h>
#include<X11/Xutil.h>


//����᥽�å�
void paint(Display*d,Window w,int screen,GC gc){
	Colormap    colormap; 
	XColor      near_color, true_color;
	//�����ѹ�
	colormap = DefaultColormap( d, screen ); 
	XAllocNamedColor( d, colormap, "red", &near_color, &true_color ); 
	XSetForeground( d, gc, near_color.pixel );
	//�ɤ�Ĥ֤����ͳѤ�����
	XFillRectangle(d,w,gc,10,10,180,180);
	XFlush( d );
}

main(){
	Display	*d;
	Window	w;
	int screen;
	XSetWindowAttributes attr;
	GC gc;
	
	d = XOpenDisplay( NULL );//X�����С�����³����
	screen=DefaultScreen(d);
	//������ɥ��κ���
	w = XCreateSimpleWindow( d, RootWindow(d,screen),
						0, 0, 200, 200,//������ɥ��Υ�����
						1,BlackPixel(d,screen),//�Ȥ�����
						WhitePixel(d,screen));//�طʿ��ο��ֹ�

	//������ɥ������Ƥ򵭲�
	attr.backing_store = WhenMapped;
	XChangeWindowAttributes( d, w, CWBackingStore, &attr); 

	//������ɥ�̾������
	XStoreName(d, w, "test Window");
	XSetIconName(d, w, "test Window");
	
	//����ե��å�����ƥ����Ȥ����
	gc = XCreateGC( d, RootWindow( d,screen ), 0, 0); 


	XEvent event;
	//���٥�ȥޥ�������Ͽ
	XSelectInput( d, w,ButtonPressMask | ExposureMask); 

	//������ɥ���°�����ѹ�
	XSetWindowAttributes wa;
	wa.override_redirect=True;
	XChangeWindowAttributes (d, w, CWOverrideRedirect, &wa);

	//�ޥåפ���ɽ��
	XMapWindow( d, w );
	XFlush( d );

	while(1){
		XNextEvent( d, &event );
		switch(event.type){
			case ButtonPress://���̾�ǥޥ����Υܥ��󤬲����줿��
				XDestroyWindow( d, w);//��λ����
				XCloseDisplay( d );
				exit(0);
			case Expose://�������׵�
				paint(d,w,screen,gc);//������
				break;
			default:
				break;
		}
	}
}




