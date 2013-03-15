all: meow

meow: libmeow.so
	go build meow.go

libmeow.so: meow.o
	gcc -shared `pkg-config --cflags --libs MagickWand zbar` -o libmeow.so meow.o

meow.o: meow.c
	gcc -c `pkg-config --cflags --libs MagickWand zbar` -fPIC meow.c

clean:
	rm -rf *.o *.so meow
