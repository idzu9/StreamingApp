#include "WebrtcPipeline.hpp"
#include <iostream>
#include <gst/gst.h>
#include <boost/json.hpp>
#include <opencv2/opencv.hpp>
#include <boost/type_index.hpp>

using namespace boost::json;

WebrtcPipeline::WebrtcPipeline(GstElement* InPipeline, GstElement* InLinkWithProvider) : Pipeline(InPipeline), LinkWithProvider(InLinkWithProvider)
{
	std::cout << boost::typeindex::type_id<WebrtcPipeline>().pretty_name() << " pipeline instance is created" << std::endl;
}

WebrtcPipeline::~WebrtcPipeline()
{
	std::cout << boost::typeindex::type_id<WebrtcPipeline>().pretty_name() << " pipeline instance is destroyed" << std::endl;
}

void WebrtcPipeline::CreatePipeline()
{
	if (!Pipeline || !LinkWithProvider)
	{
		g_printerr("[%s] Has no valid pipeline or link with media provider\n", __FUNCTION__);
		return ;
	}

	_CreatePipelineElements();
	_SetElementCapsAndProperties();
	_LinkPipelineElements();
	_ConnectElemetsPads();
	_SetupSignals();
}

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

void WebrtcPipeline::_LinkPipelineElements()
{
	gst_bin_add_many(GST_BIN(Pipeline), Vp8enc, Rtpvp8pay, Webrtcbin, nullptr);
}

void WebrtcPipeline::_SetElementCapsAndProperties()
{

}

void WebrtcPipeline::_ConnectElemetsPads()
{
	gst_element_link_many(LinkWithProvider, Vp8enc, Rtpvp8pay, nullptr);

	GstPad* Srcpad = gst_element_get_static_pad(Rtpvp8pay, "src");
	GstPad* Sinkpad = gst_element_request_pad_simple(Webrtcbin, "sink_%u");
	gst_pad_link(Srcpad, Sinkpad);
	gst_object_unref(Srcpad);
	gst_object_unref(Sinkpad);
}

void WebrtcPipeline::_SetupSignals()
{
	g_signal_connect(Webrtcbin, "on-ice-candidate", G_CALLBACK(OnIceCandidate), this);
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
