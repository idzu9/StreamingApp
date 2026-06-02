#ifndef STREAMINGAPP_GSTREAMERWEBCAMPROVIDER_HPP
#define STREAMINGAPP_GSTREAMERWEBCAMPROVIDER_HPP

#include <gst/gst.h>
#include <mutex>

class GStreamerWebcamProvider
{
public:
	GStreamerWebcamProvider();

	~GStreamerWebcamProvider();

	void InitializePipeline();

	void EnableDebug() const;

	void CreatePipeline();

	void StartPipelinePlaying();

	GstElement* Pipeline = nullptr;

	GstElement* Queue = nullptr;

private:
	static GstFlowReturn _OnCameraFrameRecieved(GstElement* Sink, gpointer UserData);

	void _CreatePipelineElements();

	void _LinkPipelineElements();

	void _SetElementCapsAndProperties();

	void _ConnectElemetsPads();

	void _SetupSignals();

	/*
	*	Pipeline elements
	*/

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