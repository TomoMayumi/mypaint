#include "inc.h"
/***************************************************************/
/************************ �Ƽﹽ¤�� ***************************/
/***************************************************************/

#define MENU_HEIGHT 10 /* ��˥塼�С��ι⤵ */
#define MAX_SUBMENU 10 /* ���祵�֥�˥塼�� */

typedef struct mainmenu{
  Window win; /* �����ƥ०����ɥ� */
  int num_item; /* ���֥�˥塼��Υ����ƥ�� */
  int width,height; /* �����ƥ�������⤵ */
  int next_x; /* �����ɲä��륢���ƥ�ΰ��� */
  mainMenuItem *item; /* �����ƥࡢ��˥塼�Υꥹ�� */
}mainMenu;

typedef struct mainmenuitem{
  Window win; /* �����ƥ०����ɥ� */
  char *text;  /* ɽ������ʸ���� */
  int width,height; /* �����ƥ�������⤵ */
  int x,y; /* �����ƥ��x��ɸ��y��ɸ */
  subMenu *sub; /* ���֥�˥塼 */
  struct mainmenuitem *next; /* ³�������ƥ� */
}mainMenuItem;

typedef struct submenu{
  Window win; /* ���֥�˥塼������ɥ� */
  int num_item; /* ���֥�˥塼��Υ����ƥ�� */
  int width,height; /* ���֥�˥塼�������⤵ */
  subMenuItem *item; /* �����ƥࡢ��˥塼�Υꥹ�� */
}subMenu;

typedef struct submenuitem{
  Window win; /* �����ƥ०����ɥ� */
  char *text;  /* ɽ������ʸ���� */
  int width,height; /* �����ƥ�������⤵ */
  subMenu *sub; /* ���֥�˥塼 */
  struct submenuitem *next; /* ³�������ƥ� */
}subMenuItem;


/***************************************************************/
/************************ ����ѿ� *****************************/
/***************************************************************/


mainMenu mainMenu; /* �ᥤ���˥塼������ɥ� */


int main_width,main_height;  /* �ᥤ�󥦥���ɥ��������⤵ */
Display *dis;  /* X�Ȥ���³ */
Window main_win; /* �ᥤ�󥦥���ɥ� */

/***************************************************************/
/************************ �饤�֥��Ū�ؿ� *********************/
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

  mainMenu.num_item = 0; /* ���֥�˥塼��Υ����ƥ�� */
  mainMenu.width = main_width;
  mainMenu.height = main_height; /* �����ƥ�������⤵ */
  mainMenu.next_x = 1; /* �����ɲä��륢���ƥ�ΰ��� */
  mainMenu.item = NULL; /* �����ƥࡢ��˥塼�Υꥹ�� */
  
}

void addMainMenuItem(mainMenu *parent,char *text){
  mainMenuItem *new,*list;
  new=(mainMenuItem *)malloc(sizeof(mainMenuItem));
  new->text = text;  /* ɽ������ʸ���� */
  new->x = parent->next_x;
  new->y = 1; /* ���֥�˥塼�������⤵ */
  new->width = strlen(test)*9;
  new->height = parent->heiht-2; /* ���֥�˥塼�������⤵ */
  new->sub = NULL; /* �����ƥࡢ��˥塼�Υꥹ�� */
  new->next = NULL; /* ³�������ƥࡢ��˥塼 */
  new->win = XCreateSimpleWindow(dis, parent->win, parent->next_x, 1,
				new->width, new->height, 1,
				GetColor( dis, "black"),
				GetColor( dis, "gray")); /* ���֥�˥塼������ɥ� */
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

  menu.num_item = 0; /* ���֥�˥塼��Υ����ƥ�� */
  menu.width = 1;
  menu.height = 1; /* �����ƥ�������⤵ */
  menu.item = NULL; /* �����ƥࡢ��˥塼�Υꥹ�� */
  
}

void addSubMenuItem(subMenu *parent,char *text){
  subMenuItem *item;
  item=(subMenuItem *)malloc(sizeof(subMenuItem));

  
}

/***************************************************************/
/************************ �ƤӽФ��ؿ� *********************/
/***************************************************************/
