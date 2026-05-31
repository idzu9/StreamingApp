#ifndef STREAMINGAPP_IMEDIAPIPELINE_HPP
#define STREAMINGAPP_IMEDIAPIPELINE_HPP

#include <string>

class IMediaPipeline
{

public:
	virtual ~IMediaPipeline() = default;

	virtual void InitializePipeline() = 0;

	virtual void CreatePipeline() = 0;

	virtual void StartPipelinePlaying() = 0;

	virtual void EnableDebug() const = 0;

	virtual void ProccessTextBuffer(const std::string& TextBuffer) = 0;
};

#endif // STREAMINGAPP_IMEDIAPIPELINE_HPP