#include "WebrtcPipeline.hpp"
#include <iostream>
#include <gst/gst.h>
#include <boost/json.hpp>
#include <opencv2/opencv.hpp>
#include <boost/type_index.hpp>

using namespace boost::json;

//std::once_flag WebrtcPipeline::InitFlag;

WebrtcPipeline::WebrtcPipeline()
{
	std::cout << boost::typeindex::type_id<WebrtcPipeline>().pretty_name() << " pipeline instance is created" << std::endl;
}

WebrtcPipeline::~WebrtcPipeline()
{
	std::cout << boost::typeindex::type_id<WebrtcPipeline>().pretty_name() << " pipeline instance is destroyed" << std::endl;
}

//void WebrtcPipeline::InitializePipeline()
//{
//	std::call_once(WebrtcPipeline::InitFlag, []()
//	{
//		gst_init(nullptr, nullptr);
//	});
//}

//void WebrtcPipeline::EnableDebug() const
//{
//	std::cout << "GST_DEBUG enabled" << std::endl;
//	setenv("GST_DEBUG", "*:WARN", 1);
//}

//void WebrtcPipeline::CreatePipeline()
//{
//	//_CreatePipelineElements();
//	//_SetElementCapsAndProperties();
//	//_LinkPipelineElements();
//	//_ConnectElemetsPads();
//	//_SetupSignals();
//	//StartPipelinePlaying();
//}

void WebrtcPipeline::_CreatePipelineElements()
{
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

	if (!Vp8enc || !Rtpvp8pay|| !Webrtcbin)
	{
		g_printerr("[%s] Not all elements could be created\n", __FUNCTION__);
		return;
	}
}

void WebrtcPipeline::_LinkPipelineElements(GstElement* Pipeline)
{
	gst_bin_add_many(GST_BIN(Pipeline), Vp8enc, Rtpvp8pay, Webrtcbin, nullptr);

	//if (!gst_element_link_many(V4l2src, Capsfilter, Decoder, Videobalance, Videoconvert, Appsink, nullptr))
	//{
	//	g_printerr("Elements could not be linked\n");
	//	g_object_unref(Pipeline);
	//	return;
	//}

	//if (!gst_element_link_many(Appsrc, VideoconvertFromAppsrc, Tee, nullptr))
	//{
	//	g_printerr("Elements could not be linked 2\n");
	//	g_object_unref(Pipeline);
	//	return;
	//}
}

void WebrtcPipeline::_SetElementCapsAndProperties()
{
	/*
		v4l2src - a standard way to capture the data from the webcam in linux system
	*/
	//g_object_set(V4l2src, "device", "/dev/video0", "do-timestamp", true, "extra-controls", false, "io-mode", 2, nullptr);

	///*
	//	set the caps for the jpeg decoder
	//*/
	//GstCaps* mjpeg_caps = gst_caps_from_string("image/jpeg, width=640, height=480, framerate=30/1");
	//g_object_set(Capsfilter, "caps", mjpeg_caps, nullptr);
	//gst_caps_unref(mjpeg_caps);

	//g_object_set(G_OBJECT(Appsink),
	//	"emit-signals", true,
	//	"sync", false,
	//	"async", false,
	//	nullptr);

	//g_object_set(G_OBJECT(Appsrc),
	//	"format", GST_FORMAT_TIME,
	//	"is-live", true,
	//	"do-timestamp", true,
	//	"block", false,
	//	nullptr);

	//GstCaps* bgr_caps = gst_caps_new_simple("video/x-raw", "format", G_TYPE_STRING, "BGR", nullptr);

	//g_object_set(G_OBJECT(Appsink), "caps", bgr_caps, nullptr);
	//gst_caps_unref(bgr_caps);
}

void WebrtcPipeline::_ConnectElemetsPads(GstElement* ElementToConnect)
{
	gst_element_link_many(ElementToConnect, Vp8enc, Rtpvp8pay, nullptr);

	GstPad* Srcpad = gst_element_get_static_pad(Rtpvp8pay, "src");
	GstPad* Sinkpad = gst_element_request_pad_simple(Webrtcbin, "sink_%u");
	gst_pad_link(Srcpad, Sinkpad);
	gst_object_unref(Srcpad);
	gst_object_unref(Sinkpad);
}

void WebrtcPipeline::_SetupSignals()
{
	g_signal_connect(Webrtcbin, "on-ice-candidate", G_CALLBACK(OnIceCandidate), this);
	//g_signal_connect(Appsink, "new-sample", G_CALLBACK(OnCameraFrameRecieved), this);
}

