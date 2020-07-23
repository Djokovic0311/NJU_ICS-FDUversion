#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"
#include<dirent.h> // for c language, file directory operations (use 'man opendir' for more information)
#include<unistd.h> // for c language, get work path (use 'man getcwd' for more information)
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
static int cmd_si( char *);
static int cmd_pwd(char *);
static int cmd_echo(char *); // define functions 
static int cmd_ls(char *);
static int cmd_info(char *);
static int cmd_x(char *);
static int cmd_w(char *);
static int cmd_d(char *);
void print_wp();
void cpu_exec(uint64_t);
void isa_reg_display();
/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);

static int cmd_p( char *args){
     char *arg = strtok(NULL,"@");
      bool success =true;
 int  result =expr( arg, &success);
if(success)
{printf("%u\n", result);
printf("%d\n", result);
printf("0x%x\n",result);
}
else
printf("Fail\n");

     return 0;
}
static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c},
  { "q", "Exit NEMU", cmd_q },
  /* TODO: Add more commands *
   * you should add more commands as described in our manual
   */
  { "x", "Scan memory from start, for total N bytes", cmd_x},
  { "info" ,"Print the information of registers or PC", cmd_info },
  { "ls", "list all of the files", cmd_ls},
  { "si", "Execute N steps , if N is not givenn, execute once", cmd_si},
  { "p", "Evaluate the expression and print the result", cmd_p },
  { "w", "Set a watchpoint", cmd_w},
  { "d", "Delete a watchpoint",cmd_d},
  { "echo", "Print the characters given by user", cmd_echo}, // add by wuran
  { "pwd", "Print current work path", cmd_pwd}, // add by wuran
};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0])) // number of commands

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("\033[1m\033[33m [%s]\033[0m - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("\033[1m\033[33m [%s]\033[0m - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

static int cmd_echo(char *args){
  // char * arg = strtok(args, " ");
  if(args != NULL)
    printf("%s\n", args);
  else printf("\n");
  return 0;
}

static int cmd_pwd(char * args){
  char buf[256];
  if(getcwd(buf, 256) != 0)
    printf("current work path: %s\n", buf);
  return 0;
}

static int cmd_si(char * args){
  int steps=0;
  char * final = strtok(NULL," ");
  if ( ! final ){
       cpu_exec(1);
      }
  else {
      sscanf(final, "%d", &steps);
      if ( steps<=0 ){
        cpu_exec(-1);
        }
      else{
        cpu_exec(steps);
        }
       }
 printf("execute for %s bytes finished\n",args);
 return 0;
}

static int cmd_ls(char * args){
  DIR *dirp,*dirq;
	struct dirent *direntp,*direntq;
printf("List the files in path:\n");
        if(args!=NULL){  
                dirp = opendir(args);
	if(dirp!=NULL){
                	while(1)
		{
			direntp = readdir(dirp);
			if(direntp == NULL)
				break;
			else if(direntp->d_name[0] != '.')
				printf("%s    ", direntp->d_name);
		}
		closedir(dirp);
                  printf("\n");

		return EXIT_SUCCESS;
             }
	
        }
       else{
        char buf[256];
        char *temp=getcwd(buf,256);
        dirq = opendir(temp);
        while(1)
                {
                        direntq = readdir(dirq);
                        if(direntq == NULL)
                                break;
                        else if(direntq->d_name[0] != '.')
                                printf("%s   ", direntq->d_name);
                }
                closedir(dirq);
                   printf("\n");

                return EXIT_SUCCESS;    
        }
	return EXIT_FAILURE;

}

static int cmd_info(char * args){
 char *arg = strtok(NULL, " ");
if( strcmp( arg, "r")==0)
     {
      isa_reg_display();}
else if(strcmp( arg,"pc")==0)
{printf("address:0x%x  ",cpu.pc);
 printf("value:0x%x\n",vaddr_read(cpu.pc,8));
}
else if(strcmp(arg, "w")==0){
print_wp();}
return 0;
}
static int cmd_x(char * args){
int i,j,k,time;
vaddr_t startpoint;
k=0;
char *arg=strtok(NULL," ");
char *argl=strtok(NULL," ");
bool success =true;
int result =expr( argl,&success);
startpoint=result;
sscanf(arg,"%d",&time);
printf("Address   Data\n");
if(success)
{
    for(i=0;i<=time;i++)
      {
          printf("%#x    ",startpoint);
          
          for(j=1;j<=4;j++){
          vaddr_t temp = vaddr_read(startpoint,j); 
          int move = (k<<3);
          temp = temp >> move;
          printf("0x%02x ",temp);
          k++;
          }
          printf("\n");
          startpoint+=4;
}
}
else
printf("Fail to eval\n");
return 0;
}

static int cmd_w(char * args){
 set_watchpoint(args);
return 0;
}

static int cmd_d(char * args){
      if(args != NULL){char * arg = strtok(args,"@");
      int NO;
      sscanf(arg,"%d",&NO);
      delete_watchpoint(NO);
      return 0;}
      else {
         delete_all_watchpoint();
         return 0;}
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }
 print_wp();
  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
