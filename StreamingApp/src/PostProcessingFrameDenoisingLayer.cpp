#include <PostProcessingFrameDenoisingLayer.hpp>
#include <boost/type_index.hpp>
#include <opencv2/core/cuda.hpp>
#include <opencv2/photo/cuda.hpp>
#include <opencv2/photo.hpp>
#include <opencv2/cudaarithm.hpp>
#include <iostream>

PostProcessingFrameDenoisingLayer::PostProcessingFrameDenoisingLayer()
{
	std::cout << boost::typeindex::type_id<PostProcessingFrameDenoisingLayer>().pretty_name() << " layer instance is created" << std::endl;

	DeactivateLayer();
}

PostProcessingFrameDenoisingLayer::~PostProcessingFrameDenoisingLayer()
{
	std::cout << boost::typeindex::type_id<PostProcessingFrameDenoisingLayer>().pretty_name() << " layer instance is destroyed" << std::endl;
}

void PostProcessingFrameDenoisingLayer::PostProcessFrame(cv::Mat& InFrame)
{
	if (cv::cuda::getCudaEnabledDeviceCount() == 0)
	{
		std::cout << cv::getBuildInformation() << std::endl;
		std::cerr << "Error: No CUDA devices found. OpenCV was not built with CUDA." << std::endl;
		return;
	}

	cv::cuda::GpuMat GpuSrc(InFrame);
	cv::cuda::GpuMat GpuDst;

	cv::cuda::fastNlMeansDenoisingColored(
		GpuSrc,
		GpuDst,
		7.5f,
		10.0f,
		9,
		3
	);

	GpuDst.download(InFrame);
}
