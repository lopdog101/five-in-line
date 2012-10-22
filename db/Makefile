#.if exists($(BOOST)/lib/libboost_filesystem-gcc-mt-1_32.so)
BOOST_LPREFIX=-gcc-mt-1_32
#.else
#BOOST_LPREFIX=
#.endif

################OPTION###################
CCOMPILE = gcc
CPPCOMPILE = g++
COMPILEOPTION = -c -O3 -pthread
INCLUDEDIR = -I$(BOOST) -I$(COMP)/cpp -I$(SHARED) -I/usr/local/include
LINK = g++
LINKOPTION = -o db -rpath-link $(LD_LIBRARY_PATH) -pthread
LIBDIRS = -L$(BOOST)/lib -L$(SHARED)/notification/free_bsd/Release -L$(SHARED)/object_progress/free_bsd/Release -L$(SHARED)/filelock -L/usr/local/lib
OBJS = db.o bin_index.o solution_tree.o solution_tree_utils.o bin_index_utils.o ../algo/field.o ../algo/game.o ../algo/wsplayer.o ../algo/gomoku_exceptions.o
OUTPUT = db
SHAREDLIB = -lnotification -lobject_progress -lfilelock -lboost_filesystem$(BOOST_LPREFIX) -lboost_thread$(BOOST_LPREFIX) -lpthread
APPENDLIB = 
PROC_OPTION = DEFINE=_PROC_ MODE=ORACLE CODE=CPP
ESQL_OPTION = 
################OPTION END################
ESQL = esql
PROC = proc
$(OUTPUT):$(OBJS) $(APPENDLIB)
	$(LINK) $(LINKOPTION) $(LIBDIRS)   $(OBJS) $(SHAREDLIB) $(APPENDLIB) 

clean: 
	rm -f $(OBJS)
	rm -f $(OUTPUT)
all: clean $(OUTPUT)
.PRECIOUS:%.cpp %.c %.C
.SUFFIXES:
.SUFFIXES:  .c .o .cpp .ecpp .pc .ec .C .cc .cxx

.cpp.o:
	$(CPPCOMPILE) -c -o $*.o $(COMPILEOPTION) $(INCLUDEDIR)  $*.cpp
	
.cc.o:
	$(CCOMPILE) -c -o $*.o $(COMPILEOPTION) $(INCLUDEDIR)  $*.cpp

.cxx.o:
	$(CPPCOMPILE) -c -o $*.o $(COMPILEOPTION) $(INCLUDEDIR)  $*.cpp

.c.o:
	$(CCOMPILE) -c -o $*.o $(COMPILEOPTION) $(INCLUDEDIR) $*.c

.C.o:
	$(CPPCOMPILE) -c -o $*.o $(COMPILEOPTION) $(INCLUDEDIR) $*.C	

.ecpp.C:
	$(ESQL) -e $(ESQL_OPTION) $(INCLUDEDIR) $*.ecpp 
	
.ec.c:
	$(ESQL) -e $(ESQL_OPTION) $(INCLUDEDIR) $*.ec
	
.pc.cpp:
	$(PROC)  CPP_SUFFIX=cpp $(PROC_OPTION)  $*.pc
