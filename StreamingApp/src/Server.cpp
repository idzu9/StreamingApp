#include "Server.hpp"
#include "WebrtcPipeline.hpp"
#include <GStreamerWebcamProvider.hpp>
#include <gst/gst.h>
#include <gst/webrtc/webrtc.h>
#include <iostream>
#include <memory>
#include <opencv2/opencv.hpp>
#include <thread>

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

void Server::CreateMediaProvider()
{
	WebcamProvider = std::make_unique<GStreamerWebcamProvider>();

	if (WebcamProvider)
	{
		WebcamProvider->InitializePipeline();
		WebcamProvider->CreatePipeline();
	}
	else
	{
		std::cout << "[" << __FUNCTION__ << "] Failed to create provider" << std::endl;
	}
}

void Server::CreateMediaPipeline()
{
	MediaPipeline = std::make_unique<WebrtcPipeline>(WebcamProvider->GetPipeline(), WebcamProvider->GetElementToConnectTo());

	if (MediaPipeline)
	{
		MediaPipeline->CreatePipeline();
	}
	else
	{
		std::cout << "[" << __FUNCTION__ << "] Failed to create a pipeline" << std::endl;
	}
}

void Server::_StartMainLoop()
{
	std::cout << "[" << __FUNCTION__ << "] is called" << std::endl;

	g_main_loop_run(MainLoop);
}

void Server::StartServer()
{
	HttpServerThread = std::thread(&Server::_StartHttpServer, this);

	HttpServerThread.detach();

	AppServerThread = std::thread(&Server::_StartServer, this);
	
	_StartMainLoop();

	if (!WebcamProvider)
	{
		std::cout << "[" << __FUNCTION__ << "] media provider is not initialized" << std::endl;
	}

	if (!MediaPipeline)
	{
		std::cout << "[" << __FUNCTION__ << "] media pipeline is not initialized" << std::endl;
		return ;
	}

	if (AppServerThread.joinable())
	{
		AppServerThread.join();
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
			tcp::socket LocalSocket{ IOC };
			Acceptor.accept(LocalSocket);

			std::cout << "[" << __FUNCTION__ << "] is waiting for the new TCP connection" << std::endl;

			/*
				this is a blocking operation, until the new connection fires
			*/
			std::cout << "[" << __FUNCTION__ << "] accepted new TCP connection" << std::endl;
			std::thread(&Server::_HandleWebsocketSession, this, std::move(LocalSocket)).detach();
		}
	}
	catch (std::exception const& exc)
	{
		std::cerr << "Exception: " << exc.what() << std::endl;
	}
}

void Server::_StartHttpServer() const
{
	int Result = std::system("python3 -m http.server 9999 -d ../../../../StreamingApp/app/");

	if (Result == 0)
	{
		std::cout << "[" << __FUNCTION__ << "] Success - http server started" << std::endl;
	}
	else
	{
		std::cerr << "[" << __FUNCTION__ << "] could not start http server" << std::endl;
	}
}


void Server::_HandleWebsocketSession(tcp::socket InSocket)
{
	try
	{
		/*
			Layered Architecture: The websocket::stream handles high-level WebSocket logic while delegating actual data transport to the underlying NextLayer
			(in this case, tcp::socket).

			Message-Oriented: Unlike raw TCP, which is a stream of bytes,
			this class provides message-oriented functionality, allowing you to read or write complete WebSocket messages.

			Synchronous and Asynchronous: It supports both blocking (e.g., .read(), .write()) and non-blocking (e.g., .async_read(), .async_write()) operations.

			perform a handshake
		*/

		WebSocket.emplace(std::move(InSocket));
		WebSocket->accept();

		std::cout << "WebSocket connection accepted" << std::endl;

		//MediaPipeline->CreatePipeline();

		//if (Webrtc)
		//{
		//	std::cout << "We are working with not Webrtc pipeline atm, but others are not supported" << std::endl;
		//	return ;
		//}

		MediaPipeline->OnIceCandidateDelegate.BindDelegate(this, &Server::_SendIceCandidateMessage);
		MediaPipeline->OnWriteMessageInBuffer.BindDelegate(this, &Server::_OnWriteMessageInBuffer);

		while (true)
		{
			beast::flat_buffer Buffer;
			WebSocket->read(Buffer);

			std::string Text = beast::buffers_to_string(Buffer.data());
			
			MediaPipeline->ProccessTextBuffer(Text);
		}
	}
	catch (beast::system_error const& Error)
	{
		if (Error.code() != websocket::error::closed)
		{
			std::cerr << "Error: " << Error.code().message() << std::endl;
		}
	}
	catch (std::exception& Exception)
	{
		std::cout << Exception.what() << std::endl;
	}
}

void Server::_SendIceCandidateMessage(guint mlineindex, gchar* candidate)
{
	std::cout << "[" << __FUNCTION__ <<  "] Sending ICE candidate nlineindex " << mlineindex << ", candidate " << candidate << std::endl;

	boost::json::object ice_json;
	ice_json["candidate"] = candidate;
	ice_json["sdpMLineIndex"] = mlineindex;

	boost::json::object msg_json;
	msg_json["type"] = "candidate";
	msg_json["ice"] = ice_json;

	std::string text = serialize(msg_json);
	WebSocket->write(net::buffer(text));

	std::cout << "[" << __FUNCTION__ << "] ICE candidate sent" << std::endl;
}

void Server::_OnWriteMessageInBuffer(std::string Message)
{
	WebSocket->write(boost::asio::buffer(Message));
}