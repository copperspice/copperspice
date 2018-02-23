CORE_DIRS += src/core/io

CORE_PUBLIC_INCLUDES += \
	Q_PID \
	QAbstractFileEngine \
	QAbstractFileEngineHandler \
	QAbstractFileEngineIterator \
	QBuffer \
	QDataStream \
	QDebug \
	QDir \
	QDirIterator \
	QFSFileEngine \
	QFile \
	QFileDevice \
	QFileInfo \
	QFileInfoList \
	QFileInfoListIterator \
	QFileSystemWatcher \
	QIODevice \
	QNoDebug \
	QProcess \
	QProcessEnvironment \
	QResource \
	QSaveFile \
	QSettings \
	QStandardPaths \
	QTS \
	QTemporaryDir \
	QTemporaryFile \
	QTextIStream \
	QTextOStream \
	QTextStream \
	QTextStreamFunction \
	QTextStreamManipulator \
	QUrl \
	QUrlQuery

CORE_IO_INCLUDES = \
	q_pid.h \
	qabstractfileengine.h \
	qabstractfileenginehandler.h \
	qabstractfileengineiterator.h \
	qbuffer.h \
	qdatastream.h \
	qdebug.h \
	qdir.h \
	qdiriterator.h \
	qfile.h \
	qfiledevice.h \
	qfileinfo.h \
	qfileinfolist.h \
	qfileinfolistiterator.h \
	qfilesystemwatcher.h \
	qfsfileengine.h \
	qiodevice.h \
	qnodebug.h \
	qprocess.h \
	qprocessenvironment.h \
	qresource.h \
	qsavefile.h \
	qsettings.h \
	qstandardpaths.h \
	qtemporarydir.h \
	qtemporaryfile.h \
	qtextistream.h \
	qtextostream.h \
	qtextstream.h \
	qtextstreamfunction.h \
	qtextstreammanipulator.h \
	qts.h \
	qurl.h \
	qurlquery.h

CORE_IO_PRIVATE_INCLUDES = \
	qabstractfileengine_p.h \
	qdatastream_p.h \
	qdataurl_p.h \
	qdir_p.h \
	qfileinfo_p.h \
	qfile_p.h \
	qfiledevice_p.h \
	qiodevice_p.h \
	qipaddress_p.h \
	qnoncontiguousbytedevice_p.h \
	qprocess_p.h \
	qresource_p.h \
	qresource_iterator_p.h \
	qurl_p.h \
	qurltlds_p.h \
	qtldurl_p.h \
	qsavefile_p.h \
	qsettings_p.h \
	qfsfileengine_p.h \
	qfsfileengine_iterator_p.h \
	qfilesystemwatcher_p.h \
	qfilesystemwatcher_dnotify_p.h \
	qfilesystemwatcher_fsevents_p.h \
	qfilesystemwatcher_inotify_p.h \
	qfilesystemwatcher_kqueue_p.h \
	qfilesystemwatcher_win_p.h \
	qfilesystementry_p.h \
	qfilesystemengine_p.h \
	qfilesystemmetadata_p.h \
	qfilesystemiterator_p.h \
	qtemporaryfile_p.h \
	qwindowspipereader_p.h \
	qwindowspipewriter_p.h

CORE_INCLUDES += $(CORE_IO_INCLUDES)
CORE_PRIVATE_INCLUDES += $(CORE_IO_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/core/io/%,$(CORE_IO_INCLUDES)) \
		$(patsubst %,src/core/io/%,$(CORE_IO_PRIVATE_INCLUDES))

lib_libCsCore@BUILD_ABI@_la_SOURCES += \
	src/core/io/qabstractfileengine.cpp \
	src/core/io/qbuffer.cpp \
	src/core/io/qdatastream.cpp \
	src/core/io/qdataurl.cpp \
	src/core/io/qtldurl.cpp \
	src/core/io/qdebug.cpp \
	src/core/io/qdir.cpp \
	src/core/io/qdiriterator.cpp \
	src/core/io/qfile.cpp \
	src/core/io/qfiledevice.cpp \
	src/core/io/qfileinfo.cpp \
	src/core/io/qiodevice.cpp \
	src/core/io/qipaddress.cpp \
	src/core/io/qnoncontiguousbytedevice.cpp \
	src/core/io/qprocess.cpp \
	src/core/io/qtextstream.cpp \
	src/core/io/qtemporarydir.cpp \
	src/core/io/qtemporaryfile.cpp \
	src/core/io/qresource.cpp \
	src/core/io/qresource_iterator.cpp \
	src/core/io/qurl.cpp \
	src/core/io/qurlquery.cpp \
	src/core/io/qurlidna.cpp \
	src/core/io/qurlrecode.cpp \
	src/core/io/qsavefile.cpp \
	src/core/io/qsettings.cpp \
	src/core/io/qstandardpaths.cpp \
	src/core/io/qfsfileengine.cpp \
	src/core/io/qfsfileengine_iterator.cpp \
	src/core/io/qfilesystemwatcher.cpp \
	src/core/io/qfilesystementry.cpp \
	src/core/io/qfilesystemengine.cpp

if OSTYPE_WIN
lib_libCsCore@BUILD_ABI@_la_SOURCES += \
	src/core/io/qwindowspipereader.cpp \
	src/core/io/qwindowspipewriter.cpp \
	src/core/io/qfilesystemiterator_win.cpp \
	src/core/io/qsettings_win.cpp \
	src/core/io/qprocess_win.cpp \
	src/core/io/qfsfileengine_win.cpp \
	src/core/io/qfilesystemengine_win.cpp \
	src/core/io/qfilesystemwatcher_win.cpp \
	src/core/io/qstandardpaths_win.cpp
endif

if OSTYPE_LINUX
lib_libCsCore@BUILD_ABI@_la_SOURCES += \
	src/core/io/qfsfileengine_unix.cpp \
	src/core/io/qprocess_unix.cpp \
	src/core/io/qfilesystemengine_unix.cpp \
	src/core/io/qfilesystemwatcher_inotify.cpp \
	src/core/io/qfilesystemwatcher_dnotify.cpp \
	src/core/io/qfilesystemiterator_unix.cpp \
	src/core/io/qstandardpaths_unix.cpp \
	src/core/io/forkfd_qt.cpp
endif

if OSTYPE_BSD
lib_libCsCore@BUILD_ABI@_la_SOURCES += \
	src/core/io/qfsfileengine_unix.cpp \
	src/core/io/qprocess_unix.cpp \
	src/core/io/qfilesystemengine_unix.cpp \
	src/core/io/qfilesystemwatcher_kqueue.cpp \
	src/core/io/qfilesystemiterator_unix.cpp \
	src/core/io/qstandardpaths_unix.cpp \
	src/core/io/forkfd_qt.cpp
endif

if OSTYPE_DARWIN
lib_libCsCore@BUILD_ABI@_la_SOURCES += \
	src/core/io/qfilesystemengine_unix.cpp \
	src/core/io/qfilesystemiterator_unix.cpp \
	src/core/io/qfilesystemwatcher_fsevents.cpp \
	src/core/io/qfilesystemwatcher_kqueue.cpp \
	src/core/io/qfsfileengine_unix.cpp \
	src/core/io/qprocess_unix.cpp \
	src/core/io/qsettings_mac.cpp \
	src/core/io/qstandardpaths_mac.cpp \
	src/core/io/forkfd_qt.cpp \
	src/core/io/qurl_mac.mm
endif
