#include <FramePostProcessingHandler.hpp>
#include <PostProcessingFaceBlurLayer.hpp>
#include <PostProcessingFaceDetectionLayer.hpp>
#include <PostProcessingFrameDenoisingLayer.hpp>
#include <PostProcessingHumanSegmentationLayer.hpp>
#include <boost/type_index.hpp>
#include <iostream>

FramePostProcessingHandler::FramePostProcessingHandler()
{
	std::cout << boost::typeindex::type_id<FramePostProcessingHandler>().pretty_name() << " handler instance is created" << std::endl;

	PostProcessLayers.emplace("face_detection", std::make_unique<PostProcessingFaceDetectionLayer>());
	PostProcessLayers.emplace("denoise", std::make_unique<PostProcessingFrameDenoisingLayer>());
	PostProcessLayers.emplace("human_segmentation", std::make_unique<PostProcessingHumanSegmentationLayer>());
	PostProcessLayers.emplace("blur_face", std::make_unique<PostProcessingFaceBlurLayer>());
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

	FeedOtherLayersWithFaceDecetionData();
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

void FramePostProcessingHandler::FeedOtherLayersWithFaceDecetionData()
{
	PostProcessingFaceDetectionLayer* FaceDetectionLayer = dynamic_cast<PostProcessingFaceDetectionLayer*>(PostProcessLayers["face_detection"].get());
	if (!FaceDetectionLayer)
	{
		std::cerr << "[" << __FUNCTION__ << "] " << "Error: Face detection layer is not valid." << std::endl;
	}

	PostProcessingFaceBlurLayer* FaceBlurLayer = dynamic_cast<PostProcessingFaceBlurLayer*>(PostProcessLayers["blur_face"].get());
	if (!FaceBlurLayer)
	{
		std::cerr << "[" << __FUNCTION__ << "] " << "Error: Face blur layer is not valid." << std::endl;
	}
	
	FaceBlurLayer->SetFacesRoi(FaceDetectionLayer->GetFacesRoi());
}