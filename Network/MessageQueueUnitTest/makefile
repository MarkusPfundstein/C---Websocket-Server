NAME=ut.out
FLAGS= -Wall -pedantic -std=c++11 -pthread

MessageQueue.o : ../MessageQueue.cc
	g++ ${FLAGS} -c ../MessageQueue.cc

main.o : main.cc
	g++ ${FLAGS} -c main.cc

debug : MessageQueue.o main.o
	g++ -pthread -o ${NAME} MessageQueue.o main.o

clean :
	rm -rf *.o ${NAME}
