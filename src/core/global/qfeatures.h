/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
* Copyright (c) 2015 The Qt Company Ltd.
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
*
* This file is part of CopperSpice.
*
* CopperSpice is free software. You can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* https://www.gnu.org/licenses/
*
***********************************************************************/

#ifndef QFeatures_H
#define QFeatures_H

// QAction
//#define QT_NO_ACTION

// Big Codecs
//#define QT_NO_BIG_CODECS

// Color Names
//#define QT_NO_COLORNAMES

// QCopChannel
//#define QT_NO_COP

// CssParser
//#define QT_NO_CSSPARSER

// QCursor
//#define QT_NO_CURSOR

// QDesktopServices
//#define QT_NO_DESKTOPSERVICES

// QDirectPainter
//#define QT_NO_DIRECTPAINTER

// Document Object Model
//#define QT_NO_DOM

// Effects
//#define QT_NO_EFFECTS

// QFileSystemIterator
//#define QT_NO_FILESYSTEMITERATOR

// QFileSystemWatcher
//#define QT_NO_FILESYSTEMWATCHER

// Freetype Font Engine
//#define QT_USE_FREETYPE

// Gesture
//#define QT_NO_GESTURES

// QGroupBox
//#define QT_NO_GROUPBOX

// QImageIOPlugin
//#define QT_NO_IMAGEFORMATPLUGIN

// BMP Image Format
//#define QT_NO_IMAGEFORMAT_BMP

// JPEG Image Format
//#define QT_NO_IMAGEFORMAT_JPEG

// PNG Image Format
//#define QT_NO_IMAGEFORMAT_PNG

// PPM Image Format
//#define QT_NO_IMAGEFORMAT_PPM

// XBM Image Format
//#define QT_NO_IMAGEFORMAT_XBM

// QImage::createHeuristicMask()
//#define QT_NO_IMAGE_HEURISTIC_MASK

// Image Text
//#define QT_NO_IMAGE_TEXT

// QLCDNumber
//#define QT_NO_LCDNUMBER

// QLineEdit
//#define QT_NO_LINEEDIT

// QMessageBox
//#define QT_NO_MESSAGEBOX

// QMovie
//#define QT_NO_MOVIE

// QNetworkInterface
//#define QT_NO_NETWORKINTERFACE

// QNetworkProxy
//#define QT_NO_NETWORKPROXY

// Qt::WA_PaintOnScreen
//#define QT_NO_PAINTONSCREEN

// QPicture
//#define QT_NO_PICTURE

// QProcess
//#define QT_NO_PROCESS

// QProgressBar
//#define QT_NO_PROGRESSBAR

// Properties
//#define QT_NO_PROPERTIES

//  Universally Unique Identifier Convertion
//#define QT_NO_QUUID_STRING

// Raster Paint Engine callback functions
//#define QT_NO_RASTERCALLBACKS

// Resize Handler
//#define QT_NO_RESIZEHANDLER

// QRubberBand
//#define QT_NO_RUBBERBAND

// Session Manager
//#define QT_NO_SESSIONMANAGER

// QSettings
//#define QT_NO_SETTINGS

// QSharedMemory
//#define QT_NO_SHAREDMEMORY

// QShortcut
//#define QT_NO_SHORTCUT

// QSignalMapper
//#define QT_NO_SIGNALMAPPER

// QSizeGrip
//#define QT_NO_SIZEGRIP

// QSlider
//#define QT_NO_SLIDER

// Sounds
//#define QT_NO_SOUND

// Spin Widget
//#define QT_NO_SPINWIDGET

// Splash screen widget
//#define QT_NO_SPLASHSCREEN

// QStackedWidget
//#define QT_NO_STACKEDWIDGET

// QStatusBar
//#define QT_NO_STATUSBAR

// Status Tip
//#define QT_NO_STATUSTIP

// QMotifStyle
//#define QT_NO_STYLE_MOTIF

// QWindowsStyle
//#define QT_NO_STYLE_WINDOWS

// QSystemSemaphore
//#define QT_NO_SYSTEMSEMAPHORE

// QSystemTrayIcon
//#define QT_NO_SYSTEMTRAYICON

// QTabletEvent
//#define QT_NO_TABLETEVENT

// QTemporaryFile
//#define QT_NO_TEMPORARYFILE

// QTextCodec
//#define QT_NO_TEXTCODEC

