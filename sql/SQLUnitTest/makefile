Database.o : ../Database.cc
	g++ -Wall -pedantic -std=c++11 -c ../Database.cc

main.o : main.cc
	g++ -Wall -pedantic -std=c++11 -c main.cc

debug : Database.o main.o
	g++ -Wall -pedantic -std=c++11 Database.o main.o -o ut.out -lmysqlclient
