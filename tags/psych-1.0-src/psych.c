/* Psych, v.1.0 - a Brainfuck interpreter
   Copyright (C) 2007, 2008 John T. Wodder II

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>. */

/* This program was last updated 13 Feb 2008 by John T. Wodder II. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <termios.h>

#define ARRAY_SIZE  30000  /* default array size */
typedef unsigned char cell;  /* data type of the array */

char* program = NULL; int level = 0;
struct {_Bool nobuff, digits, eval, ignore : 1; } flags;

void appendLine(char*);

int main(int argc, char** argv) {
 int arraySize = ARRAY_SIZE, ch, printOut=0;
 while ((ch = getopt(argc, argv, "a:bde:hip:V?")) != -1) {
  switch (ch) {
   case 'a': arraySize = (int) strtol(optarg, NULL, 10); break;
   case 'b': flags.nobuff = 1; break;
   case 'd': flags.digits = 1; break;
   case 'e': flags.eval = 1; appendLine(optarg); break;
   case 'h':
    printf("Brainfuck commands:\n"
     " >  Increment pointer\n"
     " <  Decrement pointer\n"
     " +  Increment value at pointer\n"
     " -  Decrement value at pointer\n"
     " .  Output character value at pointer\n"
     " ,  Read character and store in value at pointer\n"
     " [  Skip to corresponding ] if value at pointer is 0\n"
     " ]  Skip back to corresponding [ unless value at pointer is zero\n");
    exit(2);
   case 'i': flags.ignore = 1; break;
   case 'p': printOut = (int) strtol(optarg, NULL, 10); break;
   case 'V':
    printf("Psych, a Brainfuck interpreter, v.1.0\n"
     "Written by John T. Wodder II (minimiscience@users.sourceforge.net)\n"
     "Compiled %s, %s\n"
     "Psych is distributed under the GNU General Public License v.3\n"
     "Configuration details:\n"
     "\tSize of array cells: %zu byte(s)\n"
     "\tDefault length of array: %d\n\n"
     "Type `man psych', `psych -h', or `psych -?' for help.\n",
     __DATE__, __TIME__, sizeof(cell), ARRAY_SIZE);
    exit(2);
   case '?': default:
    printf("Usage: psych [-bdi] [-hV] [-a num] [-p num] [-e code | file]\n"
     "Options:\n"
     "\t-a  Set the size of the array to `num'\n"
     "\t-b  Turn off input buffering\n"
     "\t-d  Make `.' output values as numbers\n"
     "\t-e  Execute given Brainfuck code\n"
     "\t-h  Display summaries of the Brainfuck commands and exit\n"
     "\t-i  Ignore invalid characters in source\n"
     "\t-p  Print the first `num' elements of the array on termination\n"
     "\t-V  Display version & configuration information and exit\n");
    exit(2);
  }
 }
 if (!flags.eval) {
  FILE* bf;
  if (optind < argc && !(argv[optind][0] == '-' && argv[optind][1] == '\0'))
   bf = fopen(argv[optind], "r");
  else bf = stdin;
  if (bf == NULL) {
   fprintf(stderr, "psych: error opening `%s': ", argv[optind]);
   perror(NULL); exit(4);
  }
  char str[256], *s;
  while ((s = fgets(str, 256, bf)) != NULL) appendLine(str);
  if (ferror(bf)) {perror("psych: error reading file"); exit(4); }
  fclose(bf);
 }
 if (program == NULL || program[0] == '\0') return 0;
 if (level > 0) {
  fprintf(stderr, "psych: program contains %d unmatched opening bracket(s)\n",
   level); exit(3);
 } else if (level < 0) {
  /* This should never happen. */
  fprintf(stderr, "psych: program contains %d unmatched closing bracket(s)\n",
   -level); exit(3);
 }
 cell* dataArray = calloc(arraySize, sizeof(cell));
 if (dataArray == NULL) {
  fprintf(stderr, "psych: insufficient memory to allocate array\n"); exit(1);
 }
 struct termios oldTerm;
 if (flags.nobuff && isatty(STDIN_FILENO)) {
  if (tcgetattr(STDIN_FILENO, &oldTerm) == 0) {
   struct termios breakTerm = oldTerm;
   breakTerm.c_lflag &= ~ICANON;
   breakTerm.c_cc[VMIN] = 1;
   breakTerm.c_cc[VTIME] = 0;
   if (tcsetattr(STDIN_FILENO, TCSANOW, &breakTerm) != 0)
    perror("psych: could not disable input buffering");
  } else {perror("psych: could not disable input buffering"); }
 }
 cell* p = dataArray;
 char* q = program;
 while (*q != '\0') {
  switch (*q) {
   case '>': p++; if (p >= dataArray + arraySize) p = dataArray; break;
   case '<': p--; if (p < dataArray) p = dataArray + arraySize - 1; break;
   case '+': (*p)++; break;
   case '-': (*p)--; break;
   case '.': if (flags.digits) printf("%d ", *p); else putchar(*p); break;
   case ',': if ((ch = getchar()) != EOF) *p = (cell) ch; break;
   /* Note that EOF is not recognized when input buffering is disabled. */
   case '[': if (*p == 0) {
     level = 1;
     while (level != 0) {
      q++;
      if (*q == '[') level++;
      else if (*q == ']') level--;
      else if (*q == '\0') {
       /* This should never happen. */
       fprintf(stderr, "\npsych: program contains %d unmatched opening"
        " bracket%s\n", level, level == 1 ? "" : "s");
       exit(3);
      }
     }
    }
    break;
   case ']': if (*p) {
     level = 1;
     while (level != 0) {
      q--;
      if (q < program) {
       /* This should never happen. */
       fprintf(stderr, "\npsych: program contains %d unmatched closing"
	" bracket%s\n", -level, level == -1 ? "" : "s");
       exit(3);
      }
      if (*q == ']') level++;
      else if (*q == '[') level--;
     }
    }
    break;
  }
  q++;
 }
 putchar('\n');
 if (printOut > 0) {
  for (int i=0; i<printOut; i++) {
   if (i) putchar(' '); printf("%d", dataArray[i]);
  }
  putchar('\n');
 }
 free(dataArray);
 free(program);
 if (flags.nobuff && isatty(STDIN_FILENO) && tcsetattr(STDIN_FILENO, TCSAFLUSH,
  &oldTerm) != 0) perror("psych: error restoring input buffering settings");
 return 0;
}

