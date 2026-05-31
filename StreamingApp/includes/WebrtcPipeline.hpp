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

	static void OnIceCandidate(GstElement* Element, guint mlineindex, gchar* candidate, gpointer user_data);

	// TO REFACTOR
	static void on_set_remote_description(GstPromise* promise, gpointer user_data);

	static void on_answer_created(GstPromise* promise, gpointer user_data);

	static GstFlowReturn on_camera_frame_received(GstElement* sink, gpointer user_data);

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