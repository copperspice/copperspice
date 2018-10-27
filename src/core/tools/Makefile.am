# Harfbuzz settings
lib_libCsCore@BUILD_ABI@_la_CXXFLAGS += \
   -DHAVE_ATEXIT \
   -DHB_EXTERN=  \
   -DHB_NO_UNICODE_FUNCS \
   -DHB_NDEBUG

if OSTYPE_WIN
lib_libCsCore@BUILD_ABI@_la_CXXFLAGS += \
   -DHB_NO_WIN1256
endif

CORE_DIRS += src/core/tools

CORE_PUBLIC_INCLUDES += \
   QArrayData \
   QBitArray \
   QBitRef \
   QByteArray \
   QByteRef \
   QCache \
   QContiguousCache \
   QContiguousCacheData \
   QContiguousCacheTypedData \
   QCommandLineOption \
   QCommandLineParser \
   QCryptographicHash \
   QDate \
   QDateTime \
   QEasingCurve \
   QElapsedTimer \
   QExplicitlySharedDataPointer \
   QFlatMap \
   QHash \
   QHashIterator \
   QLine \
   QLineF \
   QLinkedList \
   QLinkedListData \
   QLinkedListIterator \
   QLinkedListNode \
   QList \
   QListIterator \
   QLocale \
   QMap \
   QMapIterator \
   QMargins \
   QMessageAuthenticationCode \
   QMultiHash \
   QMultiMap \
   QMutableHashIterator \
   QMutableLinkedListIterator \
   QMutableListIterator \
   QMutableMapIterator \
   QMutableSetIterator \
   QMutableStringListIterator \
   QMutableVectorIterator \
   QPair \
   QPoint \
   QPointF \
   QQueue \
   QRect \
   QRectF \
   QScopedArrayPointer \
   QScopedPointer \
   QScopedPointerArrayDeleter \
   QScopedPointerDeleter \
   QScopedPointerPodDeleter \
   QScopedValueRollback \
   QSet \
   QSetIterator \
   QSharedData \
   QSharedDataPointer \
   QSharedPointer \
   QSize \
   QSizeF \
   QStack \
   QStringList \
   QStringListIterator \
   QSystemLocale \
   QTextBoundaryFinder \
   QTime \
   QTimeLine \
   QVarLengthArray \
   QVector \
   QVectorData \
   QVectorIterator \
   QVectorTypedData \
   QWeakPointer \
   QtAlgorithms \
   QtContainerFwd

CORE_TOOLS_INCLUDES = \
   qalgorithms.h \
   qarraydata.h \
   qarraydataops.h \
   qbitarray.h \
   qbitref.h \
   qbytearray.h \
   qbyteref.h \
   qcache.h \
   qcontainerfwd.h \
   qcontiguouscache.h \
   qcontiguouscachedata.h \
   qcontiguouscachetypeddata.h \
   qcommandlineoption.h \
   qcommandlineparser.h \
   qcryptographichash.h \
   qdate.h \
   qdatetime.h \
   qeasingcurve.h \
   qelapsedtimer.h \
   qexplicitlyshareddatapointer.h \
   qflatmap.h \
   qhash.h \
   qhashfunc.h \
   qhashfwd.h \
   qhashiterator.h \
   qline.h \
   qlinef.h \
   qlinkedlist.h \
   qlinkedlistdata.h \
   qlinkedlistiterator.h \
   qlinkedlistnode.h \
   qlist.h \
   qlistiterator.h \
   qlocale.h \
   qmap.h \
   qmapfunc.h \
   qmapiterator.h \
   qmargins.h \
   qmessageauthenticationcode.h \
   qmultihash.h \
   qmultimap.h \
   qmutablehashiterator.h \
   qmutablelinkedlistiterator.h \
   qmutablelistiterator.h \
   qmutablemapiterator.h \
   qmutablesetiterator.h \
   qmutablestringlistiterator.h \
   qmutablevectoriterator.h \
   qpair.h \
   qpoint.h \
   qpointf.h \
   qqueue.h \
   qrect.h \
   qrectf.h \
   qrefcount.h \
   qscopedarraypointer.h \
   qscopedpointer.h \
   qscopedpointerarraydeleter.h \
   qscopedpointerdeleter.h \
   qscopedpointerpoddeleter.h \
   qscopedvaluerollback.h \
   qset.h \
   qsetiterator.h \
   qshareddata.h \
   qshareddatapointer.h \
   qsharedpointer.h \
   qsharedpointer_impl.h \
   qsize.h \
   qsizef.h \
   qstack.h \
   qstringlist.h \
   qstringlistiterator.h \
   qsystemlocale.h \
   qtalgorithms.h \
   qtcontainerfwd.h \
   qtextboundaryfinder.h \
   qtime.h \
   qtimeline.h \
   qvarlengtharray.h \
   qvector.h \
   qvectordata.h \
   qvectoriterator.h \
   qvectortypeddata.h \
   qweakpointer.h

CORE_TOOLS_PRIVATE_INCLUDES = \
   qbytedata_p.h \
   qdatetime_p.h \
   qfreelist_p.h \
   qlocale_data_p.h \
   qlocale_p.h \
   qlocale_tools_p.h \
   qpodlist_p.h \
   qringbuffer_p.h \
   qscopedpointer_p.h \
   qsimd_p.h \
   qtools_p.h \
   qunicodetables_p.h \
   qunicodetools_p.h


