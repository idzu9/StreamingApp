#ifndef STREAMINGAPP_IMEDIAPIPELINE_HPP
#define STREAMINGAPP_IMEDIAPIPELINE_HPP

class IMediaPipeline
{

public:
	virtual ~IMediaPipeline() = default;

	virtual void InitializePipeline() = 0;

	virtual void EnableDebug() const = 0;
};

#endif // STREAMINGAPP_IMEDIAPIPELINE_HPP