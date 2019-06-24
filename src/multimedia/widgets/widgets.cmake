set(MULTIMEDIA_PUBLIC_INCLUDES
    ${MULTIMEDIA_PUBLIC_INCLUDES}
    QVideoWidget
)

set(MULTIMEDIA_INCLUDES
    ${MULTIMEDIA_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qgraphicsvideoitem.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qvideowidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qvideowidgetcontrol.h
)

set(MULTIMEDIA_PRIVATE_INCLUDES
    ${MULTIMEDIA_PRIVATE_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qpaintervideosurface_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qvideowidget_p.h
)

set(MULTIMEDIA_SOURCES
    ${MULTIMEDIA_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qgraphicsvideoitem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qpaintervideosurface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qvideowidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/qvideowidgetcontrol.cpp
)
