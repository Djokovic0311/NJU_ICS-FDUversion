#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  bool is_free;
  int NO;
  char msg[100];
   // value of watch
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  
  int new_value;
  int old_value;
  
} WP;

/* TODO: if necessary, try to implement these functions freely in watchpoint.c
 * you can add other functions by yourself if necessary
 */

 WP * new_wp( );//新建监视点
 int free_wp(int NO);//释放监视点
 void print_wp();//打印监视点信息
 bool delete_watchpoint(int NO);//删除指定序号的监视点
 void delete_all_watchpoint();//删除所有监视点
 void delete_all_watchpoint_rc(WP * p);//递归删除监视点
 int set_watchpoint(char *e);//设置监视点
 int  check_wp();//检查监视点是否发生变化
#endif
