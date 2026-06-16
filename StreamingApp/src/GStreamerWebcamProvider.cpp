#include <GStreamerWebcamProvider.hpp>

#include <boost/type_index.hpp>
#include <gst/app/gstappsink.h>
#include <gst/app/gstappsrc.h>
#include <gst/video/video-info.h>
#include <gst/video/video.h>
#include <iostream>
#include <mutex>
#include <opencv2/opencv.hpp>

std::once_flag GStreamerWebcamProvider::InitFlag;

GStreamerWebcamProvider::GStreamerWebcamProvider()
{
	std::cout << boost::typeindex::type_id<GStreamerWebcamProvider>().pretty_name() << " pipeline instance is created" << std::endl;
}

GStreamerWebcamProvider::~GStreamerWebcamProvider()
{
	std::cout << boost::typeindex::type_id<GStreamerWebcamProvider>().pretty_name() << " pipeline instance is destroyed" << std::endl;
}

void GStreamerWebcamProvider::InitializePipeline()
{
	std::call_once(GStreamerWebcamProvider::InitFlag, []()
	{
		gst_init(nullptr, nullptr);
	});
}

void GStreamerWebcamProvider::_EnableDebug() const
{
	std::cout << "GST_DEBUG enabled" << std::endl;
	setenv("GST_DEBUG", "*:WARN", 1);
}

void GStreamerWebcamProvider::CreatePipeline()
{
	_EnableDebug();

	_CreatePipelineElements();
	_SetElementCapsAndProperties();
	_LinkPipelineElements();
	_ConnectElemetsPads();
	_SetupSignals();

	_StartPipelinePlaying();
}


void GStreamerWebcamProvider::_CreatePipelineElements()
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

	if (!Pipeline || !V4l2src || !Videobalance || !Videoconvert || !Queue || !Decoder
		|| !Tee || !Fakesink || !Appsink || !Appsrc || !VideoconvertFromAppsrc)
	{
		g_printerr("[%s] Not all elements could be created\n", __FUNCTION__);
		return;
	}
}

void GStreamerWebcamProvider::_LinkPipelineElements()
{
	gst_bin_add_many(GST_BIN(Pipeline), V4l2src, Capsfilter,
		Decoder, Videobalance, Videoconvert, Appsink, Appsrc, VideoconvertFromAppsrc,
		Tee, Fakesink, Queue, nullptr);

	if (!gst_element_link_many(V4l2src, Capsfilter, Decoder, Videobalance, Videoconvert, Appsink, nullptr))
	{
		g_printerr("[%s] Elements could not be linked\n", __FUNCTION__);
		g_object_unref(Pipeline);
		return;
	}

	if (!gst_element_link_many(Appsrc, VideoconvertFromAppsrc, Tee, nullptr))
	{
		g_printerr("[%s] Elements could not be linked\n", __FUNCTION__);
		g_object_unref(Pipeline);
		return;
	}
}

