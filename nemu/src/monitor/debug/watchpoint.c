#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32
int init = 0;
static WP wp_pool[NR_WP] = {}; 

// TODO: try to re-organize, you can abort head and free_ pointer while just use static int index
static WP *head, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i+1;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;
  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP* new_wp(){
  if(init == 0) 
      init_wp_pool();
  WP *p = free_;
  if(p){
      free_= free_->next;
      p->next=head;
      head=p;
      init++;
      //head->msg = msg;
       
  return p;
} else { 
      assert(0); }
}

int free_wp(int NO){
   WP *p = head;
   WP *pre = p;
   if(head == NULL){
   //   printf("There is no watchpoint!\n");
      return 0;
   }
   else if(head->NO == NO){
            head = head->next;
            p->next = free_;
            free_ = p;
            printf("Watchpoint %d is already deleted!\n", NO);
            return 1;
          }
   else {
      while(p != NULL && p->NO != NO){
           pre = p;
           p = p->next;
          }
      if(p->NO == NO){
          pre->next = p->next;
          p->next = free_;
          free_ = p;
          printf("Watchpoint %d is already deleted!\n", NO);
          init --;
          return 1;
       }
   return 0;
}
}
void print_wp(){
  WP *p = head;
  if(head == NULL)
//     printf("There is no watchpoint!\n");
   ;
  else{
        while(p){
         printf("%d %s %x\n",p->NO, p->msg, p->old_value);
         p= p->next; 
        }
  }
  return;

}
bool delete_watchpoint(int NO){
   
   if(free_wp(NO)){
        return true;}
   else{
         printf("Failed\n");
   }     
   return false;
}

int set_watchpoint(char * e){
    WP * p;
    p = new_wp();
    printf("Set watchpoint %d\n", p->NO);
    strcpy(head->msg,e);
    printf("expr: %s\n", p->msg);
    bool success = true;
    p->old_value = expr(p->msg, &success);
    if(!success) {
          printf("Fail!\n");
          return 0;
    }
    else {
        printf("value = 0x%x\n" , p->old_value);
    }
    return 1;
}

int check_wp(){
    WP * p = head;
    bool success = true;
    if(p == NULL){
  //       printf("There is no watchpoint!\n");
         return 0;
    }
    else {
       while(p) {
           p->new_value = expr(p->msg , &success);
           if(!success)
              printf("Fail at %d watchpoint!\n",p->NO);
           else {
                  if(p->new_value != p->old_value){
                       printf("Watchpoint:%d  : %s\n", p->NO, p->msg);
                       printf("old value=%x\nnew value=%x\n", p->old_value,p->new_value);
                       p->old_value = p->new_value;
                       return 1;
                  }
                }
             }
       }
      return 0;
}

void delete_all_watchpoint_rc(WP * p){
 if(p == NULL) return;
	if(p->next != NULL) delete_all_watchpoint_rc(p->next);

	free_wp(p->NO);}

void delete_all_watchpoint(){
      delete_all_watchpoint_rc(head);
	head = NULL;
}
