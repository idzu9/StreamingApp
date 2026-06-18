#include <PostProcessingLayerBase.hpp>
#include <boost/type_index.hpp>
#include <iostream>

PostProcessingLayerBase::PostProcessingLayerBase()
{
	std::cout << boost::typeindex::type_id<PostProcessingLayerBase>().pretty_name() << " layer instance is created" << std::endl;
}

PostProcessingLayerBase::~PostProcessingLayerBase()
{
	std::cout << boost::typeindex::type_id<PostProcessingLayerBase>().pretty_name() << " layer instance is destroyed" << std::endl;
}

void PostProcessingLayerBase::ActivateLayer()
{
	_bIsActive = true;
}

void PostProcessingLayerBase::DeactivateLayer()
{
	_bIsActive = false;
}

void PostProcessingLayerBase::PostProcessFrame(cv::Mat& InFrame)
{
	std::cout << boost::typeindex::type_id<PostProcessingLayerBase>().pretty_name() << "::PostProcessFrame needs to be implemented" << std::endl;
}