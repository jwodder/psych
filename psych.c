/* Psych, v.1.0.1 - a Brainfuck interpreter
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

#define ALLOW_TERMIOS		/* If defined, allow noncanonical input */
#define ARRAY_SIZE  30000	/* default array size */
typedef unsigned char cell;	/* data type of the array */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>

#ifdef ALLOW_TERMIOS
#include <termios.h>
#endif

char* program = NULL;
char* argv0;
int level = 0;
struct {
 _Bool nobuff : 1, digits : 1, eval : 1, ignore : 1, nonl : 1;
} flags = {0};

void appendLine(char*);

int main(int argc, char** argv) {
 int arraySize = ARRAY_SIZE, printOut = 0, ch;
 argv0 = argv[0];
 while ((ch = getopt(argc, argv, "a:bde:hip:V?n")) != -1) {
  switch (ch) {
   case 'a': arraySize = (int) strtol(optarg, NULL, 10); break;
   case 'b':
#ifdef ALLOW_TERMIOS
    flags.nobuff = 1;
#else
    fprintf(stderr, "%s: warning: noncanonical input processing is disabled in"
		    " this version\n", argv0);
#endif
    break;
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
	   " [  Skip to corresponding ] if value at pointer is zero\n"
	   " ]  Skip back to corresponding [ unless value at pointer is zero\n");
    return 0;
   case 'i': flags.ignore = 1; break;
   case 'n': flags.nonl = 1; break;
   case 'p': printOut = (int) strtol(optarg, NULL, 10); break;
   case 'V':
    printf("Psych, a Brainfuck interpreter, v.1.0.1\n"
	   "Written by John T. Wodder II (jwodder@sdf.lonestar.org)\n"
	   "Compiled %s, %s\n"
	   "Psych is distributed under the GNU General Public License v.3\n"
	   "Configuration details:\n"
	   "  Size of array cells: %zu byte%s\n"
	   "  Default length of data array: %d\n"
	   "  Support for noncanonical input processing "
#ifdef ALLOW_TERMIOS
	   "enabled"
#else
	   "disabled"
#endif
	   "\nType `man psych', `psych -h', or `psych -?' for help.\n",
	   __DATE__, __TIME__, sizeof(cell), sizeof(cell) > 1 ? "s" : "",
	   ARRAY_SIZE);
    return 0;
   case '?':
   default:
    printf("Usage: %s [-bdi] [-hV?] [-a num] [-p num] [-e code | file]\n\n"
	   "Options:\n"
	   "  -a num - Set the size of the array to `num'\n"
	   "  -b - Disable canonical input"
#ifndef ALLOW_TERMIOS
	   " (not available in this version)"
#endif
	   "\n"
	   "  -d - Make `.' output values as numbers\n"
	   "  -e code - Execute given Brainfuck code\n"
	   "  -h - Display summaries of the Brainfuck commands and exit\n"
	   "  -i - Ignore invalid characters in source\n"
	   "  -n - Do not print a newline on program termination\n"
	   "  -p num - Print the first `num' elements of the array on termination\n"
	   "  -V - Display version & configuration information and exit\n"
	   "  -? - Display this help message and exit\n", argv0);
    exit(2);
  }
 }
 if (!flags.eval) {
  FILE* bf = (optind < argc && !(argv[optind][0] == '-'
			      && argv[optind][1] == '\0'))
	      ? fopen(argv[optind], "r") : stdin;
  if (bf == NULL) {
   fprintf(stderr, "%s: %s: %s\n", argv0, argv[optind], strerror(errno));
   exit(5);
  }
  char str[256];
  while (fgets(str, 256, bf) != NULL) appendLine(str);
  if (ferror(bf)) {
   fprintf(stderr, "%s: error reading file: %s\n", argv0, strerror(errno));
   exit(4);
  }
  fclose(bf);
 }
 if (program == NULL || program[0] == '\0') return 0;
 if (level != 0) {
  char* desc = level > 0 ? "opening" : "closing";
  if (level < 0) level *= -1;
  /* Unmatched closing brackets should always be detected by appendLine(), but
   * just in case... */
  fprintf(stderr, "%s: program contains %d unmatched %s bracket%s\n",
	  argv0, level, desc, level == 1 ? "" : "s");
  exit(3);
 }
 cell* dataArray = calloc(arraySize, sizeof(cell));
 if (dataArray == NULL) {perror(argv0); exit(1); }
