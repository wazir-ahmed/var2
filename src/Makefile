CC = gcc
CFLAGS = -Wall
objects = main.o handler.o parser.o utils.o sockb.o cJSON.o storage.o

var2 : $(objects)
	$(CC) -o var2 $(objects) -luv -lm

main.o : var2.h handler.h parser.h utils.h sockb.h
handler.o : handler.h utils.h var2.h parser.h storage.h sockb.h
parser.o : parser.h
utils.o : var2.h utils.h
sockb.o : sockb.h utils.h
cJSON.o : cJSON.h
storage.o : storage.h cJSON.h

.PHONY : clean-all clean-obj
clean-all :
	rm -f var2 $(objects)
clean-obj :
	rm -f $(objects)
