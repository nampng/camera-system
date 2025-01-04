#include "stream.h"
#include "thread.h"

#include <string.h>
#include <gst/video/video.h>

#ifdef RELEASE
const char *SOURCE_TYPE = "udpsrc";
#else
const char *SOURCE_TYPE = "videotestsrc";
#endif

const char *SINK_TYPE = "appsink";

int initStream(Stream *stream)
{
        memset(stream, 0, sizeof(Stream));

        stream->source = gst_element_factory_make(SOURCE_TYPE, "stream-source");
        stream->raw = gst_element_factory_make("rawvideoparse", "raw");
        stream->sink = gst_element_factory_make(SINK_TYPE, "stream-sink");
        stream->pipeline = gst_pipeline_new("stream-pipeline");

        if (!stream->source || !stream->sink || !stream->pipeline || !stream->raw) {
                g_printerr("Not all elements could be created.");
                return -1;
        }

        gst_bin_add_many (GST_BIN(stream->pipeline), stream->source, stream->raw, stream->sink, NULL);
        if (gst_element_link (stream->source, stream->raw) != TRUE) {
                g_printerr("Elements could not be linked.\n");
                return -1;
        }
        if (gst_element_link (stream->raw, stream->sink) != TRUE) {
                g_printerr("Elements could not be linked.\n");
                return -1;
        }

        g_object_set(stream->source, "pattern", 0, NULL);
        g_object_set(stream->raw, "height", 480,  "width", 800, "format", GST_VIDEO_FORMAT_RGBA, NULL);

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

        GstBus *bus;
        GstMessage *msg;
        GstStateChangeReturn ret;

        ret = gst_element_set_state (threadArg->stream->pipeline, GST_STATE_PLAYING);
        if (ret == GST_STATE_CHANGE_FAILURE) {
                g_printerr("Unable to set the pipeline to the playing state.\n");
                return NULL;
        }

        bus = gst_element_get_bus(threadArg->stream->pipeline);

        while (threadArg->running) {
                msg = gst_bus_timed_pop_filtered(bus, 1000, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);
                if (msg != NULL) {
                        gst_message_unref(msg);
                }
                GstSample *sample;
                g_signal_emit_by_name(threadArg->stream->sink, "pull-sample", &sample);
                if (sample != NULL) {
                        GstBuffer *buffer = gst_sample_get_buffer(sample);
                        GstMapInfo mapInfo;
                        if (gst_buffer_map(buffer, &mapInfo, GST_MAP_READ)) {
                                g_print("Buffer (%d), Sample (%d)\n", threadArg->bufferSize, mapInfo.size);
                                if (threadArg->bufferSize > mapInfo.size) {
                                        memcpy(threadArg->buffer, mapInfo.data, mapInfo.size);
                                } else {
                                        memcpy(threadArg->buffer, mapInfo.data, threadArg->bufferSize);
                                }
                        }
                        gst_sample_unref(sample);
                }
        }

        g_print("Stream thread stopping!\n");
        gst_element_set_state(threadArg->stream->pipeline, GST_STATE_NULL);
        gst_object_unref(bus);
        pthread_exit(NULL);
}
