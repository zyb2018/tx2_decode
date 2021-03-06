#include <iostream>
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <string>
#include <mutex>
#include <chrono>
#include <queue>

#define CAPS "video/x-raw,format=BGR"

using namespace std;
class GstSinkOpenCV
{
public:
    typedef enum _debug_lvl
    {
        DEBUG_NONE,
        DEBUG_INFO,
        DEBUG_VERBOSE,
        DEBUG_FULL
    } DebugLevel;

    typedef struct _sink_stats
    {
        int frameCount;
        int frameDropped;
        int errorCount;
        int timeout;
        double fps;
    } SinkStats;

    static GstSinkOpenCV* Create(std::string input_pipeline , int bufferSize=3, int timeout_sec=15, DebugLevel debugLvl=DEBUG_NONE );
    ~GstSinkOpenCV();

    cv::Mat getLastFrame();

private:
    GstSinkOpenCV(std::string input_pipeline, int bufferSize, DebugLevel debugLvl );
    bool init(int timeout_sec);

    static GstFlowReturn on_new_sample_from_sink(GstElement* elt, GstSinkOpenCV* sinkData );

protected:

private:
    std::string mPipelineStr;

    GstElement* mPipeline;
    GstElement* mSink;

    //std::vector<cv::Mat> mFrameBuffer;
    std::queue<cv::Mat> mFrameBuffer;

    int mWidth;
    int mHeight;
    int mChannels;

    int mMaxBufferSize;

    DebugLevel mDebugLvl;
    SinkStats mSinkStats;

    std::timed_mutex mFrameMutex;

    int mFrameTimeoutMsec;
};


int main(int argc, char *argv[])
{
    gst_init( &argc, &argv );

// Use USB camera
/*
    string pipeline = "v4l2src device=/dev/video1 ! "
                      "video/x-raw,format=I420,framerate=30/1,width=1280,height=720 ! "
                      "videoconvert"
					  "! appsink name=sink caps=\"video/x-raw,format=BGR\"";
*/

// Use Video file
/*
    string pipeline ="uridecodebin uri=file:///home/nvidia/sample_720p.mp4 !" 
					" videoconvert ! videoscale ! "
                    " appsink name=sink caps=\"" CAPS "\"";
*/

// Use Rtsp camera 
/*
	string pipeline ="rtspsrc location=rtsp://184.72.239.149/vod/mp4://BigBuckBunny_175k.mov latency=200 ! " 
			   "rtph264depay ! h264parse ! omxh264dec ! "
               "nvvidconv ! video/x-raw, width=1280, height=720, format=(string)BGRx ! "
               "videoconvert ! "
			   "appsink name=sink caps=\"" CAPS "\"";
*/

//Use Jetson onboard camera
/*
	string pipeline ="nvcamerasrc ! "
               "video/x-raw(memory:NVMM), width=(int)2592, height=(int)1458, format=(string)I420, framerate=(fraction)30/1 ! "
               "nvvidconv ! video/x-raw, width=1280, height=720, format=(string)BGRx ! "
               "videoconvert ! "
			   "appsink name=sink caps=\"" CAPS "\"";
*/   


//test diffrent decode type

/*
	string pipeline=" filesrc location=/home/nvidia/sample_720p.mp4  ! qtdemux ! queue ! h264parse ! avdec_h264 skip-frame=1 ! queue ! videoconvert !"  
					"appsink name=sink caps=\"" CAPS "\"";
*/

	string pipeline=" filesrc location=/home/nvidia/sample_720p.mp4 ! qtdemux name=demux demux.video_0 ! queue ! h264parse ! omxh264dec ! videoconvert !"  
					"appsink name=sink caps=\"" CAPS "\"";

	             
    GstSinkOpenCV* ocvAppsink = GstSinkOpenCV::Create( pipeline, 5, 15, GstSinkOpenCV::DEBUG_INFO);

	int64 t0 = cv::getTickCount();
    size_t nFrames=0;
 	int number=0;
	double number_frames=0.0;
//读取的每一帧并不一定都有数据，在循环中部分读取的帧是空的
    while(1)
    {
        // Getting last frame from appsink
        cv::Mat frame = ocvAppsink->getLastFrame();

        if( !frame.empty() )
        {
			cout<<"aaa"<<endl;
            //cv::imshow( "Frame", frame );
            //opencv 测帧速
			nFrames++;
	    	if (nFrames % 10 == 0)
        	{
            	const int N = 10;
            	int64 t1 = cv::getTickCount();
            	double fps_cv=(double)cv::getTickFrequency() * N / (t1 - t0);
            cout << "    Average FPS: " << cv::format("%9.1f", fps_cv)<< "    Average time per frame1: " << cv::format("%9.2f ms", (double)(t1 - t0) * 1000.0f / (N * cv::getTickFrequency()))<< std::endl;
           		number_frames=number_frames+fps_cv;
	    		number++;
            	t0 = t1;
        }
        }
//		else
//		{
//			cout<<"empty frame"<<endl;
//		}
	
        if( cv::waitKey(5) == 'q' )
        {
            break;
        }
    }
    cout<<"video average fps: "<<(double)(number_frames/number)<<endl;
    delete ocvAppsink;
}

