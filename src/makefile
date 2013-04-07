#
# $Id: m2,v 1.1 1996/03/18 13:25:36 holtrf Exp holtrf $
#
# This is the RSL makefile.
#
# Two versions of RSL may be built:
#   `rsl'  the generic rsl,
#   `trsl' the experimental version of rsl with CGI, email, loops, other stuff.
#


CC = gcc
CCC = g++

BASE = /mongo/dest
OBJ = $(BASE)/obj

COLLECT = -collector=/usr/local/lib/gcc-lib/sparc-sun-sunos4.1/2.6.3/ld
OPTIONS = -leaks-at-exit=no -follow-child-processes=yes
PURE = purify $(OPTIONS) $(COLLECT)
PDIR = /mongo/apps/pure/purify-3.0.1-sunos4
PSTUBS = $(PDIR)/purify_stubs.a


LFLAGS = -lg++ -ll -ly
CFLAGS = -g -I/mongo/dest/rmgserver -I$(PDIR)
# -DDEBUG -DMEMCHECK

OBJS =	R_File.o R_Output.o R_String.o R_Boolean.o \
		R_Integer.o rescall.o \
		resource.o restable.o \
		ifrescall.o SLList_res_util.o \
		restree.o

Hdr = R_File.h R_Output.h R_String.h R_Boolean.h \
		R_Integer.h rescall.h \
		resource.h restable.h \
		ifrescall.h SLList_res_util.h

StdObjs = R_System.o rsl_control.o
StdHdr = R_System.h rsl_control.h

XObjs = rsl_controlX.o looprescall.o R_SystemX.o email.o

XHdr = rsl_controlX.h looprescall.h R_SystemX.h

WebObjs = $(BASE)/server/ohtml.o $(BASE)/server/htmltable.o \
		$(BASE)/server/cgi.o R_html.o
WebHdr = R_html.h

So = $(OBJ)/Socket.o $(BASE)/log/slog.o


Parser = y.tab.o lex.yy.o
# RSLOBJS = $(OBJS) $(StdObjs) $(Parser)
RSLOBJS = $(OBJS) $(XObjs) $(Parser)
TRSLOBJS = $(OBJS) $(XObjs) $(Parser)

LibName = rsl
Static_lib_name = lib$(LibName).a
Shared_lib_name = lib$(LibName).so

StaticLFLAGS = -L$(BASE)/crb -l$(LibName) $(LFLAGS)
SharedLFLAGS = -L$(BASE)/crb -Xlinker -Bdynamic $(Shared_lib_name) $(LFLAGS)

COMPILE.cc = $(CCC) $(CFLAGS)
.cc.o:
	$(COMPILE.cc) -c $<

.cc.a:
	$(COMPILE.cc) -c -o $% $<
	$(AR) $(ARFLAGS) $@ $%

.c.a:
	$(CC) -c $<
	$(AR) $(ARFLAGS) $@ $%

.c.o:
	$(CC) -c $<

all: rsl trsl

static_lib := CFLAGS += -DXPERIMENTAL
static_lib:$(Static_lib_name)($(OBJS) $(RSLOBJS))
	ranlib $(Static_lib_name)

shared_lib: $(RSLOBJS)
	ld $(RSLOBJS) -o $(Shared_lib_name)

shared_rsl: rsl-main.o $(Shared_lib_name)
	$(CCC) rsl-main.o $(WebObjs) $(So) $(SharedLFLAGS) -o shared_rsl

rsl: static_lib rsl-main.o $(WebObjs)
	$(CCC) rsl-main.o $(So) $(WebObjs) $(StaticLFLAGS) -o rsl

wrsl := CFLAGS += -DWEB
wrsl: rsl

# rsl := CFLAGS += -UWEB -USYSTEM_EMAIL -UXPERIMENTAL
# rsl: $(RSLOBJS) rsl-main.o $(WebObjs)
#	$(CCC) $(RSLOBJS) $(So) rsl-main.o $(WebObjs) $(LFLAGS) -o rsl

# rsl := CFLAGS += -DWEB -DXPERIMENTAL
# rsl: $(TRSLOBJS) rsl-main.o $(WebObjs)
#	$(CCC) $(TRSLOBJS) $(So) $(WebObjs) rsl-main.o $(LFLAGS) -o rsl

trsl := CFLAGS += -DWEB -DXPERIMENTAL
trsl: $(TRSLOBJS) rsl-main.o $(WebObjs)
	$(CCC) $(TRSLOBJS) $(So) $(WebObjs) rsl-main.o $(LFLAGS) -o trsl

purex := CFLAGS += -DWEB -DXPERIMENTAL
purex: $(OBJS) $(XObjs) $(WebObjs) rsl-main.o
	$(PURE) $(CCC) $(OBJS) $(So) $(XObjs) $(WebObjs) rsl-main.o y.tab.o lex.yy.o $(LFLAGS) -o trsl.pure

gnu.pure: gnu_pure.o
	$(PURE) $(CCC) gnu_pure.o $(LFLAGS) -o gnu.pure

rez.pure: $(OBJS) $(XObjs) $(WebObjs) rezpure.o
	$(PURE) $(CCC) $(OBJS) $(So) $(XObjs) $(WebObjs) rezpure.o y.tab.o lex.yy.o $(LFLAGS) -o rez.pure

rttest := CFLAGS += -UWEB -USYSTEM_EMAIL -UXPERIMENTAL
rttest: $(OBJS) $(StdObjs) $(WebObjs) rt-test.o
	$(CCC) $(OBJS) $(So) $(StdObjs) $(WebObjs) rt-test.o y.tab.o lex.yy.o $(LFLAGS) -o rt-test

tree := CFLAGS += -UWEB -USYSTEM_EMAIL -UXPERIMENTAL
tree: $(OBJS) $(StdObjs) $(WebObjs) rtreemain.o
	$(CCC) $(OBJS) $(So) $(StdObjs) $(WebObjs) rtreemain.o \
		y.tab.o lex.yy.o $(LFLAGS) -o treetest

purersl := CFLAGS += -UWEB -USYSTEM_EMAIL -UXPERIMENTAL
purersl: $(OBJS) $(StdObjs) $(WebObjs) rsl-main.o
	$(PURE) $(CCC) $(OBJS) $(So) $(StdObjs) $(WebObjs) rsl-main.o y.tab.o lex.yy.o $(LFLAGS) -o rsl.pure

puretrsl := CFLAGS += -DWEB -DXPERIMENTAL
puretrsl: $(OBJS) $(XObjs) $(WebObjs) rsl-main.o
	$(PURE) $(CCC) $(OBJS) $(So) $(XObjs) $(WebObjs) rsl-main.o y.tab.o lex.yy.o $(LFLAGS) -o trsl.pure

installrsl: rsl
	cp $(OBJS) $(StdObjs) lex.yy.o y.tab.o ../obj
	cp $(Hdr) $(StdHdr) ../include

typetest: rtypetest.o
	$(CCC) $(OBJS) rtypetest.o y.tab.o lex.yy.o $(LFLAGS) -o rtypetest

lexer:
	lex r2.l
	cc -c lex.yy.c

parser:
	yacc r2.y
	gcc -c y.tab.c

lp: lexer parser

sm:
	g++ -I/mongo/dest/rmgserver -c email.cc test-smtp.cc
	g++ email.o test-smtp.o /mongo/dest/rmgserver/Socket.o -o test-email

wayclean:
	rm ./*.o
