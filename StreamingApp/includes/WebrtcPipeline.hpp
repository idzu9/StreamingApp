#ifndef STREAMINGAPP_WEBRTCPIPELINE_HPP
#define STREAMINGAPP_WEBRTCPIPELINE_HPP

#include "Interfaces/IMediaPipeline.hpp"
#include "SingleCastDelegate.hpp"
#include <gst/gst.h>
#include <gst/webrtc/webrtc.h>
#include <gst/app/gstappsink.h>
#include <gst/app/gstappsrc.h>
#include <gst/video/video.h>
#include <gst/video/video-info.h>
#include <mutex>

class WebrtcPipeline : public IMediaPipeline
{
public:
	WebrtcPipeline();

	~WebrtcPipeline();

	virtual void InitializePipeline() override;

	virtual void EnableDebug() const override;

	virtual void CreatePipeline() override;

	virtual void StartPipelinePlaying() override;

	virtual void ProccessTextBuffer(const std::string& TextBuffer);

	Delegate<void, guint, gchar*> OnIceCandidateDelegate;

	Delegate<void, std::string> OnWriteMessageInBuffer;

private:

	static void OnIceCandidate(GstElement* Element, guint Mlineindex, gchar* Candidate, gpointer UserData);

	static void OnSetRemoteDescription(GstPromise* Promise, gpointer UserData);

	static void OnAnswerCreated(GstPromise* Promise, gpointer UserData);

	static GstFlowReturn OnCameraFrameRecieved(GstElement* Sink, gpointer UserData);

	void _CreatePipelineElements();

	void _LinkPipelineElements();

	void _SetElementCapsAndProperties();

	void _ConnectElemetsPads();

	void _SetupSignals();

	/*
	*	Pipeline elements
	*/
	GstElement* Pipeline = nullptr;

	GstElement* Webrtcbin = nullptr;
	
	GstElement* V4l2src = nullptr;
	
	GstElement* Videobalance = nullptr;
	
	GstElement* Videoconvert = nullptr;
	
	GstElement* Decoder = nullptr;
	
	GstElement* Appsink = nullptr;
	
	GstElement* Appsrc = nullptr;
	
	GstElement* VideoconvertFromAppsrc = nullptr;
	
	GstElement* Tee = nullptr;
	
	GstElement* Capsfilter = nullptr;;
	
	GstElement* Fakesink = nullptr;
	
	GstElement* Queue = nullptr;
	
	GstElement* Vp8enc = nullptr;
	
	GstElement* Rtpvp8pay = nullptr;

	static std::once_flag InitFlag;
};

#endif // STREAMINGAPP_WEBRTCPIPELINE_HPP