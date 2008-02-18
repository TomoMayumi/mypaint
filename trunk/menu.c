#include "inc.h"
/***************************************************************/
/************************ 各種構造体 ***************************/
/***************************************************************/

#define MENU_HEIGHT 10 /* メニューバーの高さ */
#define MAX_SUBMENU 10 /* 最大サブメニュー数 */

typedef struct mainmenu{
  Window win; /* アイテムウィンドウ */
  int num_item; /* サブメニュー内のアイテム数 */
  int width,height; /* アイテムの幅、高さ */
  int next_x; /* 次に追加するアイテムの位置 */
  mainMenuItem *item; /* アイテム、メニューのリスト */
}mainMenu;

typedef struct mainmenuitem{
  Window win; /* アイテムウィンドウ */
  char *text;  /* 表示する文字列 */
  int width,height; /* アイテムの幅、高さ */
  int x,y; /* アイテムのx座標、y座標 */
  subMenu *sub; /* サブメニュー */
  struct mainmenuitem *next; /* 続くアイテム */
}mainMenuItem;

typedef struct submenu{
  Window win; /* サブメニューウィンドウ */
  int num_item; /* サブメニュー内のアイテム数 */
  int width,height; /* サブメニューの幅、高さ */
  subMenuItem *item; /* アイテム、メニューのリスト */
}subMenu;

typedef struct submenuitem{
  Window win; /* アイテムウィンドウ */
  char *text;  /* 表示する文字列 */
  int width,height; /* アイテムの幅、高さ */
  subMenu *sub; /* サブメニュー */
  struct submenuitem *next; /* 続くアイテム */
}subMenuItem;


/***************************************************************/
/************************ 大域変数 *****************************/
/***************************************************************/


mainMenu mainMenu; /* メインメニューウィンドウ */


int main_width,main_height;  /* メインウィンドウの幅、高さ */
Display *dis;  /* Xとの接続 */
Window main_win; /* メインウィンドウ */

/***************************************************************/
/************************ ライブラリ的関数 *********************/
/***************************************************************/

void initMenu(Display *disp, Window main_window, int main_w, int main_h){

  main_width=main_w;
  main_height=main_h;
  dis=disp;
  main_win=main_window
  
  mainMenu.win = XCreateSimpleWindow(dis, main_win, 0, 0,
				     main_width, MENU_HEIGHT, 1,
				     GetColor( dis, "black"),
				     GetColor( dis, "gray"));
  XSelectInput(dis, mainMenu.win,
	       EnterWindowMask |
	       LeaveWindowMask );

  mainMenu.num_item = 0; /* サブメニュー内のアイテム数 */
  mainMenu.width = main_width;
  mainMenu.height = main_height; /* アイテムの幅、高さ */
  mainMenu.next_x = 1; /* 次に追加するアイテムの位置 */
  mainMenu.item = NULL; /* アイテム、メニューのリスト */
  
}

void addMainMenuItem(mainMenu *parent,char *text){
  mainMenuItem *new,*list;
  new=(mainMenuItem *)malloc(sizeof(mainMenuItem));
  new->text = text;  /* 表示する文字列 */
  new->x = parent->next_x;
  new->y = 1; /* サブメニューの幅、高さ */
  new->width = strlen(test)*9;
  new->height = parent->heiht-2; /* サブメニューの幅、高さ */
  new->sub = NULL; /* アイテム、メニューのリスト */
  new->next = NULL; /* 続くアイテム、メニュー */
  new->win = XCreateSimpleWindow(dis, parent->win, parent->next_x, 1,
				new->width, new->height, 1,
				GetColor( dis, "black"),
				GetColor( dis, "gray")); /* サブメニューウィンドウ */
  parent->num_item++;
  parent->next_x+=new->width+1;

  list=parent->next;
  if(list==NULL){
    parent->next=new;
  }else{
    while(list->next!=NULL){
      list=list->next;
    }
    list->next=new;
  }

}

void setSubMenu(mainMenuItem *parent){
  subMenu *menu;
  menu = (subMenu *)malloc(sizeof(subMenu));

  menu.win = XCreateSimpleWindow(dis, main_win, parent->x, mainManu.height,
				     1, 1, 1,
				     GetColor( dis, "black"),
				     GetColor( dis, "gray"));
  XSelectInput(dis, menu.win,
	       EnterWindowMask |
	       LeaveWindowMask );

  menu.num_item = 0; /* サブメニュー内のアイテム数 */
  menu.width = 1;
  menu.height = 1; /* アイテムの幅、高さ */
  menu.item = NULL; /* アイテム、メニューのリスト */
  
}

void addSubMenuItem(subMenu *parent,char *text){
  subMenuItem *item;
  item=(subMenuItem *)malloc(sizeof(subMenuItem));

  
}

/***************************************************************/
/************************ 呼び出し関数 *********************/
/***************************************************************/