#ifdef ALLOW_TERMIOS
 struct termios oldTerm;
 if (flags.nobuff && isatty(STDIN_FILENO)) {
  if (tcgetattr(STDIN_FILENO, &oldTerm) == 0) {
   struct termios breakTerm = oldTerm;
   breakTerm.c_lflag &= ~ICANON;
   breakTerm.c_cc[VMIN] = 1;
   breakTerm.c_cc[VTIME] = 0;
   if (tcsetattr(STDIN_FILENO, TCSANOW, &breakTerm) != 0) {
    fprintf(stderr, "%s: could not disable canonical input: %s\n",
	    argv0, strerror(errno));
   }
  } else {
   fprintf(stderr, "%s: could not disable canonical input: %s\n",
	   argv0, strerror(errno));
  }
 }
#endif
 cell *p = dataArray, *lastCell = dataArray + arraySize - 1;
 char* q = program;
 while (*q != '\0') {
  switch (*q) {
   case '>': p++; if (p > lastCell)  p = dataArray; break;
   case '<': p--; if (p < dataArray) p = lastCell;  break;
   case '+': (*p)++; break;
   case '-': (*p)--; break;
   case '.': if (flags.digits) printf("%d ", *p); else putchar(*p); break;
   case ',': if ((ch = getchar()) != EOF) *p = (cell) ch; break;
   /* Note that EOF is not recognized in noncanonical input mode. */
   case '[':
    if (*p == 0) {
     level = 1;
     while (level != 0) {
      q++;
      assert(*q != '\0');  /* `*q' should never be NUL. */
      if (*q == '[') level++;
      else if (*q == ']') level--;
     }
    }
    break;
   case ']':
    if (*p != 0) {
     level = 1;
     while (level != 0) {
      q--;
      assert(q >= program);  /* `q' should never be less than `program'. */
      if (*q == ']') level++;
      else if (*q == '[') level--;
     }
    }
    break;
  }
  q++;
 }
 if (!flags.nonl) putchar('\n');
 if (printOut > 0) {
  for (int i=0; i<printOut; i++) {
   if (i) putchar(' ');
   printf("%d", dataArray[i]);
  }
  putchar('\n');
 }
 free(dataArray);
 free(program);
#ifdef ALLOW_TERMIOS
 if (flags.nobuff && isatty(STDIN_FILENO)
		  && tcsetattr(STDIN_FILENO, TCSAFLUSH, &oldTerm) != 0)
  fprintf(stderr, "%s: error restoring canonical input mode: %s\n",
	  argv0, strerror(errno));
#endif
 return 0;
}

void appendLine(char* s) {
 static size_t progLength = 0, bufferLen = 0;
 static int lineNo = 0;
 static _Bool inComment = 0;
/* Add in processing of switches in #! line? */
 if (flags.eval) {lineNo++; inComment=0; }
 if (lineNo==0) lineNo++;
 if (inComment && (s = strchr(s, '\n')) == NULL) return;
 size_t len = strlen(s);
 if (bufferLen == 0) {
  program = calloc(len+1, sizeof(char));
  if (!program) {perror(argv0); exit(1); }
  bufferLen = len + 1;
 } else if (len + progLength + 1 > bufferLen) {
  program = realloc(program, (len+progLength+1) * sizeof(char));
  if (!program) {perror(argv0); exit(1); }
  bufferLen = len + progLength + 1;
  memset(program + progLength, '\0', len+1);
 }
 char* p = program + progLength;
 while (*s) {
  if (strchr("+-<>,.", *s)) {*p++ = *s; progLength++; }
  else if (*s == '[') {level++; *p++ = '['; progLength++; }
  else if (*s == ']') {
   if (--level < 0) {
    fprintf(stderr, "%s: unmatched closing bracket on line %d\n",
	    argv0, lineNo);
    exit(3);
   }
   *p++ = ']';
   progLength++;
  } else if (*s == '#') {
   s = strchr(s, '\n');
   if (s == NULL) {inComment = 1; return; }
   else s--;
  } else if (*s == '\n') {lineNo++; inComment=0; }
  else if (!isspace(*s) && !flags.ignore) {
   fprintf(stderr, "%s: invalid character `%c' at line %d discarded\n",
	   argv0, *s, lineNo);
  }
  s++;
 }
}
