#ifndef STREAMINGAPP_POSTPROCESSINGFACEDETECTIONLAYER_HPP
#define STREAMINGAPP_POSTPROCESSINGFACEDETECTIONLAYER_HPP

#include <PostProcessingLayerBase.hpp>
#include <string>
#include <vector>

class PostProcessingFaceDetectionLayer : public PostProcessingLayerBase
{
public:
	PostProcessingFaceDetectionLayer();

	virtual ~PostProcessingFaceDetectionLayer();

	virtual void PostProcessFrame(cv::Mat& InFrame) override;

private:
	/*
		models paths
	*/
	static const std::string FaceCascadePath;

	/*
		clasifiers objects
	*/
	cv::Ptr<cv::FaceDetectorYN> FaceDetector;
};

#endif // STREAMINGAPP_POSTPROCESSINGFACEDETECTIONLAYER_HPP