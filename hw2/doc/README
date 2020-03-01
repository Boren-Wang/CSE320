*****************************************************************************

	To compile vtree examine the makefile and set the appropiate options.

	type "make" to compile it, and "make install" to install it.

*****************************************************************************

Vtree version 1.1 notes

The following changes were made:

** Patches were received from the following people:
**
**	1.	Mike Howard, (...!uunet!milhow1!how)
**		Mike's patches included changes to the Makefile to
**		customize vtree to SCO Xenix for the 286 as well as the
**		386.  He also added external definitions to hash.c
**
**	2.	Andrew Weeks, (...!uunet!mcvax!doc.ic.ac.uk!aw)
**		Andrew sent me diffs to make vtree work properly under BSD
**		He also pointed out that you will need one of the PD getopt
**		packages for BSD.
**
**	3.	Ralph Chapman, (...uunet!ihnp4!ihuxy!chapman)
**		Ralph sent me changes (not diffs unfortunately) to make
**		vtree work properly under the SYS_III option.  His changes
**		were in direct.c and vtree.c
**
**	4.	David Eckelkamp notified me of a bug when printing the
**		visual tree.  The bug occured when a directory name
**		was too long.  It caused vtree to mess up the tree
**		being printed.


	The vtree program can now be compiled in a memory-based
version. This option will force vtree to read an entire directory
before doing anything with it.  This will prevent vtree from reading a
directory 2 times for certain operations.  Strangely enough, the
memory-based version doesn't seem to be much faster than the disk-based
version.  If anybody has any suggestions as to why I would appreciate
them.

	A minor compile-time option has been added to control the format
of the first line.  If specified, then vtree checks the first line to
make sure it is only one directory name (no "/"s).  If not then vtree
will print the first line by itself, and then print the LAST subdir on
the next line to begin the tree.

	Two new runtime options have been added. The "-o" option will now
sort the directories before processing them.  It is only available with
the memory-based version of the program.  The "-f" option specifies
floating column widths.  The width of each column will be only as wide
as necessary.

	The visual display has been cleaned up a bit.

*****

	I did the development for vtree on an SCO Xenix 386 system. 
The System III  routines and the BSD routines are untested by myself. 
Based on the replies I received from Andrew Weeks and Ralph Chapman it
should compile and execute on those systems.  If you have to make any
local modifications to the program to make it work I would appreciate
hearing about them so I can keep the program up to date.



Jonathan B. Bayer
Intelligent Software Products, Inc.
Rockville Centre, NY   11570
Phone: (516) 766-2867
usenet:	uunet!ispi!root


*****************************************************************************
Vtree version 1.0 notes

	This is the first release of the VTREE (please pronounce this
V-TREE, for "visual files") program.  The program is designed to show
the layout of a directory tree or filesystem.  It has options to show
the amount of storage being taken up in each directory, count the number
of inodes, etc.

	VTREE is dependent on the UCB directory reading routines. 
Public-domain routines for System V have been released to the Usenet
(comp.sources.unix) by Doug Gwyn (gwyn@brl.mil).  If you don't have
them, they're worth your trouble to get.  Still, you may be able to use
the System III configuration of the Makefile as a stopgap measure. 


	The program was originally the program AGEF, written by David S.
Hayes.  As it stands now the hashing routines are untouched by myself,
but most of the rest of the program is different.  The System III
routines are also his.


	I hope this program will be useful to you.  If you find bugs in
it or have any suggestions for improvements, I'd like to hear about
them.

Jonathan B. Bayer
Intelligent Software Products, Inc.
Rockville Centre, NY   11570
Phone: (516) 766-2867
usenet:	uunet!ispi!root

