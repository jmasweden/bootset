# makefile

COMP += gcc
CFLAGS += --no-warnings -Zomf -c -O2 -Wwrite-strings -fpermissive
LFLAGS += -Zomf 

ODIR := ../stlib2
OBJS := st_std.obj st_vio.obj
OSRC := $(addprefix $(ODIR)/, $(OBJS))


all: bst bse 

bst : bootset.exe
bse : bsedit.exe

bootset.exe : bootset.obj bootset.def $(OSRC)
	$(COMP) $(LFLAGS) bootset.obj $(OSRC) bootset.def -o bootset.exe

bootset.obj : bootset.cpp bootset.h
	$(COMP) $(CFLAGS) bootset.cpp -o bootset.obj

bsedit.exe : bsedit.obj bsedit.def $(OSRC)
	$(COMP) $(LFLAGS) bsedit.obj $(OSRC) bsedit.def -o bsedit.exe

bsedit.obj : bsedit.cpp bootset.h
	$(COMP) $(CFLAGS) bsedit.cpp -o bsedit.obj

clean :
	del *exe  *obj

