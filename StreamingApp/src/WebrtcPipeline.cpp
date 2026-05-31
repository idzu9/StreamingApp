#include "WebrtcPipeline.hpp"
#include <iostream>
#include <gst/gst.h>
#include <boost/json.hpp>
#include <opencv2/opencv.hpp>
#include <boost/type_index.hpp>

using namespace boost::json;

std::once_flag WebrtcPipeline::InitFlag;

WebrtcPipeline::WebrtcPipeline()
{
	std::cout << boost::typeindex::type_id<WebrtcPipeline>().pretty_name() << " pipeline instance is created" << std::endl;
}

WebrtcPipeline::~WebrtcPipeline()
{
	std::cout << boost::typeindex::type_id<WebrtcPipeline>().pretty_name() << " pipeline instance is destroyed" << std::endl;
}

void WebrtcPipeline::InitializePipeline()
{
	std::call_once(WebrtcPipeline::InitFlag, []()
	{
		gst_init(nullptr, nullptr);
	});
}

void WebrtcPipeline::EnableDebug() const
{
	std::cout << "GST_DEBUG enabled" << std::endl;
	setenv("GST_DEBUG", "*:WARN", 1);
}

void WebrtcPipeline::CreatePipeline()
{
	Pipeline = gst_pipeline_new("pipeline");

	/*
		On Linux systems, it is the standard way to pull live video data from:
		Webcams (USB-connected cameras).
		TV tuner cards.
		Capture cards or built-in camera sensors
	*/
	V4l2src = gst_element_factory_make("v4l2src", "V4l2src");
	Videobalance = gst_element_factory_make("videobalance", "Videobalance");
	Videoconvert = gst_element_factory_make("videoconvert", "Videoconvert");
	Decoder = gst_element_factory_make("jpegdec", "Decoder");
	Tee = gst_element_factory_make("tee", "Tee");

	Appsink = gst_element_factory_make("appsink", "Appsink");
	Appsrc = gst_element_factory_make("appsrc", "Appsrc");
	VideoconvertFromAppsrc = gst_element_factory_make("videoconvert", "VideoconvertFromAppsrc");

	Capsfilter = gst_element_factory_make("capsfilter", "Capsfilter");
	Fakesink = gst_element_factory_make("fakesink", "Fakesink");

	/*
		Without a queue, a GStreamer pipeline typically runs in a single thread
		(the source pushes data directly to the sink). When you add a queue, it acts as a buffer that decouples the elements before it from the elements after it.
		What it does:
			1. Threading: It starts a new thread for the downstream part of the pipeline.
			2. Buffering: It stores data in a memory buffer. If the source is faster than the sink, the queue holds the data until the sink is ready.
			3. Prevents Blocking: It ensures that if one part of the pipeline (like an encoder) is slow, it doesn't freeze the entire pipeline (like the camera capture).

		When you MUST use it:
			1. Multithreading: Whenever you want different parts of your pipeline to run on different CPU cores.
			2. Branching (Tees): If you are splitting one video source to two places (e.g., saving to a file AND streaming via WebSocket), you should put a queue at the start of each branch so one doesn't slow down the other.
			3. Network Streaming: It is essential for absorbing "jitter" (small timing variations) in network speeds.
	*/

	Queue = gst_element_factory_make("queue", "Queue");
	
	/*
		GStreamer element that performs the heavy lifting of compressing raw video into the VP8 format.
		This is the "gold standard" codec for WebRTC because it is royalty-free and supported by every modern web browser (Chrome, Firefox, Safari).
	*/
	Vp8enc = gst_element_factory_make("vp8enc", "Vp8enc");
	Rtpvp8pay = gst_element_factory_make("rtpvp8pay", "Rtpvp8pay");

	/*
		webrtcbin is the "brain" of your WebRTC connection in GStreamer.
		It is an extremely powerful element that handles the complex networking, security, and protocol negotiation required to talk to a web browser.
	*/
	Webrtcbin = gst_element_factory_make("webrtcbin", "Webrtcbin");

	if (!Pipeline || !V4l2src || !Videobalance || !Videoconvert || !Queue || !Vp8enc
		|| !Rtpvp8pay || !Decoder || !Tee || !Fakesink
		|| !Webrtcbin || !Appsink || !Appsrc || !VideoconvertFromAppsrc)
	{
		g_printerr("1. Not all elements could be created\n");
		return;
	}

	/*
		v4l2src - a standard way to capture the data from the webcam in linux system
	*/
	g_object_set(V4l2src, "device", "/dev/video0", "do-timestamp", true, "extra-controls", false, "io-mode", 2, nullptr);

	/*
		set the caps for the jpeg decoder
	*/
	GstCaps* mjpeg_caps = gst_caps_from_string("image/jpeg, width=640, height=480, framerate=30/1");
	g_object_set(Capsfilter, "caps", mjpeg_caps, nullptr);
	gst_caps_unref(mjpeg_caps);

	gst_bin_add_many(GST_BIN(Pipeline), V4l2src, Capsfilter,
		Decoder, Videobalance, Videoconvert, Appsink, Appsrc, VideoconvertFromAppsrc,
		Tee, Fakesink, Queue, Vp8enc, Rtpvp8pay, Webrtcbin, nullptr);

	g_object_set(G_OBJECT(Appsink),
		"emit-signals", true,
		"sync", false,			// DO NOT synchronize frames against a clock here
		"async", false,
		"is-live", true,		// Declare it as a live stream
		"do-timestamp", true,	// CRITICAL: Tells appsink to skip waiting for preroll
		nullptr);

	g_object_set(G_OBJECT(Appsrc),
		"format", GST_FORMAT_TIME,
		"is-live", true,
		"do-timestamp", true,
		"block", false,
		nullptr);

	GstCaps* bgr_caps = gst_caps_new_simple("video/x-raw", "format", G_TYPE_STRING, "BGR", nullptr);

	g_object_set(G_OBJECT(Appsink), "caps", bgr_caps, nullptr);
	gst_caps_unref(bgr_caps);

	if (!gst_element_link_many(V4l2src, Capsfilter, Decoder, Videobalance, Videoconvert, Appsink, nullptr))
	{
		g_printerr("Elements could not be linked\n");
		g_object_unref(Pipeline);
		return;
	}

	if (!gst_element_link_many(Appsrc, VideoconvertFromAppsrc, Tee, nullptr))
	{
		g_printerr("Elements could not be linked 2\n");
		g_object_unref(Pipeline);
		return;
	}

	GstPad* TeePadForFakesink = gst_element_request_pad_simple(Tee, "src_%u");
	GstPad* FakesinkPad = gst_element_get_static_pad(Fakesink, "sink");
	gst_pad_link(TeePadForFakesink, FakesinkPad);
	gst_object_unref(TeePadForFakesink);
	gst_object_unref(FakesinkPad);

	GstPad* TeePadForQueue = gst_element_request_pad_simple(Tee, "src_%u");
	GstPad* SinkPadQueue = gst_element_get_static_pad(Queue, "sink");
	gst_pad_link(TeePadForQueue, SinkPadQueue);
	gst_object_unref(TeePadForQueue);
	gst_object_unref(SinkPadQueue);

	gst_element_link_many(Queue, Vp8enc, Rtpvp8pay, nullptr);

	GstPad* Srcpad = gst_element_get_static_pad(Rtpvp8pay, "src");
	GstPad* Sinkpad = gst_element_request_pad_simple(Webrtcbin, "sink_%u");
	gst_pad_link(Srcpad, Sinkpad);
	gst_object_unref(Srcpad);
	gst_object_unref(Sinkpad);

	g_signal_connect(Webrtcbin, "on-ice-candidate", G_CALLBACK(OnIceCandidate), this);
	g_signal_connect(Appsink, "new-sample", G_CALLBACK(on_camera_frame_received), this);

	StartPipelinePlaying();
}

