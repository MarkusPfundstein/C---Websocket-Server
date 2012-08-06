#include <thread>
#include <unistd.h>
#include <sstream>
#include <iostream>
#include <thread>
#include <chrono>
#include <sys/wait.h>
#include <sys/signal.h>
#include "Engine.h"
#include "Network/Reactor.h"
#include "Network/MessageQueue.h"
#include "Network/Connection.h"

bool g_keep_running;

static void CatchSigTerm(int sig)
{
	std::cout << "Catched Sigterm signal" << std::endl;
	g_keep_running = false;
	signal(sig, CatchSigTerm);
}

int main(int argc, char **argv)
{
	signal(SIGCHLD, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
	if (signal(SIGTERM, CatchSigTerm) == SIG_IGN) {
		signal(SIGTERM, SIG_IGN);
	}

	int daemon_id = fork();
	if (daemon_id < 0) {
		std::cerr << "Couldn't create daemon" << std::endl;
		return -1;	
	}
	else if (daemon_id > 0) {
		std::cout << "Parent exit" << std::endl;
		return -1;
	}

	if (setsid() < 0) {
		std::cerr << "setsid()" << std::endl;
		return -1;
	}
	std::cout << "Run as daemon" << std::endl;

	engine::Engine engine;

	if (!engine.Run("engine_init.txt")) {
		std::cerr << "Main - Couldn't run engine" << std::endl;
		engine.Stop();
		return -1;
	}
	g_keep_running = true;
	while(g_keep_running) {
		sleep(30);
	}		

	engine.Stop();
	
	std::cout << "bye bye" << std::endl;

	return 0;

}
