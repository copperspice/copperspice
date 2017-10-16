GUI_DIRS += src/gui/text

GUI_PUBLIC_INCLUDES += \
	QAbstractTextDocumentLayout \
	QAbstractGraphicsShapeItem \
	QAbstractUndoItem \
	QFont \
	QFontDatabase \
	QFontInfo \
	QFontMetrics \
	QFontMetricsF \
	QGlyphRun \
	QRawFont \
	QPlatformFontDatabase \
	QStaticText \
	QSupportedWritingSystems \
	QSyntaxHighlighter \
	QTextCursor \
	QTextBlock \
	QTextBlockFormat \
	QTextBlockGroup \
	QTextBlockUserData \
	QTextCharFormat \
	QTextDocument \
	QTextDocumentFragment \
	QTextDocumentWriter \
	QTextFormat \
	QTextFragment \
	QTextFrame \
	QTextFrameFormat \
	QTextFrameLayoutData \
	QTextInlineObject \
	QTextImageFormat \
	QTextLayout \
	QTextLine \
	QTextListFormat \
	QTextLength \
	QTextList \
	QTextObject \
	QTextObjectInterface \
	QTextOption \
	QTextTable \
	QTextTableCell \
	QTextTableCellFormat \
	QTextTableFormat

GUI_TEXT_INCLUDES = \
	qabstractgraphicsshapeitem.h \
	qabstracttextdocumentlayout.h \
	qabstractundoitem.h \
	qfont.h \
	qfontdatabase.h \
	qfontinfo.h \
	qfontmetrics.h \
	qfontmetricsf.h \
	qglyphrun.h \
	qplatformfontdatabase.h \
	qplatformfontdatabase_qpa.h \
	qrawfont.h \
	qstatictext.h \
	qsupportedwritingsystems.h \
	qsyntaxhighlighter.h \
	qtextblock.h \
	qtextblockformat.h \
	qtextblockgroup.h \
	qtextblockuserdata.h \
	qtextcharformat.h \
	qtextcursor.h \
	qtextdocument.h \
	qtextdocumentfragment.h \
	qtextdocumentwriter.h \
	qtextformat.h \
	qtextfragment.h \
	qtextframe.h \
	qtextframeformat.h \
	qtextframelayoutdata.h \
	qtextinlineobject.h \
	qtextimageformat.h \
	qtextlayout.h \
	qtextline.h \
	qtextlistformat.h \
	qtextlength.h \
	qtextlist.h \
	qtextobject.h \
	qtextobjectinterface.h \
	qtextoption.h \
	qtexttable.h \
	qtexttablecell.h \
	qtexttablecellformat.h \
	qtexttableformat.h

GUI_TEXT_PRIVATE_INCLUDES = \
	qabstracttextdocumentlayout_p.h \
	qcssparser_p.h \
	qfont_p.h \
	qfontdatabase_mac.cpp \
	qfontengine_coretext_p.h \
	qfontengine_mac_p.h \
	qfontengine_p.h \
	qfontengine_win_p.h \
	qfontengine_x11_p.h \
	qfontengine_ft_p.h \
	qfontengineglyphcache_p.h \
	qfontsubset_p.h \
	qfragmentmap_p.h \
	qglyphrun_p.h \
	qrawfont_p.h \
	qstatictext_p.h \
	qtextcontrol_p.h \
	qtextcontrol_p_p.h \
	qtextcursor_p.h \
	qtextdocument_p.h \
	qtextdocumentfragment_p.h \
	qtextdocumentlayout_p.h \
	qtextengine_p.h \
	qtextengine_mac.cpp \
	qtextformat_p.h \
	qtexthtmlparser_p.h \
	qtextimagehandler_p.h \
	qtextobject_p.h \
	qtextodfwriter_p.h \
	qtexttable_p.h \
	qzipreader_p.h \
	qzipwriter_p.h \
	qfontdatabase_x11.cpp \
	qfontdatabase_win.cpp \
	qfontenginedirectwrite_p.h \
	qcssscanner.cpp


GUI_INCLUDES += $(GUI_TEXT_INCLUDES)
GUI_PRIVATE_INCLUDES += $(GUI_TEXT_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/gui/text/%,$(GUI_TEXT_INCLUDES)) \
		$(patsubst %,src/gui/text/%,$(GUI_TEXT_PRIVATE_INCLUDES))

lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/text/qfont.cpp \
	src/gui/text/qfontengine.cpp \
	src/gui/text/qfontsubset.cpp \
	src/gui/text/qfontmetrics.cpp \
	src/gui/text/qfontdatabase.cpp \
	src/gui/text/qtextcontrol.cpp \
	src/gui/text/qtextengine.cpp \
	src/gui/text/qtextlayout.cpp \
	src/gui/text/qtextformat.cpp \
	src/gui/text/qtextobject.cpp \
	src/gui/text/qtextoption.cpp \
	src/gui/text/qtextdocument.cpp \
	src/gui/text/qtextdocument_p.cpp \
	src/gui/text/qtexthtmlparser.cpp \
	src/gui/text/qabstracttextdocumentlayout.cpp \
	src/gui/text/qtextdocumentlayout.cpp \
	src/gui/text/qtextcursor.cpp \
	src/gui/text/qtextdocumentfragment.cpp \
	src/gui/text/qtextimagehandler.cpp \
	src/gui/text/qtexttable.cpp \
	src/gui/text/qtextlist.cpp \
	src/gui/text/qtextdocumentwriter.cpp \
	src/gui/text/qsyntaxhighlighter.cpp \
	src/gui/text/qcssparser.cpp \
	src/gui/text/qzip.cpp \
	src/gui/text/qtextodfwriter.cpp \
	src/gui/text/qstatictext.cpp \
	src/gui/text/qrawfont.cpp \
	src/gui/text/qglyphrun.cpp

if OSTYPE_WIN
lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/text/qfont_win.cpp \
	src/gui/text/qfontengine_win.cpp \
	src/gui/text/qfontenginedirectwrite.cpp \
	src/gui/text/qrawfont_win.cpp
endif

if X11
lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/text/qfont_x11.cpp \
	src/gui/text/qfontengine_x11.cpp \
	src/gui/text/qfontengine_ft.cpp \
	src/gui/text/qrawfont_ft.cpp
endif

if COCOA
lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/text/qfont_mac.cpp \
	src/gui/text/qrawfont_mac.cpp \
	src/gui/text/qfontengine_mac.mm \
	src/gui/text/qfontengine_coretext.mm
endif

# missing a bunch of files from 3rdparty/freetype, not sure