void GStreamerWebcamProvider::_SetElementCapsAndProperties()
{
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

	g_object_set(G_OBJECT(Appsink),
		"emit-signals", true,
		"sync", false,
		"async", false,
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
}

void GStreamerWebcamProvider::_ConnectElemetsPads()
{
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
}

void GStreamerWebcamProvider::_SetupSignals()
{
	g_signal_connect(Appsink, "new-sample", G_CALLBACK(_OnCameraFrameRecieved), this);
}

GstFlowReturn GStreamerWebcamProvider::_OnCameraFrameRecieved(GstElement* Sink, gpointer UserData)
{
	GStreamerWebcamProvider* WebcamProvider = static_cast<GStreamerWebcamProvider*>(UserData);
	GstMapInfo Map;

	GstSample* Sample = gst_app_sink_pull_sample(GST_APP_SINK(Sink));
	if (!Sample)
	{
		return GST_FLOW_OK;
	}

	GstBuffer* Buffer = gst_sample_get_buffer(Sample);
	if (Buffer)
	{
		gst_buffer_ref(Buffer);

		GstCaps* caps = gst_sample_get_caps(Sample);
		if (caps)
		{
			gst_app_src_set_caps(GST_APP_SRC(WebcamProvider->Appsrc), caps);
		}

		GstVideoInfo Info;
		if (!gst_video_info_from_caps(&Info, caps))
		{
			g_printerr("[%s] Failed to parse caps to video info\n", __FUNCTION__);
			gst_sample_unref(Sample);
			return GST_FLOW_ERROR;
		}

		int Width = GST_VIDEO_INFO_WIDTH(&Info);
		int Height = GST_VIDEO_INFO_HEIGHT(&Info);

		Buffer = gst_buffer_make_writable(Buffer);
		if (gst_buffer_map(Buffer, &Map, GST_MAP_READWRITE))
		{

			cv::Mat Frame(cv::Size(Width, Height), CV_8UC3, (void*)Map.data, cv::Mat::AUTO_STEP);

			/*
				Flip the frame
			*/
			cv::flip(Frame, Frame, 1);

			/*
				Face detection part
			*/
			std::string cascade_path = "../../../../StreamingApp/resources/haarcascades/haarcascade_frontalface_default.xml";

			cv::CascadeClassifier faceCascade;
			if (!faceCascade.load(cascade_path))
			{
				std::cerr << "Error: Could not load cascade classifier XML file." << std::endl;
				return GST_FLOW_OK;
			}

			std::vector<cv::Rect> faces;
			faceCascade.detectMultiScale(Frame, faces, 1.1, 4, 0, cv::Size(30, 30));

			cv::Mat BluredFrame;
			cv::GaussianBlur(Frame, BluredFrame, cv::Size(51, 51), 0);



			cv::Mat background = cv::imread("../../../../StreamingApp/resources/backgrounds/background.png");

			cv::Mat ResizedBackground;
			cv::resize(background, ResizedBackground, Frame.size());

			cv::Mat FrameCopy = Frame.clone();

			ResizedBackground.copyTo(Frame);

			for (const auto& face : faces)
			{
				cv::rectangle(Frame, face, cv::Scalar(0, 255, 0), 3);


				/*
					1. Blur only face
				*/
				//cv::Mat faceROI = FrameCopy(face);
				//cv::GaussianBlur(faceROI, faceROI, cv::Size(51, 51), 0);
				
				/*
					2. Blur everything except face
				*/
				//cv::Mat faceROI = Frame(face);
				//cv::Mat targetROI = BluredFrame(face);
				//faceROI.copyTo(targetROI);
				//BluredFrame.copyTo(Frame);
				//faceROI.copyTo(Frame);

				/*
					put my face on top of background
				*/
				cv::Mat faceROI = FrameCopy(face);
				cv::Mat targetROI = Frame(face);
				faceROI.copyTo(targetROI);
			}



			cv::putText(Frame,
				"WebRTC Stream Live",
				cv::Point(10, 15),
				cv::FONT_HERSHEY_SIMPLEX,
				0.6,
				cv::Scalar(0, 255, 0),
				1,
				cv::FILLED);

			// Apply a Bilateral Filter to remove noise while keeping edges sharp
			//cv::bilateralFilter(Frame, ProcessedFrame, 9, 75, 75);

			// Convert to a more accurate color space if streaming over networks
			//cv::cvtColor(ProcessedFrame, ProcessedFrame, cv::COLOR_BGR2YCrCb);

			//cv::fastNlMeansDenoisingColored(Frame, Frame);

			//Frame = ProcessedFrame;

			gst_buffer_unmap(Buffer, &Map);
		}

		GstFlowReturn FlowReturn = gst_app_src_push_buffer(GST_APP_SRC(WebcamProvider->Appsrc), Buffer);
		if (FlowReturn != GST_FLOW_OK)
		{
			gst_buffer_unref(Buffer);
		}
	}

	gst_sample_unref(Sample);

	return GST_FLOW_OK;
}

void GStreamerWebcamProvider::_StartPipelinePlaying() const
{
	GstStateChangeReturn StateReturn = gst_element_set_state(Pipeline, GST_STATE_PLAYING);
	if (StateReturn == GST_STATE_CHANGE_FAILURE)
	{
		g_printerr("[%s] Unable to set the pipelne to the playing state\n", __FUNCTION__);
		return;
	}

	std::cout << "[" << __FUNCTION__  << "] GStreamer pipeline set state to Playing." << std::endl;
}