// HtmlParser
//#define QT_NO_TEXTHTMLPARSER

// QToolTip
//#define QT_NO_TOOLTIP

// QUdpSocket
//#define QT_NO_UDPSOCKET

// QUndoCommand
//#define QT_NO_UNDOCOMMAND

// QUrlInfo
//#define QT_NO_URLINFO

// QValidator
//#define QT_NO_VALIDATOR

// QWheelEvent
//#define QT_NO_WHEELEVENT


// QButtonGroup
#if !defined(QT_NO_BUTTONGROUP) && (defined(QT_NO_GROUPBOX))
#define QT_NO_BUTTONGROUP
#endif

// Codecs
#if !defined(QT_NO_CODECS) && (defined(QT_NO_TEXTCODEC))
#define QT_NO_CODECS
#endif

// QDial
#if !defined(QT_NO_DIAL) && (defined(QT_NO_SLIDER))
#define QT_NO_DIAL
#endif

// QFileSystemModel
#if !defined(QT_NO_FILESYSTEMMODEL) && (defined(QT_NO_FILESYSTEMWATCHER))
#define QT_NO_FILESYSTEMMODEL
#endif

// QMenu
#if !defined(QT_NO_MENU) && (defined(QT_NO_ACTION))
#define QT_NO_MENU
#endif

// QNetworkDiskCache
#if !defined(QT_NO_NETWORKDISKCACHE) && (defined(QT_NO_TEMPORARYFILE))
#define QT_NO_NETWORKDISKCACHE
#endif

// QProgressDialog
#if !defined(QT_NO_PROGRESSDIALOG) && (defined(QT_NO_PROGRESSBAR))
#define QT_NO_PROGRESSDIALOG
#endif

// QScrollBar
#if !defined(QT_NO_SCROLLBAR) && (defined(QT_NO_SLIDER))
#define QT_NO_SCROLLBAR
#endif

//  SOCKS5
#if !defined(QT_NO_SOCKS5) && (defined(QT_NO_NETWORKPROXY))
#define QT_NO_SOCKS5
#endif

// QSplitter
#if !defined(QT_NO_SPLITTER) && (defined(QT_NO_RUBBERBAND))
#define QT_NO_SPLITTER
#endif

// QCDEStyle
#if !defined(QT_NO_STYLE_CDE) && (defined(QT_NO_STYLE_MOTIF))
#define QT_NO_STYLE_CDE
#endif

// QWindowsXPStyle
#if !defined(QT_NO_STYLE_WINDOWSXP) && (defined(QT_NO_STYLE_WINDOWS))
#define QT_NO_STYLE_WINDOWSXP
#endif

// QToolButton
#if !defined(QT_NO_TOOLBUTTON) && (defined(QT_NO_ACTION))
#define QT_NO_TOOLBUTTON
#endif

// QUndoStack
#if !defined(QT_NO_UNDOSTACK) && (defined(QT_NO_UNDOCOMMAND))
#define QT_NO_UNDOSTACK
#endif

// Context menu
#if !defined(QT_NO_CONTEXTMENU) && (defined(QT_NO_MENU))
#define QT_NO_CONTEXTMENU
#endif

// QtDBus module
#if !defined(QT_NO_DBUS) && defined(QT_NO_DOM)
#define QT_NO_DBUS
#endif

// File Transfer Protocol
#if !defined(QT_NO_FTP) && (defined(QT_NO_URLINFO))
#define QT_NO_FTP
#endif

// QScrollArea
#if !defined(QT_NO_SCROLLAREA) && (defined(QT_NO_SCROLLBAR))
#define QT_NO_SCROLLAREA
#endif

// QWindowsVistaStyle
#if !defined(QT_NO_STYLE_WINDOWSVISTA) && (defined(QT_NO_STYLE_WINDOWSXP))
#define QT_NO_STYLE_WINDOWSVISTA
#endif

// QTabBar
#if !defined(QT_NO_TABBAR) && (defined(QT_NO_TOOLBUTTON))
#define QT_NO_TABBAR
#endif

// OdfWriter
#if ! defined(QT_NO_TEXTODFWRITER) && (0)   // removed test which was always false
#define QT_NO_TEXTODFWRITER
#endif

// QUndoGroup
#if !defined(QT_NO_UNDOGROUP) && (defined(QT_NO_UNDOSTACK))
#define QT_NO_UNDOGROUP
#endif

