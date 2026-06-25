#ifndef STREAMINGAPP_GSTREAMERWEBCAMPROVIDER_HPP
#define STREAMINGAPP_GSTREAMERWEBCAMPROVIDER_HPP

#include <Interfaces/IMediaPipeline.hpp>
#include <SingleCastDelegate.hpp>
#include <gst/gst.h>
#include <mutex>
#include <opencv2/opencv.hpp>
#include <vector>

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

	GstElement* RequestQueue();

	void StartPipelinePlaying() const;

	/*
		Frame processing delegate
	*/
	Delegate<void, cv::Mat&> PostProcessFrameDelegate;

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

	void _AddQueueToPieplineAndLink(GstElement* InQueue);

	void _SetQueueProperties(GstElement* InQueue);

	/*
	*	Pipeline elements
	*/
	GstElement* Pipeline = nullptr;

	std::vector<GstElement*> Queues;

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