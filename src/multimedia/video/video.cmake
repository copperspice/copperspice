list(APPEND MULTIMEDIA_PUBLIC_INCLUDES
   QVideoFrame
   QAbstractVideoBuffer
   QAbstractVideoSurface
   QVideoSurfaceFormat
)

list(APPEND MULTIMEDIA_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/video/qabstractvideofilter.h
   ${CMAKE_CURRENT_SOURCE_DIR}/video/qabstractvideobuffer.h
   ${CMAKE_CURRENT_SOURCE_DIR}/video/qabstractvideosurface.h
   ${CMAKE_CURRENT_SOURCE_DIR}/video/qvideoframe.h
   ${CMAKE_CURRENT_SOURCE_DIR}/video/qvideosurfaceformat.h
   ${CMAKE_CURRENT_SOURCE_DIR}/video/qvideoprobe.h
)

list(APPEND MULTIMEDIA_PRIVATE_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/video/qabstractvideobuffer_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/video/qabstractvideosurface_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/video/qimagevideobuffer_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/video/qmemoryvideobuffer_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/video/qvideoframe_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/video/qvideoframeconversionhelper_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/video/qvideosurfaceoutput_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/video/qvideooutputorientationhandler_p.h
)

target_sources(CsMultimedia
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/video/qabstractvideofilter.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/video/qabstractvideobuffer.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/video/qabstractvideosurface.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/video/qimagevideobuffer.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/video/qmemoryvideobuffer.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/video/qvideoframe.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/video/qvideosurfaceformat.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/video/qvideoframeconversionhelper.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/video/qvideosurfaceoutput.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/video/qvideoprobe.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/video/qvideooutputorientationhandler.cpp
)
