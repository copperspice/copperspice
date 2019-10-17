list(APPEND MULTIMEDIA_PUBLIC_INCLUDES
   QAudioRecorder
   QMediaRecorder
)

list(APPEND MULTIMEDIA_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/recording/qaudiorecorder.h
   ${CMAKE_CURRENT_SOURCE_DIR}/recording/qmediaencodersettings.h
   ${CMAKE_CURRENT_SOURCE_DIR}/recording/qmediarecorder.h
)

list(APPEND MULTIMEDIA_PRIVATE_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/recording/qmediarecorder_p.h
)

target_sources(CsMultimedia
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/recording/qaudiorecorder.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/recording/qmediaencodersettings.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/recording/qmediarecorder.cpp
)
