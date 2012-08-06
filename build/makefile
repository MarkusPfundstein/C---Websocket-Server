NAME=ut.out
FLAGS= -Wall -pedantic -std=c++11 -pthread -ggdb -g3 -D_GLIBCXX_DEBUG
LIBS=-lssl -lcrypto -lmysqlclient -ljson
HEADERS=-I/usr/local/include

Database.o : ../sql/Database.cc
	g++ ${FLAGS} -c ../sql/Database.cc

MessageQueue.o : ../Network/MessageQueue.cc
	g++ ${FLAGS} -c ../Network/MessageQueue.cc

Reactor.o : ../Network/Reactor.cc
	g++ ${FLAGS} -c ../Network/Reactor.cc

Socket.o : ../Network/Socket.cc
	g++ ${FLAGS} -c ../Network/Socket.cc

Connection.o : ../Network/Connection.cc
	g++ ${FLAGS} -c ../Network/Connection.cc

ConnectionManager.o : ../Network/ConnectionManager.cc
	g++ ${FLAGS} -c ../Network/ConnectionManager.cc

Base64.o : ../Network/Base64.cc
	g++ -Wall -pedantic -c ../Network/Base64.cc

Engine.o : ../Engine.cc
	g++ ${FLAGS} -c ../Engine.cc

main.o : ../main.cc
	g++ ${FLAGS} -c ../main.cc

debug : Database.o MessageQueue.o Base64.o ConnectionManager.o Connection.o Socket.o Reactor.o Engine.o main.o
	g++ ${FLAGS} -o ${NAME} Database.o MessageQueue.o Base64.o ConnectionManager.o Connection.o Socket.o Reactor.o Engine.o main.o ${LIBS}

clean :
	rm -rf *.o ${NAME}
