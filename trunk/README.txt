README.txt
========================

	Psych is yet another interpreter for Urban Müller's Brainfuck
programming language.  Besides the standard features, Psych also allows
resizing of the array at runtime, unbuffered input for `,', output in the form
of numbers rather than characters, output of a portion of the data array on
program termination, and running code passed via the command line.  You can
even use the -h switch to print out a short summary of the Brainfuck commands
if, say, you just can't remember which is the input command & which is the
output.

	Psych was written by John T. Wodder II (jwodder@sdf.lonestar.org) and
is licensed under the GNU GPL, version 3 or later.  The location of the current
Psych tarball will probably be subject to change a bit, though if you have
access to a Gopher client, you should be able to find the current source at
<gopher://sdf.lonestar.org/9/users/jwodder/src/psych-src.tgz>.

CONTENTS
========================

COPYING - the GNU GPL, v.3, under which Psych is licensed
Makefile - the makefile
README.txt - this file
psych.1 - the documentation in manpage format
psych.c - the source code for Psych
psych.pod - the Plain Old Documentation source for the manpage

COMPILING
========================

	In order to compile Psych properly, you will need a standard C compiler
that is at least partially C99-compliant along with a decent POSIX
implementation (specifically, termios and the getopt() function).  There are
two aspects of the program that are abstracted so as to ease reconfiguration if
desired: the default size of the data array, set on line 26, and the data type
used for the array elements, set on line 27.  If you feel like changing
anything else, go ahead, but don't blame me if your computer explodes.

	To build the program, simply run `make' or `make psych' (the only
difference is that the former will also remake the manpage if the POD source
has been updated).  Note that, unlike fancier open source projects, there is no
`make install'; you'll have to move the executable & manpage by hand.
