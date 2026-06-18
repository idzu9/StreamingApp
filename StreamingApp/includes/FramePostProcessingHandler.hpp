#ifndef STREAMINGAPP_FRAMEPOSTPROCESSINGHANDLER_HPP
#define STREAMINGAPP_FRAMEPOSTPROCESSINGHANDLER_HPP

#include <SingleCastDelegate.hpp>
#include <memory>
#include <opencv2/opencv.hpp>
#include <vector>

class PostProcessingLayerBase;

class FramePostProcessingHandler
{
public:
	FramePostProcessingHandler();

	~FramePostProcessingHandler();

	void PostProcessFrame(cv::Mat& InFrame);

private:
	std::vector<std::unique_ptr<PostProcessingLayerBase>> PostProcessLayers;
};

#endif // STREAMINGAPP_FRAMEPOSTPROCESSINGHANDLER_HPP