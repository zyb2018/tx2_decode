cmake_minimum_required(VERSION 2.8)
project( H264Decoder)

find_package( OpenCV REQUIRED )

set(CMAKE_CXX_STANDARD 11)
set(files
        decode_video.cpp decode_video.h
		test.cpp #for sample
        )

include_directories(
        ${OpenCV_INCLUDE_DIRS}
        /usr/lib/aarch64-linux-gnu/libavcodec
        /usr/lib/aarch64-linux-gnu/libavformat
        /usr/lib/aarch64-linux-gnu/libswscale
)
#add_library(H264Decoder SHARED ${files})  for build a .so file
add_executable(H264Decoder ${files})
target_link_libraries(H264Decoder
        ${OpenCV_LIBS}
        /usr/lib/aarch64-linux-gnu/libavcodec.so
        /usr/lib/aarch64-linux-gnu/libavformat.so
        /usr/lib/aarch64-linux-gnu/libswscale.so
		/usr/lib/aarch64-linux-gnu/libavutil.so
        )






