CXX       = clang
#OPT       = -O -g
#FLAGS     = $(OPT) -ansi -W -Wall -L.
THINGS =  imagebld.o sha1.o


all: clean image

%.o	: %.c
	${CXX} ${FLAGS} $(EXTRA_CFLAGS) -o $@ -c $<

image: ${THINGS} 
	${CXX} $(EXTRA_CFLAGS) -o $@ ${THINGS} 
	
clean:
	-rm -f *.o  image core
