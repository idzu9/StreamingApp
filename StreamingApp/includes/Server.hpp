#ifndef STREAMINGAPP_SERVER_HPP
#define STREAMINGAPP_SERVER_HPP

#include <thread>
#include <memory>
#include <optional>
#include <boost/asio.hpp>
#include <glib.h>

#include "Interfaces/IMediaPipeline.hpp"

/*
	stun server and port we are going to work on
*/
#define STUN_SERVER "stun://stun.l.google.com:19302"
#define SERVER_PORT 8443

class Server
{

public:
	Server();
	~Server();

	void CreateMainLoop();
	void CreateMediaPipeline();
	void StartServer();

private:
	/*
		Internal function to run the server
	*/
	void _StartServer();

	void _StartHttpServer() const;

	/*
		Start of the event loop
	*/
	void _StartMainLoop();

	GMainLoop* MainLoop;

	std::thread AppServerThread;

	std::thread HttpServerThread;


	std::unique_ptr<IMediaPipeline> MediaPipeline;

	std::optional<boost::asio::ip::tcp::socket> Socket;
};

#endif // STREAMINGAPP_SERVER_HPP