CORE_DIRS += src/core/kernel

CORE_PUBLIC_INCLUDES += \
	QAbstractEventDispatcher \
	QAbstractItemModel \
	QAbstractListModel \
	QAbstractTableModel \
	QBasicTimer \
	QArgument \
	QChildEvent \
	QCoreApplication \
	QCustomEvent \
	QDynamicPropertyChangeEvent \
	QEventLoop \
	QGenericArgument \
	QGenericReturnArgument \
	QMetaClassInfo \
	QMetaEnum \
	QMetaMethod \
	QMetaObject \
	QMetaObjectAccessor \
	QMetaObjectExtraData \
	QMetaProperty \
	QMetaType \
	QMetaTypeId \
	QMetaTypeId2 \
	QMimeData \
	QModelIndex \
	QModelIndexList \
	QObject \
	QObjectCleanupHandler \
	QPersistentModelIndex \
	QPointer \
	QObjectData \
	QObjectUserData \
	QObjectList \
	QReturnArgument \
	QSharedMemory \
	QSignalMapper \
	QSocketNotifier \
	QSystemSemaphore \
	QTimer \
	QTimerEvent \
	QTranslator \
	QVariant \
	QVariantComparisonHelper \
	QVariantList \
	QVariantHash \
	QVariantMap \
	QWinEventNotifier \
	QtCleanUpFunction \
	QtMsgHandler

CORE_KERNEL_INCLUDES = \
	qabstracteventdispatcher.h \
	qabstractitemmodel.h \
	qabstractlistmodel.h \
	qabstracttablemodel.h \
	qargument.h \
	qbasictimer.h \
	qchildevent.h \
	qcoreapplication.h \
	qcoreevent.h \
	qcustomevent.h \
	qdynamicpropertychangeevent.h \
	qeventloop.h \
	qgenericargument.h \
	qgenericreturnargument.h \
	qmath.h \
	qmetaclassinfo.h \
	qmetaenum.h \
	qmetamethod.h \
	qmetaobject.h \
	qmetaobjectaccessor.h \
	qmetaobjectextradata.h \
	qmetaproperty.h \
	qmetatype.h \
	qmetatypeid.h \
	qmetatypeid2.h \
	qmimedata.h \
	qmodelindex.h \
	qmodelindexlist.h \
	qtmsghandler.h \
	qobject.h \
	qobjectdata.h \
	qobjectuserdata.h \
	qobjectlist.h \
	qobjectcleanuphandler.h \
	qpersistentmodelindex.h \
	qpointer.h \
	qreturnargument.h \
	qsharedmemory.h \
	qsignalmapper.h \
	qsocketnotifier.h \
	qsystemsemaphore.h \
	qtcleanupfunction.h \
	qtimer.h \
	qtimerevent.h \
	qtranslator.h \
	qvariant.h \
	qvariantcomparisonhelper.h \
	qvariantlist.h \
	qvarianthash.h \
	qvariantmap.h \
	qwineventnotifier.h \
	qmetatype.h \
	csmeta.h \
	csmeta_callevent.h \
	csmeta_internal_1.h \
	csmeta_internal_2.h \
	csobject_internal.h \
	csobject_macro.h \
	csregister1.h \
	csregister2.h

CORE_KERNEL_PRIVATE_INCLUDES = \
	qabstractitemmodel_p.h \
	qcorecmdlineargs_p.h \
	qcore_unix_p.h \
	qcore_mac_p.h \
	qtranslator_p.h \
	qabstracteventdispatcher_p.h \
	qcoreapplication_p.h \
	qvariant_p.h \
	qcoreglobaldata_p.h \
	qsharedmemory_p.h \
	qsystemsemaphore_p.h \
	qeventdispatcher_win_p.h \
	qeventdispatcher_unix_p.h \
	qfunctions_p.h \
	qcrashhandler_p.h \
	qeventdispatcher_glib_p.h \
	qsystemerror_p.h

