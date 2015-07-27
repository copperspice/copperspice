set(GUI_PUBLIC_INCLUDES
    ${GUI_PUBLIC_INCLUDES}
    QAbstractPageSetupDialog
    QAbstractPrintDialog
    QColorDialog
    QDialog
    QErrorMessage
    QFileDialog
    QFileSystemModel
    QFontDialog
    QInputDialog
    QMessageBox
    QPageSetupDialog
    QPrintDialog
    QUnixPrintWidget
    QPrintPreviewDialog
    QProgressDialog
    QWizard
    QWizardPage
    PARENT_SCOPE
)

set(GUI_INCLUDES
    ${GUI_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/qabstractpagesetupdialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qabstractprintdialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qcolordialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qdialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qunixprintwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qerrormessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qfiledialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qfilesystemmodel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qfontdialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qinputdialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qmessagebox.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qpagesetupdialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qprintdialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qprintpreviewdialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qprogressdialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qwizardpage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qwizard.h
    PARENT_SCOPE
)

set(GUI_PRIVATE_INCLUDES
    ${GUI_PRIVATE_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/qabstractpagesetupdialog_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qabstractprintdialog_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qcolordialog_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qdialog_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qfiledialog_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qfileinfogatherer_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qfilesystemmodel_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qwizard_win_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qfiledialog_win_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qpagesetupdialog_unix_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qfontdialog_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qfscompleter_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qsidebar_p.h
    PARENT_SCOPE
)

set(GUI_SOURCES
    ${GUI_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/qabstractpagesetupdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qabstractprintdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qcolordialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qerrormessage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qfiledialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qfileinfogatherer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qfilesystemmodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qfontdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qinputdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qmessagebox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qpagesetupdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qprintpreviewdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qprogressdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qsidebar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qwizard.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qfiledialog.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/qrc_qmessagebox.cpp
    PARENT_SCOPE
)

set(EXTRA_DIST
    ${CMAKE_CURRENT_SOURCE_DIR}/qmessagebox.qrc
    ${CMAKE_CURRENT_SOURCE_DIR}/qprintdialog.qrc
    ${CMAKE_CURRENT_SOURCE_DIR}/images/cslogo-64.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/fit-page-24.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/fit-page-32.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/fit-width-24.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/fit-width-32.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/go-first-24.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/go-first-32.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/go-last-24.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/go-last-32.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/go-next-24.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/go-next-32.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/go-previous-24.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/go-previous-32.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/layout-landscape-24.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/layout-landscape-32.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/layout-portrait-24.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/layout-portrait-32.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/page-setup-24.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/page-setup-32.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/print-24.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/print-32.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/view-page-multi-24.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/view-page-multi-32.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/view-page-one-24.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/view-page-one-32.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/view-page-sided-24.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/view-page-sided-32.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/zoom-in-24.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/zoom-in-32.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/zoom-out-24.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/zoom-out-32.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/status-color.png
    ${CMAKE_CURRENT_SOURCE_DIR}/images/status-gray-scale.png
)

# FIXME: how to generate ui and qrc files?
# $(top_srcdir)/${CMAKE_CURRENT_SOURCE_DIR}/qfiledialog.cpp: | ${CMAKE_CURRENT_SOURCE_DIR}/ui_qfiledialog.h
# $(top_srcdir)/${CMAKE_CURRENT_SOURCE_DIR}/qpagesetupdialog_unix_p.h: | ${CMAKE_CURRENT_SOURCE_DIR}/ui_qpagesetupwidget.h
# 
# $(top_srcdir)/${CMAKE_CURRENT_SOURCE_DIR}/qprintdialog_unix.cpp: |
#       ${CMAKE_CURRENT_SOURCE_DIR}/ui_qprintpropertieswidget.h
#       ${CMAKE_CURRENT_SOURCE_DIR}/ui_qprintsettingsoutput.h
#       ${CMAKE_CURRENT_SOURCE_DIR}/ui_qprintwidget.h
#       ${CMAKE_CURRENT_SOURCE_DIR}/qrc_qprintdialog.cpp

if(${CMAKE_SYSTEM_NAME} MATCHES "Win32")
    set(GUI_SOURCES
        ${GUI_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/qfiledialog_win.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qpagesetupdialog_win.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qprintdialog_win.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qwizard_win.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qrc_qprintdialog.cpp
        PARENT_SCOPE
    )
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(GUI_SOURCES
        ${GUI_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/qprintdialog_unix.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qpagesetupdialog_unix.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qprintsettingsoutput.ui
        ${CMAKE_CURRENT_SOURCE_DIR}/qprintwidget.ui
        ${CMAKE_CURRENT_SOURCE_DIR}/qprintpropertieswidget.ui
        PARENT_SCOPE
    )
endif()

# FIXME: COCOCA?
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(GUI_SOURCES
        ${GUI_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/qfiledialog_mac.mm
        ${CMAKE_CURRENT_SOURCE_DIR}/qfontdialog_mac.mm
        ${CMAKE_CURRENT_SOURCE_DIR}/qnspanelproxy_mac.mm
        ${CMAKE_CURRENT_SOURCE_DIR}/qpagesetupdialog_mac.mm
        ${CMAKE_CURRENT_SOURCE_DIR}/qprintdialog_mac.mm
        ${CMAKE_CURRENT_SOURCE_DIR}/qcolordialog_mac.mm
        ${CMAKE_CURRENT_SOURCE_DIR}/qrc_qprintdialog.cpp
        PARENT_SCOPE
    )
endif()
