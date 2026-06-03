#ifndef STREAMINGAPP_WEBRTCPIPELINE_HPP
#define STREAMINGAPP_WEBRTCPIPELINE_HPP

#include "Interfaces/IMediaPipeline.hpp"
#include "SingleCastDelegate.hpp"
#include <gst/app/gstappsink.h>
#include <gst/app/gstappsrc.h>
#include <gst/gst.h>
#include <gst/video/video-info.h>
#include <gst/video/video.h>
#include <gst/webrtc/webrtc.h>
#include <memory>
#include <optional>

class WebrtcPipeline : public IMediaPipeline
{
public:
	WebrtcPipeline() = delete;

	WebrtcPipeline(GstElement* InPipeline, GstElement* InLinkWithProvider);

	virtual ~WebrtcPipeline();

	/*
		IMediaPipeline start
	*/
	virtual void CreatePipeline() override;
	/*
		IMediaPipeline end
	*/

	void ProccessTextBuffer(const std::string& TextBuffer);

	Delegate<void, guint, gchar*> OnIceCandidateDelegate;

	Delegate<void, std::string> OnWriteMessageInBuffer;

private:

	static void OnIceCandidate(GstElement* Element, guint Mlineindex, gchar* Candidate, gpointer UserData);

	static void OnSetRemoteDescription(GstPromise* Promise, gpointer UserData);

	static void OnAnswerCreated(GstPromise* Promise, gpointer UserData);

	/*
		IMediaPipeline start
	*/
	virtual void _CreatePipelineElements() override;

	virtual void _LinkPipelineElements() override;

	virtual void _SetElementCapsAndProperties() override;

	virtual void _SetupSignals() override;

	void _ConnectElemetsPads();
	/*
		IMediaPipeline end
	*/

	/*
	*	Pipeline elements
	*/
	GstElement* Pipeline;

	GstElement* Webrtcbin = nullptr;

	GstElement* LinkWithProvider = nullptr;
	
	GstElement* Vp8enc = nullptr;
	
	GstElement* Rtpvp8pay = nullptr;
};

#endif // STREAMINGAPP_WEBRTCPIPELINE_HPP