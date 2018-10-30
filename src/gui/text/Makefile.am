# Harfbuzz settings
lib_libCsGui@BUILD_ABI@_la_CXXFLAGS += \
   -DHAVE_ATEXIT \
   -DHB_EXTERN=  \
   -DHB_NO_UNICODE_FUNCS \
   -DHB_NDEBUG

if OSTYPE_WIN
lib_libCsGui@BUILD_ABI@_la_CXXFLAGS += \
   -DHB_NO_WIN1256
endif

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
	qcssscanner.cpp \
	qharfbuzz_p.h


GUI_INCLUDES += $(GUI_TEXT_INCLUDES)
GUI_PRIVATE_INCLUDES += $(GUI_TEXT_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/gui/text/%,$(GUI_TEXT_INCLUDES)) \
	$(patsubst %,src/gui/text/%,$(GUI_TEXT_PRIVATE_INCLUDES)) \
   src/3rdparty/harfbuzz/src/hb.h \
   src/3rdparty/harfbuzz/src/hb.hh \
   src/3rdparty/harfbuzz/src/hb-atomic.hh \
   src/3rdparty/harfbuzz/src/hb-aat-ltag-table.hh \
   src/3rdparty/harfbuzz/src/hb-aat-layout.hh \
   src/3rdparty/harfbuzz/src/hb-aat-layout-ankr-table.hh \
   src/3rdparty/harfbuzz/src/hb-aat-layout-bsln-table.hh \
   src/3rdparty/harfbuzz/src/hb-aat-layout-common.hh \
   src/3rdparty/harfbuzz/src/hb-aat-layout-feat-table.hh \
   src/3rdparty/harfbuzz/src/hb-aat-layout-kerx-table.hh \
   src/3rdparty/harfbuzz/src/hb-aat-layout-morx-table.hh \
   src/3rdparty/harfbuzz/src/hb-aat-layout-trak-table.hh \
   src/3rdparty/harfbuzz/src/hb-aat-map.hh \
   src/3rdparty/harfbuzz/src/hb-blob.h \
   src/3rdparty/harfbuzz/src/hb-blob.hh \
   src/3rdparty/harfbuzz/src/hb-buffer.h \
   src/3rdparty/harfbuzz/src/hb-buffer.hh \
   src/3rdparty/harfbuzz/src/hb-common.h \
   src/3rdparty/harfbuzz/src/hb-deprecated.h \
   src/3rdparty/harfbuzz/src/hb-debug.hh \
   src/3rdparty/harfbuzz/src/hb-dsalgs.hh \
   src/3rdparty/harfbuzz/src/hb-face.h \
   src/3rdparty/harfbuzz/src/hb-face.hh \
   src/3rdparty/harfbuzz/src/hb-font.h \
   src/3rdparty/harfbuzz/src/hb-font.hh \
   src/3rdparty/harfbuzz/src/hb-iter.hh \
   src/3rdparty/harfbuzz/src/hb-machinery.hh \
   src/3rdparty/harfbuzz/src/hb-map.h \
   src/3rdparty/harfbuzz/src/hb-map.hh \
   src/3rdparty/harfbuzz/src/hb-mutex.hh \
   src/3rdparty/harfbuzz/src/hb-null.hh \
   src/3rdparty/harfbuzz/src/hb-object.hh \
   src/3rdparty/harfbuzz/src/hb-open-file.hh \
   src/3rdparty/harfbuzz/src/hb-open-type.hh \
   src/3rdparty/harfbuzz/src/hb-ot.h \
   src/3rdparty/harfbuzz/src/hb-ot-color.h \
   src/3rdparty/harfbuzz/src/hb-ot-color-cbdt-table.hh \
   src/3rdparty/harfbuzz/src/hb-ot-color-sbix-table.hh \
   src/3rdparty/harfbuzz/src/hb-ot-color-svg-table.hh \
   src/3rdparty/harfbuzz/src/hb-ot-cmap-table.hh \
   src/3rdparty/harfbuzz/src/hb-ot-face.hh \
   src/3rdparty/harfbuzz/src/hb-ot-font.h \
   src/3rdparty/harfbuzz/src/hb-ot-glyf-table.hh \
   src/3rdparty/harfbuzz/src/hb-ot-head-table.hh \
   src/3rdparty/harfbuzz/src/hb-ot-hhea-table.hh \
   src/3rdparty/harfbuzz/src/hb-ot-hmtx-table.hh \
   src/3rdparty/harfbuzz/src/hb-ot-kern-table.hh \
   src/3rdparty/harfbuzz/src/hb-ot-layout.h \
   src/3rdparty/harfbuzz/src/hb-ot-layout.hh \
   src/3rdparty/harfbuzz/src/hb-ot-layout-base-table.hh \
   src/3rdparty/harfbuzz/src/hb-ot-layout-common.hh \
   src/3rdparty/harfbuzz/src/hb-ot-layout-gdef-table.hh \
   src/3rdparty/harfbuzz/src/hb-ot-layout-gpos-table.hh \
   src/3rdparty/harfbuzz/src/hb-ot-layout-gsub-table.hh \
   src/3rdparty/harfbuzz/src/hb-ot-layout-gsubgpos.hh \
   src/3rdparty/harfbuzz/src/hb-ot-layout-jstf-table.hh \
   src/3rdparty/harfbuzz/src/hb-ot-map.hh \
   src/3rdparty/harfbuzz/src/hb-ot-maxp-table.hh \
   src/3rdparty/harfbuzz/src/hb-ot-math.h \
   src/3rdparty/harfbuzz/src/hb-ot-name.h \
   src/3rdparty/harfbuzz/src/hb-ot-name-language.hh \
   src/3rdparty/harfbuzz/src/hb-ot-name-table.hh \
   src/3rdparty/harfbuzz/src/hb-ot-os2-table.hh \
   src/3rdparty/harfbuzz/src/hb-ot-os2-unicode-ranges.hh \
   src/3rdparty/harfbuzz/src/hb-ot-post-macroman.hh \
   src/3rdparty/harfbuzz/src/hb-ot-post-table.hh \
   src/3rdparty/harfbuzz/src/hb-ot-shape.h \
   src/3rdparty/harfbuzz/src/hb-ot-shape.hh \
   src/3rdparty/harfbuzz/src/hb-ot-shape-complex.hh \
   src/3rdparty/harfbuzz/src/hb-ot-shape-complex-arabic.hh \
   src/3rdparty/harfbuzz/src/hb-ot-shape-complex-arabic-table.hh \
   src/3rdparty/harfbuzz/src/hb-ot-shape-complex-arabic-fallback.hh \
   src/3rdparty/harfbuzz/src/hb-ot-shape-complex-indic.hh \
   src/3rdparty/harfbuzz/src/hb-ot-shape-complex-indic-machine.hh \
   src/3rdparty/harfbuzz/src/hb-ot-shape-complex-khmer.hh \
   src/3rdparty/harfbuzz/src/hb-ot-shape-complex-khmer-machine.hh \
   src/3rdparty/harfbuzz/src/hb-ot-shape-complex-myanmar.hh \
   src/3rdparty/harfbuzz/src/hb-ot-shape-complex-myanmar-machine.hh \
   src/3rdparty/harfbuzz/src/hb-ot-shape-complex-use.hh \
   src/3rdparty/harfbuzz/src/hb-ot-shape-complex-use-machine.hh \
   src/3rdparty/harfbuzz/src/hb-ot-shape-complex-vowel-constraints.hh \
   src/3rdparty/harfbuzz/src/hb-ot-shape-fallback.hh \
   src/3rdparty/harfbuzz/src/hb-ot-shape-normalize.hh \
   src/3rdparty/harfbuzz/src/hb-ot-tag-table.hh \
   src/3rdparty/harfbuzz/src/hb-ot-var.h \
   src/3rdparty/harfbuzz/src/hb-ot-var-avar-table.hh \
   src/3rdparty/harfbuzz/src/hb-ot-var-fvar-table.hh \
   src/3rdparty/harfbuzz/src/hb-ot-var-hvar-table.hh \
   src/3rdparty/harfbuzz/src/hb-ot-var-mvar-table.hh \
   src/3rdparty/harfbuzz/src/hb-ot-vorg-table.hh \
   src/3rdparty/harfbuzz/src/hb-set.h \
   src/3rdparty/harfbuzz/src/hb-set.hh \
   src/3rdparty/harfbuzz/src/hb-set-digest.hh \
   src/3rdparty/harfbuzz/src/hb-shape.h \
   src/3rdparty/harfbuzz/src/hb-shape-plan.h \
   src/3rdparty/harfbuzz/src/hb-shape-plan.hh \
   src/3rdparty/harfbuzz/src/hb-shaper.hh \
   src/3rdparty/harfbuzz/src/hb-shaper-impl.hh \
   src/3rdparty/harfbuzz/src/hb-shaper-list.hh \
   src/3rdparty/harfbuzz/src/hb-string-array.hh \
   src/3rdparty/harfbuzz/src/hb-subset.h \
   src/3rdparty/harfbuzz/src/hb-subset.hh \
   src/3rdparty/harfbuzz/src/hb-subset-glyf.hh \
   src/3rdparty/harfbuzz/src/hb-subset-input.hh \
   src/3rdparty/harfbuzz/src/hb-subset-plan.hh \
   src/3rdparty/harfbuzz/src/hb-utf.hh \
   src/3rdparty/harfbuzz/src/hb-unicode.h \
   src/3rdparty/harfbuzz/src/hb-unicode.hh \
   src/3rdparty/harfbuzz/src/hb-unicode-emoji-table.hh \
   src/3rdparty/harfbuzz/src/hb-vector.hh \
   src/3rdparty/harfbuzz/src/hb-version.h

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
	src/gui/text/qglyphrun.cpp \
   src/gui/text/qharfbuzz.cpp \
   \
   src/3rdparty/harfbuzz/src/hb-aat-layout.cc \
   src/3rdparty/harfbuzz/src/hb-aat-map.cc \
   src/3rdparty/harfbuzz/src/hb-blob.cc \
   src/3rdparty/harfbuzz/src/hb-buffer.cc \
   src/3rdparty/harfbuzz/src/hb-common.cc \
   src/3rdparty/harfbuzz/src/hb-face.cc \
   src/3rdparty/harfbuzz/src/hb-font.cc \
   src/3rdparty/harfbuzz/src/hb-ot-face.cc \
   src/3rdparty/harfbuzz/src/hb-ot-font.cc \
   src/3rdparty/harfbuzz/src/hb-ot-layout.cc \
   src/3rdparty/harfbuzz/src/hb-ot-map.cc \
   src/3rdparty/harfbuzz/src/hb-ot-shape.cc \
   src/3rdparty/harfbuzz/src/hb-ot-shape-complex-arabic.cc \
   src/3rdparty/harfbuzz/src/hb-ot-shape-complex-default.cc \
   src/3rdparty/harfbuzz/src/hb-ot-shape-complex-hangul.cc \
   src/3rdparty/harfbuzz/src/hb-ot-shape-complex-hebrew.cc \
   src/3rdparty/harfbuzz/src/hb-ot-shape-complex-indic.cc \
   src/3rdparty/harfbuzz/src/hb-ot-shape-complex-indic-table.cc \
   src/3rdparty/harfbuzz/src/hb-ot-shape-complex-khmer.cc \
   src/3rdparty/harfbuzz/src/hb-ot-shape-complex-myanmar.cc \
   src/3rdparty/harfbuzz/src/hb-ot-shape-complex-thai.cc \
   src/3rdparty/harfbuzz/src/hb-ot-shape-complex-use.cc \
   src/3rdparty/harfbuzz/src/hb-ot-shape-complex-use-table.cc \
   src/3rdparty/harfbuzz/src/hb-ot-shape-complex-vowel-constraints.cc \
   src/3rdparty/harfbuzz/src/hb-ot-shape-fallback.cc \
   src/3rdparty/harfbuzz/src/hb-ot-shape-normalize.cc \
   src/3rdparty/harfbuzz/src/hb-ot-tag.cc \
   src/3rdparty/harfbuzz/src/hb-ot-var.cc \
   src/3rdparty/harfbuzz/src/hb-set.cc \
   src/3rdparty/harfbuzz/src/hb-shape.cc \
   src/3rdparty/harfbuzz/src/hb-shape-plan.cc \
   src/3rdparty/harfbuzz/src/hb-shaper.cc \
   src/3rdparty/harfbuzz/src/hb-static.cc \
   src/3rdparty/harfbuzz/src/hb-unicode.cc

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

