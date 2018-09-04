CC = g++
CFLAGS  := -Wall -O3 -std=c++0x 

OPENCV_INC_ROOT = /usr/local/include/opencv 
OPENCV_LIB_ROOT = /usr/local/lib
OPENCV_INC= -I $(OPENCV_INC_ROOT)
MY_INC =-I/usr/include -I/usr/include/gstreamer-1.0  -I/usr/lib/x86_64-linux-gnu/gstreamer-1.0 -I/usr/lib/x86_64-linux-gnu/gstreamer-1.0/include -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/lib/x86_64-linux-gnu/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include

EXT_INC = $(OPENCV_INC) $(MY_INC)

OPENCV_LIB_PATH = -L $(OPENCV_LIB_ROOT)
MY_LIB_PATH =-L/usr/lib -L/usr/lib/glib-2.0/include -L/usr/lib/x86_64-linux-gnu -lgio-2.0 -lglib-2.0 -lgobject-2.0 -lgmodule-2.0 -lgstreamer-1.0 -lgstbase-1.0 -lgstapp-1.0 -lgstnet-1.0

EXT_LIB = $(OPENCV_LIB_PATH) $(MY_LIB_PATH) 

OPENCV_LIB_NAME = -lopencv_highgui -lopencv_imgproc -lopencv_core 
all:test

test:main.o gst_decode.o
	$(CC) main.o gst_decode.o $(CFLAGS) $(EXT_INC) $(EXT_LIB) $(OPENCV_LIB_NAME) -o test
main.o:main.cpp
	$(CC) -c $(CFLAGS) main.cpp $(EXT_INC) $(EXT_LIB) $(OPENCV_LIB_NAME) -o main.o
gst_decode.o:gst_decode.cpp
	$(CC) -c $(CFLAGS) gst_decode.cpp $(EXT_INC) $(EXT_LIB) $(OPENCV_LIB_NAME) -o gst_decode.o
clean:
	rm -rf test
