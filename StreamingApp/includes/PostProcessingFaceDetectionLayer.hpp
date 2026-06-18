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

	static const std::string EyePath;

	static const std::string SmilePath;

	/*
		clasifiers objects
	*/
	cv::CascadeClassifier FaceCascade;

	cv::CascadeClassifier EyeCascade;

	cv::CascadeClassifier SmileCascade;

	/*
		rectangles with detected objects
	*/
	std::vector<cv::Rect> Faces;

	std::vector<cv::Rect> Eyes;
	
	std::vector<cv::Rect> Smiles;
};

#endif // STREAMINGAPP_POSTPROCESSINGFACEDETECTIONLAYER_HPP