#include "GtkWindowPipeline.hpp"
#include <iostream>
#include <gst/gst.h>
#include <boost/json.hpp>
#include <opencv2/opencv.hpp>
#include <boost/type_index.hpp>

using namespace boost::json;

GtkWindowPipeline::GtkWindowPipeline(GstElement* InPipeline, GstElement* InLinkWithProvider) : Pipeline(InPipeline), LinkWithProvider(InLinkWithProvider)
{
	std::cout << boost::typeindex::type_id<GtkWindowPipeline>().pretty_name() << " pipeline instance is created" << std::endl;
}

GtkWindowPipeline::~GtkWindowPipeline()
{
	std::cout << boost::typeindex::type_id<GtkWindowPipeline>().pretty_name() << " pipeline instance is destroyed" << std::endl;
	gtk_main_quit();
}

void GtkWindowPipeline::CreatePipeline()
{
	if (!Pipeline || !LinkWithProvider)
	{
		g_printerr("[%s] Has no valid pipeline or link with media provider\n", __FUNCTION__);
		return ;
	}

	gtk_init(0, 0);

	_CreatePipelineElements();
	_SetElementCapsAndProperties();
	_CreateWindow();
	_LinkPipelineElements();
	_ConnectElemetsPads();
}

void GtkWindowPipeline::_CreatePipelineElements()
{
	VideoSink = gst_element_factory_make("glsinkbin", "glsinkbin");
	Gtkglsink = gst_element_factory_make("gtkglsink", "gtkglsink");
	Videoconvert = gst_element_factory_make("videoconvert", "Videoconvert");

	if (!VideoSink || !Videoconvert || !Gtkglsink)
	{
		g_printerr("[%s] Not all elements could be created\n", __FUNCTION__);
		return;
	}
}

void GtkWindowPipeline::_LinkPipelineElements()
{
	gst_bin_add_many(GST_BIN(Pipeline), VideoSink, nullptr);
}

void GtkWindowPipeline::_SetElementCapsAndProperties()
{
	g_object_set(VideoSink, "sink", Gtkglsink, nullptr);
	g_object_get(Gtkglsink, "widget", &SinkWidget, nullptr);

	g_object_set(G_OBJECT(VideoSink),
		"sync", false,
		"async", false,
		nullptr);
}

void GtkWindowPipeline::_SetupSignals()
{

}

void GtkWindowPipeline::_ConnectElemetsPads()
{
	gst_element_link_many(LinkWithProvider, VideoSink, nullptr);
}

void GtkWindowPipeline::_CreateWindow()
{
	MainWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	MainHbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start(GTK_BOX(MainHbox), SinkWidget, TRUE, TRUE, 0);

	MainBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_pack_start(GTK_BOX(MainBox), MainHbox, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(MainWindow), MainBox);
	gtk_window_set_default_size(GTK_WINDOW(MainWindow), 640, 480);

	gtk_widget_show_all(MainWindow);
}
