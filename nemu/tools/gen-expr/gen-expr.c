#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536]="";
static const char * op[] = {"+","-","*","/","&&","||","==","!="};
static const char * num[] = {"0","1","2","3","4","5","6","7","8","9"};
// TODO: implement these functions: choose, gen_rand_op, gen_num, gen_rand_expr
static inline uint32_t choose(uint32_t n) {
  return rand()%n;
}

static inline void gen_rand_op(){
 int k = rand() % 8;
	//printf("%s",op[k]);
	strcat(buf,op[k]);
 return;
}

static inline void gen_num(){
 int byte = rand()%3+1;
   int p = 0, q = 0;
   int i = 0;
   for(i = 1; i <= byte; i++){
   	if(i==1){
   		p = rand()%9+1;
   		//printf("%s",num[p]);
   		strcat(buf,num[p]);
	   }
   	else {
   		q = rand()%10;
   		//printf("%s",num[q]);
   		strcat(buf,num[q]);
	   }	       
   }
  return;
}
static inline void gen_blank(){
	//printf(" ");
	strcat(buf," ");
}

static inline void gen_left(){
	//printf("(");
	strcat(buf,"(");
}

static inline void gen_right(){
	//printf(")");
	strcat(buf,")");
}

static inline void gen_no(){
	//printf("!");
	strcat(buf,"!");
}
static inline void gen_rand_expr() {
 // buf[0] = 50;
 // buf[1] = '\0';
switch(choose(7)){
  	case 0:
  		//j++;
  		gen_blank();
  		gen_rand_expr();
  		break;
  	case 1:
  		//j++;
  		gen_num();
  		break;
  	case 2:
  		//j++;
  		gen_rand_expr(); 
        gen_rand_op(); // 产生随机操作符
        gen_rand_expr();
        break;
    case 3:
    	//j++;
    	gen_left();
        gen_rand_expr();
        gen_right(); // "(" <expr> ")"也是表达式
        break;
    case 4:
    	//j++;
    	gen_no();
    	gen_rand_expr();
    	break;
    case 5:
         gen_rand_expr();
         gen_rand_op();
         gen_rand_expr();
         break;
    default:
    	gen_num();
    	break;
  }
	return;
}
// TODO: if necessary, try to re-implement main function for better generation of expression


static char code_buf[65536];
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0;  i < loop; i ++) {
    buf[0]='\0';
	  gen_rand_expr();
    if (strlen(buf)<=200 )
    sprintf(code_buf, code_format, buf);
    else continue;

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc -Werror /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}


