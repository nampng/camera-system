#include "stream.h"
#include "thread.h"
#include "fifo.h"

#include <string.h>
#include <gst/video/video.h>
#include <gst/app/gstappsink.h>



int initStream(Stream *stream)
{
        memset(stream, 0, sizeof(Stream));

        GError *error = NULL;
        stream->pipeline = gst_parse_launch(
                "rtspsrc location=rtsp://localhost:8554/test "
                "! rtph264depay ! h264parse "
                #ifdef ON_TARGET
                "! openh264dec "
                #else
                "! avdec_h264 "
                #endif
                "! videoconvert ! video/x-raw,width=800,height=480,format=RGBA "
                "! appsink name=stream-sink sync=false", &error);


        if (error) {
                g_printerr("Failed to create pipeline: %s\n", error->message);
                g_error_free(error);
                return -1;
        }

        // Get the appsink for later sample pulling
        stream->sink = gst_bin_get_by_name(GST_BIN(stream->pipeline), "stream-sink");
        if (!stream->sink) {
                g_printerr("Could not get appsink from pipeline\n");
                return -1;
        }

        // Configure appsink
        g_object_set(G_OBJECT(stream->sink), 
                        "drop", TRUE,
                        "max-buffers", 1,
                        NULL);

        return 0; 
}

void freeStream(Stream *stream)
{
        if (stream->pipeline) {
                gst_element_set_state(stream->pipeline, GST_STATE_NULL);
                gst_object_unref(stream->pipeline);
        }
}

void *startStream(void *arg)
{
        g_print("Stream thread started!\n");
        ThreadArg *threadArg = (ThreadArg*)arg;

        Stream *stream = threadArg->stream;
        GstBus *bus;
        GstMessage *msg;
        GstStateChangeReturn ret;

        ret = gst_element_set_state (stream->pipeline, GST_STATE_PLAYING);
        if (ret == GST_STATE_CHANGE_FAILURE) {
                g_printerr("Unable to set the pipeline to the playing state.\n");
                return NULL;
        }

        g_print("Pipeline set to PLAYING.\n");

        bus = gst_element_get_bus(stream->pipeline);

        g_print("Starting stream loop...\n");
        while (threadArg->running) {
                msg = gst_bus_timed_pop_filtered(bus, 100, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);
                if (msg != NULL) {
                        g_print("Message received...\n");
                        GError *err;
                        gchar *debug_info;

                        switch (GST_MESSAGE_TYPE(msg)) {
                        case GST_MESSAGE_ERROR:
                                gst_message_parse_error(msg, &err, &debug_info);
                                g_printerr("Error received from element %s: %s\n", GST_OBJECT_NAME(msg->src), err->message);
                                g_printerr("Debugging information: %s\n", debug_info ? debug_info : "none");
                                g_clear_error(&err);
                                g_free(debug_info);
                                break;
                        case GST_MESSAGE_EOS:
                                g_print("End-Of-Stream reached.\n");
                                break;
                        default:
                                g_print("Unknown message received.\n");
                                break;
                        }

                        gst_message_unref(msg);
                }

                //g_print("Checking sink state...\n");
                GstState currState, pendingState;
                gst_element_get_state(stream->sink, &currState, &pendingState, GST_SECOND); 

                if (currState != GST_STATE_PLAYING) {
                        continue;
                }

                //g_print("Pulling sample...\n");
                GstSample *sample = gst_app_sink_pull_sample(GST_APP_SINK(stream->sink));
                if (sample != NULL) {
                        GstBuffer *buffer = gst_sample_get_buffer(sample);
                        GstMapInfo mapInfo;
                        if (gst_buffer_map(buffer, &mapInfo, GST_MAP_READ)) {
                                pushFifo(&threadArg->buffer, mapInfo.data);
                                gst_buffer_unmap(buffer, &mapInfo);
                        }
                        gst_sample_unref(sample);
                }
        }

        g_print("Stream thread stopping!\n");
        threadArg->running = 0;
        gst_element_set_state(stream->pipeline, GST_STATE_NULL);
        gst_object_unref(bus);
        pthread_exit(NULL);
}
