CC = g++
CFLAGS  := -Wall -O3 -std=c++0x 

OPENCV_INC_ROOT = /usr/local/include/opencv 
OPENCV_LIB_ROOT = /usr/local/lib
OPENCV_INC= -I $(OPENCV_INC_ROOT)
MY_INC =-I/usr/include -I/usr/lib/aarch64-linux-gnu/libavcodec -I/usr/lib/aarch64-linux-gnu/libavformat -I/usr/lib/aarch64-linux-gnu/libswscale -I/usr/lib/aarch64-linux-gnu/libavutil

EXT_INC = $(OPENCV_INC) $(MY_INC)

OPENCV_LIB_PATH = -L $(OPENCV_LIB_ROOT)
MY_LIB_PATH =-L/usr/lib -L/usr/lib/aarch64-linux-gnu -lavcodec -lavformat -lswscale -lavutil

EXT_LIB = $(OPENCV_LIB_PATH) $(MY_LIB_PATH) 

OPENCV_LIB_NAME = -lopencv_highgui -lopencv_imgproc -lopencv_core 
all:test

test:main.o decode_video.o
	$(CC) $(CFLAGS) main.o decode_video.o $(EXT_INC) $(EXT_LIB) $(OPENCV_LIB_NAME) -Wall -o test
main.o:main.cpp
	$(CC) $(CFLAGS) -c main.cpp $(EXT_INC) $(EXT_LIB) $(OPENCV_LIB_NAME) -o main.o
decode_video.o:decode_video.cpp
	$(CC) $(CFLAGS) -c decode_video.cpp $(EXT_INC) $(EXT_LIB) $(OPENCV_LIB_NAME) -o decode_video.o
clean:
	rm -rf test
