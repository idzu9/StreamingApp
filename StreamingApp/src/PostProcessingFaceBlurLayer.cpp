#include <PostProcessingFaceBlurLayer.hpp>
#include <boost/type_index.hpp>
#include <iostream>

PostProcessingFaceBlurLayer::PostProcessingFaceBlurLayer()
{
	std::cout << boost::typeindex::type_id<PostProcessingFaceBlurLayer>().pretty_name() << " layer instance is created" << std::endl;
}

PostProcessingFaceBlurLayer::~PostProcessingFaceBlurLayer()
{
	std::cout << boost::typeindex::type_id<PostProcessingFaceBlurLayer>().pretty_name() << " layer instance is destroyed" << std::endl;
}

void PostProcessingFaceBlurLayer::PostProcessFrame(cv::Mat& InFrame)
{
	for (const cv::Mat& FaceRoi : FacesRoi)
	{
		cv::GaussianBlur(FaceRoi, FaceRoi, cv::Size(51, 51), 0);
	}
}

void PostProcessingFaceBlurLayer::SetFacesRoi(const std::vector<cv::Mat>& InFacesRoi)
{
	FacesRoi = InFacesRoi;
}
