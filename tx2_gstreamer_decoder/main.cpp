#include <iostream>
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <string>
#include <mutex>
#include <chrono>
#include <queue>

#include "gst_decode.h"

#define CAPS "video/x-raw,format=BGR"
using namespace std;

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
    string pipeline ="uridecodebin uri=file:///home/zyb/sample_720p.mp4 !" 
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


	string pipeline=" filesrc location=/home/zyb/sample_720p.mp4  ! qtdemux ! queue ! h264parse ! avdec_h264 skip-frame=1 ! queue ! videoconvert !"  
					"appsink name=sink caps=\"" CAPS "\"";

/*
	string pipeline=" filesrc location=/home/zyb/sample_720p.mp4 ! qtdemux name=demux demux.video_0 ! queue ! h264parse ! omxh264dec ! videoconvert !"  
					"appsink name=sink caps=\"" CAPS "\"";
*/
/*
	string pipeline=" filesrc location=/home/zyb/sample_720p.mp4 ! qtdemux name=demux demux.video_0 ! queue ! h264parse ! omxh264dec ! videocuda !"  
					"appsink name=sink caps=\"" CAPS "\"";
*/
	             
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