CORE_PUBLIC_INCLUDES += $(CORE_TOOLS_PUBLIC_INCLUDES)
CORE_INCLUDES += $(CORE_TOOLS_INCLUDES)
CORE_PRIVATE_INCLUDES += $(CORE_TOOLS_PRIVATE_INCLUDES)

lib_libCsCore@BUILD_ABI@_la_SOURCES += \
   src/core/tools/qarraydata.cpp \
   src/core/tools/qbitarray.cpp \
   src/core/tools/qbytearray.cpp \
   src/core/tools/qcryptographichash.cpp \
   src/core/tools/qdatetime.cpp \
   src/core/tools/qeasingcurve.cpp \
   src/core/tools/qelapsedtimer.cpp \
   src/core/tools/qfreelist.cpp \
   src/core/tools/qflatmap.cpp \
   src/core/tools/qhash.cpp \
   src/core/tools/qline.cpp \
   src/core/tools/qlinkedlist.cpp \
   src/core/tools/qlist.cpp \
   src/core/tools/qlocale.cpp \
   src/core/tools/qlocale_tools.cpp \
   src/core/tools/qpoint.cpp \
   src/core/tools/qmap.cpp \
   src/core/tools/qmargins.cpp \
   src/core/tools/qmessageauthenticationcode.cpp \
   src/core/tools/qcontiguouscache.cpp \
   src/core/tools/qcommandlineoption.cpp \
   src/core/tools/qcommandlineparser.cpp \
   src/core/tools/qrect.cpp \
   src/core/tools/qrefcount.cpp \
   src/core/tools/qshareddata.cpp \
   src/core/tools/qsharedpointer.cpp \
   src/core/tools/qsimd.cpp \
   src/core/tools/qsize.cpp \
   src/core/tools/qstringlist.cpp \
   src/core/tools/qtextboundaryfinder.cpp \
   src/core/tools/qtimeline.cpp \
   src/core/tools/qunicodetools.cpp \
   src/core/tools/qvector.cpp \
   \
   src/3rdparty/harfbuzz/fonts/resource_harfbuzz.cpp \
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
   src/3rdparty/harfbuzz/src/hb-unicode.cc \
   \
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
   src/3rdparty/harfbuzz/src/hb-ot-shape-complex-use-table.cc


if OSTYPE_WIN
lib_libCsCore@BUILD_ABI@_la_SOURCES += \
   src/core/tools/qelapsedtimer_win.cpp \
   src/core/tools/qlocale_win.cpp
endif

if OSTYPE_LINUX
lib_libCsCore@BUILD_ABI@_la_SOURCES += \
   src/core/tools/qelapsedtimer_unix.cpp \
   src/core/tools/qlocale_unix.cpp
endif

if OSTYPE_BSD
lib_libCsCore@BUILD_ABI@_la_SOURCES += \
   src/core/tools/qelapsedtimer_unix.cpp \
   src/core/tools/qlocale_unix.cpp
endif

if OSTYPE_DARWIN
lib_libCsCore@BUILD_ABI@_la_SOURCES += \
   src/core/tools/qelapsedtimer_mac.cpp \
   src/core/tools/qlocale_mac.mm \
   src/core/tools/qbytearray_mac.mm
endif

noinst_HEADERS+=$(patsubst %,src/core/tools/%,$(CORE_TOOLS_INCLUDES)) \
   $(patsubst %,src/core/tools/%,$(CORE_TOOLS_PRIVATE_INCLUDES)) \
   src/3rdparty/md5/md5.h \
   src/3rdparty/md4/md4.h \
   src/3rdparty/rfc6234/sha.h \
   src/3rdparty/rfc6234/sha-private.h \
   src/3rdparty/sha3/brg_endian.h \
   src/3rdparty/sha3/KeccakSponge.h \
   src/3rdparty/sha3/KeccakNISTInterface.h \
   src/3rdparty/sha3/KeccakF-1600-int-set.h \
   src/3rdparty/sha3/KeccakF-1600-interface.h \
   src/3rdparty/sha3/KeccakF-1600-opt32-settings.h \
   src/3rdparty/sha3/KeccakF-1600-opt64-settings.h \
   \
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



EXTRA_DIST+= \
   src/3rdparty/easing/easing.cpp \
   src/3rdparty/md5/md5.cpp \
   src/3rdparty/md4/md4.cpp \
   src/3rdparty/sha1/sha1.cpp \
   src/3rdparty/rfc6234/sha224-256.c \
   src/3rdparty/rfc6234/sha384-512.c \
   src/3rdparty/sha3/KeccakSponge.c \
   src/3rdparty/sha3/KeccakNISTInterface.c \
   src/3rdparty/sha3/KeccakF-1600-opt32.c \
   src/3rdparty/sha3/KeccakF-1600-opt64.c \
   src/3rdparty/sha3/KeccakF-1600-32-rvk.macros \
   src/3rdparty/sha3/KeccakF-1600-32.macros \
   src/3rdparty/sha3/KeccakF-1600-64.macros \
   src/3rdparty/sha3/KeccakF-1600-unrolling.macros\
   src/3rdparty/harfbuzz/fonts/harfbuzz.qrc \
   src/3rdparty/harfbuzz/fonts/DejaVuSans.ttf \
   src/core/tools/qunicodetables.cpp