GstSinkOpenCV::GstSinkOpenCV( std::string input_pipeline, int bufferSize, DebugLevel debugLvl )
{
    mPipelineStr = input_pipeline;
    mPipeline = NULL;
    mSink = NULL;

    mMaxBufferSize = bufferSize;

    mDebugLvl = debugLvl;

    mSinkStats.fps = 0.0;
    mSinkStats.frameCount = 0;
    mSinkStats.frameDropped = 0;
    mSinkStats.errorCount = 0;
    mSinkStats.timeout = 0;

    mFrameTimeoutMsec = 50;
}

GstSinkOpenCV::~GstSinkOpenCV()
{
    if( mPipeline )
    {
        /* cleanup and exit */
        gst_element_set_state( mPipeline, GST_STATE_NULL );
        gst_object_unref( mPipeline );
        mPipeline = NULL;
    }
}

GstSinkOpenCV* GstSinkOpenCV::Create(string input_pipeline, int bufferSize, int timeout_sec , DebugLevel debugLvl )
{
    GstSinkOpenCV* gstSinkOpencv = new GstSinkOpenCV( input_pipeline, bufferSize, debugLvl );

    if( !gstSinkOpencv->init( timeout_sec ) )
    {
        delete gstSinkOpencv;
        return NULL;
    }

    return gstSinkOpencv;
}

