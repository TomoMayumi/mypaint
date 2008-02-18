#include <X11/Xlib.h>
#include <X11/Xutil.h>

#define WIN_WIDTH  350
#define WIN_HEIGHT 200
#define TEXT_WIDTH  50
#define TEXT_HEIGHT 30
#define BUTTON_WIDTH 50
#define BUTTON_HEIGHT 20
#define BAR_WIDTH (10+256)
//#define BAR_WIDTH (20+10+256)
#define BAR_HEIGHT 30
#define COLOR_WIDTH 30
#define COLOR_HEIGHT 30

#define WHITE_COLOR 0xffffff
#define GRAY_COLOR 0xbbbbbb
#define RED_COLOR 0xff0000
#define GREEN_COLOR 0x00ff00
#define BLUE_COLOR 0x0000ff

Window cwin;
Window ok,cancel;
Window oldcolor_win;
Window newcolor_win;
Window bar_area[3];
Window text[3];
Window bar[3];
Window rgb[3];

GC black_gc;


void initColorSelect(Display *dis,Window main){
  int i;
  XWindowAttributes main_info;
  int main_width=600,main_height=600;
  unsigned long rgb_pixel[3]={RED_COLOR,GREEN_COLOR,BLUE_COLOR};

  black_gc = XCreateGC( dis, DefaultRootWindow(dis), 0, 0 );
  XSetForeground( dis, black_gc, 0  );    // 図形の色を設定

  XGetWindowAttributes(dis, main, &main_info);

  main_width = main_info.width;
  main_height = main_info.height;

  cwin = XCreateSimpleWindow( dis, main,//DefaultRootWindow(dis),
			      main_width/2-WIN_WIDTH/2,
			      main_height/2-WIN_HEIGHT/2,
			      WIN_WIDTH, WIN_HEIGHT,
			      2, 0, GRAY_COLOR );
  XSelectInput( dis, cwin, ExposureMask ); 

  ok = XCreateSimpleWindow( dis, cwin,
			    WIN_WIDTH-(BUTTON_WIDTH+20)*2,
			    WIN_HEIGHT-BUTTON_HEIGHT-20,
			    BUTTON_WIDTH, BUTTON_HEIGHT,
			    2, 0, GRAY_COLOR );
  XSelectInput(dis, ok, ButtonPressMask);
  
  cancel = XCreateSimpleWindow( dis, cwin,
				WIN_WIDTH-(BUTTON_WIDTH+20),
				WIN_HEIGHT-BUTTON_HEIGHT-20,
				BUTTON_WIDTH, BUTTON_HEIGHT,
				2, 0, GRAY_COLOR );
  XSelectInput(dis, cancel, ButtonPressMask);
  
  oldcolor_win = XCreateSimpleWindow( dis, cwin,
				      WIN_WIDTH/2-30-COLOR_WIDTH, 30,
				      COLOR_WIDTH, COLOR_HEIGHT,
				      0, 0, GRAY_COLOR );
  newcolor_win = XCreateSimpleWindow( dis, cwin,
				      WIN_WIDTH/2+30, 30,
				      COLOR_WIDTH, COLOR_HEIGHT,
				      0, 0, GRAY_COLOR );
  
  for ( i=0 ; i<3 ; i++ ){
    bar_area[i] = XCreateSimpleWindow(dis, cwin,
				      WIN_WIDTH/2-(BAR_WIDTH+4+TEXT_WIDTH+4)/2,
				      WIN_HEIGHT/2-(BAR_HEIGHT+4)*3/2+(BAR_HEIGHT+4)*i,
				      BAR_WIDTH, BAR_HEIGHT, 2,
				      rgb_pixel[i], GRAY_COLOR);
    bar[i] = XCreateSimpleWindow(dis, bar_area[i],
				 0, 0,
				 10-2, BAR_HEIGHT-2, 1,
				 0, 0x999999);
    text[i] = XCreateSimpleWindow(dis, cwin,
				  WIN_WIDTH/2-(BAR_WIDTH+4+TEXT_WIDTH+4)/2+BAR_WIDTH+4,
				  WIN_HEIGHT/2-(BAR_HEIGHT+4)*3/2+(BAR_HEIGHT+4)*i,
				  TEXT_WIDTH, TEXT_HEIGHT, 2,
				  rgb_pixel[i], WHITE_COLOR);
    /*rgb[i] = XCreateSimpleWindow(dis, bar_area[i],
				 0, 0,
				 20, BAR_HEIGHT, 0,
				 0, rgb_pixel[i]);
    slider[i] = XCreateSimpleWindow(dis, bar_area[i],
				    25, BAR_HEIGHT/2-4,
				    BAR_WIDTH-30, 8, 0,
				    0, GRAY_COLOR);
    bar[i] = XCreateSimpleWindow(dis, bar_area[i],
				 20, 0,
				 10-2, BAR_HEIGHT-2, 1,
				 0, 0x999999);*/
    XSelectInput( dis, bar_area[i],
		  Button1MotionMask
		  |ButtonPressMask
		  |ButtonReleaseMask ); 
  }
  
}

