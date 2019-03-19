/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
*
* This file is part of CopperSpice.
*
* CopperSpice is free software. You can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* https://www.gnu.org/licenses/
*
***********************************************************************/

#include <qdatawidgetmapper.h>

#ifndef QT_NO_DATAWIDGETMAPPER

#include <qabstractitemmodel.h>
#include <qitemdelegate.h>
#include <qmetaobject.h>
#include <qwidget.h>
#include <qabstractitemmodel_p.h>

QT_BEGIN_NAMESPACE

class QDataWidgetMapperPrivate
{

 public:
   Q_DECLARE_PUBLIC(QDataWidgetMapper)

   QDataWidgetMapperPrivate()
      : model(QAbstractItemModelPrivate::staticEmptyModel()), delegate(0),
        orientation(Qt::Horizontal), submitPolicy(QDataWidgetMapper::AutoSubmit) {
   }

   virtual ~QDataWidgetMapperPrivate() {}

   QAbstractItemModel *model;
   QAbstractItemDelegate *delegate;
   Qt::Orientation orientation;
   QDataWidgetMapper::SubmitPolicy submitPolicy;
   QPersistentModelIndex rootIndex;
   QPersistentModelIndex currentTopLeft;

   inline int itemCount() {
      return orientation == Qt::Horizontal
             ? model->rowCount(rootIndex)
             : model->columnCount(rootIndex);
   }

   inline int currentIdx() const {
      return orientation == Qt::Horizontal ? currentTopLeft.row() : currentTopLeft.column();
   }

   inline QModelIndex indexAt(int itemPos) {
      return orientation == Qt::Horizontal
             ? model->index(currentIdx(), itemPos, rootIndex)
             : model->index(itemPos, currentIdx(), rootIndex);
   }

   inline void flipEventFilters(QAbstractItemDelegate *oldDelegate, QAbstractItemDelegate *newDelegate) {
      for (int i = 0; i < widgetMap.count(); ++i) {
         QWidget *w = widgetMap.at(i).widget;

         if (!w) {
            continue;
         }
         w->removeEventFilter(oldDelegate);
         w->installEventFilter(newDelegate);
      }
   }

   void populate();

   // private slots
   void _q_dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
   void _q_commitData(QWidget *);
   void _q_closeEditor(QWidget *, QAbstractItemDelegate::EndEditHint);
   void _q_modelDestroyed();

   struct WidgetMapper {
      inline WidgetMapper(QWidget *w = 0, int c = 0, const QModelIndex &i = QModelIndex())
         : widget(w), section(c), currentIndex(i) {}

      inline WidgetMapper(QWidget *w, int c, const QModelIndex &i, const QString &p)
         : widget(w), section(c), currentIndex(i), property(p) {}

      QPointer<QWidget> widget;
      int section;
      QPersistentModelIndex currentIndex;
      QString property;
   };

   void populate(WidgetMapper &m);
   int findWidget(QWidget *w) const;

   bool commit(const WidgetMapper &m);

   QList<WidgetMapper> widgetMap;

 protected:
   QDataWidgetMapper *q_ptr;

};

int QDataWidgetMapperPrivate::findWidget(QWidget *w) const
{
   for (int i = 0; i < widgetMap.count(); ++i) {
      if (widgetMap.at(i).widget == w) {
         return i;
      }
   }
   return -1;
}

bool QDataWidgetMapperPrivate::commit(const WidgetMapper &m)
{
   if (m.widget.isNull()) {
      return true;   // just ignore
   }

   if (!m.currentIndex.isValid()) {
      return false;
   }

   // Create copy to avoid passing the widget mappers data
   QModelIndex idx = m.currentIndex;
   if (m.property.isEmpty()) {
      delegate->setModelData(m.widget, model, idx);
   } else {
      model->setData(idx, m.widget->property(m.property), Qt::EditRole);
   }

   return true;
}

void QDataWidgetMapperPrivate::populate(WidgetMapper &m)
{
   if (m.widget.isNull()) {
      return;
   }

   m.currentIndex = indexAt(m.section);
   if (m.property.isEmpty()) {
      delegate->setEditorData(m.widget, m.currentIndex);
   } else {
      m.widget->setProperty(m.property, m.currentIndex.data(Qt::EditRole));
   }
}