// QWhatsThis
#if !defined(QT_NO_WHATSTHIS) && (defined(QT_NO_TOOLBUTTON))
#define QT_NO_WHATSTHIS
#endif

// Drag and drop
#if !defined(QT_NO_DRAGANDDROP) && defined(QT_NO_IMAGEFORMAT_XPM)
#define QT_NO_DRAGANDDROP
#endif

// QGraphicsView
#if !defined(QT_NO_GRAPHICSVIEW) && (defined(QT_NO_SCROLLAREA))
#define QT_NO_GRAPHICSVIEW
#endif

// Hyper Text Transfer Protocol
#if !defined(QT_NO_HTTP) && defined(QT_NO_HOSTINFO)
#define QT_NO_HTTP
#endif

// QMdiArea
#if !defined(QT_NO_MDIAREA) && (defined(QT_NO_SCROLLAREA))
#define QT_NO_MDIAREA
#endif

// QPrinter
#if !defined(QT_NO_PRINTER) && (defined(QT_NO_PICTURE) || defined(QT_NO_TEMPORARYFILE))
#define QT_NO_PRINTER
#endif

// QSpinBox
#if !defined(QT_NO_SPINBOX) && (defined(QT_NO_SPINWIDGET) || defined(QT_NO_LINEEDIT) || defined(QT_NO_VALIDATOR))
#define QT_NO_SPINBOX
#endif

// QCleanLooksStyle
#if !defined(QT_NO_STYLE_CLEANLOOKS) && (defined(QT_NO_STYLE_WINDOWS) || defined(QT_NO_IMAGEFORMAT_XPM))
#define QT_NO_STYLE_CLEANLOOKS
#endif

// QPlastiqueStyle
#if !defined(QT_NO_STYLE_PLASTIQUE) && (defined(QT_NO_STYLE_WINDOWS) || defined(QT_NO_IMAGEFORMAT_XPM))
#define QT_NO_STYLE_PLASTIQUE
#endif

// QStyleSheetStyle
#if !defined(QT_NO_STYLE_STYLESHEET) && (defined(QT_NO_STYLE_WINDOWS) || defined(QT_NO_CSSPARSER))
#define QT_NO_STYLE_STYLESHEET
#endif

// QtSvg module
#if !defined(QT_NO_SVG) && defined(QT_NO_CSSPARSER)
#define QT_NO_SVG
#endif

// QTextCodecPlugin
#if !defined(QT_NO_TEXTCODECPLUGIN) && (defined(QT_NO_TEXTCODEC))
#define QT_NO_TEXTCODECPLUGIN
#endif

// QColorDialog
#if !defined(QT_NO_COLORDIALOG) && (defined(QT_NO_SPINBOX))
#define QT_NO_COLORDIALOG
#endif

// QGraphicsEffect
#if !defined(QT_NO_GRAPHICSEFFECT) && (defined(QT_NO_GRAPHICSVIEW))
#define QT_NO_GRAPHICSEFFECT
#endif

// The Model/View Framework
#if !defined(QT_NO_ITEMVIEWS) && (defined(QT_NO_RUBBERBAND) || defined(QT_NO_SCROLLAREA))
#define QT_NO_ITEMVIEWS
#endif

// QMenuBar
#if !defined(QT_NO_MENUBAR) && (defined(QT_NO_MENU) || defined(QT_NO_TOOLBUTTON))
#define QT_NO_MENUBAR
#endif

// QSvgGenerator
#if !defined(QT_NO_SVGGENERATOR) && (defined(QT_NO_SVG))
#define QT_NO_SVGGENERATOR
#endif

// QSvgRenderer
#if !defined(QT_NO_SVGRENDERER) && (defined(QT_NO_SVG))
#define QT_NO_SVGRENDERER
#endif

// QTabWidget
#if !defined(QT_NO_TABWIDGET) && (defined(QT_NO_TABBAR) || defined(QT_NO_STACKEDWIDGET))
#define QT_NO_TABWIDGET
#endif

// QTextEdit
#if !defined(QT_NO_TEXTEDIT) && (defined(QT_NO_SCROLLAREA))
#define QT_NO_TEXTEDIT
#endif

// QErrorMessage
#if !defined(QT_NO_ERRORMESSAGE) && (defined(QT_NO_TEXTEDIT))
#define QT_NO_ERRORMESSAGE
#endif

