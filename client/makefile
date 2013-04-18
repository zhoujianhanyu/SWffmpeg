

MYINCLUDES=-I/home/zhoujian/bin/ffmpeg/include

MYLIBS=-L/home/zhoujian/bin/ffmpeg/lib -lavutil \
	-lavformat -lavcodec -lswscale -lpthread -lz -lm
	
all:test.o swencodec.o swqueue.o swpacket.o swclient.o
	gcc -o client test.o swencodec.o swqueue.o swpacket.o swclient.o $(MYLIBS)

#test.o:test.c MVencodec.h MVdecodec.h
#	gcc -c test.c $(MYINCLUDES)
test.o:test.c swencodec.h swclient.h swqueue.h swpacket.h
	gcc -c test.c $(MYINCLUDES)

swencodec.o:swencodec.c swqueue.h swpacket.h
	gcc -c swencodec.c $(MYINCLUDES)	

swqueue.o:swqueue.c 
	gcc -c swqueue.c

swpacket.o:swpacket.c
	gcc -c swpacket.c

swclient.o:swclient.c swpacket.h
	gcc -c swclient.c $(MYINCLUDES)

.PHONY:clean

clean:
	-rm *.o client
