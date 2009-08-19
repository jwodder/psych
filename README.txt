=== Psych - v.1.0.1 - README.txt ===

1. ABOUT

Psych is yet another interpreter for Urban Müller's Brainfuck programming
language.  Besides the standard features, Psych allows resizing of the data
array at runtime, noncanonical input for `,', output in the form of numbers
rather than characters, output of a portion of the data array on program
termination, and running code passed via the command line.  You can even use
the -h switch to print out a short summary of the Brainfuck commands if, say,
you just can't remember which is the input command & which is the output.

Psych was written by John T. Wodder II <jwodder@sdf.lonestar.org> and is
licensed under the GNU GPL, version 3.0 or later.  The location of the current
source tarball will probably be subject to change a bit, though if you have
access to a Gopher client, you should be able to find it at
<gopher://sdf.lonestar.org/9/users/jwodder/src/psych-src.tgz>.


2. FILE LIST

COPYING - the GNU GPL v.3, under which Psych is licensed
Makefile - the makefile
README.txt - this file
psych.1 - the Psych manpage
psych.c - the source code for Psych
psych.pod - the Plain Old Documentation source for the manpage


3. COMPILING

In order to compile Psych properly, you will need a standard C compiler that is
at least partially C99-compliant (the Makefile assumes gcc) along with some
POSIX features (specifically, the getopt() function and, if you want
noncanonical input, the <termios.h> routines).

There are three features of the program that are designed to be easily
reconfigurable at compile time.  The default size for the data array can be set
by redefining the ARRAY_SIZE macro on line 20, and the datatype of the array
cells can be set on line 21.  Additionally, if your machine does not have the
<termios.h> routines, or if you simply want to disable the option of
noncanonical input, comment out or delete the definition of ALLOW_TERMIOS on
line 19.

To build the program, simply run `make' or `make psych' (the only difference is
that the former will also remake the manpage if the POD source has been
updated).  Note that, unlike fancier open source projects, there is no `make
install'; you'll have to move the executable & manpage to their destination
directories by hand.
