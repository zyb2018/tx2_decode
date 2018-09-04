#include "decode_video.h"
#include <iostream>
#include <assert.h>
using namespace std;
#define JUMP_RATE 10
//#define DEBUG_PRINT(FORMAT,VALUE) printf("file:%s lines:%d,"#VALUE " is "FORMAT "\n", __FILE__, __LINE__, VALUE)

int main(void)
{
    int ret = -1;
    cv::Mat img;
    int count = 0;
    cv::Mat* pCvMat = new cv::Mat(); 
    video_t* handel = video_init("sample_720p.mp4",&ret); 
    assert(handel != NULL);
    assert(ret == 0);
    int alltime = video_get_alltime(handel); 
    //DEBUG_PRINT("%d",alltime);
	printf("alltime is %d\n",alltime);

    int state = video_seek_frame(handel,0); 
    //DEBUG_PRINT("%d",state);
	
	size_t nFrames = 0;
    int64 t0 = cv::getTickCount();
    double number_frames=0.0;
    int number=0;

    while(1)
    {
        //if(count % JUMP_RATE == 0)
        //{
            int num = video_get_frame(handel,pCvMat);

			nFrames++;
			if (nFrames % 10 == 0)
        	{
            	const int N = 10;
            	int64 t1 = cv::getTickCount();
            	double fps=(double)cv::getTickFrequency() * N / (t1 - t0);
            	cout << "    Average FPS: " << cv::format("%9.1f", fps)
                 << "    Average time per frame: " << cv::format("%9.2f ms", (double)(t1 - t0) * 1000.0f / (N * cv::getTickFrequency()))
                 << std::endl;
            	number_frames=number_frames+fps;
	   	 		number++;
            	t0 = t1;
           }

           if(!(*pCvMat).empty())
           {
               //(*pCvMat).copyTo(img);
               //cv::resize(img,img,cv::Size(640,480));
               //cv::imshow("Test",img);
               //cv::waitKey(1);
			   //count = 0;
           }
           if(num < 0)
           {
               break;
           }
		   if (cv::waitKey(5) >= 0)
            break;
       //}
		//count++;
		//cout<<"count is :"<<count<<endl;
    }
	cout<<"video average fps: "<<(double)(number_frames/number)<<endl;
    pCvMat->release();
    video_uninit(handel);
    return 0;
}
