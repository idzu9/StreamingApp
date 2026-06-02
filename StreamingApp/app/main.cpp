#include <iostream>
#include <memory>
#include "Server.hpp"
#include "WebrtcPipeline.hpp"

int main(int argc, char* argv[])
{
	/*
		Create a server instance
	*/
	std::unique_ptr<Server> ServerInstance = std::make_unique<Server>();

	/*
		Create loop and pipeline instance (webrtc for now)
	*/
	ServerInstance->CreateMainLoop();
	ServerInstance->CreateMediaProvider();
	ServerInstance->CreateMediaPipeline();
	ServerInstance->StartServer();

	return 0;
}