void QDataWidgetMapperPrivate::populate()
{
   for (int i = 0; i < widgetMap.count(); ++i) {
      populate(widgetMap[i]);
   }
}

static bool qContainsIndex(const QModelIndex &idx, const QModelIndex &topLeft,
                           const QModelIndex &bottomRight)
{
   return idx.row() >= topLeft.row() && idx.row() <= bottomRight.row()
          && idx.column() >= topLeft.column() && idx.column() <= bottomRight.column();
}

void QDataWidgetMapperPrivate::_q_dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
   if (topLeft.parent() != rootIndex) {
      return;   // not in our hierarchy
   }

   for (int i = 0; i < widgetMap.count(); ++i) {
      WidgetMapper &m = widgetMap[i];
      if (qContainsIndex(m.currentIndex, topLeft, bottomRight)) {
         populate(m);
      }
   }
}

void QDataWidgetMapperPrivate::_q_commitData(QWidget *w)
{
   if (submitPolicy == QDataWidgetMapper::ManualSubmit) {
      return;
   }

   int idx = findWidget(w);
   if (idx == -1) {
      return;   // not our widget
   }

   commit(widgetMap.at(idx));
}

class QFocusHelper: public QWidget
{
 public:
   bool focusNextPrevChild(bool next) override{
      return QWidget::focusNextPrevChild(next);
   }

   static inline void focusNextPrevChild(QWidget *w, bool next) {
      static_cast<QFocusHelper *>(w)->focusNextPrevChild(next);
   }
};

void QDataWidgetMapperPrivate::_q_closeEditor(QWidget *w, QAbstractItemDelegate::EndEditHint hint)
{
   int idx = findWidget(w);
   if (idx == -1) {
      return;   // not our widget
   }

   switch (hint) {
      case QAbstractItemDelegate::RevertModelCache: {
         populate(widgetMap[idx]);
         break;
      }
      case QAbstractItemDelegate::EditNextItem:
         QFocusHelper::focusNextPrevChild(w, true);
         break;
      case QAbstractItemDelegate::EditPreviousItem:
         QFocusHelper::focusNextPrevChild(w, false);
         break;
      case QAbstractItemDelegate::SubmitModelCache:
      case QAbstractItemDelegate::NoHint:
         // nothing
         break;
   }
}

void QDataWidgetMapperPrivate::_q_modelDestroyed()
{
   Q_Q(QDataWidgetMapper);

   model = 0;
   q->setModel(QAbstractItemModelPrivate::staticEmptyModel());
}

QDataWidgetMapper::QDataWidgetMapper(QObject *parent)
   : QObject(parent), d_ptr(new QDataWidgetMapperPrivate)
{
   d_ptr->q_ptr = this;
   setItemDelegate(new QItemDelegate(this));
}

QDataWidgetMapper::~QDataWidgetMapper()
{
}

void QDataWidgetMapper::setModel(QAbstractItemModel *model)
{
   Q_D(QDataWidgetMapper);

   if (d->model == model) {
      return;
   }

   if (d->model) {
      disconnect(d->model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
                 this, SLOT(_q_dataChanged(const QModelIndex &, const QModelIndex &)));

      disconnect(d->model, SIGNAL(destroyed()), this, SLOT(_q_modelDestroyed()));
   }
   clearMapping();
   d->rootIndex = QModelIndex();
   d->currentTopLeft = QModelIndex();

   d->model = model;

   connect(model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
           this, SLOT(_q_dataChanged(const QModelIndex &, const QModelIndex &)));

   connect(model, SIGNAL(destroyed()), this, SLOT(_q_modelDestroyed()));
}

/*!
    Returns the current model.

    \sa setModel()
 */
QAbstractItemModel *QDataWidgetMapper::model() const
{
   Q_D(const QDataWidgetMapper);
   return d->model == QAbstractItemModelPrivate::staticEmptyModel()
          ? static_cast<QAbstractItemModel *>(0)
          : d->model;
}

/*!
    Sets the item delegate to \a delegate. The delegate will be used to write
    data from the model into the widget and from the widget to the model,
    using QAbstractItemDelegate::setEditorData() and QAbstractItemDelegate::setModelData().

    The delegate also decides when to apply data and when to change the editor,
    using QAbstractItemDelegate::commitData() and QAbstractItemDelegate::closeEditor().

    \warning You should not share the same instance of a delegate between widget mappers
    or views. Doing so can cause incorrect or unintuitive editing behavior since each
    view connected to a given delegate may receive the \l{QAbstractItemDelegate::}{closeEditor()}
    signal, and attempt to access, modify or close an editor that has already been closed.
 */
