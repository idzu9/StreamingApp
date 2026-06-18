#ifndef STREAMINGAPP_POSTPROCESSINGLAYERBASE_HPP
#define STREAMINGAPP_POSTPROCESSINGLAYERBASE_HPP

#include <opencv2/opencv.hpp>

class PostProcessingLayerBase
{
public:
	PostProcessingLayerBase();

	virtual ~PostProcessingLayerBase();

	void ActivateLayer();

	void DeactivateLayer();

	bool IsLayerActive() const { return _bIsActive; }

	virtual void PostProcessFrame(cv::Mat& InFrame);

private:
	bool _bIsActive = true;
};

#endif // STREAMINGAPP_POSTPROCESSINGLAYERBASE_HPP