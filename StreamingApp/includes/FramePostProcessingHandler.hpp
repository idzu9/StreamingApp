#ifndef STREAMINGAPP_FRAMEPOSTPROCESSINGHANDLER_HPP
#define STREAMINGAPP_FRAMEPOSTPROCESSINGHANDLER_HPP

#include <SingleCastDelegate.hpp>
#include <opencv2/opencv.hpp>

class FramePostProcessingHandler
{
public:
	FramePostProcessingHandler();

	~FramePostProcessingHandler();

	void PostProcessFrame(cv::Mat& InFrame);
};

#endif // STREAMINGAPP_FRAMEPOSTPROCESSINGHANDLER_HPP