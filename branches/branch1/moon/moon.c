/*
	 月齢表示 24 時間時計 mclock  V0.12
	                                 1998 Oct. Y.Senta
*/																	 

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/shape.h>

#define WIN_SIZE  120           /* デフォルトの描画サイズ */

#define MOONOFFSET  610967      /* 1970 年 1 月 1 日の月齢 ( 単位 秒 ) */
                                /* けっこういいかげんに決定した        */
#define INT_MONTHSEC 2551443
#define REA_MONTHSEC 2551442.89

#define COLORNAMESIZE 20
#define PI M_PI

long TimeZone =9;           /* GMT からのオフセット */

Display *display;
int screen;
Window root,window;
unsigned long border_color,fgbg[2];
unsigned width,height,border_width = 0;
unsigned long event_mask;
XColor pixels[12],C0;
XWindowAttributes wind;
GC gc, shapegc;
Pixmap shapemask;
Colormap cmap;
Atom WM_PROTOCOLS,WM_DELETE_WINDOW;
Visual *visual;
XPoint tmp_points[5];
double points[5][2];
Pixmap pixmap;

int flags={DoRed | DoBlue | DoGreen};

char colorname[5][COLORNAMESIZE]=
  { "black", "white", "gray", "red", "yellow" };

/*
  Xサーバへの接続
*/

int connect_X_server(const char *server_name) {
  int i;
  
  if((display = XOpenDisplay(server_name)) == NULL) {
    fprintf(stderr,"X server \"%s\" connection failed.\n",
            server_name);
    return -1;
  }
  
  /* 各種パラメータの収得 */
  screen = DefaultScreen(display);
  root = RootWindow(display,screen);
  XGetWindowAttributes(display, root, &wind);
  
  visual = XDefaultVisual(display, screen);
  XSync(display,1);
  
  cmap=DefaultColormap(display, screen);
  
  for (i=0;i<5;i++)
    XAllocNamedColor (display,cmap,colorname[i], &pixels[i],&C0);
  
  XAllocColorCells(display,cmap,False,NULL,0,fgbg,2);
  
  fgbg[0] = BlackPixel(display,screen);
  fgbg[1] = WhitePixel(display,screen);
  
  WM_PROTOCOLS = XInternAtom(display,"WM_PROTOCOLS",False);
  WM_DELETE_WINDOW = XInternAtom(display,"WM_DELETE_WINDOW",False);
  
  return ConnectionNumber(display);
}

/*
  ウィンドウの生成
*/

Window create_window( int x, int y ){
  window = XCreateSimpleWindow(display,root,x,y,
			       width,height,border_width,fgbg[0],fgbg[1]);
  if(window)
    XSetWMProtocols(display,window,&WM_DELETE_WINDOW,1);
  
  shapemask = XCreatePixmap (display, window, width , height , 1);
  shapegc = XCreateGC (display, shapemask,0,0);
  XFillRectangle(display,shapemask,shapegc,0,0,width,height);
  XSetForeground (display, shapegc, 1);
  XFillArc(display, shapemask, shapegc, 0,0, width, height, 0, 23040);
  
  XShapeCombineMask (display, window, ShapeBounding,
		     0, 0, shapemask, ShapeSet);
  XShapeCombineMask (display, window, ShapeClip,
		     0, 0, shapemask, ShapeSet);
  
  XFreeGC (display, shapegc);
  
  pixmap=XCreatePixmap(display,window,width,height,DefaultDepth(display,0));
  
  return window;
}

/* GCの生成 */

GC create_gc(Drawable d)
{
  gc = XCreateGC(display,d,0,0);
  
  XSetBackground(display,gc,fgbg[1]);
  XSetForeground(display,gc,fgbg[0]);
  
  return gc;
}

/*  描画関数  */

void expose1(void)
{
  unsigned long moon,t;
  Window dummy_window;
  unsigned int dummy_x,dummy_y,dummy_w,dummy_r;
  
  time (&t);
  
  /* 現在の月齢  ( 単位 秒 ) */
  
  moon = ( t - MOONOFFSET ) % INT_MONTHSEC;
  
  t+=TimeZone*3600L;
  t%=86400L;
  
  XGetGeometry(display,window,&dummy_window,&dummy_x,&dummy_y,
	       &width,&height,&dummy_w,&dummy_r);
  
  /* 縁の黒 */
  
  XSetForeground(display,gc,pixels[0].pixel);
  XFillArc(display,pixmap,gc,0,0,width,height,0,23040);
  
  /* 文字盤の灰色 */
  
  XSetForeground(display,gc,pixels[2].pixel);
  XFillArc(display,pixmap,gc,width/40,height/40,
	   width-width/20,height-height/20,0,23040);
  
  /* 12 時の黒 */
  
  XSetForeground(display,gc,pixels[0].pixel);
  XFillArc(display,pixmap,gc,
	   width/2-width/30,
	   height/30,
	   width/15,height/15,0,23040);
  
  /* 現在の月 ( 黄色 ) */
  
  XSetForeground(display,gc,pixels[4].pixel);
  XFillArc(display,pixmap,gc,
	   width/2-width*9/24*sin((double)moon/REA_MONTHSEC*2.*PI)-width/12,
	   height/2+height*9/24*cos((double)moon/REA_MONTHSEC*2.*PI)-height/12,
	   width/6,height/6,0,-11520);
  
  /* 24 時間針 ( 赤 ) */
  
  rotatepolygon((double)t/3600./24.*2.*PI,0.,0.5);
  XSetForeground(display,gc,pixels[3].pixel);
  XFillPolygon (display, pixmap, gc, tmp_points, 5, Convex, CoordModeOrigin);
  XDrawLine (display, pixmap, gc,
	     width/2+width/4.*cos((double)t/3600./24.*2.*PI),
	     height/2+height/4.*sin((double)t/3600./24.*2.*PI),
	     width/2-width/4.*cos((double)t/3600./24.*2.*PI),
	     height/2-height/4.*sin((double)t/3600./24.*2.*PI));
  
  /* 短針 ( 白 ) */
  
  rotatepolygon((double)t/3600./12.*2.*PI,(double)width/8.,1.);
  XSetForeground(display,gc,pixels[1].pixel);
  XFillPolygon (display, pixmap, gc, tmp_points, 5, Convex, CoordModeOrigin);
  
  /* 長針 ( 白) */
  
  rotatepolygon((double)(t%3600)/3600.*2.*PI,0.,1.);
  XFillPolygon (display, pixmap, gc, tmp_points, 5, Convex, CoordModeOrigin);
  
  /* こぴぃ    */
  
  XCopyArea (display,pixmap,window,gc,0,0,width,height,0,0);
}

