#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>
#define BAD_EXP -1111

enum {
  TK_NOTYPE = 256, TK_EQ, NUM, ADD, MINUS, MULTIPLY, DIVIDE, LBRACKET, RBRACKET, REG, HEX,
    AND, OR, NEQ,TK_POINT = 2,TK_NEG = 3 

  /* TODO: Add more token types */

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", ADD},         // plus
  {"==", TK_EQ},        // equal
  {"0[xX][0-9a-fA-F]+", HEX},   // hex number
  {"[0-9]+", NUM},      // numbers
  {"\\-", MINUS},       // minus
  {"\\*", MULTIPLY},    // multiply
  {"\\/", DIVIDE},      // divide
  {"\\(", LBRACKET},    // left bracket
  {"\\)", RBRACKET},    // right bracket
  {"\\$e[abc]x", REG},  // register
  {"\\$e[bs]p", REG},
  {"\\$e[sd]i", REG},
  {"\\$pc", REG},
  {"&&", AND},          // and
  {"\\|\\|", OR},       // or
  {"!=", NEQ},           // not equal
  {"!",'!'}
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX]={};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[1000] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  //printf("expression = %s\n", e);
  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        tokens[nr_token].type = rules[i].token_type;
        switch (rules[i].token_type) {
          case TK_NOTYPE:
            break;
          case NUM:
          case REG:
          case HEX: 
           for (i = 0; i < substr_len; i++)
              tokens[nr_token].str[i] = substr_start[i];
            tokens[nr_token].str[i] = '\0';
            nr_token++;
            break;
          case ADD:
          case MINUS:
          case MULTIPLY:
          case DIVIDE:
          case LBRACKET:
          case RBRACKET:
          case '!':
            tokens[nr_token].str[0] = substr_start[0];
            tokens[nr_token++].str[1] = '\0';
            break;
          case AND:
          case OR:
          case TK_EQ:
          case NEQ:
            tokens[nr_token].str[0] = substr_start[0];
            tokens[nr_token].str[1] = substr_start[1];
            tokens[nr_token++].str[2] = '\0';
            break;
          default: TODO();
        }

//        printf("token = %s\n", tokens[nr_token-1].str);

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

uint32_t expr(char *e, bool *success);
uint32_t eval(int p, int q);
uint32_t getnum(char str);
bool check_parentheses(int p, int q);
int find_dominant_operator(int p, int q);



uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  int i;
  for( i=nr_token-1; i>=0; i--){
//  printf("%d ",tokens[i].type);
      if(tokens[i].type == MULTIPLY  && (i==0 || tokens[i-1].type== ADD|| tokens[i-1].type==MINUS || tokens[i-1].type== MULTIPLY || tokens[i-1].type==DIVIDE)){
          tokens[i].type = TK_POINT;
        }
      if(tokens[i].type == MINUS && (i==0 || tokens[i-1].type== ADD|| tokens[i-1].type== MINUS || tokens[i-1].type== MULTIPLY || tokens[i-1].type== DIVIDE)){
         // printf("index:%d\n",i);
           tokens[i].type = TK_NEG;
        }

    }
  // for( i=0; i< nr_token; i++){ printf("%d\n",tokens[i].type);}
//  printf("%s", e);
    return eval(0, nr_token - 1);

  //TODO();
  return 0;
}