void WebrtcPipeline::StartPipelinePlaying()
{
	GstStateChangeReturn StateReturn = gst_element_set_state(Pipeline, GST_STATE_PLAYING);
	if (StateReturn == GST_STATE_CHANGE_FAILURE)
	{
		g_printerr("Unable to set the pipelne to the playing state.\n");
		return;
	}

	std::cout << "GStreamer pipeline set state to Playing." << std::endl;
}

void WebrtcPipeline::OnIceCandidate(GstElement* Element, guint Mlineindex, gchar* Candidate, gpointer UserData)
{
	std::cout << "[" << __FUNCTION__ <<  "] ICE candidate generated Mlineindex = " << Mlineindex << ", Candidate = " << Candidate << std::endl;

	WebrtcPipeline* Pipeline = static_cast<WebrtcPipeline*>(UserData);

	Pipeline->OnIceCandidateDelegate.ExecuteIfBound(Mlineindex, Candidate);
}

void WebrtcPipeline::ProccessTextBuffer(const std::string& TextBuffer)
{
	boost::json::value JsonValue = boost::json::parse(TextBuffer);
	boost::json::object JsonObject = JsonValue.as_object();
	std::string type = JsonObject["type"].as_string().c_str();

	if (type == "offer")
	{
		std::cout << "Recieved offer: " << TextBuffer << std::endl;

		/*
			SDP - session description protocol used in WebRTC to establish
			audio/video or data streaming connections between peers
		*/
		std::string SDP = JsonObject["sdp"].as_string().c_str();

		GstSDPMessage* SDPMessage;
		gst_sdp_message_new_from_text(SDP.c_str(), &SDPMessage);
		GstWebRTCSessionDescription* Offer = gst_webrtc_session_description_new(GST_WEBRTC_SDP_TYPE_OFFER, SDPMessage);
		GstPromise* Promise = gst_promise_new_with_change_func(on_set_remote_description, this, nullptr);
		g_signal_emit_by_name(Webrtcbin, "set-remote-description", Offer, Promise);
		gst_webrtc_session_description_free(Offer);
	}
	else if (type == "candidate")
	{
		std::cout << "Received ICE candidate: " << TextBuffer << std::endl;

		object ice = JsonObject["ice"].as_object();
		std::string Candidate = ice["candidate"].as_string().c_str();
		guint SDPMLineIndex = ice["sdpMLineIndex"].as_int64();
		g_signal_emit_by_name(Webrtcbin, "add-ice-candidate", SDPMLineIndex, Candidate.c_str());

		std::cout << "Added ICE candidate" << std::endl;
	}
	//else if (type == "action")
	//{
	//	if (json_object["action"] == "filter_grayscale")
	//	{
	//		server_data.is_grayscale_enabled = !server_data.is_grayscale_enabled;
	//		const float saturation = server_data.is_grayscale_enabled ? 0.f : 1.0f;
	//		g_object_set(G_OBJECT(server_data.videobalance), "saturation", saturation, nullptr);
	//	}
	//}
}

