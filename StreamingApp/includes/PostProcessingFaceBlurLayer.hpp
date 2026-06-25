#ifndef STREAMINGAPP_POSTPROCESSINGFACEBLURLAYER_HPP
#define STREAMINGAPP_POSTPROCESSINGFACEBLURLAYER_HPP

#include <PostProcessingLayerBase.hpp>
#include <string>
#include <vector>

class PostProcessingFaceBlurLayer : public PostProcessingLayerBase
{
public:
	PostProcessingFaceBlurLayer();

	virtual ~PostProcessingFaceBlurLayer();

	virtual void PostProcessFrame(cv::Mat& InFrame) override;

	void SetFacesRoi(const std::vector<cv::Mat>& InFacesRoi);

private:
	/*
		Detected faces regions
	*/
	std::vector<cv::Mat> FacesRoi;
};

#endif // STREAMINGAPP_POSTPROCESSINGFACEBLURLAYER_HPP