set(MULTIMEDIA_PUBLIC_INCLUDES
   ${MULTIMEDIA_PUBLIC_INCLUDES}
   QAudioRecorder
   QMediaRecorder
)

set(MULTIMEDIA_INCLUDES
    ${MULTIMEDIA_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/recording/qaudiorecorder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/recording/qmediaencodersettings.h
    ${CMAKE_CURRENT_SOURCE_DIR}/recording/qmediarecorder.h
)

set(MULTIMEDIA_PRIVATE_INCLUDES
    ${MULTIMEDIA_PRIVATE_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/recording/qmediarecorder_p.h
)

set(MULTIMEDIA_SOURCES
    ${MULTIMEDIA_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/recording/qaudiorecorder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/recording/qmediaencodersettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/recording/qmediarecorder.cpp
)
