#ifndef STREAMINGAPP_SERVER_HPP
#define STREAMINGAPP_SERVER_HPP

#include <thread>
#include <memory>
#include <optional>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/json.hpp>
#include <boost/type_index.hpp>
#include <glib.h>

//#include "Interfaces/IMediaPipeline.hpp"

class WebrtcPipeline;
class GStreamerWebcamProvider;

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = net::ip::tcp;
using namespace boost::json;

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
	void CreateMediaProvider();
	void CreateMediaPipeline();
	void StartServer();

private:
	/*
		Internal function to run the server
	*/
	void _StartServer();

	void _StartHttpServer() const;

	void _HandleWebsocketSession(tcp::socket Socket);

	void _SendIceCandidateMessage(guint mlineindex, gchar* candidate);

	void _OnWriteMessageInBuffer(std::string Message);

	/*
		Start of the event loop
	*/
	void _StartMainLoop();

	GMainLoop* MainLoop;

	std::thread AppServerThread;

	std::thread HttpServerThread;

	std::optional<websocket::stream<tcp::socket>> WebSocket;

	std::unique_ptr<WebrtcPipeline> MediaPipeline;

	std::unique_ptr<GStreamerWebcamProvider> WebcamProvider;

	std::optional<boost::asio::ip::tcp::socket> Socket;
};

#endif // STREAMINGAPP_SERVER_HPP