# various tests
CPPFLAGS=-g
CFLAGS=-O3 -Wall -pedantic -Wno-variadic-macros -Wmaybe-uninitialized
LDTHR=-lpthread -lrt
LDLIBS=-lasound -lsndfile -lfftw3 $(LDTHR) -lm

TESTS=hello_pow hello_shuf hello_fft hello_testand hello_testand2 hello_testor hello_testxor hello_fftwav hello_testchar
LIBS=../autil.o ../wav.o ../mutils.o ../pnet.o ../pnetlib.o ../pitch.o

.PHONY: all
all: $(TESTS) 

# various tests
hello_fft: ../pitch.o
hello_fftwav: ../wav.o ../autil.o
hello_pow: ../wav.o ../autil.o
hello_shuf: ../mutils.o
hello_testand: ../pnetlib.o ../pnet.o
hello_testand2: ../pnetlib.o ../pnet.o
hello_testor: ../pnetlib.o ../pnet.o
hello_testxor: ../pnetlib.o ../pnet.o
hello_testchar: ../pnetlib.o ../pnet.o

# clean progs
.PHONY: clean
clean:
	rm -f *~ $(TESTS) makefile.bak *.o *.dat *.txt

# clean training logs
.PHONY: ct
ct:
	rm -i *~ logs/*

dep:
	makedepend -Y -- $(CFLAGS) -- *.c

# DO NOT DELETE

hello_fft.o: ../pitch.h
hello_fftwav.o: ../wav.h ../autil.h
hello_pow.o: ../wav.h ../autil.h
hello_shuf.o: ../mutils.h
hello_testand.o: ../pnet.h ../pnetlib.h ../pnet.h
hello_testand2.o: ../pnet.h ../pnetlib.h ../pnet.h
hello_testchar.o: ../pnetlib.h ../pnet.h
hello_testor.o: ../pnet.h ../pnetlib.h ../pnet.h
hello_testxor.o: ../pnet.h ../pnetlib.h ../pnet.h
