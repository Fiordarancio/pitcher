CPPFLAGS=-g
CFLAGS=-O3 -Wall -pedantic -Wno-variadic-macros -Wmaybe-uninitialized
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
fpitcher.o: pnet.h autil.h pitch.h ptask_time.h mutils.h
ftrain.o: pnetlib.h pnet.h pitch.h
mutils.o: mutils.h
pitch.o: pitch.h
pitcher.o: autil.h ptask_time.h
pitcher3.o: pnet.h pitch.h autil.h ptask_time.h mutils.c mutils.h
pnet.o: pnet.h
pnetlib.o: pnetlib.h pnet.h
ptask_time.o: ptask_time.h
train.o: pnetlib.h pnet.h pitch.h autil.h
wav.o: wav.h autil.h