// QListView
#if !defined(QT_NO_LISTVIEW) && (defined(QT_NO_ITEMVIEWS))
#define QT_NO_LISTVIEW
#endif

// QMainWindow
#if !defined(QT_NO_MAINWINDOW) && (defined(QT_NO_MENU) || defined(QT_NO_RESIZEHANDLER) || defined(QT_NO_TOOLBUTTON))
#define QT_NO_MAINWINDOW
#endif

// QAbstractProxyModel
#if !defined(QT_NO_PROXYMODEL) && (defined(QT_NO_ITEMVIEWS))
#define QT_NO_PROXYMODEL
#endif

// QStandardItemModel
#if !defined(QT_NO_STANDARDITEMMODEL) && (defined(QT_NO_ITEMVIEWS))
#define QT_NO_STANDARDITEMMODEL
#endif

// QStringListModel
#if !defined(QT_NO_STRINGLISTMODEL) && (defined(QT_NO_ITEMVIEWS))
#define QT_NO_STRINGLISTMODEL
#endif

// QSvgWidget
#if !defined(QT_NO_SVGWIDGET) && (defined(QT_NO_SVGRENDERER))
#define QT_NO_SVGWIDGET
#endif

// QSyntaxHighlighter
#if !defined(QT_NO_SYNTAXHIGHLIGHTER) && (defined(QT_NO_TEXTEDIT))
#define QT_NO_SYNTAXHIGHLIGHTER
#endif

// QTableView
#if !defined(QT_NO_TABLEVIEW) && (defined(QT_NO_ITEMVIEWS))
#define QT_NO_TABLEVIEW
#endif

// QTextBrowser
#if !defined(QT_NO_TEXTBROWSER) && (defined(QT_NO_TEXTEDIT))
#define QT_NO_TEXTBROWSER
#endif

// QToolBox
#if !defined(QT_NO_TOOLBOX) && (defined(QT_NO_TOOLBUTTON) || defined(QT_NO_SCROLLAREA))
#define QT_NO_TOOLBOX
#endif

// QTreeView
#if !defined(QT_NO_TREEVIEW) && (defined(QT_NO_ITEMVIEWS))
#define QT_NO_TREEVIEW
#endif

// Accessibility
#if !defined(QT_NO_ACCESSIBILITY) && defined(QT_NO_MENUBAR)
#define QT_NO_ACCESSIBILITY
#endif

// QColumnView
#if !defined(QT_NO_COLUMNVIEW) && (defined(QT_NO_LISTVIEW))
#define QT_NO_COLUMNVIEW
#endif

// QCompleter
#if !defined(QT_NO_COMPLETER) && (defined(QT_NO_PROXYMODEL))
#define QT_NO_COMPLETER
#endif

// Common UNIX Printing System
#if !defined(QT_NO_CUPS) && (defined(QT_NO_PRINTER))
#define QT_NO_CUPS
#endif

// QDataWidgetMapper
#if !defined(QT_NO_DATAWIDGETMAPPER) && defined(QT_NO_ITEMVIEWS)
#define QT_NO_DATAWIDGETMAPPER
#endif

// QListWidget
#if ! defined(QT_NO_LISTWIDGET) && defined(QT_NO_LISTVIEW)
#define QT_NO_LISTWIDGET
#endif

// QSortFilterProxyModel
#if !defined(QT_NO_SORTFILTERPROXYMODEL) && (defined(QT_NO_PROXYMODEL))
#define QT_NO_SORTFILTERPROXYMODEL
#endif

// QTableWidget
#if !defined(QT_NO_TABLEWIDGET) && (defined(QT_NO_TABLEVIEW))
#define QT_NO_TABLEWIDGET
#endif

// QToolBar
#if !defined(QT_NO_TOOLBAR) && (defined(QT_NO_MAINWINDOW))
#define QT_NO_TOOLBAR
#endif

// QTreeWidget
#if !defined(QT_NO_TREEWIDGET) && (defined(QT_NO_TREEVIEW))
#define QT_NO_TREEWIDGET
#endif

// QDirModel
#if !defined(QT_NO_DIRMODEL) && (defined(QT_NO_ITEMVIEWS) || defined(QT_NO_FILESYSTEMMODEL))
#define QT_NO_DIRMODEL
#endif

// QDockwidget
#if !defined(QT_NO_DOCKWIDGET) && (defined(QT_NO_RUBBERBAND) || defined(QT_NO_MAINWINDOW))
#define QT_NO_DOCKWIDGET
#endif

