# Makefile for bootset (Open Watcom C on OS/2)
# wmake 2.0.1 on OS/2 - explicit per-file rules (no pattern rules)

# ============================================================================
# Configuration
# ============================================================================

NAME1   = bootset
NAME2   = bsedit
SRCDIR  = src
BINDIR  = bin

STLIB2OBJ = , ..\stlib2\bin\st_std.obj, ..\stlib2\bin\st_vio.obj
STLIB2H   =  ../stlib2/src/st_std.h ../stlib2/src/st_vio.h

# ============================================================================
# Tools
# ============================================================================

CC      = wcc386
LINK    = wlink
RC      = wrc

# ============================================================================
# Flags
# ============================================================================

CFLAGS  = -bt=os2 -mf -5 -fpi -Oaxt -W3 -ze -d0 -bm
CFLAGS  = $(CFLAGS) -i=$(OS2TK)\h -i=$(SRCDIR)

RCFLAGS = -r -bt=os2 -i=$(OS2TK)\h -i=$(SRCDIR)

LFLAGS  = system os2v2
LFLAGS  = $(LFLAGS) option stack=65536

LFLAGS1  = $(LFLAGS) option map=$(BINDIR)\$(NAME1).map
LFLAGS2  = $(LFLAGS) option map=$(BINDIR)\$(NAME2).map

# ============================================================================
# Source files
# ============================================================================

OBJ1 = $(BINDIR)\bootset.obj
OBJ2 = $(BINDIR)\bsedit.obj

DEFFILE1 = $(SRCDIR)\$(NAME1).def
DEFFILE2 = $(SRCDIR)\$(NAME2).def

# ============================================================================
# Targets
# ============================================================================

all : $(BINDIR)\$(NAME1).exe $(BINDIR)\$(NAME2).exe .SYMBOLIC

$(BINDIR) :
	@if not exist $(BINDIR) mkdir $(BINDIR)

$(BINDIR)\$(NAME1).exe : $(OBJ1) $(ROBJ) $(DEFFILE1)
	@echo Linking $(NAME1).exe...
	@$(LINK) $(LFLAGS1) name $(BINDIR)\$(NAME1).exe file $(OBJ1) $(STLIB2OBJ) library os2386.lib
	@if exist $(BINDIR)\$(NAME1).exe echo BUILD OK

$(BINDIR)\$(NAME2).exe : $(OBJ2) $(ROBJ) $(DEFFILE2)
	@echo Linking $(NAME2).exe...
	@$(LINK) $(LFLAGS2) name $(BINDIR)\$(NAME2).exe file $(OBJ2) $(STLIB2OBJ) library os2386.lib
	@if exist $(BINDIR)\$(NAME2).exe echo BUILD OK

# ============================================================================
# Compiling objects
# ============================================================================
$(BINDIR)\bootset.obj : $(SRCDIR)\bootset.cpp $(SRCDIR)\bootset.h $(SRCDIR)\english.h $(STLIB2H) $(BINDIR)
	@echo Compiling src\bootset.cpp
	@$(CC) $(CFLAGS) -fo=$@ $(SRCDIR)\bootset.cpp

$(BINDIR)\bsedit.obj : $(SRCDIR)\bsedit.cpp $(SRCDIR)\bootset.h $(SRCDIR)\english.h $(STLIB2H) $(BINDIR)
	@echo Compiling src\bsedit.cpp
	@$(CC) $(CFLAGS) -fo=$@ $(SRCDIR)\bsedit.cpp

# ============================================================================
# Clean
# ============================================================================
clean : .SYMBOLIC
	@if exist $(BINDIR)\*.obj del $(BINDIR)\*.obj >nul
	@if exist $(BINDIR)\$(NAME1).exe del $(BINDIR)\$(NAME1).exe >nul
	@if exist $(BINDIR)\$(NAME1).map del $(BINDIR)\$(NAME1).map >nul
	@if exist $(BINDIR)\$(NAME2).exe del $(BINDIR)\$(NAME2).exe >nul
	@if exist $(BINDIR)\$(NAME2).map del $(BINDIR)\$(NAME2).map >nul
	@echo Clean complete