bool GstSinkOpenCV::init( int timeout_sec )
{
    GError *error = NULL;
    GstStateChangeReturn ret;

    //mPipelineStr += " ! appsink name=sink caps=\"video/x-raw,format=BGR\"";

    switch(mDebugLvl)
    {
    case DEBUG_NONE:
        break;
    case DEBUG_FULL:
    case DEBUG_VERBOSE:
    case DEBUG_INFO:
        cout << "[GstSinkOpenCV] Input pipeline:" << endl << mPipelineStr << endl << endl;
    }

    mPipeline = gst_parse_launch( mPipelineStr.c_str(), &error );

    if (error != NULL)
    {
        switch(mDebugLvl)
        {
        case DEBUG_NONE:
            break;
        case DEBUG_FULL:
        case DEBUG_VERBOSE:
        case DEBUG_INFO:
            cout << "[GstSinkOpenCV] could not construct pipeline: " << error->message << endl;
        }

        g_clear_error (&error);
        return NULL;
    }

    /* set to PAUSED to make the first frame arrive in the sink */
    ret = gst_element_set_state (mPipeline, GST_STATE_PLAYING);
    switch (ret)
    {
    case GST_STATE_CHANGE_FAILURE:

        switch(mDebugLvl)
        {
        case DEBUG_NONE:
            break;
        case DEBUG_FULL:
        case DEBUG_VERBOSE:
        case DEBUG_INFO:
            cout << "[GstSinkOpenCV] Failed to play the pipeline" << endl;
        }

        return false;

    case GST_STATE_CHANGE_NO_PREROLL:
        /* for live sources, we need to set the pipeline to PLAYING before we can
           * receive a buffer. We don't do that yet */
        switch(mDebugLvl)
        {
        case DEBUG_NONE:
            break;
        case DEBUG_FULL:
        case DEBUG_VERBOSE:
        case DEBUG_INFO:
            cout << "[GstSinkOpenCV] Waiting for first frame" << endl;
            break;
        }

    default:
        break;
    }

    /* This can block for up to "timeout_sec" seconds. If your machine is really overloaded,
       * it might time out before the pipeline prerolled and we generate an error. A
       * better way is to run a mainloop and catch errors there. */
    ret = gst_element_get_state( mPipeline, NULL, NULL, timeout_sec * GST_SECOND );
    if (/*ret == GST_STATE_CHANGE_FAILURE*/ret!=GST_STATE_CHANGE_SUCCESS)
    {
        switch(mDebugLvl)
        {
        case DEBUG_NONE:
            break;
        case DEBUG_FULL:
        case DEBUG_VERBOSE:
        case DEBUG_INFO:
            cout << "[GstSinkOpenCV] Source connection timeout" << endl;
            break;
        }

        return false;
    }

    /* get sink */
    mSink = gst_bin_get_by_name (GST_BIN (mPipeline), "sink");

    GstSample *sample;
    g_signal_emit_by_name (mSink, "pull-preroll", &sample, NULL);

    /* if we have a buffer now, convert it to a pixbuf. It's possible that we
       * don't have a buffer because we went EOS right away or had an error. */
    if (sample)
    {
        GstBuffer *buffer;
        GstCaps *caps;
        GstStructure *s;
        GstMapInfo map;

        gint width, height;

        gboolean res;

        /* get the snapshot buffer format now. We set the caps on the appsink so
         * that it can only be an rgb buffer. The only thing we have not specified
         * on the caps is the height, which is dependant on the pixel-aspect-ratio
         * of the source material */
        caps = gst_sample_get_caps (sample);
        if (!caps)
        {
            switch(mDebugLvl)
            {
            case DEBUG_NONE:
                break;
            case DEBUG_FULL:
            case DEBUG_VERBOSE:
            case DEBUG_INFO:
                cout << "[GstSinkOpenCV] could not get the format of the first frame" << endl;
                break;
            }

            return false;
        }
        s = gst_caps_get_structure (caps, 0);

        /* we need to get the final caps on the buffer to get the size */
        res = gst_structure_get_int (s, "width", &width);
        res |= gst_structure_get_int (s, "height", &height);
        if (!res)
        {
            switch(mDebugLvl)
            {
            case DEBUG_NONE:
                break;
            case DEBUG_FULL:
            case DEBUG_VERBOSE:
            case DEBUG_INFO:
                cout << "[GstSinkOpenCV] could not get the dimensions of the first frame" << endl;
                break;
            }

            return false;
        }

        //cerr << width << height << endl;

        /* create pixmap from buffer and save, gstreamer video buffers have a stride
         * that is rounded up to the nearest multiple of 4 */
        buffer = gst_sample_get_buffer (sample);

        /* Mapping a buffer can fail (non-readable) */
        if (gst_buffer_map (buffer, &map, GST_MAP_READ))
        {
            mWidth = width;
            mHeight = height;
            mChannels = map.size / (mWidth*mHeight);

            //mFrameBuffer[mPushIdx] = cv::Mat( mHeight, mWidth, CV_8UC3 );

            cv::Mat frame( mHeight, mWidth, CV_8UC3 );

            memcpy( frame.data, map.data, map.size );

            mFrameBuffer.push( frame );

            switch(mDebugLvl)
            {
            case DEBUG_NONE:
                break;
            case DEBUG_FULL:
                cv::imshow( "Sink frame", frame );
                cv::waitKey( 100 );
            case DEBUG_VERBOSE:
            case DEBUG_INFO:
                cout << "[GstSinkOpenCV] Received first frame [" << mWidth << "x" << mHeight << "x" << mChannels << "]" << endl;
            }

            gst_buffer_unmap( buffer, &map );
        }
        gst_sample_unref (sample);
    }
    else
    {
        switch(mDebugLvl)
        {
        case DEBUG_NONE:
            break;
        case DEBUG_FULL:
        case DEBUG_VERBOSE:
        case DEBUG_INFO:
            cout << "[GstSinkOpenCV] could not get first frame" << endl;
        }

        return false;
    }

    /* we use appsink in push mode, it sends us a signal when data is available
       * and we pull out the data in the signal callback. We want the appsink to
       * push as fast as it can, hence the sync=false */
    g_object_set (G_OBJECT (mSink), "emit-signals", TRUE, "sync", FALSE, NULL);
    g_signal_connect( mSink, "new-sample", G_CALLBACK(on_new_sample_from_sink), this );

    return true;
}

