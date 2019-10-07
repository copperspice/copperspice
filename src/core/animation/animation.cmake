list(APPEND CORE_PUBLIC_INCLUDES
   QAbstractAnimation
   QAnimationDriver
   QAnimationGroup
   QParallelAnimationGroup
   QPauseAnimation
   QPropertyAnimation
   QSequentialAnimationGroup
   QVariantAnimation
)

list(APPEND CORE_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/animation/qabstractanimation.h
   ${CMAKE_CURRENT_SOURCE_DIR}/animation/qanimationdriver.h
   ${CMAKE_CURRENT_SOURCE_DIR}/animation/qanimationgroup.h
   ${CMAKE_CURRENT_SOURCE_DIR}/animation/qparallelanimationgroup.h
   ${CMAKE_CURRENT_SOURCE_DIR}/animation/qpauseanimation.h
   ${CMAKE_CURRENT_SOURCE_DIR}/animation/qpropertyanimation.h
   ${CMAKE_CURRENT_SOURCE_DIR}/animation/qsequentialanimationgroup.h
   ${CMAKE_CURRENT_SOURCE_DIR}/animation/qvariantanimation.h
)

list(APPEND CORE_PRIVATE_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/animation/qabstractanimation_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/animation/qanimationgroup_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/animation/qparallelanimationgroup_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/animation/qpropertyanimation_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/animation/qsequentialanimationgroup_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/animation/qvariantanimation_p.h
)

target_sources(CsCore
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/animation/qabstractanimation.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/animation/qvariantanimation.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/animation/qpropertyanimation.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/animation/qanimationgroup.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/animation/qsequentialanimationgroup.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/animation/qparallelanimationgroup.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/animation/qpauseanimation.cpp
)
