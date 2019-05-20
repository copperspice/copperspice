# Harfbuzz settings
lib_libCsGui@BUILD_ABI@_la_CXXFLAGS += \
   -DHAVE_ATEXIT \
   -DHB_EXTERN=  \
   -DHB_NO_UNICODE_FUNCS \
   -DHB_NDEBUG \
   -DFT2_BUILD_LIBRARY

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
	qfontengine_p.h \
	qfontengine_qpf2_p.h \
	qfontengineglyphcache_p.h \
	qfontsubset_p.h \
	qfontsubset_agl_p.h \
	qfragmentmap_p.h \
	qglyphrun_p.h \
	qinputcontrol_p.h \
	qrawfont_p.h \
	qstatictext_p.h \
	qtextcontrol_p.h \
	qtextcontrol_p_p.h \
	qtextcursor_p.h \
	qtextdocument_p.h \
	qtextdocumentfragment_p.h \
	qtextdocumentlayout_p.h \
	qtextengine_p.h \
	qtextformat_p.h \
	qtexthtmlparser_p.h \
	qtextimagehandler_p.h \
	qtextobject_p.h \
	qtextodfwriter_p.h \
	qtexttable_p.h \
	qzipreader_p.h \
	qzipwriter_p.h \
	qcssscanner.cpp \
	qharfbuzz_gui_p.h

GUI_PUBLIC_INCLUDES += $(GUI_TEXT_PUBLIC_INCLUDES)
GUI_INCLUDES += $(GUI_TEXT_INCLUDES)
GUI_PRIVATE_INCLUDES += $(GUI_TEXT_PRIVATE_INCLUDES)

lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/text/qcssparser.cpp \
	src/gui/text/qfont.cpp \
	src/gui/text/qfontengine.cpp \
	src/gui/text/qfontengine_qpf2.cpp \
	src/gui/text/qfontengineglyphcache.cpp \
	src/gui/text/qfontsubset.cpp \
	src/gui/text/qfontmetrics.cpp \
	src/gui/text/qfontdatabase.cpp \
	src/gui/text/qharfbuzz_gui.cpp \
	src/gui/text/qglyphrun.cpp \
	src/gui/text/qinputcontrol.cpp \
	src/gui/text/qrawfont.cpp \
	src/gui/text/qstatictext.cpp \
	src/gui/text/qsyntaxhighlighter.cpp \
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
	src/gui/text/qtextodfwriter.cpp \
	src/gui/text/qzip.cpp \
	\
	src/3rdparty/freetype/src/autofit/autofit.c \
	src/3rdparty/freetype/src/base/ftapi.c \
	src/3rdparty/freetype/src/base/ftbase.c \
	src/3rdparty/freetype/src/base/ftbbox.c \
	src/3rdparty/freetype/src/base/ftbdf.c \
	src/3rdparty/freetype/src/base/ftbitmap.c \
	src/3rdparty/freetype/src/base/ftcid.c \
	src/3rdparty/freetype/src/base/ftdebug.c \
	src/3rdparty/freetype/src/base/ftfstype.c \
	src/3rdparty/freetype/src/base/ftgasp.c \
	src/3rdparty/freetype/src/base/ftglyph.c \
	src/3rdparty/freetype/src/base/ftgxval.c \
	src/3rdparty/freetype/src/base/ftinit.c \
	src/3rdparty/freetype/src/base/ftlcdfil.c \
	src/3rdparty/freetype/src/base/ftmm.c \
	src/3rdparty/freetype/src/base/ftotval.c \
	src/3rdparty/freetype/src/base/ftpatent.c \
	src/3rdparty/freetype/src/base/ftpfr.c \
	src/3rdparty/freetype/src/base/ftstroke.c \
	src/3rdparty/freetype/src/base/ftsynth.c \
	src/3rdparty/freetype/src/base/ftsystem.c \
	src/3rdparty/freetype/src/base/fttype1.c \
	src/3rdparty/freetype/src/base/ftwinfnt.c \
	src/3rdparty/freetype/src/base/ftxf86.c \
	src/3rdparty/freetype/src/base/md5.c \
	src/3rdparty/freetype/src/bdf/bdf.c \
	src/3rdparty/freetype/src/bzip2/ftbzip2.c \
	src/3rdparty/freetype/src/cache/ftccache.c \
	src/3rdparty/freetype/src/cache/ftcmru.c \
	src/3rdparty/freetype/src/cache/ftcmanag.c \
	src/3rdparty/freetype/src/cff/cff.c \
	src/3rdparty/freetype/src/cid/type1cid.c \
	src/3rdparty/freetype/src/gxvalid/gxvalid.c \
	src/3rdparty/freetype/src/gzip/ftgzip.c \
	src/3rdparty/freetype/src/lzw/ftlzw.c \
	src/3rdparty/freetype/src/otvalid/otvalid.c \
	src/3rdparty/freetype/src/pcf/pcf.c \
	src/3rdparty/freetype/src/pfr/pfrcmap.c \
	src/3rdparty/freetype/src/pfr/pfrdrivr.c \
	src/3rdparty/freetype/src/pfr/pfrgload.c \
	src/3rdparty/freetype/src/pfr/pfrload.c \
	src/3rdparty/freetype/src/pfr/pfrobjs.c \
	src/3rdparty/freetype/src/pfr/pfrsbit.c \
	src/3rdparty/freetype/src/psaux/psaux.c \
	src/3rdparty/freetype/src/pshinter/pshinter.c \
	src/3rdparty/freetype/src/psnames/psnames.c \
	src/3rdparty/freetype/src/raster/raster.c \
	src/3rdparty/freetype/src/sfnt/sfnt.c \
	src/3rdparty/freetype/src/smooth/smooth.c \
	src/3rdparty/freetype/src/truetype/truetype.c \
	src/3rdparty/freetype/src/type1/type1.c \
	src/3rdparty/freetype/src/type42/type42.c \
	src/3rdparty/freetype/src/winfonts/winfnt.c \
	\
	src/3rdparty/harfbuzz/fonts/resource_harfbuzz.cpp \
	src/3rdparty/harfbuzz/src/hb-aat-layout.cc \
	src/3rdparty/harfbuzz/src/hb-aat-map.cc \
	src/3rdparty/harfbuzz/src/hb-blob.cc \
	src/3rdparty/harfbuzz/src/hb-buffer-serialize.cc \
	src/3rdparty/harfbuzz/src/hb-buffer.cc \
	src/3rdparty/harfbuzz/src/hb-common.cc \
	src/3rdparty/harfbuzz/src/hb-face.cc \
	src/3rdparty/harfbuzz/src/hb-font.cc \
	src/3rdparty/harfbuzz/src/hb-ft.cc \
	src/3rdparty/harfbuzz/src/hb-map.cc \
	src/3rdparty/harfbuzz/src/hb-ot-cff1-table.cc \
	src/3rdparty/harfbuzz/src/hb-ot-cff2-table.cc \
	src/3rdparty/harfbuzz/src/hb-ot-color.cc \
	src/3rdparty/harfbuzz/src/hb-ot-face.cc \
	src/3rdparty/harfbuzz/src/hb-ot-font.cc \
	src/3rdparty/harfbuzz/src/hb-ot-layout.cc \
	src/3rdparty/harfbuzz/src/hb-ot-map.cc \
	src/3rdparty/harfbuzz/src/hb-ot-math.cc \
	src/3rdparty/harfbuzz/src/hb-ot-name-language.cc \
	src/3rdparty/harfbuzz/src/hb-ot-name.cc \
	src/3rdparty/harfbuzz/src/hb-ot-shape.cc \
	src/3rdparty/harfbuzz/src/hb-ot-shape-complex-arabic.cc \
	src/3rdparty/harfbuzz/src/hb-ot-shape-complex-default.cc \
	src/3rdparty/harfbuzz/src/hb-ot-shape-complex-hangul.cc \
	src/3rdparty/harfbuzz/src/hb-ot-shape-complex-hebrew.cc \
	src/3rdparty/harfbuzz/src/hb-ot-shape-complex-indic-table.cc \
	src/3rdparty/harfbuzz/src/hb-ot-shape-complex-indic.cc \
	src/3rdparty/harfbuzz/src/hb-ot-shape-complex-khmer.cc \
	src/3rdparty/harfbuzz/src/hb-ot-shape-complex-myanmar.cc \
	src/3rdparty/harfbuzz/src/hb-ot-shape-complex-thai.cc \
	src/3rdparty/harfbuzz/src/hb-ot-shape-complex-use-table.cc \
	src/3rdparty/harfbuzz/src/hb-ot-shape-complex-use.cc \
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
	src/3rdparty/harfbuzz/src/hb-subset.cc \
	src/3rdparty/harfbuzz/src/hb-subset-cff1.cc \
	src/3rdparty/harfbuzz/src/hb-subset-cff2.cc \
	src/3rdparty/harfbuzz/src/hb-subset-cff-common.cc \
	src/3rdparty/harfbuzz/src/hb-subset-glyf.cc \
	src/3rdparty/harfbuzz/src/hb-subset-input.cc \
	src/3rdparty/harfbuzz/src/hb-subset-plan.cc \
	src/3rdparty/harfbuzz/src/hb-unicode.cc \
	src/3rdparty/harfbuzz/src/hb-warning.cc

if GTK
lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/3rdparty/harfbuzz/src/hb-glib.cc \
	src/3rdparty/harfbuzz/src/hb-gobject-structs.cc
endif

if OSTYPE_DARWIN
# lib_libCsGui@BUILD_ABI@_la_SOURCES +=
#	src/3rdparty/harfbuzz/src/hb-coretext.cc
endif

noinst_HEADERS+=$(patsubst %,src/gui/text/%,$(GUI_TEXT_INCLUDES)) \
	$(patsubst %,src/gui/text/%,$(GUI_TEXT_PRIVATE_INCLUDES))