// !!!! REFACTOR
void WebrtcPipeline::on_set_remote_description(GstPromise* Promise, gpointer UserData)
{
	std::cout << "Remote description set, creating answer" << std::endl;

	WebrtcPipeline* Pipeline = static_cast<WebrtcPipeline*>(UserData);

	GstPromise* AnswerPromise = gst_promise_new_with_change_func(WebrtcPipeline::on_answer_created, UserData, nullptr);

	g_signal_emit_by_name(Pipeline->Webrtcbin, "create-answer", nullptr, AnswerPromise);
}

// !!!! REFACTOR
void WebrtcPipeline::on_answer_created(GstPromise* Promise, gpointer UserData)
{
	std::cout << "Answer created" << std::endl;

	WebrtcPipeline* Pipeline = static_cast<WebrtcPipeline*>(UserData);
	GstWebRTCSessionDescription* answer = nullptr;
	const GstStructure* reply = gst_promise_get_reply(Promise);
	gst_structure_get(reply, "answer", GST_TYPE_WEBRTC_SESSION_DESCRIPTION, &answer, nullptr);

	GstPromise* local_promise = gst_promise_new();
	g_signal_emit_by_name(Pipeline->Webrtcbin, "set-local-description", answer, local_promise);

	boost::json::object sdp_json;
	sdp_json["type"] = "answer";
	sdp_json["sdp"] = gst_sdp_message_as_text(answer->sdp);
	std::string text = serialize(sdp_json);
	Pipeline->OnWriteMessageInBuffer.ExecuteIfBound(text);

	std::cout << "Local description set and answer sent: " << text << std::endl;

	gst_webrtc_session_description_free(answer);
}



