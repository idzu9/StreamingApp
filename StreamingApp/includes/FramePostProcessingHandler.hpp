#ifndef STREAMINGAPP_FRAMEPOSTPROCESSINGHANDLER_HPP
#define STREAMINGAPP_FRAMEPOSTPROCESSINGHANDLER_HPP

#include <SingleCastDelegate.hpp>
#include <map>
#include <memory>
#include <opencv2/opencv.hpp>
#include <string>

class PostProcessingLayerBase;

class FramePostProcessingHandler
{
public:
	FramePostProcessingHandler();

	~FramePostProcessingHandler();

	void PostProcessFrame(cv::Mat& InFrame);

	void ToggleLayerByName(const std::string& InLayerName);

private:
	std::map<std::string, std::unique_ptr<PostProcessingLayerBase>> PostProcessLayers;

	void FeedOtherLayersWithFaceDecetionData();
};

#endif // STREAMINGAPP_FRAMEPOSTPROCESSINGHANDLER_HPP