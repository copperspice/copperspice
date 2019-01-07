CORE_DIRS += src/core/containers

CORE_PUBLIC_INCLUDES += \
   QCache \
   QContiguousCache \
   QFlatMap \
   QHash \
   QHashIterator \
   QLinkedList \
   QLinkedListIterator \
   QList \
   QListIterator \
   QMap \
   QMapIterator \
   QMultiHash \
   QMultiMap \
   QMutableHashIterator \
   QMutableLinkedListIterator \
   QMutableListIterator \
   QMutableMapIterator \
   QMutableSetIterator \
   QMutableStringListIterator \
   QMutableVectorIterator \
   QQueue \
   QSet \
   QSetIterator \
   QStringList \
   QStringListIterator \
   QStack \
   QVector \
   QVectorIterator

CORE_CONTAINERS_INCLUDES = \
   qcache.h \
   qcontainerfwd.h \
   qcontiguouscache.h \
   qflatmap.h \
   qhash.h \
   qhashfunc.h \
   qhashfwd.h \
   qhashiterator.h \
   qlinkedlist.h \
   qlinkedlistiterator.h \
   qlist.h \
   qlistiterator.h \
   qmap.h \
   qmapfunc.h \
   qmapiterator.h \
   qmultihash.h \
   qmultimap.h \
   qmutablehashiterator.h \
   qmutablelinkedlistiterator.h \
   qmutablelistiterator.h \
   qmutablemapiterator.h \
   qmutablesetiterator.h \
   qmutablestringlistiterator.h \
   qmutablevectoriterator.h \
   qqueue.h \
   qset.h \
   qsetiterator.h \
   qstringlist.h \
   qstringlistiterator.h \
   qstack.h \
   qvector.h \
   qvectoriterator.h


CORE_PUBLIC_INCLUDES += $(CORE_CONTAINERS_PUBLIC_INCLUDES)
CORE_INCLUDES += $(CORE_CONTAINERS_INCLUDES)
CORE_PRIVATE_INCLUDES += $(CORE_CONTAINERS_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/core/containers/%,$(CORE_CONTAINERS_INCLUDES)) \
		$(patsubst %,src/core/containers/%,$(CORE_CONTAINERS_PRIVATE_INCLUDES))

lib_libCsCore@BUILD_ABI@_la_SOURCES += \
   src/core/containers/qcontiguouscache.cpp \
   src/core/containers/qflatmap.cpp \
   src/core/containers/qhash.cpp \
   src/core/containers/qlinkedlist.cpp \
   src/core/containers/qlist.cpp \
   src/core/containers/qmap.cpp \
   src/core/containers/qqueue.cpp  \
   src/core/containers/qstringlist.cpp \
   src/core/containers/qvector.cpp


