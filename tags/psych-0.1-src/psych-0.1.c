#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
typedef unsigned char cell;

struct {
 _Bool nobuff : 1; /*_Bool comments : 1;*/ _Bool digits : 1; _Bool eval : 1;
} flags;

struct evalStr {char* s; struct evalStr* next; };
struct evalStr* evalLines;

int main(int argc, char** argv) {
 int buffsize = 30000, ch;
 while ((ch = getopt(argc, argv, "hb:le:vd")) != -1) {
  switch (ch) {
   case 'h':
    printf("Brainfuck commands:\n >  Increment pointer\n <  Decrement pointer\n +  Increment value at pointer\n -  Decrement value at pointer\n .  Output character value at pointer\n ,  Read character and store in value at pointer\n [  Skip to corresponding ] if value at pointer is 0\n ]  Skip back to corresponding [ unless value at pointer is zero\n");
     exit(0);
   case 'b': buffsize = (int) strtol(optarg, NULL, 10); break;
   case 'l': flags.nobuff = 1; break;
   /*case 'c': flags.comments = 1; break;*/
    /* Comment support is enabled by default until further notice */
   case 'd': flags.digits = 1; break;
   case 'e':
    {struct evalStr* estr = malloc(sizeof(struct evalStr));
    if (!estr) {fprintf(stderr, "Error: not enough memory available.\n"); exit(1); }
    estr->s = optarg; estr->next = evalLines;
    evalLines = estr;
    flags.eval = 1;
    break; }
   case 'v': /* echo version & configuration information; IMPLEMENT */
   case '?': default: fprintf(stderr, "Unrecognized option \"-%c\"\n", ch);
  }
 }
 FILE* bf;
 if (flags.eval) {
  bf = tmpfile();
  if (bf == NULL) {
   fprintf(stderr, "There was an error in opening the file.\n"); exit(1);
  }
  while (evalLines) {
   ungetc(10, bf);
   int len = strlen(evalLines->s);
   for (int i=len-1; i>=0; i--) ungetc(evalLines->s[i], bf);
   struct evalStr* next = evalLines->next;
   free(evalLines);
   evalLines = next;
  }
 } else if (optind < argc /*Right?*/) bf = fopen(argv[optind], "r");
 else bf = stdin;
 if (bf == NULL) {
  fprintf(stderr, "There was an error in opening the file.\n");
  exit(1);
 }
 if (flags.nobuff) setbuf(stdin, NULL);
 cell* buffer = calloc(buffsize, sizeof(cell));
 if (buffer == NULL) {
  fprintf(stderr, "There was an error in allocating the buffer.\n");
  exit(1);
 }
 cell* p = buffer;
 /* Insert switch processing of #! line in file */
 while (ch = getc(bf)) {
  switch (ch) {
   case '>': p++; if (p >= &buffer[buffsize]) p = buffer; break;
   case '<': p--; if (p < buffer) p = &buffer[buffsize-1]; break;
   case '+': (*p)++; break;
   case '-': (*p)--; break;
   case '.': if (flags.digits) printf("%d ", *p); else putchar(*p); break;
   case ',': ch = getchar(); if (ch != EOF) *p = (cell) ch; break;
    /* Should there be a way for the user to redefine the above behavior? */
   case '[': if (*p == 0) {
     int level = 1;
     while (level) {
      ch = getc(bf);
      if (ch == '[') level++;
      else if (ch == ']') level--;
      else if (ch == EOF) {fprintf(stderr, "\nError: Missing %d closing bracket(s) from file.\n", level); exit(1); }
     }
    }
    break;
   case ']': if (*p) {
     int level = 1;
     while (level) {
      if (fseek(bf, -2L, SEEK_CUR) == -1) {fprintf(stderr, "\nError: %d unmatched closing bracket(s) contained in file.\n", level); exit(1); }
      ch = getc(bf);
      if (ch == ']') level++;
      else if (ch == '[') level--;
      else if (ch == EOF) goto end;
     }
    }
    break;
   case '#': /*if (flags.comments) {*/
    while (ch != 10 && ch != 13 && ch != EOF) {ch = getc(bf); }
   /*}*/
    break;
   case EOF: goto end;
  }
 }
 end: free(buffer);
 printf("\n");
 return 0;
}
