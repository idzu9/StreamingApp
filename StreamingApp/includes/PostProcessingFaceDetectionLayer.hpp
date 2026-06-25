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

	const std::vector<cv::Mat>& GetFacesRoi() const;

private:
	/*
		models paths
	*/
	static const std::string FaceCascadePath;

	/*
		clasifiers objects
	*/
	cv::Ptr<cv::FaceDetectorYN> FaceDetector;

	/*
		Detected faces
	*/
	std::vector<cv::Mat> FacesRoi;

	/*
		enable debug drawing of the rectangles/circles around face and eyes
	*/
	bool bDebugDrawEnabled = false;
};

#endif // STREAMINGAPP_POSTPROCESSINGFACEDETECTIONLAYER_HPP