CORE_INCLUDES += $(CORE_KERNEL_INCLUDES)
CORE_PRIVATE_INCLUDES += $(CORE_KERNEL_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/core/kernel/%,$(CORE_KERNEL_INCLUDES)) \
		$(patsubst %,src/core/kernel/%,$(CORE_KERNEL_PRIVATE_INCLUDES))

lib_libCsCore@BUILD_ABI@_la_SOURCES += \
	src/core/kernel/qabstracteventdispatcher.cpp \
	src/core/kernel/qabstractitemmodel.cpp \
	src/core/kernel/qbasictimer.cpp \
	src/core/kernel/qcoreapplication.cpp \
	src/core/kernel/qcoreevent.cpp \
	src/core/kernel/qcoreglobaldata.cpp \
	src/core/kernel/qeventloop.cpp \
	src/core/kernel/qmath.cpp \
	src/core/kernel/qmetaobject.cpp \
	src/core/kernel/qmetatype.cpp \
	src/core/kernel/qmimedata.cpp \
	src/core/kernel/qobject.cpp \
	src/core/kernel/qobjectcleanuphandler.cpp \
	src/core/kernel/qpointer.cpp \
	src/core/kernel/qsharedmemory.cpp \
	src/core/kernel/qsignalmapper.cpp \
	src/core/kernel/qsocketnotifier.cpp \
	src/core/kernel/qsystemerror.cpp \
	src/core/kernel/qsystemsemaphore.cpp \
	src/core/kernel/qtimer.cpp \
	src/core/kernel/qtranslator.cpp \
	src/core/kernel/qvariant.cpp  \
	src/core/kernel/csmeta_callevent.cpp \
	src/core/kernel/csmeta_classinfo.cpp \
	src/core/kernel/csmeta_enum.cpp \
	src/core/kernel/csmeta_method.cpp \
	src/core/kernel/csmeta_prop.cpp \
	src/core/kernel/csobject_debug.cpp \
	src/core/kernel/csobject_private.cpp


if OSTYPE_WIN
lib_libCsCore@BUILD_ABI@_la_SOURCES += \
	src/core/kernel/qeventdispatcher_win.cpp \
	src/core/kernel/qcoreapplication_win.cpp \
	src/core/kernel/qwineventnotifier.cpp \
	src/core/kernel/qsharedmemory_win.cpp \
	src/core/kernel/qsystemsemaphore_win.cpp
endif

if OSTYPE_LINUX
lib_libCsCore@BUILD_ABI@_la_SOURCES += \
	src/core/kernel/qcore_unix.cpp \
	src/core/kernel/qcrashhandler.cpp \
	src/core/kernel/qsharedmemory_unix.cpp \
	src/core/kernel/qsystemsemaphore_unix.cpp \
	src/core/kernel/qeventdispatcher_unix.cpp \
	src/core/kernel/qeventdispatcher_glib.cpp
endif

if OSTYPE_BSD
lib_libCsCore@BUILD_ABI@_la_SOURCES += \
        src/core/kernel/qcore_unix.cpp \
        src/core/kernel/qcrashhandler.cpp \
        src/core/kernel/qsharedmemory_unix.cpp \
        src/core/kernel/qsystemsemaphore_unix.cpp \
	src/core/kernel/qeventdispatcher_unix.cpp \
	src/core/kernel/qeventdispatcher_glib.cpp
endif

if OSTYPE_DARWIN
lib_libCsCore@BUILD_ABI@_la_SOURCES += \
	src/core/kernel/qcore_mac.cpp \
	src/core/kernel/qcore_unix.cpp \
	src/core/kernel/qcoreapplication_mac.cpp \
	src/core/kernel/qeventdispatcher_unix.cpp \
	src/core/kernel/qsharedmemory_unix.cpp \
	src/core/kernel/qsystemsemaphore_unix.cpp
endif
