#include <FramePostProcessingHandler.hpp>
#include <PostProcessingFaceDetectionLayer.hpp>
#include <PostProcessingFrameDenoisingLayer.hpp>
#include <boost/type_index.hpp>
#include <iostream>

FramePostProcessingHandler::FramePostProcessingHandler()
{
	std::cout << boost::typeindex::type_id<FramePostProcessingHandler>().pretty_name() << " handler instance is created" << std::endl;

	PostProcessLayers.emplace("face_detection", std::make_unique<PostProcessingFaceDetectionLayer>());
	PostProcessLayers.emplace("denoise", std::make_unique<PostProcessingFrameDenoisingLayer>());
}

FramePostProcessingHandler::~FramePostProcessingHandler()
{
	std::cout << boost::typeindex::type_id<FramePostProcessingHandler>().pretty_name() << " handler instance is destroyed" << std::endl;
}

void FramePostProcessingHandler::PostProcessFrame(cv::Mat& InFrame)
{
	for (const auto& Layer : PostProcessLayers)
	{
		if (Layer.second->IsLayerActive())
		{
			Layer.second->PostProcessFrame(InFrame);
		}
	}
}

void FramePostProcessingHandler::ToggleLayerByName(const std::string& InLayerName)
{
	if (PostProcessLayers.contains(InLayerName))
	{
		bool bNewLayerState = !PostProcessLayers[InLayerName]->IsLayerActive();
		const std::string LayerStateStr = bNewLayerState ? " activated" : " deactivated";
		std::cout << boost::typeindex::type_id<FramePostProcessingHandler>().pretty_name() << " " << InLayerName << " has been " << LayerStateStr << std::endl;

		if (bNewLayerState)
		{
			PostProcessLayers[InLayerName]->ActivateLayer();
		}
		else
		{
			PostProcessLayers[InLayerName]->DeactivateLayer();

		}
	}
	else
	{
		std::cout << boost::typeindex::type_id<FramePostProcessingHandler>().pretty_name() << " has no Layer with name " << InLayerName << std::endl;
	}
}
