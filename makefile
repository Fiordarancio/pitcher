# debugging
CPPFLAGS=-g
CFLAGS=-O0 -Wall -pedantic -Wno-variadic-macros -Wmaybe-uninitialized -D__DEBUG_PRINTF__

#release
#CPPFLAGS=
#CFLAGS=-O3 -Wall -pedantic -Wno-variadic-macros -Wmaybe-uninitialized

LDTHR=-lpthread -lrt
LDLIBS=-lasound -lsndfile -lfftw3 $(LDTHR) -lm

PROGS=ftrain fpitcher 
OTHER=pitcher3 pitcher train
LIBS=autil.o wav.o mutils.o pnet.o pnetlib.o pitch.o ptask_time.o trainlib.o

.PHONY: all
all: $(PROGS) 

.PHONY: universe
universe: $(PROGS) $(OTHER)

# BASE PROGRAM
train: pnet.o pitch.o pnetlib.o
ftrain: pnet.o pitch.o pnetlib.o

# BASE PROGRAM
pitcher: autil.o pnet.o pitch.o ptask_time.o mutils.o
fpitcher: autil.o pnet.o pitch.o ptask_time.o mutils.o

# debug capture
pitcher3: autil.o pnet.o pitch.o ptask_time.o

# clean workspace
.PHONY: clean
clean:
	rm -f *~ $(PROGS) $(OTHER) makefile.bak *.o *.dat *.txt

# clean training logs
.PHONY: ct
ct:
	rm -f *~ logs/* *.txt

dep:
	makedepend -Y -- $(CFLAGS) -- *.c

# DO NOT DELETE

autil.o: autil.h
ftrain.o: pnetlib.h pnet.h debug.h pitch.h
mutils.o: mutils.h
pitch.o: pitch.h
pitcher3.o: pnet.h debug.h pitch.h autil.h mutils.c mutils.h
pnet.o: pnet.h debug.h
pnetlib.o: pnetlib.h pnet.h debug.h
train.o: pnetlib.h pnet.h debug.h pitch.h autil.h
wav.o: wav.h autil.h
