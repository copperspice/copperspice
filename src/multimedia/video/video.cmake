set(MULTIMEDIA_PUBLIC_INCLUDES
    ${MULTIMEDIA_PUBLIC_INCLUDES}
    QVideoFrame
    QAbstractVideoBuffer
    QAbstractVideoSurface
    QVideoSurfaceFormat
    QtMultimedia
)

set(MULTIMEDIA_INCLUDES
    ${MULTIMEDIA_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/video/qabstractvideobuffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/video/qabstractvideosurface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/video/qvideoframe.h
    ${CMAKE_CURRENT_SOURCE_DIR}/video/qvideosurfaceformat.h
    ${CMAKE_CURRENT_SOURCE_DIR}/video/qtmultimedia.h
)

set(MULTIMEDIA_PRIVATE_INCLUDES
    ${MULTIMEDIA_PRIVATE_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/video/qabstractvideobuffer_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/video/qabstractvideosurface_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/video/qimagevideobuffer_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/video/qmemoryvideobuffer_p.h
)

set(MULTIMEDIA_SOURCES
    ${MULTIMEDIA_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/video/qabstractvideobuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/video/qabstractvideosurface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/video/qimagevideobuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/video/qmemoryvideobuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/video/qvideoframe.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/video/qvideosurfaceformat.cpp
)
