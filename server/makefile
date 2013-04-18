

MYINCLUDES=-I/home/zhoujian/bin/ffmpeg/include

MYLIBS=-L/home/zhoujian/bin/ffmpeg/lib -lavutil \
	-lavformat -lavcodec -lswscale -lpthread -lz -lm
	
all:test.o swpacket.o swserver.o swaviwrite.o
	gcc -o server test.o swpacket.o swserver.o swaviwrite.o $(MYLIBS)

test.o:test.c swaviwrite.h swserver.h swpacket.h
	gcc -c test.c $(MYINCLUDES)
	
swserver.o:swserver.c 
	gcc -c swserver.c

swpacket.o:swpacket.c
	gcc -c swpacket.c

swaviwrite.o:swaviwrite.c swpacket.h
	gcc -c swaviwrite.c $(MYINCLUDES)

.PHONY:clean

clean:
	-rm *.o server


