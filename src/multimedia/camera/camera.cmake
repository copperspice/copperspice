list(APPEND MULTIMEDIA_PUBLIC_INCLUDES
   QCamera
   QCameraExposure
   QCameraFocus
   QCameraImageCapture
   QCameraImageProcessing
   QCameraInfo
   QCameraViewfinderSettings
)

list(APPEND MULTIMEDIA_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/camera/qcamera.h
   ${CMAKE_CURRENT_SOURCE_DIR}/camera/qcameraexposure.h
   ${CMAKE_CURRENT_SOURCE_DIR}/camera/qcamerafocus.h
   ${CMAKE_CURRENT_SOURCE_DIR}/camera/qcameraimagecapture.h
   ${CMAKE_CURRENT_SOURCE_DIR}/camera/qcameraimageprocessing.h
   ${CMAKE_CURRENT_SOURCE_DIR}/camera/qcamerainfo.h
   ${CMAKE_CURRENT_SOURCE_DIR}/camera/qcameraviewfindersettings.h
)

list(APPEND MULTIMEDIA_PRIVATE_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/camera/qcamera_p.h
)

target_sources(CsMultimedia
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/camera/qcamera.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/camera/qcameraexposure.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/camera/qcamerafocus.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/camera/qcameraimagecapture.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/camera/qcameraimageprocessing.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/camera/qcamerainfo.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/camera/qcameraviewfindersettings.cpp

)
