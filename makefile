CC = gcc
CFFLAGS = -Wall

INCLUDE = source/include
SRC = source
LOL = del


all : build_realase doc

build : bin obj obj/debug bin/psort_dbg
build_realase : bin obj obj/realase bin/psort
doc: Doxyfile creatdoc
test: build_realase runtest
	./runtest

creatdoc: 
	doxygen Doxyfile && cd latex && make

obj:
	mkdir -p obj
obj/debug: obj
	mkdir -p obj/debug
obj/realase: obj
	mkdir -p obj/realase
bin:
	mkdir -p bin

bin/psort_dbg : obj/debug/manager.o obj/debug/partitioner.o obj/debug/sorter.o obj/debug/merger.o obj/debug/log.o obj/debug/pserror.o
	$(CC) obj/debug/manager.o obj/debug/partitioner.o obj/debug/sorter.o obj/debug/merger.o obj/debug/log.o obj/debug/pserror.o $(CFFLAGS) -o bin/psort_dbg -g
obj/debug/manager.o : $(SRC)/manager.c $(INCLUDE)/pserror.h $(INCLUDE)/log.h $(INCLUDE)/psortlib.h
	$(CC) $(SRC)/manager.c $(CFFLAGS) -o obj/debug/manager.o -c -I $(INCLUDE) -g
obj/debug/partitioner.o : $(SRC)/partitioner.c $(INCLUDE)/pserror.h $(INCLUDE)/log.h $(INCLUDE)/psortlib.h
	$(CC) $(SRC)/partitioner.c $(CFFLAGS) -o obj/debug/partitioner.o -c -I $(INCLUDE) -g
obj/debug/sorter.o : $(SRC)/sorter.c $(INCLUDE)/pserror.h $(INCLUDE)/log.h $(INCLUDE)/psortlib.h
	$(CC) $(SRC)/sorter.c $(CFFLAGS) -o obj/debug/sorter.o -c -I $(INCLUDE) -g
obj/debug/merger.o : $(SRC)/merger.c $(INCLUDE)/pserror.h $(INCLUDE)/log.h $(INCLUDE)/psortlib.h
	$(CC) $(SRC)/merger.c $(CFFLAGS) -o obj/debug/merger.o -c -I $(INCLUDE) -g
obj/debug/log.o : $(SRC)/log.c $(INCLUDE)/log.h
	$(CC) $(SRC)/log.c $(CFFLAGS) -o obj/debug/log.o -c -I $(INCLUDE) -g
obj/debug/pserror.o : $(SRC)/pserror.c $(INCLUDE)/pserror.h
	$(CC) $(SRC)/pserror.c $(CFFLAGS) -o obj/debug/pserror.o -c -I $(INCLUDE) -g

bin/psort : obj/realase/manager.o obj/realase/partitioner.o obj/realase/sorter.o obj/realase/merger.o obj/realase/log.o obj/realase/pserror.o
	$(CC) obj/realase/manager.o obj/realase/partitioner.o obj/realase/sorter.o obj/realase/merger.o obj/realase/log.o obj/realase/pserror.o $(CFFLAGS) -o bin/psort -O3
obj/realase/manager.o : $(SRC)/manager.c $(INCLUDE)/pserror.h $(INCLUDE)/log.h $(INCLUDE)/psortlib.h
	$(CC) $(SRC)/manager.c $(CFFLAGS) -o obj/realase/manager.o -c -I $(INCLUDE) -O3
obj/realase/partitioner.o : $(SRC)/partitioner.c $(INCLUDE)/pserror.h $(INCLUDE)/log.h $(INCLUDE)/psortlib.h
	$(CC) $(SRC)/partitioner.c $(CFFLAGS) -o obj/realase/partitioner.o -c -I $(INCLUDE) -O3
obj/realase/sorter.o : $(SRC)/sorter.c $(INCLUDE)/pserror.h $(INCLUDE)/log.h $(INCLUDE)/psortlib.h
	$(CC) $(SRC)/sorter.c $(CFFLAGS) -o obj/realase/sorter.o -c -I $(INCLUDE) -O3
obj/realase/merger.o : $(SRC)/merger.c $(INCLUDE)/pserror.h $(INCLUDE)/log.h $(INCLUDE)/psortlib.h
	$(CC) $(SRC)/merger.c $(CFFLAGS) -o obj/realase/merger.o -c -I $(INCLUDE) -O3
obj/realase/log.o : $(SRC)/log.c $(INCLUDE)/log.h
	$(CC) $(SRC)/log.c $(CFFLAGS) -o obj/realase/log.o -c -I $(INCLUDE) -O3
obj/realase/pserror.o : $(SRC)/pserror.c $(INCLUDE)/pserror.h
	$(CC) $(SRC)/pserror.c $(CFFLAGS) -o obj/realase/pserror.o -c -I $(INCLUDE) -O3

clean:
	rm -r bin obj latex html
