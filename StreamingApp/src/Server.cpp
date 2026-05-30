#include "Server.hpp"
#include "WebrtcPipeline.hpp"
#include <boost/beast.hpp>
#include <boost/json.hpp>
#include <boost/type_index.hpp>
#include <iostream>

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = net::ip::tcp;
using namespace boost::json;

Server::Server()
{
	std::cout << boost::typeindex::type_id<Server>().pretty_name() << " instance is created" << std::endl;
}

Server::~Server()
{
	std::cout << boost::typeindex::type_id<Server>().pretty_name() << " instance is destroyed" << std::endl;
	g_main_loop_unref(MainLoop);
}

void Server::CreateMainLoop()
{
	MainLoop = g_main_loop_new(nullptr, false);
}

void Server::CreateMediaPipeline()
{
	MediaPipeline = std::make_unique<WebrtcPipeline>();
}

void Server::StartMainLoop()
{
	std::cout << "[" << __FUNCTION__ << "] is called" << std::endl;

	g_main_loop_run(MainLoop);
}

void Server::StartServer()
{
	ServerThread = std::thread(&Server::_StartServer, this);
	
	StartMainLoop();

	if (ServerThread.joinable())
	{
		ServerThread.join();
	}
}

void Server::_StartServer()
{
	std::cout << "[" << __FUNCTION__ << "] is called" << std::endl;

	try
	{
		/*
			this is the object that handles connections to the operating system's networking features like waiting data on socket
			{1} means we are going to run it on one thread
		*/
		net::io_context IOC{ 1 };

		/*
			this code sets up a Listener. It tells the program to claim a port and wait for other devices to try to connect to it

			acceptor is responsible for accepting incomming connection requests
			tcp::endpoint - defines the address and port componation
			tcp::v4() tells the acceptor to listen tp a;; available IPv4 addresses on my machine

			Socket Creation: It opens a socket in the background.Binding:
			It "locks" the SERVER_PORT so no other app can use it. Listening: It tells the Operating System: "If anyone knocks on this port, let me know."
		*/
		tcp::acceptor Acceptor{ IOC, tcp::endpoint{tcp::v4(), SERVER_PORT} };

		while (true)
		{
			Socket.emplace(IOC);
			Acceptor.accept(Socket.value());

			std::cout << "[" << __FUNCTION__ << "] is waiting for the new TCP connection" << std::endl;

			/*
				this is a blocking operation, until the new connection fires
			*/
			std::cout << "[" << __FUNCTION__ << "] accepted new TCP connection" << std::endl;
			//std::thread{ handle_websocket_session, std::move(socket) }.detach();
		}
	}
	catch (std::exception const& exc)
	{
		std::cerr << "Exception: " << exc.what() << std::endl;
	}
}