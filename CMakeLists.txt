cmake_minimum_required(VERSION 2.8)
project( HBAnalysis )
find_package( OpenCV REQUIRED )
add_executable( HBAnalysis HBAnalysis.cc CaptureInit.cc CommandProcessing.cc Global.cc )
add_executable( HBControl HBControl.cc CaptureInit.cc CommandProcessing.cc Global.cc HBComm.cc)
target_link_libraries( HBAnalysis ${OpenCV_LIBS} )
target_link_libraries( HBControl ${OpenCV_LIBS} )
