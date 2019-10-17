list(APPEND GUI_PUBLIC_INCLUDES
   QGenericMatrix
   QMatrix2x2
   QMatrix2x3
   QMatrix2x4
   QMatrix3x2
   QMatrix3x3
   QMatrix3x4
   QMatrix4x2
   QMatrix4x3
   QMatrix4x4
   QQuaternion
   QVector2D
   QVector3D
   QVector4D
)

list(APPEND GUI_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/math3d/qgenericmatrix.h
   ${CMAKE_CURRENT_SOURCE_DIR}/math3d/qmatrix2x2.h
   ${CMAKE_CURRENT_SOURCE_DIR}/math3d/qmatrix2x3.h
   ${CMAKE_CURRENT_SOURCE_DIR}/math3d/qmatrix2x4.h
   ${CMAKE_CURRENT_SOURCE_DIR}/math3d/qmatrix3x2.h
   ${CMAKE_CURRENT_SOURCE_DIR}/math3d/qmatrix3x3.h
   ${CMAKE_CURRENT_SOURCE_DIR}/math3d/qmatrix3x4.h
   ${CMAKE_CURRENT_SOURCE_DIR}/math3d/qmatrix4x2.h
   ${CMAKE_CURRENT_SOURCE_DIR}/math3d/qmatrix4x3.h
   ${CMAKE_CURRENT_SOURCE_DIR}/math3d/qmatrix4x4.h
   ${CMAKE_CURRENT_SOURCE_DIR}/math3d/qquaternion.h
   ${CMAKE_CURRENT_SOURCE_DIR}/math3d/qvector2d.h
   ${CMAKE_CURRENT_SOURCE_DIR}/math3d/qvector3d.h
   ${CMAKE_CURRENT_SOURCE_DIR}/math3d/qvector4d.h
)

list(APPEND GUI_PRIVATE_INCLUDES
)

target_sources(CsGui
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/math3d/qgenericmatrix.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/math3d/qmatrix4x4.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/math3d/qquaternion.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/math3d/qvector2d.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/math3d/qvector3d.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/math3d/qvector4d.cpp
)
