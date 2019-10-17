list(APPEND GUI_PUBLIC_INCLUDES
   QKeyEventTransition
   QMouseEventTransition
)

list(APPEND GUI_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/statemachine/qkeyeventtransition.h
   ${CMAKE_CURRENT_SOURCE_DIR}/statemachine/qmouseeventtransition.h
)

list(APPEND GUI_PRIVATE_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/statemachine/qbasickeyeventtransition_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/statemachine/qbasicmouseeventtransition_p.h
)

target_sources(CsGui
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/statemachine/qkeyeventtransition.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/statemachine/qmouseeventtransition.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/statemachine/qbasickeyeventtransition.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/statemachine/qbasicmouseeventtransition.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/statemachine/qguistatemachine.cpp
)
