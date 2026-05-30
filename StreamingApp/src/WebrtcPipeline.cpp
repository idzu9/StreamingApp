#include "WebrtcPipeline.hpp"
#include <iostream>
#include <gst/gst.h>
#include <boost/type_index.hpp>

std::once_flag WebrtcPipeline::InitFlag;

WebrtcPipeline::WebrtcPipeline()
{
	std::cout << boost::typeindex::type_id<WebrtcPipeline>().pretty_name() << " pipeline instance is created" << std::endl;
}

WebrtcPipeline::~WebrtcPipeline()
{
	std::cout << boost::typeindex::type_id<WebrtcPipeline>().pretty_name() << " pipeline instance is destroyed" << std::endl;
}

void WebrtcPipeline::InitializePipeline()
{
	std::call_once(WebrtcPipeline::InitFlag, []() {
			gst_init(nullptr, nullptr);
		});
}