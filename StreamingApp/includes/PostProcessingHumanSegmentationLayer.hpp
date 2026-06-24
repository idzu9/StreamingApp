#ifndef STREAMINGAPP_POSTPROCESSINGFHUMANSEGMENTATIONLAYER_HPP
#define STREAMINGAPP_POSTPROCESSINGFHUMANSEGMENTATIONLAYER_HPP

#include <PostProcessingLayerBase.hpp>
#include <string>
#include <vector>

class PostProcessingHumanSegmentationLayer : public PostProcessingLayerBase
{
public:
	PostProcessingHumanSegmentationLayer();

	virtual ~PostProcessingHumanSegmentationLayer();

	virtual void PostProcessFrame(cv::Mat& InFrame) override;

	void extractMask(const cv::Mat& srcFrame, cv::Mat& outMask);

	void processFrame(cv::Mat& ioFrame);

private:
	/*
		models paths
	*/
	static const std::string HumanSegmentationModelPath;

	/*
		background
	*/
	static const std::string BackgroundPath;

	/*
		clasifiers objects
	*/
	cv::dnn::Net HumanSegmentationModel;

	//cv::Scalar imageMean = cv::Scalar(1, 1, 1);
	//
	//cv::Scalar imageStd = cv::Scalar(1, 1.0, 1.0);
	
	cv::Size ModelInputSize = cv::Size(192, 192);
	
	cv::Size InputSize = cv::Size(192, 192);

	const cv::String InputNames = "x";
	
	const cv::String OutputNames = "save_infer_model/scale_0.tmp_1";

	cv::Mat HistoryScores;

	cv::Mat BackgroundImage;
};

#endif // STREAMINGAPP_POSTPROCESSINGFHUMANSEGMENTATIONLAYER_HPP