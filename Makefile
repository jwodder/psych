all : psych psych.1

psych : psych.o
	gcc -o psych psych.o

psych.o : psych.c
	gcc -c psych.c -O2 -std=c99

psych.1 : psych.pod
	pod2man -c '' -r 'Psych 1.0.1' psych.pod psych.1
