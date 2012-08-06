NAME=ut.out
FLAGS= -Wall -pedantic -std=c++11 -pthread -ggdb -g3 -D_GLIBCXX_DEBUG
LIBS=-ljson
HEADERS=-I/usr/local/include

main.o : main.cc
	g++ ${HEADERS} ${FLAGS} -c main.cc

debug : main.o
	g++ ${FLAGS} -o ${NAME} main.o ${LIBS}

clean :
	rm -rf *.o ${NAME}