void setdisp(int dummy){
  create_gc(window);
  XSetWindowBackgroundPixmap(display,window,None);
  
  expose1();
  XFlush(display);
  signal(SIGALRM,setdisp);
  alarm((int)(3600./2./PI/(-(double)points[1][1]))+1);
}

int rotatepolygon(double angle, double offset_y,double slimerate){
  int i;
  
  for (i=0;i<5;i++){
    if (i>=3)
      offset_y=0;
    tmp_points[i].x =   points[i][0]*slimerate*cos(angle)
      - (points[i][1]+offset_y)*sin(angle)
      + (double)width/2.;
    tmp_points[i].y =   points[i][0]*slimerate*sin(angle)
      + (points[i][1]+offset_y)*cos(angle)
      + (double)height/2.;
  }
  
  return 0;
}

void err(int number){
  char *cmt[]={
    "Usage: mclock [timezone hour] [hand color] [hour color] [bg color] [xy nxm] [size n]",
    "I can't get number.",
    "Unknown option."
  };
  fprintf (stderr,"%s\n",cmt[number]);
  exit(1);
}

int main(int argc,char *argv[])
{
  int i,flg;
  unsigned xx,yy,ww;
  XSizeHints *sz;
  const char *server_name = NULL;
  
  ww=WIN_SIZE;
  flg=0;
  xx=yy=0;
  
  if (argc%2 == 0) err(0);
  for (i=1;i<argc-1;i++){
    if (!strncmp(argv[i],"timezone",8)){
      if ((sscanf(argv[++i],"%ld",&TimeZone))!=1 )
	err(1);
      continue;
    }
    if (!strncmp(argv[i],"hand",4)){
      strncpy(colorname[1],argv[++i],COLORNAMESIZE-1);
      continue;
    }
    if (!strncmp(argv[i],"bg",2)){
      strncpy(colorname[2],argv[++i],COLORNAMESIZE-1);
      continue;
    }
    if (!strncmp(argv[i],"hour",4)){
      strncpy(colorname[3],argv[++i],COLORNAMESIZE-1);
      continue;
    }
    if (!strncmp(argv[i],"xy",2)){
      if ((sscanf(argv[++i],"%dx%d",&xx,&yy))!=2)
	err(1);
      flg=1;
      continue;
    }
    if (!strncmp(argv[i],"size",4)){
      if ((sscanf(argv[++i],"%d",&ww))!=1)
	err(1);
      continue;
    }
    err(2);
  }
  
  width=height=ww;
  
  if(connect_X_server(server_name) < 0)
    return 1;
  
  if ((sz = XAllocSizeHints()) ==NULL)
    return 1;
  
  sz->min_width=width;
  sz->max_width=width;
  sz->min_height=height;
  sz->max_height=height;
  sz->flags|=PMinSize|PMaxSize;
  
  if (flg)
    sz->flags|=PPosition|USPosition;
  
  if(!create_window(xx,yy))
    return 1;
  
  XSetWMNormalHints(display,window,sz);
  
  if(!create_gc(window))
    return 1;
  
  /* 矢印の形 */
  
  points[0][0]= (double)width/36.; points[0][1]=-(double)height*3./8.;
  points[1][0]= 0.;                points[1][1]=-(double)height*5./12.;
  points[2][0]=-(double)width/36.; points[2][1]=-(double)height*3./8.;
  points[3][0]=-(double)width/36.; points[3][1]=(double)width/36.;
  points[4][0]= (double)width/36.; points[4][1]=(double)width/36.;
  
  event_mask = ExposureMask;
  XSelectInput(display,window,event_mask);
  
  XSetIconName(display, window, "mclock");
  XStoreName(display,window,"mclock");
  
  XMapWindow(display,window);
  
  setdisp(0);
  for(;;) {
    XEvent event;
    
    XNextEvent(display,&event);
    
    if(event.type == Expose)
      expose1();
    else if(event.type == ClientMessage) {
      if(event.xclient.message_type == WM_PROTOCOLS
	 && event.xclient.data.l[0] == WM_DELETE_WINDOW)
	break;
    }
  }
  
  XCloseDisplay(display);
  
  return 0;
}

