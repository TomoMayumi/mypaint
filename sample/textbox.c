#include<X11/Xlib.h>
#include<X11/Xutil.h>

void print(Display *d,Window w,GC gc,char*str){
	XClearWindow(d,w);//������ɥ��Υ��ꥢ
	int width=2;
	int height=10;
	XDrawString( d, w, gc, width, height, str, strlen(str));
	XFlush( d );
}

void addStr(char*buf,char*str){
	if(str[0]==8){//BSʸ����Ƚ��
		if(strlen(buf)) buf[strlen(buf)-1]='\0';//ʸ�����Ĺ����0�ʾ�ʤ��ʸ�����
	}else{
		//ʸ�����Ϣ��
		char strpos[128];
		strcpy(strpos,buf);
		sprintf(buf,"%s%s",strpos,str);
	}
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

	//�ƥ����ȥܥå�������----------------------------------------------------------------------
	//������ɥ��򥦥���ɥ�����˺������ƥƥ����ȥܥå����ˤ���
	char text1[128]="";//�б�����ʸ���ΥХåե�
	Window edit1=XCreateSimpleWindow( d, w,
							10, 10, 80, 15,//������ɥ��Υ�����
							1,BlackPixel(d,screen),//�Ȥ�����
							WhitePixel(d,screen));//�طʿ��ο��ֹ�
	//�ޥåפ���ɽ��
	XMapWindow( d,edit1 );
	//���٥�ȥޥ�������Ͽ
	XSelectInput( d, edit1,KeyPressMask | ButtonPressMask); 

	//----------------------------------------------------------------------

	//�ƥ����ȥܥå���2�����----------------------------------------------------------------------
	//������ɥ��򥦥���ɥ�����˺������ƥƥ����ȥܥå����ˤ���
	char text2[128]="";//�б�����ʸ���ΥХåե�
	Window edit2=XCreateSimpleWindow( d, w,
							10, 50, 80, 15,//������ɥ��Υ�����
							1,BlackPixel(d,screen),//�Ȥ�����
							WhitePixel(d,screen));//�طʿ��ο��ֹ�
	//�ޥåפ���ɽ��
	XMapWindow( d,edit2 );
	//���٥�ȥޥ�������Ͽ
	XSelectInput( d, edit2,KeyPressMask | ButtonPressMask); 
	//----------------------------------------------------------------------

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
	XSelectInput( d, w,ExposureMask | KeyPressMask | ButtonPressMask); 

	//�ޥåפ���ɽ��
	XMapWindow( d, w );
	XFlush( d );
	KeySym key;
	char strbuf[10];

	//����å����줿Window����¸���Ƥ����ѿ�
	//�����ܡ������ϻ�������Window���б������Хåե���ʸ�����ɲä����
	Window wpos;

	while(1){
		XNextEvent( d, &event );
		switch(event.type){
			case Expose:
				//���ƤΥƥ����ȥܥå����κ�����
				print(d,edit1,gc,text1);
				print(d,edit2,gc,text2);
				break;
			case ButtonPress://���̾�ǥޥ����Υܥ��󤬲����줿����Window�����
				wpos=event.xexpose.window;
				break;
			case KeyPress:
				XLookupString((XKeyEvent *)&event, strbuf, sizeof(strbuf), &key, NULL);
				if(wpos==edit1){//���ݤ��줿Window���б�����ʸ���Хåե���ʸ�����ɲ�
					addStr(text1,strbuf);
				}else if(wpos==edit2){
					addStr(text2,strbuf);
				}
				//���ƤΥƥ����ȥܥå����κ�����
				print(d,edit1,gc,text1);
				print(d,edit2,gc,text2);
				break;
			default:
				break;
		}
	}

}
