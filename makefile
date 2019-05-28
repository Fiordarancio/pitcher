## debugging
#CPPFLAGS=-g
#CFLAGS=-O0 -Wall -pedantic -Wno-variadic-macros -Wmaybe-uninitialized -D__PNET_DEBUG__

#release
CPPFLAGS=
CFLAGS=-O3 -Wall -pedantic -Wno-variadic-macros -Wmaybe-uninitialized

LDTHR=-lpthread -lrt
LDLIBS=-lasound -lsndfile -lfftw3 $(LDTHR) -lm

PROGS=ftrain fpitcher fcross
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
fpitcher: autil.o pnet.o pitch.o ptask_time.o mutils.o pnetlib.o
fcross: pnetlib.o pnet.o pitch.o autil.o ptask_time.o mutils.o

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
fcross.o: autil.h pitch.h pnetlib.h pnet.h debug.h ptask_time.h mutils.h
fpitcher.o: pnet.h debug.h autil.h pitch.h ptask_time.h mutils.h pnetlib.h
ftrain.o: pnetlib.h pnet.h debug.h pitch.h
mutils.o: mutils.h
pitch.o: pitch.h
pitcher3.o: pnet.h debug.h pitch.h autil.h ptask_time.h mutils.c mutils.h
pnet.o: pnet.h debug.h
pnetlib.o: pnetlib.h pnet.h debug.h
ptask_time.o: ptask_time.h
train.o: pnetlib.h pnet.h debug.h pitch.h autil.h
wav.o: wav.h autil.h
