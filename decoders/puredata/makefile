NAME=teabox
CSYM=teabox

current: pd_irix6

# ----------------------- NT -----------------------

pd_nt: $(NAME).dll

.SUFFIXES: .dll

# define PD_0_36 to compilie with pd0.36 and below
PDNTCFLAGS = /W3 /WX /MD /O2 /G6 /DNT /DPD /DPD_0_37 /DMAXLIB /nologo
VC="C:\Programme\Microsoft Visual Studio\VC98"

PDNTINCLUDE = /I. /Ic:\pd\tcl\include /Ic:\pd\src /I$(VC)\include /Iinclude

PDNTLDIR = $(VC)\Lib
PDNTLIB = $(PDNTLDIR)\msvcrt.lib \
	$(PDNTLDIR)\oldnames.lib \
	$(PDNTLDIR)\kernel32.lib \
	$(PDNTLDIR)\user32.lib \
	c:\pd\bin\pd.lib
        
PDNTEXTERNALS = teabox~.obj teabox.count~.obj teabox.bits~.obj

.c.dll:
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\teabox~.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\teabox.count~.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c src\teabox.bits~.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c $*.c
	link /dll /export:$(CSYM)_setup $*.obj $(PDNTEXTERNALS) $(PDNTLIB)


# ----------------------- IRIX 6.5 -----------------------

pd_irix6: $(NAME).pd_irix6

.SUFFIXES: .pd_irix6

SGICFLAGS6 = -n32 -DPD -DPD_0_37 -DUNIX -DIRIX -DN32 -woff 1080,1064,1185 \
	-OPT:roundoff=3 -OPT:IEEE_arithmetic=3 -OPT:cray_ivdep=true \
	-Ofast=ip32 -r12000 -shared

SGIINCLUDE =  -I../../src

SGIGCFLAGS = -mabi=n32 -DPD -DMAXLIB -DUNIX -DIRIX -DN32 -O3 -funroll-loops -fomit-frame-pointer \
    -Wall -W -Wshadow -Wstrict-prototypes \
    -Wno-unused -Wno-parentheses -Wno-switch -mips4

SGIEXTERNALS = teabox~.o teabox.count~.o teabox.bits~.o

.c.pd_irix6:
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/teabox~.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/teabox.count~.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c src/teabox.bits~.c
	gcc $(SGIGCFLAGS) $(SGIINCLUDE) -c $*.c
	ld -n32 -IPA -shared -rdata_shared -o $*.pd_irix6 $*.o $(SGIEXTERNALS)
	rm $*.o

# ----------------------- Mac OS X (Darwin) -----------------------


pd_darwin: $(NAME).pd_darwin

.SUFFIXES: .pd_darwin

DARWINCFLAGS = -DPD -DPD_0_37 -DMAXLIB -DUNIX -DMACOSX -O2 \
    -Wall -W -Wshadow -Wstrict-prototypes \
    -Wno-unused -Wno-parentheses -Wno-switch

# where is your m_pd.h ???
DARWININCLUDE =  -I../../src -I../../obj

DARWINEXTERNALS = teabox~.o teabox.count~.o teabox.bits~.o

.c.pd_darwin:
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/teabox~.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/teabox.count~.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c src/teabox.bits~.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c $*.c 
	cc -bundle -undefined suppress -flat_namespace -o $*.pd_darwin $*.o $(DARWINEXTERNALS)
	rm -f $*.o ../$*.pd_darwin
	ln -s $*/$*.pd_darwin ..

# ----------------------- LINUX i386 -----------------------

pd_linux: $(NAME).pd_linux

.SUFFIXES: .pd_linux

LINUXCFLAGS = -DPD -DMAXLIB -DUNIX -DPD_0_37 -O2 -funroll-loops -fomit-frame-pointer \
    -Wall -W -Wshadow \
    -Wno-unused -Wno-parentheses -Wno-switch

# where is your m_pd.h ???
LINUXINCLUDE =  -I/usr/local/include -I./include

LINUXEXTERNALS = teabox~.o teabox_count~.o teabox_bits~.o

.c.pd_linux:
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/teabox~.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/teabox.count~.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c src/teabox.bits~.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c $*.c
	ld -export_dynamic  -shared -o $*.pd_linux $*.o $(LINUXEXTERNALS) -lc
	strip --strip-unneeded $*.pd_linux

# ----------------------------------------------------------

PDDIR=/usr/local/lib/pd

install:
	install -d $(PDDIR)/doc/5.reference/teabox
	cp help/help-*.pd $(PDDIR)/doc/5.reference/teabox

clean:
	rm -f *.o *.pd_* so_locations
