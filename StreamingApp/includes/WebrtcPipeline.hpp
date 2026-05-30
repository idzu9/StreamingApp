#include "Interfaces/IMediaPipeline.hpp"
#include <mutex>

class WebrtcPipeline : public IMediaPipeline
{
public:
	WebrtcPipeline();

	~WebrtcPipeline();

	virtual void InitializePipeline() override;

	virtual void EnableDebug() const override;

private:
	static std::once_flag InitFlag;
};