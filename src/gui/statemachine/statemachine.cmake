set(GUI_PUBLIC_INCLUDES
    ${GUI_PUBLIC_INCLUDES}
    QKeyEventTransition
    QMouseEventTransition
)

set(GUI_INCLUDES
    ${GUI_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/statemachine/qkeyeventtransition.h
    ${CMAKE_CURRENT_SOURCE_DIR}/statemachine/qmouseeventtransition.h
)

set(GUI_PRIVATE_INCLUDES
    ${GUI_PRIVATE_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/statemachine/qbasickeyeventtransition_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/statemachine/qbasicmouseeventtransition_p.h
)

set(GUI_SOURCES
    ${GUI_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/statemachine/qkeyeventtransition.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/statemachine/qmouseeventtransition.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/statemachine/qbasickeyeventtransition.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/statemachine/qbasicmouseeventtransition.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/statemachine/qguistatemachine.cpp
)
