list(APPEND GUI_PUBLIC_INCLUDES
   QColorDialog
   QDialog
   QErrorMessage
   QFileDialog
   QFileSystemModel
   QFontDialog
   QInputDialog
   QMessageBox
   QProgressDialog
   QWizard
   QWizardPage
)

list(APPEND GUI_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/qcolordialog.h
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/qdialog.h
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/qerrormessage.h
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/qfiledialog.h
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/qfilesystemmodel.h
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/qfontdialog.h
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/qinputdialog.h
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/qmessagebox.h
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/qprogressdialog.h
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/qwizardpage.h
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/qwizard.h
)

list(APPEND GUI_PRIVATE_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/qdialog_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/qfiledialog_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/qfileinfogatherer_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/qfilesystemmodel_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/qfontdialog_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/qfscompleter_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/qsidebar_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/qwizard_win_p.h
)

target_sources(CsGui
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/qcolordialog.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/qdialog.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/qerrormessage.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/qfiledialog.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/qfiledialog_p.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/qfileinfogatherer.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/qfilesystemmodel.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/qfontdialog.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/qinputdialog.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/qmessagebox.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/qprogressdialog.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/qsidebar.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/qwizard.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/qfiledialog.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/qmessagebox.qrc
   ${CMAKE_CURRENT_BINARY_DIR}/qrc_qmessagebox.cpp
)

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
   target_sources(CsGui
      PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/qwizard_win.cpp
   )
endif()