void QDataWidgetMapper::setItemDelegate(QAbstractItemDelegate *delegate)
{
   Q_D(QDataWidgetMapper);
   QAbstractItemDelegate *oldDelegate = d->delegate;
   if (oldDelegate) {
      disconnect(oldDelegate, SIGNAL(commitData(QWidget *)), this, SLOT(_q_commitData(QWidget *)));
      disconnect(oldDelegate, SIGNAL(closeEditor(QWidget *, QAbstractItemDelegate::EndEditHint)),
                 this, SLOT(_q_closeEditor(QWidget *, QAbstractItemDelegate::EndEditHint)));
   }

   d->delegate = delegate;

   if (delegate) {
      connect(delegate, SIGNAL(commitData(QWidget *)), this, SLOT(_q_commitData(QWidget *)));
      connect(delegate, SIGNAL(closeEditor(QWidget *, QAbstractItemDelegate::EndEditHint)), this,
              SLOT(_q_closeEditor(QWidget *, QAbstractItemDelegate::EndEditHint)));
   }

   d->flipEventFilters(oldDelegate, delegate);
}

/*!
    Returns the current item delegate.
 */
QAbstractItemDelegate *QDataWidgetMapper::itemDelegate() const
{
   Q_D(const QDataWidgetMapper);
   return d->delegate;
}

/*!
    Sets the root item to \a index. This can be used to display
    a branch of a tree. Pass an invalid model index to display
    the top-most branch.

    \sa rootIndex()
 */
void QDataWidgetMapper::setRootIndex(const QModelIndex &index)
{
   Q_D(QDataWidgetMapper);
   d->rootIndex = index;
}

/*!
    Returns the current root index.

    \sa setRootIndex()
*/
QModelIndex QDataWidgetMapper::rootIndex() const
{
   Q_D(const QDataWidgetMapper);
   return QModelIndex(d->rootIndex);
}

void QDataWidgetMapper::addMapping(QWidget *widget, int section)
{
   Q_D(QDataWidgetMapper);

   removeMapping(widget);
   d->widgetMap.append(QDataWidgetMapperPrivate::WidgetMapper(widget, section, d->indexAt(section)));
   widget->installEventFilter(d->delegate);
}

/*!
  \since 4.3

  Essentially the same as addMapping(), but adds the possibility to specify
  the property to use specifying \a propertyName.

  \sa addMapping()
*/

void QDataWidgetMapper::addMapping(QWidget *widget, int section, const QString &propertyName)
{
   Q_D(QDataWidgetMapper);

   removeMapping(widget);
   d->widgetMap.append(QDataWidgetMapperPrivate::WidgetMapper(widget, section, d->indexAt(section), propertyName));
   widget->installEventFilter(d->delegate);
}

/*!
    Removes the mapping for the given \a widget.

    \sa addMapping(), clearMapping()
 */
void QDataWidgetMapper::removeMapping(QWidget *widget)
{
   Q_D(QDataWidgetMapper);

   int idx = d->findWidget(widget);
   if (idx == -1) {
      return;
   }

   d->widgetMap.removeAt(idx);
   widget->removeEventFilter(d->delegate);
}

/*!
    Returns the section the \a widget is mapped to or -1
    if the widget is not mapped.

    \sa addMapping(), removeMapping()
 */
int QDataWidgetMapper::mappedSection(QWidget *widget) const
{
   Q_D(const QDataWidgetMapper);

   int idx = d->findWidget(widget);
   if (idx == -1) {
      return -1;
   }

   return d->widgetMap.at(idx).section;
}

QString QDataWidgetMapper::mappedPropertyName(QWidget *widget) const
{
   Q_D(const QDataWidgetMapper);

   int idx = d->findWidget(widget);

   if (idx == -1) {
      return QByteArray();
   }

   const QDataWidgetMapperPrivate::WidgetMapper &m = d->widgetMap.at(idx);

   if (m.property.isEmpty()) {
      return m.widget->metaObject()->userProperty().name();

   } else {
      return m.property;
   }
}

