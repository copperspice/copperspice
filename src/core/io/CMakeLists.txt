set(CORE_PUBLIC_INCLUDES
    ${CORE_PUBLIC_INCLUDES}
    Q_PID
    QAbstractFileEngine
    QAbstractFileEngineHandler
    QAbstractFileEngineIterator
    QBuffer
    QDataStream
    QDebug
    QDir
    QDirIterator
    QFSFileEngine
    QFile
    QFileDevice
    QFileInfo
    QFileInfoList
    QFileInfoListIterator
    QFileSystemWatcher
    QIODevice
    QNoDebug
    QProcess
    QProcessEnvironment
    QResource
    QSaveFile
    QSettings
    QStandardPaths
    QTS
    QTemporaryDir
    QTemporaryFile
    QTextIStream
    QTextOStream
    QTextStream
    QTextStreamFunction
    QTextStreamManipulator
    QUrl
    QtDebug
    PARENT_SCOPE
)

set(CORE_INCLUDES
    ${CORE_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/q_pid.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qabstractfileengine.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qabstractfileenginehandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qabstractfileengineiterator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qbuffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qdatastream.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qdebug.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qdir.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qdiriterator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qfile.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qfiledevice.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qfileinfo.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qfileinfolist.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qfileinfolistiterator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qfilesystemwatcher.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qfsfileengine.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qiodevice.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qnodebug.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qprocess.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qprocessenvironment.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qresource.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qsavefile.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qsettings.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qstandardpaths.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qtdebug.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qtemporarydir.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qtemporaryfile.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qtextistream.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qtextostream.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qtextstream.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qtextstreamfunction.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qtextstreammanipulator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qts.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qurl.h
    PARENT_SCOPE
)

set(CORE_PRIVATE_INCLUDES
    ${CORE_PRIVATE_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/qabstractfileengine_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qdatastream_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qdataurl_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qdir_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qfileinfo_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qfile_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qfiledevice_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qiodevice_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qipaddress_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qnoncontiguousbytedevice_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qprocess_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qresource_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qresource_iterator_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qurltlds_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qtldurl_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qsavefile_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qsettings_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qfsfileengine_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qfsfileengine_iterator_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qfilesystemwatcher_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qfilesystemwatcher_dnotify_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qfilesystemwatcher_fsevents_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qfilesystemwatcher_inotify_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qfilesystemwatcher_kqueue_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qfilesystemwatcher_win_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qfilesystementry_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qfilesystemengine_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qfilesystemmetadata_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qfilesystemiterator_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qtemporaryfile_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qwindowspipewriter_p.h
    PARENT_SCOPE
)

set(CORE_SOURCES
    ${CORE_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/qabstractfileengine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qbuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qdatastream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qdataurl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtldurl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qdebug.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qdir.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qdiriterator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qfile.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qfiledevice.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qfileinfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qiodevice.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qipaddress.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qnoncontiguousbytedevice.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qprocess.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtextstream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtemporarydir.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtemporaryfile.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qresource.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qresource_iterator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qurl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qsavefile.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qstandardpaths.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qfsfileengine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qfsfileengine_iterator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qfilesystemwatcher.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qfilesystementry.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qfilesystemengine.cpp
    PARENT_SCOPE
)

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

if(${CMAKE_SYSTEM_NAME} MATCHES "Win32")
    set(CORE_SOURCES
        ${CORE_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/qwindowspipewriter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qfilesystemiterator_win.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qsettings_win.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qprocess_win.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qfsfileengine_win.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qfilesystemengine_win.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qfilesystemwatcher_win.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qstandardpaths_win.cpp
        PARENT_SCOPE
    )
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(CORE_SOURCES
        ${CORE_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/qfsfileengine_unix.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qprocess_unix.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qfilesystemengine_unix.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qfilesystemwatcher_inotify.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qfilesystemwatcher_dnotify.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qfilesystemiterator_unix.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qstandardpaths_unix.cpp
        PARENT_SCOPE
    )
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CORE_SOURCES
        ${CORE_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/qfilesystemengine_unix.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qfilesystemiterator_unix.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qfilesystemwatcher_fsevents.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qfilesystemwatcher_kqueue.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qfsfileengine_unix.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qprocess_unix.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qsettings_mac.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qstandardpaths_mac.cpp
        PARENT_SCOPE
    )
endif()
