#include <FramePostProcessingHandler.hpp>
#include <PostProcessingFaceDetectionLayer.hpp>
#include <boost/type_index.hpp>
#include <iostream>

FramePostProcessingHandler::FramePostProcessingHandler()
{
	std::cout << boost::typeindex::type_id<FramePostProcessingHandler>().pretty_name() << " pipeline instance is created" << std::endl;

	PostProcessLayers.push_back(std::make_unique<PostProcessingFaceDetectionLayer>());
}

FramePostProcessingHandler::~FramePostProcessingHandler()
{
	std::cout << boost::typeindex::type_id<FramePostProcessingHandler>().pretty_name() << " pipeline instance is destroyed" << std::endl;
}

void FramePostProcessingHandler::PostProcessFrame(cv::Mat& InFrame)
{
	for (const auto& Layer : PostProcessLayers)
	{
		if (Layer->IsLayerActive())
		{
			Layer->PostProcessFrame(InFrame);
		}
	}
}