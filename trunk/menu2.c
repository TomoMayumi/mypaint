#include "inc.h"
/***************************************************************/
/************************ �Ƽﹽ¤�� ***************************/
/***************************************************************/

#define MENU_HEIGHT 10 /* ��˥塼�С��ι⤵ */
#define ITEM_HEIGHT 10 /* ���֥�˥塼�Υ����ƥ�ι⤵ */
#define MAX_SUBMENU 10 /* ���祵�֥�˥塼�� */

typedef struct mainmenu{
  Window win; /* �����ƥ०����ɥ� */
  int num_item; /* ���֥�˥塼��Υ����ƥ�� */
  int count; /* ���ֺѤߤΥ����ƥ�� */
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
}mainMenuItem;

typedef struct submenu{
  Window win; /* ���֥�˥塼������ɥ� */
  int num_item; /* ���֥�˥塼��Υ����ƥ�� */
  int count; /* ���ֺѤߤΥ����ƥ�� */
  int width,height; /* ���֥�˥塼�������⤵ */
  subMenuItem *item; /* �����ƥࡢ��˥塼�Υꥹ�� */
}subMenu;

typedef struct submenuitem{
  Window win; /* �����ƥ०����ɥ� */
  char *text;  /* ɽ������ʸ���� */
  int width,height; /* �����ƥ�������⤵ */
  subMenu *sub; /* ���֥�˥塼 */
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

////////////////////
// ��˥塼�����
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

  mainMenu.num_item = num; /* ���֥�˥塼��Υ����ƥ�� */
  mainMenu.count = 0;
  mainMenu.width = main_width;
  mainMenu.height = main_height;
  mainMenu.next_x = 1; /* �����ɲä��륢���ƥ�ΰ��� */
  mainMenu.item = (mainMenuItem *)malloc(sizeof(mainMenuItem)* num);
  
}

//////////////////////////////////////
// �ᥤ���˥塼�Υ����ƥ������
void setMainMenuItem(mainMenu *parent,char *text){
  mainMenuItem *new;

  if(parent->count>=parent->num_item){
    fprintf(stderr,"* too mach items (mainMenu)*\n");
    exit(0);
  }

  new=&(parent->item[parent->count]);
  new->text = text;  /* ɽ������ʸ���� */
  new->x = parent->next_x;
  new->y = 1; 
  new->width = strlen(test)*9;
  new->height = parent->heiht-2;
  new->sub = NULL; 
  new->win = XCreateSimpleWindow(dis, parent->win, parent->next_x, 1,
				 new->width, new->height, 1,
				 GetColor( dis, "black"),
				 GetColor( dis, "gray")); /* �����ƥ०����ɥ� */
  parent->count++;
  parent->next_x+=new->width+1;
}

///////////////////////
// ���֥�˥塼���ɲ�
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
// ���֥�˥塼�Υ����ƥ������
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
				 GetColor( dis, "gray")); /* �����ƥ०����ɥ� */
  parent->count++;
}
  
}

/***************************************************************/
/************************ �ƤӽФ��ؿ� *********************/
/***************************************************************/

void makeMenu(){

  setMainMenuItem(&mainMenu,"")