/*!
    Returns the widget that is mapped at \a section, or
    0 if no widget is mapped at that section.

    \sa addMapping(), removeMapping()
 */
QWidget *QDataWidgetMapper::mappedWidgetAt(int section) const
{
   Q_D(const QDataWidgetMapper);

   for (int i = 0; i < d->widgetMap.count(); ++i) {
      if (d->widgetMap.at(i).section == section) {
         return d->widgetMap.at(i).widget;
      }
   }

   return 0;
}

/*!
    Repopulates all widgets with the current data of the model.
    All unsubmitted changes will be lost.

    \sa submit(), setSubmitPolicy()
 */
void QDataWidgetMapper::revert()
{
   Q_D(QDataWidgetMapper);

   d->populate();
}

/*!
    Submits all changes from the mapped widgets to the model.

    For every mapped section, the item delegate reads the current
    value from the widget and sets it in the model. Finally, the
    model's \l {QAbstractItemModel::}{submit()} method is invoked.

    Returns true if all the values were submitted, otherwise false.

    Note: For database models, QSqlQueryModel::lastError() can be
    used to retrieve the last error.

    \sa revert(), setSubmitPolicy()
 */
bool QDataWidgetMapper::submit()
{
   Q_D(QDataWidgetMapper);

   for (int i = 0; i < d->widgetMap.count(); ++i) {
      const QDataWidgetMapperPrivate::WidgetMapper &m = d->widgetMap.at(i);
      if (!d->commit(m)) {
         return false;
      }
   }

   return d->model->submit();
}

/*!
    Populates the widgets with data from the first row of the model
    if the orientation is horizontal (the default), otherwise
    with data from the first column.

    This is equivalent to calling \c setCurrentIndex(0).

    \sa toLast(), setCurrentIndex()
 */
void QDataWidgetMapper::toFirst()
{
   setCurrentIndex(0);
}

/*!
    Populates the widgets with data from the last row of the model
    if the orientation is horizontal (the default), otherwise
    with data from the last column.

    Calls setCurrentIndex() internally.

    \sa toFirst(), setCurrentIndex()
 */
void QDataWidgetMapper::toLast()
{
   Q_D(QDataWidgetMapper);
   setCurrentIndex(d->itemCount() - 1);
}


/*!
    Populates the widgets with data from the next row of the model
    if the orientation is horizontal (the default), otherwise
    with data from the next column.

    Calls setCurrentIndex() internally. Does nothing if there is
    no next row in the model.

    \sa toPrevious(), setCurrentIndex()
 */
void QDataWidgetMapper::toNext()
{
   Q_D(QDataWidgetMapper);
   setCurrentIndex(d->currentIdx() + 1);
}

/*!
    Populates the widgets with data from the previous row of the model
    if the orientation is horizontal (the default), otherwise
    with data from the previous column.

    Calls setCurrentIndex() internally. Does nothing if there is
    no previous row in the model.

    \sa toNext(), setCurrentIndex()
 */
void QDataWidgetMapper::toPrevious()
{
   Q_D(QDataWidgetMapper);
   setCurrentIndex(d->currentIdx() - 1);
}

/*!
    \property QDataWidgetMapper::currentIndex
    \brief the current row or column

    The widgets are populated with with data from the row at \a index
    if the orientation is horizontal (the default), otherwise with
    data from the column at \a index.

    \sa setCurrentModelIndex(), toFirst(), toNext(), toPrevious(), toLast()
*/
void QDataWidgetMapper::setCurrentIndex(int index)
{
   Q_D(QDataWidgetMapper);

   if (index < 0 || index >= d->itemCount()) {
      return;
   }
   d->currentTopLeft = d->orientation == Qt::Horizontal
                       ? d->model->index(index, 0, d->rootIndex)
                       : d->model->index(0, index, d->rootIndex);
   d->populate();

   emit currentIndexChanged(index);
}

int QDataWidgetMapper::currentIndex() const
{
   Q_D(const QDataWidgetMapper);
   return d->currentIdx();
}