uint32_t eval(int p, int q) {
//  printf("in the eval p = %d, q = %d\n", p, q);
  if (p > q){
    return false;
  }
  else if (p == q){
    if (tokens[p].type == NUM) {
      return atoi(tokens[p].str);
    }
    else if (tokens[p].type == REG) {
      if (strcmp(tokens[p].str, "$eax") == 0){  
        //printf("eax = %u\n", cpu.eax);
        return cpu.eax;}
      else if (strcmp(tokens[p].str, "$ebx") == 0)  return cpu.ebx;
      else if (strcmp(tokens[p].str, "$ecx") == 0)  return cpu.ecx;
      else if (strcmp(tokens[p].str, "$edx") == 0)  return cpu.edx;
      else if (strcmp(tokens[p].str, "$ebp") == 0)  return cpu.ebp;
      else if (strcmp(tokens[p].str, "$esp") == 0)  return cpu.esp;
      else if (strcmp(tokens[p].str, "$esi") == 0)  return cpu.esi;
      else if (strcmp(tokens[p].str, "$edi") == 0)  return cpu.edi;
      else if (strcmp(tokens[p].str, "$pc") == 0)  return cpu.pc;
    }
    else if (tokens[p].type == HEX) {
      int cnt, i, len, sum = 0;
      len = strlen(tokens[p].str);
      cnt = 1;

      for (i = len-1; i >= 0; i--) {
        sum = sum + cnt * getnum(tokens[p].str[i]);
        cnt *= 16;
      }
      return sum;
    }
  }
  else if (check_parentheses(p, q)){
    return eval(p + 1, q - 1);
  }
  else {
   int op = find_dominant_operator(p, q);
   // printf("op = %d\n", op);
   // uint32_t val1 = eval(p, op - 1);
   // uint32_t val2 = eval(op + 1, q);
   // printf("op = %d val1 = %u val2 = %u\n", op, val1, val2);    
  /*  if(op == p && tokens[p].type == TK_POINT)
        return vaddr_read(eval(p+1,q), 4);
    else if(op == p && tokens[p].type =='-')
        return -eval(p+1,q);
    else if(op == p && tokens[p].type =='!')
        return !eval(p+1,q);
    int  val1 = eval(p, op - 1);
    int  val2 = eval(op + 1, q);*/
 int val1 = 0, val2 = 0;   
 if(tokens[op].type!='!'&&tokens[op].type!=TK_NEG&& tokens[op].type!= TK_POINT) 
    val1 = eval(p, op-1);
    val2 = eval(op+1, q);
           
//    printf("op = %d val1 = %d val2 = %d\n", op, val1, val2);
    switch (tokens[op].type) {
      case ADD:
        return val1 + val2;
      case MINUS:
        return val1 - val2;
      case MULTIPLY:
        return val1 * val2;
      case DIVIDE:
        return val1 / val2;
      case AND:
        return val1 && val2;
      case OR:
        return val1 || val2;
      case TK_EQ:
        return val1 == val2;
      case NEQ:
        return val1 != val2;
      case TK_NEG:
        return -val2;break;
      case '!':
        return !val2;
      case TK_POINT:
        return vaddr_read(eval(p+1,q), 4);
     
      default:

        assert(0);
    }
  }
  return 1;

}

bool check_parentheses(int p, int q) {
  int i, bra = 0;

  for (i = p; i <= q; i++) {
    if (tokens[i].type == LBRACKET) {
      bra++;
    }
    if (tokens[i].type == RBRACKET) {
      bra--;
    }
    if(bra == 0 && i < q) {
      return false;
    }
  }

  return true;

}

int find_dominant_operator(int p, int q) {
  int i = 0, match = 0;//op=flagpos=op
  int op = 0,  pos = -1;
    for (i=p; i<q; i++){ 
     if (tokens[i].type == LBRACKET)       match++;
     if (tokens[i].type == RBRACKET)       match--;        
     if(match == 0){
     if(op <= 5 && (tokens[i].type ==OR)){
              pos = i;
              op = 5;
     }
     if(op <= 4 && (tokens[i].type == AND)){
              pos = i;
              op = 4;
     }
     if(op <= 3 && (tokens[i].type ==TK_EQ||tokens[i].type == NEQ)){
              pos = i;
              op = 3;
     }
     if(op <= 2 && (tokens[i].type == ADD || tokens[i].type == MINUS)){
              pos = i;
              op = 2;
     }
     if(op <= 1 && (tokens[i].type == MULTIPLY || tokens[i].type == DIVIDE)){
              pos = i;
              op = 1;
     }
     if(op <= 0 &&( tokens[i].type== TK_NEG) ) ;

  }
if(pos == -1)
     pos = p;
}
 // printf("pos = %d\n",   pos);
  return pos;
}



uint32_t getnum(char str)
{
  if (str >= '0' && str <= '9') 
    return str - '0';
  else if (str >= 'a' && str <= 'f') 
    return str - 'a' + 10;
  else if (str >= 'A' && str <= 'F') 
    return str - 'A' + 10;
  return 0;
}

