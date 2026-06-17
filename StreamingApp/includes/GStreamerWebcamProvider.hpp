#ifndef STREAMINGAPP_GSTREAMERWEBCAMPROVIDER_HPP
#define STREAMINGAPP_GSTREAMERWEBCAMPROVIDER_HPP

#include <Interfaces/IMediaPipeline.hpp>
#include <gst/gst.h>
#include <mutex>

class GStreamerWebcamProvider : public IMediaPipeline
{
public:
	GStreamerWebcamProvider();

	~GStreamerWebcamProvider();

	void InitializePipeline();

	/*
		IMediaPipeline start
	*/
	virtual void CreatePipeline() override;
	/*
		IMediaPipeline end
	*/

	GstElement*  GetPipeline() const { return Pipeline; }

	GstElement* GetElementToConnectToWebrtc() const { return WebrtcQueue; }

	GstElement* GetElementToConnectToAutovideo() const { return AutovideoQueue; }

	void _StartPipelinePlaying()const;

private:
	static GstFlowReturn _OnCameraFrameRecieved(GstElement* Sink, gpointer UserData);

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

	void _EnableDebug() const;

	/*
	*	Pipeline elements
	*/
	GstElement* Pipeline = nullptr;

	GstElement* WebrtcQueue = nullptr;

	GstElement* AutovideoQueue = nullptr;

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

	static std::once_flag InitFlag;
};

#endif // STREAMINGAPP_GSTREAMERWEBCAMPROVIDER_HPP