/*!
    Sets the current index to the row of the \a index if the
    orientation is horizontal (the default), otherwise to the
    column of the \a index.

    Calls setCurrentIndex() internally. This convenience slot can be
    connected to the signal \l
    {QItemSelectionModel::}{currentRowChanged()} or \l
    {QItemSelectionModel::}{currentColumnChanged()} of another view's
    \l {QItemSelectionModel}{selection model}.

    The following example illustrates how to update all widgets
    with new data whenever the selection of a QTableView named
    \c myTableView changes:

    \snippet doc/src/snippets/code/src_gui_itemviews_qdatawidgetmapper.cpp 2

    \sa currentIndex()
*/
void QDataWidgetMapper::setCurrentModelIndex(const QModelIndex &index)
{
   Q_D(QDataWidgetMapper);

   if (!index.isValid()
         || index.model() != d->model
         || index.parent() != d->rootIndex) {
      return;
   }

   setCurrentIndex(d->orientation == Qt::Horizontal ? index.row() : index.column());
}

/*!
    Clears all mappings.

    \sa addMapping(), removeMapping()
 */
void QDataWidgetMapper::clearMapping()
{
   Q_D(QDataWidgetMapper);

   while (!d->widgetMap.isEmpty()) {
      QWidget *w = d->widgetMap.takeLast().widget;
      if (w) {
         w->removeEventFilter(d->delegate);
      }
   }
}

/*!
    \property QDataWidgetMapper::orientation
    \brief the orientation of the model

    If the orientation is Qt::Horizontal (the default), a widget is
    mapped to a column of a data model. The widget will be populated
    with the model's data from its mapped column and the row that
    currentIndex() points at.

    Use Qt::Horizontal for tabular data that looks like this:

    \table
    \row \o 1 \o Qt Norway       \o Oslo
    \row \o 2 \o Qt Australia    \o Brisbane
    \row \o 3 \o Qt USA          \o Silicon Valley
    \row \o 4 \o Qt China        \o Beijing
    \row \o 5 \o Qt Germany      \o Berlin
    \endtable

    If the orientation is set to Qt::Vertical, a widget is mapped to
    a row. Calling setCurrentIndex() will change the current column.
    The widget will be populates with the model's data from its
    mapped row and the column that currentIndex() points at.

    Use Qt::Vertical for tabular data that looks like this:

    \table
    \row \o 1 \o 2 \o 3 \o 4 \o 5
    \row \o Qt Norway \o Qt Australia \o Qt USA \o Qt China \o Qt Germany
    \row \o Oslo \o Brisbane \o Silicon Valley \o Beijing \i Berlin
    \endtable

    Changing the orientation clears all existing mappings.
*/
void QDataWidgetMapper::setOrientation(Qt::Orientation orientation)
{
   Q_D(QDataWidgetMapper);

   if (d->orientation == orientation) {
      return;
   }

   clearMapping();
   d->orientation = orientation;
}

Qt::Orientation QDataWidgetMapper::orientation() const
{
   Q_D(const QDataWidgetMapper);
   return d->orientation;
}

/*!
    \property QDataWidgetMapper::submitPolicy
    \brief the current submit policy

    Changing the current submit policy will revert all widgets
    to the current data from the model.
*/
void QDataWidgetMapper::setSubmitPolicy(SubmitPolicy policy)
{
   Q_D(QDataWidgetMapper);
   if (policy == d->submitPolicy) {
      return;
   }

   revert();
   d->submitPolicy = policy;
}

QDataWidgetMapper::SubmitPolicy QDataWidgetMapper::submitPolicy() const
{
   Q_D(const QDataWidgetMapper);
   return d->submitPolicy;
}

void QDataWidgetMapper::_q_dataChanged(const QModelIndex &un_named_arg1, const QModelIndex &un_named_arg2)
{
   Q_D(QDataWidgetMapper);
   d->_q_dataChanged(un_named_arg1, un_named_arg2);
}

void QDataWidgetMapper::_q_commitData(QWidget *un_named_arg1)
{
   Q_D(QDataWidgetMapper);
   d->_q_commitData(un_named_arg1);
}

void QDataWidgetMapper::_q_closeEditor(QWidget *un_named_arg1, QAbstractItemDelegate::EndEditHint un_named_arg2)
{
   Q_D(QDataWidgetMapper);
   d->_q_closeEditor(un_named_arg1, un_named_arg2);
}

void QDataWidgetMapper::_q_modelDestroyed()
{
   Q_D(QDataWidgetMapper);
   d->_q_modelDestroyed();
}

QT_END_NAMESPACE

#endif // QT_NO_DATAWIDGETMAPPER
