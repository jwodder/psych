/* psych, v.0.4 - a Brainfuck interpreter
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

/* This program was last updated 2 Feb 2008 by John T. Wodder II. */

/* See <http://en.wikipedia.org/wiki/Brainfuck> for more information about the
 * Brainfuck language. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

typedef unsigned char cell;
struct {_Bool nobuff, digits, eval : 1; } flags;
char* program = NULL;
int level = 0;

void appendLine(char*, int);

int main(int argc, char** argv) {
 int buffsize = 30000, ch;
 while ((ch = getopt(argc, argv, "hb:le:Vd?")) != -1) {
  switch (ch) {
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
    exit(0);
   case 'b': buffsize = (int) strtol(optarg, NULL, 10); break;
   case 'l': flags.nobuff = 1; break;
   case 'd': flags.digits = 1; break;
   case 'e': appendLine(optarg, strlen(optarg)); flags.eval = 1; break;
   case 'V':
    printf("psych, a Brainfuck interpreter, v.0.4\n"
     "Written by John T. Wodder II (minimiscience+psych@gmail.com)\n"
     "Compiled %s, %s\n"
     "Configuration details:\n"
     "\tSize of array cells: %d byte(s)\n"
     "\tDefault length of array: %d\n\n"
     "Type ``psych -h'' to see the list of Brainfuck commands.\n"
     "Type ``psych -?'' to see a list of command-line options.\n",
     __DATE__, __TIME__, sizeof(cell), buffsize);
    exit(0);
   case '?': default:
    printf("Usage: psych [-hV] [-dl] [-b number] [-e code] [filename]\n"
     "Options:\n"
     "\t-h  Display summaries of the Brainfuck commands and exit\n"
     "\t-V  Display version & configuration information and exit\n"
     "\t-d  Make ``.'' output values as numbers\n"
     "\t-l  Turn off input buffering for ``,''\n"
     "\t-b  Set the size of the array\n"
     "\t-e  Execute Brainfuck code\n");
    exit(0);
  }
 }
 if (!flags.eval) {
  FILE* bf;
  if (optind < argc) bf = fopen(argv[optind], "r");
  else bf = stdin;
  if (bf == NULL) {
   fprintf(stderr, "There was an error in opening the file.\n");
   exit(1);
  }
  char* s; size_t len;
  while (s = fgetln(bf, &len)) {s[len-1] = 0; appendLine(s, len-1); }
  fclose(bf);
 }
 if (program[0] == 0) return 0;
 if (level > 0) {
  fprintf(stderr, "psych: program contains %d unmatched opening bracket(s).\n",
   level); exit(1);
 } else if (level < 0) {
  fprintf(stderr, "psych: program contains %d unmatched closing bracket(s).\n",
   -level); exit(1);
 }
 cell* buffer = calloc(buffsize, sizeof(cell));
 if (buffer == NULL) {
  fprintf(stderr, "There was an error in allocating the buffer.\n");
  exit(1);
 }
 if (flags.nobuff) system("stty cbreak </dev/tty >/dev/tty 2>&1");
  /* Apparently, the above only works on BSD-like systems.  I should look into
   * this more. */
 cell* p = buffer;
 char* q = program;
 while (*q) {
  switch (*q) {
   case '>': p++; if (p >= &buffer[buffsize]) p = buffer; break;
   case '<': p--; if (p < buffer) p = &buffer[buffsize-1]; break;
   case '+': (*p)++; break;
   case '-': (*p)--; break;
   case '.': if (flags.digits) printf("%d ", *p); else putchar(*p); break;
   case ',': ch = getchar(); if (ch != EOF) *p = (cell) ch; break;
    /* There should be a way for the user to redefine the above behavior. */
    /* Also, note that, when input is unbuffered, EOF no longer seems to be
     * recognized.  I need to figure out some way around that. */
   case '[': if (*p == 0) {
     level = 1;
     while (level != 0) {
      q++;
      if (*q == '[') level++;
      else if (*q == ']') level--;
      else if (*q == '\0') {fprintf(stderr, "\npsych: program contains %d unmatched opening bracket(s).\n", level); exit(1); } /* This should never happen. */
     }
    }
    break;
   case ']': if (*p) {
     level = 1;
     while (level != 0) {
      q--;
      if (q < program) {fprintf(stderr, "\npsych: program contains %d unmatched closing bracket(s).\n", -level); exit(1); } /* This should never happen. */
      if (*q == ']') level++;
      else if (*q == '[') level--;
     }
    }
    break;
  }
  q++;
 }
 end: free(buffer);
 free(program);
 if (flags.nobuff) system("stty -cbreak </dev/tty >/dev/tty 2>&1");
  /* ^^ Only works in BSD ^^ */
 printf("\n");
 return 0;
}

void appendLine(char* s, int len) {
 static int progLength = 0, bufferLen = 0, lineNo = 0;
 /* Insert switch processing of #! line in file */
 lineNo++;
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
 char* p = &program[progLength];
 while (*s) {
  if (strchr("+-<>,.", *s)) {*p++ = *s; progLength++; }
  else if (*s == '[') {level++; *p++ = '['; progLength++; }
  else if (*s == ']') {level--; *p++ = ']'; progLength++; }
  else if (*s == '#') return;
  else if (!isspace(*s)) fprintf(stderr, "Invalid character ``%c'' at line %d discarded.\n", *s, lineNo);
  s++;
 }
}