//void WebrtcPipeline::StartPipelinePlaying()
//{
//	GstStateChangeReturn StateReturn = gst_element_set_state(Pipeline, GST_STATE_PLAYING);
//	if (StateReturn == GST_STATE_CHANGE_FAILURE)
//	{
//		g_printerr("Unable to set the pipelne to the playing state.\n");
//		return;
//	}
//
//	std::cout << "GStreamer pipeline set state to Playing." << std::endl;
//}

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
		std::cout << "[" << __FUNCTION__ << "] Recieved offer: " << TextBuffer << std::endl;

		/*
			SDP - session description protocol used in WebRTC to establish
			audio/video or data streaming connections between peers
		*/
		std::string SDP = JsonObject["sdp"].as_string().c_str();

		GstSDPMessage* SDPMessage;
		gst_sdp_message_new_from_text(SDP.c_str(), &SDPMessage);
		GstWebRTCSessionDescription* Offer = gst_webrtc_session_description_new(GST_WEBRTC_SDP_TYPE_OFFER, SDPMessage);
		GstPromise* Promise = gst_promise_new_with_change_func(OnSetRemoteDescription, this, nullptr);
		g_signal_emit_by_name(Webrtcbin, "set-remote-description", Offer, Promise);
		gst_webrtc_session_description_free(Offer);
	}
	else if (type == "candidate")
	{
		std::cout << "[" << __FUNCTION__ << "] Received ICE candidate: " << TextBuffer << std::endl;

		object ice = JsonObject["ice"].as_object();
		std::string Candidate = ice["candidate"].as_string().c_str();
		guint SDPMLineIndex = ice["sdpMLineIndex"].as_int64();
		g_signal_emit_by_name(Webrtcbin, "add-ice-candidate", SDPMLineIndex, Candidate.c_str());

		std::cout << "[" << __FUNCTION__ << "] Added ICE candidate" << std::endl;
	}
}

void WebrtcPipeline::OnSetRemoteDescription(GstPromise* Promise, gpointer UserData)
{
	std::cout << "[" << __FUNCTION__ << "] Remote description set, creating answer" << std::endl;

	WebrtcPipeline* Pipeline = static_cast<WebrtcPipeline*>(UserData);
	GstPromise* AnswerPromise = gst_promise_new_with_change_func(WebrtcPipeline::OnAnswerCreated, UserData, nullptr);
	g_signal_emit_by_name(Pipeline->Webrtcbin, "create-answer", nullptr, AnswerPromise);
}

void WebrtcPipeline::OnAnswerCreated(GstPromise* Promise, gpointer UserData)
{
	std::cout << "[" << __FUNCTION__ << "] Answer created" << std::endl;

	WebrtcPipeline* Pipeline = static_cast<WebrtcPipeline*>(UserData);
	GstWebRTCSessionDescription* Answer = nullptr;
	const GstStructure* Reply = gst_promise_get_reply(Promise);
	gst_structure_get(Reply, "answer", GST_TYPE_WEBRTC_SESSION_DESCRIPTION, &Answer, nullptr);

	GstPromise* LocalPromise = gst_promise_new();
	g_signal_emit_by_name(Pipeline->Webrtcbin, "set-local-description", Answer, LocalPromise);

	boost::json::object SdpJson;
	SdpJson["type"] = "answer";
	SdpJson["sdp"] = gst_sdp_message_as_text(Answer->sdp);
	std::string TextBuffer = serialize(SdpJson);
	Pipeline->OnWriteMessageInBuffer.ExecuteIfBound(TextBuffer);

	std::cout << "[" << __FUNCTION__ << "] Local description set and answer sent: " << TextBuffer << std::endl;

	gst_webrtc_session_description_free(Answer);
}

//GstFlowReturn WebrtcPipeline::OnCameraFrameRecieved(GstElement* Sink, gpointer UserData)
//{
//	WebrtcPipeline* Pipeline = static_cast<WebrtcPipeline*>(UserData);
//	GstMapInfo Map;
//
//	GstSample* Sample = gst_app_sink_pull_sample(GST_APP_SINK(Sink));
//	if (!Sample)
//	{
//		return GST_FLOW_OK;
//	}
//
//	GstBuffer* Buffer = gst_sample_get_buffer(Sample);
//	if (Buffer)
//	{
//		gst_buffer_ref(Buffer);
//
//		GstCaps* caps = gst_sample_get_caps(Sample);
//		if (caps)
//		{
//			gst_app_src_set_caps(GST_APP_SRC(Pipeline->Appsrc), caps);
//		}
//
//		GstVideoInfo Info;
//		if (!gst_video_info_from_caps(&Info, caps))
//		{
//			g_printerr("Failed to parse caps to video info\n");
//			gst_sample_unref(Sample);
//			return GST_FLOW_ERROR;
//		}
//
//		int Width = GST_VIDEO_INFO_WIDTH(&Info);
//		int Height = GST_VIDEO_INFO_HEIGHT(&Info);
//
//		Buffer = gst_buffer_make_writable(Buffer);
//		if (gst_buffer_map(Buffer, &Map, GST_MAP_READWRITE))
//		{
//
//			cv::Mat frame(cv::Size(Width, Height), CV_8UC3, (void*)Map.data, cv::Mat::AUTO_STEP);
//
//			cv::flip(frame, frame, 1);
//
//			cv::putText(frame,
//				"WebRTC Stream Live",
//				cv::Point(10, 15),
//				cv::FONT_HERSHEY_SIMPLEX,
//				0.6,
//				cv::Scalar(0, 255, 0),
//				1,
//				cv::FILLED);
//
//			gst_buffer_unmap(Buffer, &Map);
//		}
//
//		GstFlowReturn FlowReturn = gst_app_src_push_buffer(GST_APP_SRC(Pipeline->Appsrc), Buffer);
//		if (FlowReturn != GST_FLOW_OK)
//		{
//			gst_buffer_unref(Buffer);
//		}
//	}
//
//	gst_sample_unref(Sample);
//
//	return GST_FLOW_OK;
//}
