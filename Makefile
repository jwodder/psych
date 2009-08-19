all : psych psych.1

psych : psych.c
	gcc -o psych -O2 -std=c99 psych.c

psych.1 : psych.pod
	pod2man -c '' -r 'Psych 1.0.1' psych.pod psych.1
