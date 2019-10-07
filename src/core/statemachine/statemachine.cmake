list(APPEND CORE_PUBLIC_INCLUDES
   QAbstractState
   QAbstractTransition
   QEventTransition
   QFinalState
   QHistoryState
   QSignalTransition
   QState
   QStateMachine
)

list(APPEND CORE_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/statemachine/qabstractstate.h
   ${CMAKE_CURRENT_SOURCE_DIR}/statemachine/qabstracttransition.h
   ${CMAKE_CURRENT_SOURCE_DIR}/statemachine/qeventtransition.h
   ${CMAKE_CURRENT_SOURCE_DIR}/statemachine/qfinalstate.h
   ${CMAKE_CURRENT_SOURCE_DIR}/statemachine/qhistorystate.h
   ${CMAKE_CURRENT_SOURCE_DIR}/statemachine/qsignaltransition.h
   ${CMAKE_CURRENT_SOURCE_DIR}/statemachine/qstate.h
   ${CMAKE_CURRENT_SOURCE_DIR}/statemachine/qstatemachine.h
)

list(APPEND CORE_PRIVATE_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/statemachine/qabstractstate_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/statemachine/qabstracttransition_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/statemachine/qeventtransition_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/statemachine/qhistorystate_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/statemachine/qsignaleventgenerator_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/statemachine/qstate_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/statemachine/qstatemachine_p.h
)

target_sources(CsCore
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/statemachine/qstatemachine.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/statemachine/qabstractstate.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/statemachine/qeventtransition.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/statemachine/qstate.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/statemachine/qfinalstate.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/statemachine/qhistorystate.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/statemachine/qabstracttransition.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/statemachine/qsignaltransition.cpp
)