void remap(Display *dis,int rgb_color[]){
  int i;
  char buf[32];

  sprintf(buf,"[ * Color Select * ]");
  XDrawString(dis,cwin,black_gc,WIN_WIDTH/2-60,15,buf,strlen(buf));
  sprintf(buf,"Ok");
  XDrawString(dis,ok,black_gc,20,15,buf,strlen(buf));
  sprintf(buf,"Cancel");
  XDrawString(dis,cancel,black_gc,8,15,buf,strlen(buf));
  for(i=0;i<3;i++){
    sprintf(buf,"%3d",rgb_color[i]);
    XClearWindow( dis, text[i]);
    XDrawString(dis,text[i],black_gc,15,20,buf,strlen(buf));
  }
}

unsigned long callColorSelect( Display *dis, unsigned long oldcolor){
  int i;
  int rgb_color[3]={oldcolor>>16&0xff,
		    oldcolor>>8&0xff,
		    oldcolor&0xff};
  unsigned long newcolor=oldcolor;
  XEvent ev;


  XSetWindowBackground( dis, oldcolor_win, oldcolor);
  XSetWindowBackground( dis, newcolor_win, newcolor);
  //XClearWindow( dis, oldcolor_win);
  //XClearWindow( dis, newcolor_win);

  XMapWindow( dis, cwin);
  XMapSubwindows( dis, cwin);
  for(i=0;i<3;i++){
    XMapSubwindows( dis, bar_area[i]);
    XMoveWindow( dis, bar[i], rgb_color[i], 0 );
  }
  remap(dis,rgb_color);

  while(1){
    XNextEvent( dis, &ev );
    switch(ev.type){
    case ButtonPress:
      
      if( ev.xbutton.button == 1 ){//左クリックの時
	
	if ( ev.xany.window == ok ){
	  XUnmapWindow(dis,cwin);
	  return(newcolor);
	  break;
	}
	if ( ev.xany.window == cancel ){
	  XUnmapWindow(dis,cwin);
	  return(oldcolor);
	  break;
	}
	for(i=0;i<3;i++){
	  if ( ev.xany.window == bar_area[i] ){
	    int x;
	    char buf[4];
	    do{
	      if(ev.type == MotionNotify || ev.type == ButtonPress){
		x=ev.xbutton.x-5;
		
		if(x<0)x=0;
		if(x>255)x=255;

		rgb_color[i]=x;
		newcolor=(rgb_color[0]<<16)+(rgb_color[1]<<8)+rgb_color[2];

		XSetWindowBackground( dis, newcolor_win, newcolor);
		XClearWindow( dis, newcolor_win);
		
		sprintf(buf,"%3d",rgb_color[i]);
		XClearWindow( dis, text[i]);
		XDrawString(dis,text[i],black_gc,15,20,buf,strlen(buf));

		XMoveWindow( dis, bar[i], x, 0 );
		
		XNextEvent( dis, &ev );
	      }
	    }while( !(ev.type == ButtonRelease && ev.xbutton.button == 1) );
	      
	    break;
	  }
	}
	
      }
      break;
    case Expose:
      if ( ev.xany.window == cwin ){
	remap(dis,rgb_color);		/* 再描画 */
      }
      break;
      
    case MotionNotify:	/* ボタンを押しながらマウスが動いた */
      break;
      
    default:
      break;
      
    } 
  } 

}
