#include "inc.h"
/***************************************************************/
/************************ 各種構造体 ***************************/
/***************************************************************/

#define MENU_HEIGHT 10 /* メニューバーの高さ */
#define ITEM_HEIGHT 10 /* サブメニューのアイテムの高さ */
#define MAX_SUBMENU 10 /* 最大サブメニュー数 */

typedef struct mainmenu{
  Window win; /* アイテムウィンドウ */
  int num_item; /* サブメニュー内のアイテム数 */
  int count; /* 設置済みのアイテム数 */
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
}mainMenuItem;

typedef struct submenu{
  Window win; /* サブメニューウィンドウ */
  int num_item; /* サブメニュー内のアイテム数 */
  int count; /* 設置済みのアイテム数 */
  int width,height; /* サブメニューの幅、高さ */
  subMenuItem *item; /* アイテム、メニューのリスト */
}subMenu;

typedef struct submenuitem{
  Window win; /* アイテムウィンドウ */
  char *text;  /* 表示する文字列 */
  int width,height; /* アイテムの幅、高さ */
  subMenu *sub; /* サブメニュー */
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

////////////////////
// メニュー初期化
void initMenu(Display *disp, Window main_window, int main_w, int main_h,int num){

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

  mainMenu.num_item = num; /* サブメニュー内のアイテム数 */
  mainMenu.count = 0;
  mainMenu.width = main_width;
  mainMenu.height = main_height;
  mainMenu.next_x = 1; /* 次に追加するアイテムの位置 */
  mainMenu.item = (mainMenuItem *)malloc(sizeof(mainMenuItem)* num);
  
}

//////////////////////////////////////
// メインメニューのアイテムの設定
void setMainMenuItem(mainMenu *parent,char *text){
  mainMenuItem *new;

  if(parent->count>=parent->num_item){
    fprintf(stderr,"* too mach items (mainMenu)*\n");
    exit(0);
  }

  new=&(parent->item[parent->count]);
  new->text = text;  /* 表示する文字列 */
  new->x = parent->next_x;
  new->y = 1; 
  new->width = strlen(test)*9;
  new->height = parent->heiht-2;
  new->sub = NULL; 
  new->win = XCreateSimpleWindow(dis, parent->win, parent->next_x, 1,
				 new->width, new->height, 1,
				 GetColor( dis, "black"),
				 GetColor( dis, "gray")); /* アイテムウィンドウ */
  parent->count++;
  parent->next_x+=new->width+1;
}

///////////////////////
// サブメニューの追加
void setSubMenu(mainMenuItem *parent,int num){
  subMenu *menu;
  menu = (subMenu *)malloc(sizeof(subMenu));

  menu.win = XCreateSimpleWindow(dis, main_win, parent->x, mainManu.height,
				 100, (ITEM_HEIGHT+1)*num +1, 1,
				 GetColor( dis, "black"),
				 GetColor( dis, "gray"));
  XSelectInput(dis, menu.win,
	       EnterWindowMask |
	       LeaveWindowMask );

  menu.num_item = num;
  menu.count = 0;
  menu.width = 100;
  menu.height = (ITEM_HEIGHT+1)*num+1;
  menu.item = (subMenuItem *)malloc(sizeof(subMenuItem)*num);

  parent->sub=menu;
  
}

/////////////////////////////////
// サブメニューのアイテムの設定
void addSubMenuItem(subMenu *parent,char *text){
  subMenuItem *item;

  if(parent->count>=parent->num_item){
    fprintf(stderr,"* too mach items (subMenu)*\n");
    exit(0);
  }

  item=&(parent->item[parent->count]);
  item->text = text;
  item->x = 1;
  item->y = (ITEM_HEIGHT+1)*parent->count+1; 
  item->width = parent->width-2;
  item->height = ITEM_HEIGHT;
  item->sub = NULL; 
  item->win = XCreateSimpleWindow(dis, parent->win, item->x, item->y,
				 item->width, item->height, 1,
				 GetColor( dis, "black"),
				 GetColor( dis, "gray")); /* アイテムウィンドウ */
  parent->count++;
}
  
}

/***************************************************************/
/************************ 呼び出し関数 *********************/
/***************************************************************/

void makeMenu(){

  setMainMenuItem(&mainMenu,"")
