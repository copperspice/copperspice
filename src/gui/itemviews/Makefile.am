GUI_DIRS += src/gui/itemviews

GUI_PUBLIC_INCLUDES += \
	QAbstractItemDelegate \
	QAbstractItemView \
	QAbstractProxyModel \
	QColumnView \
	QDataWidgetMapper \
	QDirModel \
	QFileIconProvider \
	QHeaderView \
	QIdentityProxyModel \
	QItemDelegate \
	QItemEditorCreator \
	QItemEditorCreatorBase \
	QItemEditorFactory \
	QItemSelection \
	QItemSelectionRange \
	QItemSelectionModel \
	QListView \
	QListWidget \
	QListWidgetItem \
	QProxyModel \
	QSortFilterProxyModel \
	QStandardItem \
	QStandardItemEditorCreator \
	QStandardItemModel \
	QStringListModel \
	QStyledItemDelegate \
	QTableView \
	QTableWidget \
	QTableWidgetItem \
	QTableWidgetSelectionRange \
	QTreeView \
	QTreeWidget \
	QTreeWidgetItem \
	QTreeWidgetItemIterator

GUI_ITEMVIEWS_INCLUDES = \
	qabstractitemdelegate.h \
	qabstractitemview.h \
	qabstractproxymodel.h \
	qcolumnview.h  \
	qdatawidgetmapper.h \
	qdirmodel.h \
	qfileiconprovider.h \
	qheaderview.h \
	qidentityproxymodel.h \
	qitemdelegate.h \
	qitemeditorcreator.h \
	qitemeditorcreatorbase.h \
	qitemeditorfactory.h \
	qitemselection.h \
	qitemselectionrange.h \
	qitemselectionmodel.h \
	qlistview.h \
	qlistwidget.h \
	qlistwidgetitem.h \
	qproxymodel.h \
	qsortfilterproxymodel.h \
	qstandarditem.h \
	qstandarditemeditorcreator.h \
	qstandarditemmodel.h \
	qstringlistmodel.h \
	qstyleditemdelegate.h \
	qtableview.h \
	qtablewidget.h \
	qtablewidgetitem.h \
	qtablewidgetselectionrange.h \
	qtreeview.h \
	qtreewidget.h \
	qtreewidgetitem.h \
	qtreewidgetitemiterator.h

GUI_ITEMVIEWS_PRIVATE_INCLUDES = \
	qabstractitemview_p.h \
	qabstractproxymodel_p.h \
	qbsptree_p.h \
	qcolumnview_p.h \
	qcolumnviewgrip_p.h \
	qfileiconprovider_p.h \
	qitemeditorfactory_p.h \
	qitemselectionmodel_p.h \
	qheaderview_p.h \
	qlistview_p.h \
	qlistwidget_p.h \
	qproxymodel_p.h \
	qstandarditemmodel_p.h \
	qtableview_p.h \
	qtablewidget_p.h \
	qtreeview_p.h \
	qtreewidgetitemiterator_p.h \
	qtreewidget_p.h \
	qwidgetitemdata_p.h

GUI_INCLUDES += $(GUI_ITEMVIEWS_INCLUDES)
GUI_PRIVATE_INCLUDES += $(GUI_ITEMVIEWS_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/gui/itemviews/%,$(GUI_ITEMVIEWS_INCLUDES)) \
		$(patsubst %,src/gui/itemviews/%,$(GUI_ITEMVIEWS_PRIVATE_INCLUDES))

lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/itemviews/qabstractitemdelegate.cpp \
	src/gui/itemviews/qabstractitemview.cpp \
	src/gui/itemviews/qabstractproxymodel.cpp \
	src/gui/itemviews/qbsptree.cpp \
	src/gui/itemviews/qcolumnview.cpp \
	src/gui/itemviews/qcolumnviewgrip.cpp \
	src/gui/itemviews/qdatawidgetmapper.cpp \
	src/gui/itemviews/qdirmodel.cpp \
	src/gui/itemviews/qfileiconprovider.cpp \
	src/gui/itemviews/qheaderview.cpp \
	src/gui/itemviews/qidentityproxymodel.cpp \
	src/gui/itemviews/qitemdelegate.cpp \
	src/gui/itemviews/qitemeditorfactory.cpp \
	src/gui/itemviews/qitemselectionmodel.cpp \
	src/gui/itemviews/qlistview.cpp \
	src/gui/itemviews/qlistwidget.cpp \
	src/gui/itemviews/qproxymodel.cpp \
	src/gui/itemviews/qsortfilterproxymodel.cpp \
	src/gui/itemviews/qstandarditemmodel.cpp \
	src/gui/itemviews/qstringlistmodel.cpp \
	src/gui/itemviews/qstyleditemdelegate.cpp \
	src/gui/itemviews/qtableview.cpp \
	src/gui/itemviews/qtablewidget.cpp \
	src/gui/itemviews/qtreeview.cpp \
	src/gui/itemviews/qtreewidget.cpp \
	src/gui/itemviews/qtreewidgetitemiterator.cpp
