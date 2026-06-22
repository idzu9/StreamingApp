#ifndef STREAMINGAPP_POSTPROCESSINGFRAMEDENOISINGLAYER_HPP
#define STREAMINGAPP_POSTPROCESSINGFRAMEDENOISINGLAYER_HPP

#include <PostProcessingLayerBase.hpp>

class PostProcessingFrameDenoisingLayer : public PostProcessingLayerBase
{
public:
	PostProcessingFrameDenoisingLayer();

	virtual ~PostProcessingFrameDenoisingLayer();

	virtual void PostProcessFrame(cv::Mat& InFrame) override;
};

#endif // STREAMINGAPP_POSTPROCESSINGFRAMEDENOISINGLAYER_HPP