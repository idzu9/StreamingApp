#ifndef STREAMINGAPP_GTKWINDOWPIPELINE_HPP
#define STREAMINGAPP_GTKWINDOWPIPELINE_HPP

#include "Interfaces/IMediaPipeline.hpp"
#include "SingleCastDelegate.hpp"
#include <gst/app/gstappsink.h>
#include <gst/app/gstappsrc.h>
#include <gst/gst.h>
#include <gtk/gtk.h>
#include <memory>

class GtkWindowPipeline : public IMediaPipeline
{
public:
	GtkWindowPipeline() = delete;

	GtkWindowPipeline(GstElement* InPipeline, GstElement* InLinkWithProvider);

	virtual ~GtkWindowPipeline();

	/*
		IMediaPipeline start
	*/
	virtual void CreatePipeline() override;
	/*
		IMediaPipeline end
	*/
private:
	/*
		IMediaPipeline start
	*/
	virtual void _CreatePipelineElements() override;

	virtual void _LinkPipelineElements() override;

	virtual void _SetElementCapsAndProperties() override;

	virtual void _SetupSignals() override;

	void _ConnectElemetsPads() override;
	/*
		IMediaPipeline end
	*/

	void _CreateWindow();

	/*
	*	Pipeline elements
	*/
	GstElement* Pipeline;

	GstElement* LinkWithProvider = nullptr;

	GstElement* Videoconvert = nullptr;
	
	GstElement* VideoSink = nullptr;

	GstElement* Gtkglsink = nullptr;

	GtkWidget* SinkWidget;

	/*
		Output window elements
	*/
	GtkWidget* MainWindow;
	
	GtkWidget* MainBox;
	
	GtkWidget* MainHbox;
};

#endif // STREAMINGAPP_GTKWINDOWPIPELINE_HPP