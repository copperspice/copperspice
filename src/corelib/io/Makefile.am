CORE_DIRS += src/corelib/io

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
	QtDebug

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
	qtdebug.h \
	qtemporarydir.h \
	qtemporaryfile.h \
	qtextistream.h \
	qtextostream.h \
	qtextstream.h \
	qtextstreamfunction.h \
	qtextstreammanipulator.h \
	qts.h \
	qurl.h

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
	qwindowspipewriter_p.h

CORE_INCLUDES += $(CORE_IO_INCLUDES)
CORE_PRIVATE_INCLUDES += $(CORE_IO_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/corelib/io/%,$(CORE_IO_INCLUDES)) \
		$(patsubst %,src/corelib/io/%,$(CORE_IO_PRIVATE_INCLUDES))

lib_libCsCore1_la_SOURCES += \
	src/corelib/io/qabstractfileengine.cpp \
	src/corelib/io/qbuffer.cpp \
	src/corelib/io/qdatastream.cpp \
	src/corelib/io/qdataurl.cpp \
	src/corelib/io/qtldurl.cpp \
	src/corelib/io/qdebug.cpp \
	src/corelib/io/qdir.cpp \
	src/corelib/io/qdiriterator.cpp \
	src/corelib/io/qfile.cpp \
	src/corelib/io/qfiledevice.cpp \
	src/corelib/io/qfileinfo.cpp \
	src/corelib/io/qiodevice.cpp \
	src/corelib/io/qipaddress.cpp \
	src/corelib/io/qnoncontiguousbytedevice.cpp \
	src/corelib/io/qprocess.cpp \
	src/corelib/io/qtextstream.cpp \
	src/corelib/io/qtemporarydir.cpp \
	src/corelib/io/qtemporaryfile.cpp \
	src/corelib/io/qresource.cpp \
	src/corelib/io/qresource_iterator.cpp \
	src/corelib/io/qurl.cpp \
	src/corelib/io/qsavefile.cpp \
	src/corelib/io/qsettings.cpp \
	src/corelib/io/qstandardpaths.cpp \
	src/corelib/io/qfsfileengine.cpp \
	src/corelib/io/qfsfileengine_iterator.cpp \
	src/corelib/io/qfilesystemwatcher.cpp \
	src/corelib/io/qfilesystementry.cpp \
	src/corelib/io/qfilesystemengine.cpp

## FIXME platform-specific
# qfsfileengine_unix.cpp
# qprocess_unix.cpp
# qfilesystemwatcher_fsevents_p.h
# qsettings_mac.cpp
# qfilesystemwatcher_fsevents.cpp
# qfilesystemwatcher_inotify_p.h
# qfilesystemwatcher_dnotify_p.h
# qfilesystemwatcher_kqueue.cpp
# qfilesystemwatcher_kqueue_p.h

if OSTYPE_WIN
lib_libCsCore1_la_SOURCES += \
	src/corelib/io/qwindowspipewriter.cpp \
	src/corelib/io/qfilesystemiterator_win.cpp \
	src/corelib/io/qsettings_win.cpp \
	src/corelib/io/qprocess_win.cpp \
	src/corelib/io/qfsfileengine_win.cpp \
	src/corelib/io/qfilesystemengine_win.cpp \
	src/corelib/io/qfilesystemwatcher_win.cpp \
	src/corelib/io/qstandardpaths_win.cpp
endif

if OSTYPE_LINUX
lib_libCsCore1_la_SOURCES += \
	src/corelib/io/qfsfileengine_unix.cpp \
	src/corelib/io/qprocess_unix.cpp \
	src/corelib/io/qfilesystemengine_unix.cpp \
	src/corelib/io/qfilesystemwatcher_inotify.cpp \
	src/corelib/io/qfilesystemwatcher_dnotify.cpp \
	src/corelib/io/qfilesystemiterator_unix.cpp \
	src/corelib/io/qstandardpaths_unix.cpp
endif

if OSTYPE_DARWIN
lib_libCsCore1_la_SOURCES += \
	src/corelib/io/qfilesystemengine_unix.cpp \
	src/corelib/io/qfilesystemiterator_unix.cpp \
	src/corelib/io/qfilesystemwatcher_fsevents.cpp \
	src/corelib/io/qfilesystemwatcher_kqueue.cpp \
	src/corelib/io/qfsfileengine_unix.cpp \
	src/corelib/io/qprocess_unix.cpp \
	src/corelib/io/qsettings_mac.cpp \
	src/corelib/io/qstandardpaths_mac.cpp
endif