GstFlowReturn GstSinkOpenCV::on_new_sample_from_sink( GstElement* elt, GstSinkOpenCV* sinkData )
{
    GstSample *sample;
    //GstFlowReturn ret;

    /* get the sample from appsink */
    sample = gst_app_sink_pull_sample( GST_APP_SINK (elt) );

    if (sample)
    {
        // >>>>> FPS calculation and automatic frame timeout
        static std::chrono::time_point<std::chrono::system_clock> start = std::chrono::high_resolution_clock::now();
        std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();

        double elapsed_msec = std::chrono::duration_cast<std::chrono::microseconds>( end-start ).count()/1000.0;
        start = end;
        sinkData->mSinkStats.fps = 1000.0/elapsed_msec;
		switch(sinkData->mDebugLvl)
        {
        case DEBUG_NONE:
        case DEBUG_FULL:
		case DEBUG_INFO:
		case DEBUG_VERBOSE:
			cout << "gst decode fps: "<< sinkData->mSinkStats.fps<< endl;       
        }
        sinkData->mFrameTimeoutMsec = static_cast<int>(elapsed_msec*1.5);
        // <<<<< FPS calculation and automatic frame timeout

        GstBuffer *buffer;
        GstCaps *caps;
        GstStructure *s;
        GstMapInfo map;

        gint width, height;

        gboolean res;

        /* get the snapshot buffer format now. We set the caps on the appsink so
         * that it can only be an rgb buffer. The only thing we have not specified
         * on the caps is the height, which is dependant on the pixel-aspect-ratio
         * of the source material */
        caps = gst_sample_get_caps (sample);
        if (!caps)
        {
            switch(sinkData->mDebugLvl)
            {
            case DEBUG_NONE:
                break;
            case DEBUG_FULL:
            case DEBUG_VERBOSE:
            case DEBUG_INFO:
                cout << "[GstSinkOpenCV] could not get the format of the frame" << endl;
                break;
            }

            return GST_FLOW_CUSTOM_ERROR;
        }
        s = gst_caps_get_structure (caps, 0);

        /* we need to get the final caps on the buffer to get the size */
        res = gst_structure_get_int (s, "width", &width);
        res |= gst_structure_get_int (s, "height", &height);
        if (!res)
        {
            switch(sinkData->mDebugLvl)
            {
            case DEBUG_NONE:
                break;
            case DEBUG_FULL:
            case DEBUG_VERBOSE:
            case DEBUG_INFO:
                cout << "[GstSinkOpenCV] could not get the dimensions of the frame" << endl;
                break;
            }

            return GST_FLOW_CUSTOM_ERROR;
        }

        /* create pixmap from buffer and save, gstreamer video buffers have a stride
         * that is rounded up to the nearest multiple of 4 */
        buffer = gst_sample_get_buffer (sample);

        /* Mapping a buffer can fail (non-readable) */
        if (gst_buffer_map (buffer, &map, GST_MAP_READ))
        {
            if( width!=sinkData->mWidth || height!=sinkData->mHeight ) // New size?
            {
                switch(sinkData->mDebugLvl)
                {
                case DEBUG_NONE:
                    break;
                case DEBUG_FULL:
                case DEBUG_VERBOSE:
                case DEBUG_INFO:
                    cout << "[GstSinkOpenCV] New Frame Size [" << width << "x" << height << "x" << sinkData->mChannels << "]" << endl;
                    break;
                }

                sinkData->mWidth = width;
                sinkData->mHeight = height;
                sinkData->mChannels = map.size / (sinkData->mWidth*sinkData->mHeight);

                if( sinkData->mChannels!=3 )
                {
                    switch(sinkData->mDebugLvl)
                    {
                    case DEBUG_NONE:
                        break;
                    case DEBUG_FULL:
                    case DEBUG_VERBOSE:
                        cout << "[GstSinkOpenCV] Only BGR image are supported" << endl;
                    case DEBUG_INFO:
                        break;
                    }

                    return GST_FLOW_CUSTOM_ERROR;
                }
            }

            sinkData->mFrameMutex.lock();

            int bufferSize = static_cast<int>(sinkData->mFrameBuffer.size());

            if( bufferSize==sinkData->mMaxBufferSize )
            {
                sinkData->mSinkStats.frameDropped++;

                switch(sinkData->mDebugLvl)
                {
                case DEBUG_NONE:
                    break;
                case DEBUG_FULL:
                case DEBUG_VERBOSE:
                    cout << "[GstSinkOpenCV] Dropped frame #" << sinkData->mSinkStats.frameDropped << endl;
                    cout << "[GstSinkOpenCV] bufferSize: " << bufferSize << "/" << sinkData->mMaxBufferSize << endl;
                case DEBUG_INFO:
                    break;
                }
            }
            else
            {
                cv::Mat frame(height,width,CV_8UC3);

                memcpy( frame.data, map.data, map.size );

                sinkData->mFrameBuffer.push( frame );

                sinkData->mSinkStats.frameCount++;

                switch(sinkData->mDebugLvl)
                {
                case DEBUG_NONE:
                    break;
                case DEBUG_FULL:
                    cv::imshow( "Sink frame", frame );
                    cv::waitKey( 1 );
                case DEBUG_VERBOSE:
                    cout << "[GstSinkOpenCV] Received frame #" << sinkData->mSinkStats.frameCount << endl;
                    cout << "[GstSinkOpenCV] bufferSize: " << bufferSize << "/" << sinkData->mMaxBufferSize << endl;
                case DEBUG_INFO:
                    break;
                }
            }

            sinkData->mFrameMutex.unlock();

            gst_buffer_unmap (buffer, &map);
        }
        gst_sample_unref (sample);


    }
    else
    {
        sinkData->mSinkStats.errorCount++;

        switch(sinkData->mDebugLvl)
        {
        case DEBUG_NONE:
            break;
        case DEBUG_FULL:
        case DEBUG_VERBOSE:
            cout << "[GstSinkOpenCV] Error receiving frame" << endl;
        case DEBUG_INFO:
            break;
        }

        return GST_FLOW_CUSTOM_ERROR;
    }


    return GST_FLOW_OK;
}

cv::Mat GstSinkOpenCV::getLastFrame()
{
    if( !mFrameMutex.try_lock_for( std::chrono::milliseconds(mFrameTimeoutMsec) ) )
    {
        mSinkStats.timeout++;

        switch(mDebugLvl)
        {
        case DEBUG_NONE:
            break;
        case DEBUG_FULL:
        case DEBUG_VERBOSE:
            cout << "[GstSinkOpenCV] Getting frame timeout #" << mSinkStats.timeout << endl;
        case DEBUG_INFO:
            break;
        }

        return cv::Mat();
    }

    if( mFrameBuffer.size()==0 )
    {
        switch(mDebugLvl)
        {
        case DEBUG_NONE:
            break;
        case DEBUG_FULL:
            cout << "[GstSinkOpenCV] Frame Buffer empty" << endl;
        case DEBUG_VERBOSE:
        case DEBUG_INFO:
            break;
        }

        mFrameMutex.unlock();

        return cv::Mat();
    }

    cv::Mat frame = mFrameBuffer.front();

    mFrameBuffer.pop();

    mFrameMutex.unlock();

    return frame;
}