/* Add in processing of switches in #! line? */

void appendLine(char* s) {
 static size_t progLength=0, bufferLen=0;
 static int lineNo=0;
 static _Bool inComment=0;
 if (flags.eval) {lineNo++; inComment=0; }
 if (lineNo==0) lineNo++;
 if (inComment && (s = strchr(s, '\n')) == NULL) return;
 size_t len = strlen(s);
 if (bufferLen == 0) {
  program = calloc(len+1, sizeof(char));
  if (!program) {fprintf(stderr, "psych: out of memory\n"); exit(1); }
  bufferLen = len + 1;
 } else if (len + progLength + 1 > bufferLen) {
  program = realloc(program, (len+progLength+1)*sizeof(char));
  if (!program) {fprintf(stderr, "psych: out of memory\n"); exit(1); }
  bufferLen = len + progLength + 1;
  memset(program+progLength, '\0', len+1);
 }
 char* p = program + progLength;
 while (*s) {
  if (strchr("+-<>,.", *s)) {*p++ = *s; progLength++; }
  else if (*s == '[') {level++; *p++ = '['; progLength++; }
  else if (*s == ']') {
   if (--level < 0) {fprintf(stderr, "psych: closing bracket on line %d encountered while no brackets were open\n", lineNo); exit(3); }
   *p++ = ']'; progLength++;
  } else if (*s == '#') {if (!(s = strchr(s, '\n'))) {inComment=1; return; } }
  else if (*s == '\n') {lineNo++; inComment=0; }
  else if (!isspace(*s) && !flags.ignore) fprintf(stderr, "psych: Invalid character `%c' at line %d discarded\n", *s, lineNo);
  s++;
 }
}