GstFlowReturn WebrtcPipeline::on_camera_frame_received(GstElement* Sink, gpointer UserData)
{
	WebrtcPipeline* Pipeline = static_cast<WebrtcPipeline*>(UserData);

	std::cout << "HMMMMMMMMMMMMMMMMMMMMMMMMM 0" << std::endl;
	GstMapInfo Map;


	// 1. Pull the sample (Guaranteed to be there instantly because the signal fired)
	GstSample* sample = gst_app_sink_pull_sample(GST_APP_SINK(Sink));
	if (!sample) return GST_FLOW_OK;

	std::cout << "HMMMMMMMMMMMMMMMMMMMMMMMMM 1" << std::endl;

	// 2. Extract buffer
	GstBuffer* buffer = gst_sample_get_buffer(sample);
	if (buffer) {
		// Retain memory reference for appsrc
		gst_buffer_ref(buffer);

		// Map caps dynamically so WebRTC knows the video dimension/format
		GstCaps* caps = gst_sample_get_caps(sample);
		if (caps) {
			gst_app_src_set_caps(GST_APP_SRC(Pipeline->Appsrc), caps);
		}

		GstVideoInfo info;
		if (!gst_video_info_from_caps(&info, caps)) {
			g_printerr("Failed to parse caps to video info\n");
			gst_sample_unref(sample);
			return GST_FLOW_ERROR;
		}

		// 1. Extract resolution

		int width = GST_VIDEO_INFO_WIDTH(&info);
		int height = GST_VIDEO_INFO_HEIGHT(&info);

		buffer = gst_buffer_make_writable(buffer);

		GstMapInfo map;
		if (gst_buffer_map(buffer, &map, GST_MAP_READWRITE)) {

			// map.data points to the raw pixel data (e.g., BGR, RGB, YUV)
			// map.size gives the total size of the frame in bytes
			// --- MANIPULATE THE BUFFER HERE ---
			// Example: Invert all the bits/pixels in the frame (for demonstration)
/*			for (gsize i = 0; i < map.size; i++) {
				map.data[i] = ~map.data[i];
			}*/

			cv::Mat frame(cv::Size(width, height), CV_8UC3, (void*)map.data, cv::Mat::AUTO_STEP);

			cv::flip(frame, frame, 1);

			// 4. Burn the text onto the raw buffer memory
			cv::putText(frame,
				"WebRTC Stream Live",
				cv::Point(10, 15),
				cv::FONT_HERSHEY_SIMPLEX,
				0.6,
				cv::Scalar(0, 255, 0), // Red text for BGR
				1,
				cv::FILLED);

			// 4. Unmap the buffer after manipulation
			gst_buffer_unmap(buffer, &map);
		}


		// 3. Immediately push it into appsrc
		GstFlowReturn ret = gst_app_src_push_buffer(GST_APP_SRC(Pipeline->Appsrc), buffer);

		if (ret != GST_FLOW_OK) {
			// If downstream WebRTC rejected it or is flushing, unref to prevent memory leak
			gst_buffer_unref(buffer);
		}
	}

	// 4. Free the sample wrapper
	gst_sample_unref(sample);
	return GST_FLOW_OK;
}
