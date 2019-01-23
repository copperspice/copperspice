set(GUI_PUBLIC_INCLUDES
    ${GUI_PUBLIC_INCLUDES}
    QAbstractPrintDialog
    QPageLayout
    QPageSetupDialog
    QPageSize
    QPagedPaintDevice
    QPdfWriter
    QPrinter
    QPrinterInfo
    QPrintDialog
    QPrintEngine
    QPrintPreviewDialog
    QPrintPreviewWidget
    QUnixPrintWidget
)

set(GUI_INCLUDES
    ${GUI_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/printing/qabstractprintdialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/printing/qpagelayout.h
    ${CMAKE_CURRENT_SOURCE_DIR}/printing/qpagesize.h
    ${CMAKE_CURRENT_SOURCE_DIR}/printing/qpagesetupdialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/printing/qpagedpaintdevice.h
    ${CMAKE_CURRENT_SOURCE_DIR}/printing/qpdfwriter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/printing/qprinter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/printing/qprintengine.h
    ${CMAKE_CURRENT_SOURCE_DIR}/printing/qprinterinfo.h
    ${CMAKE_CURRENT_SOURCE_DIR}/printing/qprintdialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/printing/qprintpreviewdialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/printing/qprintpreviewwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/printing/qunixprintwidget.h
)

set(GUI_PRIVATE_INCLUDES
    ${GUI_PRIVATE_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/printing/qabstractprintdialog_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/printing/qcups_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/printing/qpagesetupdialog_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/printing/qpagesetupdialog_unix_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/printing/qpdf_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/printing/qprint_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/printing/qprintdevice_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/printing/qprinter_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/printing/qprinterinfo_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/printing/qprintengine_pdf_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/printing/qprintengine_win_p.h
)

set(GUI_SOURCES
    ${GUI_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/printing/qabstractprintdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/printing/qpagedpaintdevice.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/printing/qpagelayout.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/printing/qpagesetupdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/printing/qpagesize.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/printing/qpdf.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/printing/qpdfwriter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/printing/qprintengine_pdf.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/printing/qprintdevice.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/printing/qprinter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/printing/qprinterinfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/printing/qprintpreviewdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/printing/qprintpreviewwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/printing/qprintdialog.qrc
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(GUI_SOURCES
        ${GUI_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/printing/qpagesetupdialog_win.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/printing/qprintdialog_win.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/printing/qprintengine_win.cpp
	     ${CMAKE_CURRENT_BINARY_DIR}/qrc_qprintdialog.cpp
    )

elseif(${CMAKE_SYSTEM_NAME} MATCHES "(Linux|OpenBSD|FreeBSD|NetBSD)")
    set(GUI_SOURCES
        ${GUI_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/printing/qcups.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/printing/qprintdialog_unix.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/printing/qpagesetupdialog_unix.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/printing/qprintsettingsoutput.ui
        ${CMAKE_CURRENT_SOURCE_DIR}/printing/qprintwidget.ui
        ${CMAKE_CURRENT_SOURCE_DIR}/printing/qprintpropertieswidget.ui
        ${CMAKE_CURRENT_SOURCE_DIR}/printing/qpagesetupwidget.ui
    )

elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(GUI_SOURCES
        ${GUI_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/printing/qpagesetupdialog_mac.mm
        ${CMAKE_CURRENT_SOURCE_DIR}/printing/qprintdialog_mac.mm
        ${CMAKE_CURRENT_BINARY_DIR}/qrc_qprintdialog.cpp
    )
endif()