// QUndoView
#if !defined(QT_NO_UNDOVIEW) && (defined(QT_NO_UNDOSTACK) || defined(QT_NO_LISTVIEW))
#define QT_NO_UNDOVIEW
#endif

// QCompleter
#if !defined(QT_NO_FSCOMPLETER) && (defined(QT_NO_FILESYSTEMMODEL) || defined(QT_NO_COMPLETER))
#define QT_NO_FSCOMPLETER
#endif

// QGraphicsSvgItem
#if !defined(QT_NO_GRAPHICSSVGITEM) && (defined(QT_NO_SVGRENDERER) || defined(QT_NO_GRAPHICSVIEW))
#define QT_NO_GRAPHICSSVGITEM
#endif

// QComboBox
#if !defined(QT_NO_COMBOBOX) && (defined(QT_NO_LINEEDIT) || defined(QT_NO_STANDARDITEMMODEL) || defined(QT_NO_LISTVIEW))
#define QT_NO_COMBOBOX
#endif

// QWorkSpace
#if !defined(QT_NO_WORKSPACE) && (defined(QT_NO_SCROLLBAR) || defined(QT_NO_MAINWINDOW) || defined(QT_NO_MENUBAR))
#define QT_NO_WORKSPACE
#endif

// QPrintPreviewWidget
#if !defined(QT_NO_PRINTPREVIEWWIDGET) && (defined(QT_NO_GRAPHICSVIEW) || defined(QT_NO_PRINTER) || defined(QT_NO_MAINWINDOW))
#define QT_NO_PRINTPREVIEWWIDGET
#endif

// QCalendarWidget
#if !defined(QT_NO_CALENDARWIDGET) && (defined(QT_NO_TABLEVIEW) || defined(QT_NO_MENU) || defined(QT_NO_SPINBOX) || defined(QT_NO_TOOLBUTTON))
#define QT_NO_CALENDARWIDGET
#endif

// QDateTimeEdit
#if !defined(QT_NO_DATETIMEEDIT) && defined(QT_NO_CALENDARWIDGET)
#define QT_NO_DATETIMEEDIT
#endif

// QInputDialog
#if !defined(QT_NO_INPUTDIALOG) && (defined(QT_NO_COMBOBOX) || defined(QT_NO_SPINBOX) || defined(QT_NO_STACKEDWIDGET))
#define QT_NO_INPUTDIALOG
#endif

// QFontComboBox
#if !defined(QT_NO_FONTCOMBOBOX) && (defined(QT_NO_COMBOBOX) || defined(QT_NO_STRINGLISTMODEL))
#define QT_NO_FONTCOMBOBOX
#endif

// QFontDialog
#if !defined(QT_NO_FONTDIALOG) && (defined(QT_NO_STRINGLISTMODEL) || defined(QT_NO_COMBOBOX) || defined(QT_NO_VALIDATOR) || defined(QT_NO_GROUPBOX))
#define QT_NO_FONTDIALOG
#endif

// QPrintDialog
#if !defined(QT_NO_PRINTDIALOG) && (defined(QT_NO_PRINTER) || defined(QT_NO_COMBOBOX) || defined(QT_NO_BUTTONGROUP) || defined(QT_NO_SPINBOX) || defined(QT_NO_TREEVIEW) || defined(QT_NO_TABWIDGET))
#define QT_NO_PRINTDIALOG
#endif

// QFileDialog
#if !defined(QT_NO_FILEDIALOG) && (defined(QT_NO_DIRMODEL) || defined(QT_NO_TREEVIEW) || defined(QT_NO_COMBOBOX) || defined(QT_NO_TOOLBUTTON) || defined(QT_NO_BUTTONGROUP) || defined(QT_NO_TOOLTIP) || defined(QT_NO_SPLITTER) || defined(QT_NO_STACKEDWIDGET) || defined(QT_NO_PROXYMODEL))
#define QT_NO_FILEDIALOG
#endif

// QPrintPreviewDialog
#if !defined(QT_NO_PRINTPREVIEWDIALOG) && (defined(QT_NO_PRINTPREVIEWWIDGET) || defined(QT_NO_PRINTDIALOG) || defined(QT_NO_TOOLBAR))
#define QT_NO_PRINTPREVIEWDIALOG
#endif

//Input Method
#if defined(QT_NO_IM)
#define QT_NO_XIM
#endif

#endif