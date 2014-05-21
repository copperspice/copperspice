CORE_DIRS += src/corelib/kernel

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
	QEvent \
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
	qevent.h \
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
	qwineventnotifier_p.h \
	qfunctions_p.h \
	qcrashhandler_p.h \
	qeventdispatcher_glib_p.h \
	qsystemerror_p.h

CORE_INCLUDES += $(CORE_KERNEL_INCLUDES)
CORE_PRIVATE_INCLUDES += $(CORE_KERNEL_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/corelib/kernel/%,$(CORE_KERNEL_INCLUDES)) \
		$(patsubst %,src/corelib/kernel/%,$(CORE_KERNEL_PRIVATE_INCLUDES))

lib_libCsCore1_la_SOURCES += \
	src/corelib/kernel/qabstracteventdispatcher.cpp \
	src/corelib/kernel/qabstractitemmodel.cpp \
	src/corelib/kernel/qbasictimer.cpp \
	src/corelib/kernel/qcoreapplication.cpp \
	src/corelib/kernel/qcoreevent.cpp \
	src/corelib/kernel/qcoreglobaldata.cpp \
	src/corelib/kernel/qeventloop.cpp \
	src/corelib/kernel/qmath.cpp \
	src/corelib/kernel/qmetaobject.cpp \
	src/corelib/kernel/qmetatype.cpp \
	src/corelib/kernel/qmimedata.cpp \
	src/corelib/kernel/qobject.cpp \
	src/corelib/kernel/qobjectcleanuphandler.cpp \
	src/corelib/kernel/qpointer.cpp \
	src/corelib/kernel/qsharedmemory.cpp \
	src/corelib/kernel/qsignalmapper.cpp \
	src/corelib/kernel/qsocketnotifier.cpp \
	src/corelib/kernel/qsystemerror.cpp \
	src/corelib/kernel/qsystemsemaphore.cpp \
	src/corelib/kernel/qtimer.cpp \
	src/corelib/kernel/qtranslator.cpp \
	src/corelib/kernel/qvariant.cpp  \
	src/corelib/kernel/csmeta_callevent.cpp \
	src/corelib/kernel/csmeta_classinfo.cpp \
	src/corelib/kernel/csmeta_enum.cpp \
	src/corelib/kernel/csmeta_method.cpp \
	src/corelib/kernel/csmeta_prop.cpp \
	src/corelib/kernel/csobject_debug.cpp \
	src/corelib/kernel/csobject_private.cpp


if OSTYPE_WIN
lib_libCsCore1_la_SOURCES += \
	src/corelib/kernel/qeventdispatcher_win.cpp \
	src/corelib/kernel/qcoreapplication_win.cpp \
	src/corelib/kernel/qwineventnotifier_p.cpp \
	src/corelib/kernel/qsharedmemory_win.cpp \
	src/corelib/kernel/qsystemsemaphore_win.cpp
endif

if OSTYPE_LINUX
lib_libCsCore1_la_SOURCES += \
	src/corelib/kernel/qcore_unix.cpp \
	src/corelib/kernel/qcrashhandler.cpp \
	src/corelib/kernel/qsharedmemory_unix.cpp \
	src/corelib/kernel/qsystemsemaphore_unix.cpp \
	src/corelib/kernel/qeventdispatcher_unix.cpp \
	src/corelib/kernel/qeventdispatcher_glib.cpp
endif

if OSTYPE_DARWIN
lib_libCsCore1_la_SOURCES += \
	src/corelib/kernel/qcore_mac.cpp \
	src/corelib/kernel/qcore_unix.cpp \
	src/corelib/kernel/qcoreapplication_mac.cpp \
	src/corelib/kernel/qeventdispatcher_unix.cpp \
	src/corelib/kernel/qsharedmemory_unix.cpp \
	src/corelib/kernel/qsystemsemaphore_unix.cpp
endif