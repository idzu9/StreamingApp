#ifndef STREAMINGAPP_IMEDIAPIPELINE_HPP
#define STREAMINGAPP_IMEDIAPIPELINE_HPP

class IMediaPipeline
{

public:
	virtual ~IMediaPipeline() = default;

	virtual void CreatePipeline() = 0;

protected:
	virtual void _CreatePipelineElements() = 0;

	virtual void _LinkPipelineElements() = 0;

	virtual void _SetElementCapsAndProperties() = 0;

	virtual void _SetupSignals() = 0;

	virtual void _ConnectElemetsPads() = 0;
};

#endif // STREAMINGAPP_IMEDIAPIPELINE_HPP