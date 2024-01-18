/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
* Copyright (c) 2015 The Qt Company Ltd.
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

class QDataWidgetMapperPrivate
{

 public:
   Q_DECLARE_PUBLIC(QDataWidgetMapper)

   QDataWidgetMapperPrivate()
      : model(QAbstractItemModelPrivate::staticEmptyModel()), delegate(nullptr),
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

      for (auto item : widgetMap) {
         QWidget *w = item.widget;

         if (! w) {
            continue;
         }

         w->removeEventFilter(oldDelegate);
         w->installEventFilter(newDelegate);
      }
   }

   void populate();

   // private slots
   void _q_dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
   void _q_commitData(QWidget *currentWidget);
   void _q_closeEditor(QWidget *currentWidget, QAbstractItemDelegate::EndEditHint hint);
   void _q_modelDestroyed();

   struct WidgetMapper {
      inline WidgetMapper(QWidget *w = nullptr, int c = 0, const QModelIndex &i = QModelIndex())
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
   for (auto iter = widgetMap.cbegin(), end = widgetMap.cend(); iter != end; ++iter) {
      if (iter->widget == w) {
         return int(std::distance(widgetMap.cbegin(), iter));
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
   for (auto &item : widgetMap) {
      populate(item);
   }
}

static bool qContainsIndex(const QModelIndex &idx, const QModelIndex &topLeft,
   const QModelIndex &bottomRight)
{
   return idx.row() >= topLeft.row() && idx.row() <= bottomRight.row()
      && idx.column() >= topLeft.column() && idx.column() <= bottomRight.column();
}

void QDataWidgetMapperPrivate::_q_dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
   (void) roles;

   if (topLeft.parent() != rootIndex) {
      return;   // not in our hierarchy
   }

   for (auto &item : widgetMap) {
      if (qContainsIndex(item.currentIndex, topLeft, bottomRight)) {
         populate(item);
      }
   }
}

void QDataWidgetMapperPrivate::_q_commitData(QWidget *currentWidget)
{
   if (submitPolicy == QDataWidgetMapper::ManualSubmit) {
      return;
   }

   int idx = findWidget(currentWidget);
   if (idx == -1) {
      return;   // not our widget
   }

   commit(widgetMap.at(idx));
}

void QDataWidgetMapperPrivate::_q_closeEditor(QWidget *currentWidget, QAbstractItemDelegate::EndEditHint hint)
{
   int idx = findWidget(currentWidget);

   if (idx == -1) {
      return;   // not our widget
   }

   switch (hint) {
      case QAbstractItemDelegate::RevertModelCache: {
         populate(widgetMap[idx]);
         break;
      }

      case QAbstractItemDelegate::EditNextItem:
         currentWidget->focusNextChild();
         break;

      case QAbstractItemDelegate::EditPreviousItem:
         currentWidget->focusPreviousChild();
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

   model = nullptr;
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
      disconnect(d->model, &QAbstractItemModel::dataChanged, this, &QDataWidgetMapper::_q_dataChanged);
      disconnect(d->model, &QAbstractItemModel::destroyed,   this, &QDataWidgetMapper::_q_modelDestroyed);
   }

   clearMapping();
   d->rootIndex = QModelIndex();
   d->currentTopLeft = QModelIndex();

   d->model = model;

   connect(model, &QAbstractItemModel::dataChanged, this, &QDataWidgetMapper::_q_dataChanged);
   connect(model, &QAbstractItemModel::destroyed,   this, &QDataWidgetMapper::_q_modelDestroyed);
}

QAbstractItemModel *QDataWidgetMapper::model() const
{
   Q_D(const QDataWidgetMapper);
   return d->model == QAbstractItemModelPrivate::staticEmptyModel()
                  ? static_cast<QAbstractItemModel *>(nullptr) : d->model;
}

void QDataWidgetMapper::setItemDelegate(QAbstractItemDelegate *delegate)
{
   Q_D(QDataWidgetMapper);

   QAbstractItemDelegate *oldDelegate = d->delegate;

   if (oldDelegate) {
      disconnect(oldDelegate, &QAbstractItemDelegate::commitData,  this, &QDataWidgetMapper::_q_commitData);
      disconnect(oldDelegate, &QAbstractItemDelegate::closeEditor, this, &QDataWidgetMapper::_q_closeEditor);
   }

   d->delegate = delegate;

   if (delegate) {
      connect(delegate, &QAbstractItemDelegate::commitData,  this, &QDataWidgetMapper::_q_commitData);
      connect(delegate, &QAbstractItemDelegate::closeEditor, this, &QDataWidgetMapper::_q_closeEditor);
   }

   d->flipEventFilters(oldDelegate, delegate);
}

QAbstractItemDelegate *QDataWidgetMapper::itemDelegate() const
{
   Q_D(const QDataWidgetMapper);
   return d->delegate;
}

void QDataWidgetMapper::setRootIndex(const QModelIndex &index)
{
   Q_D(QDataWidgetMapper);
   d->rootIndex = index;
}

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

void QDataWidgetMapper::addMapping(QWidget *widget, int section, const QString &propertyName)
{
   Q_D(QDataWidgetMapper);

   removeMapping(widget);
   d->widgetMap.append(QDataWidgetMapperPrivate::WidgetMapper(widget, section, d->indexAt(section), propertyName));
   widget->installEventFilter(d->delegate);
}

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

   for (auto &item : d->widgetMap) {
      if (item.section == section) {
         return item.widget;
      }
   }

   return nullptr;
}

void QDataWidgetMapper::revert()
{
   Q_D(QDataWidgetMapper);

   d->populate();
}


bool QDataWidgetMapper::submit()
{
   Q_D(QDataWidgetMapper);

   for (auto &item : d->widgetMap) {
      if (! d->commit(item)) {
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

   QList<QDataWidgetMapperPrivate::WidgetMapper> copy;
   d->widgetMap.swap(copy);

   for (auto &item : copy) {

      if (item.widget) {
         item.widget->removeEventFilter(d->delegate);
      }
   }
}

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

void QDataWidgetMapper::_q_dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
   Q_D(QDataWidgetMapper);
   d->_q_dataChanged(topLeft, bottomRight, roles);
}

void QDataWidgetMapper::_q_commitData(QWidget *currentWidget)
{
   Q_D(QDataWidgetMapper);
   d->_q_commitData(currentWidget);
}

void QDataWidgetMapper::_q_closeEditor(QWidget *currentWidget, QAbstractItemDelegate::EndEditHint hint)
{
   Q_D(QDataWidgetMapper);
   d->_q_closeEditor(currentWidget, hint);
}

void QDataWidgetMapper::_q_modelDestroyed()
{
   Q_D(QDataWidgetMapper);
   d->_q_modelDestroyed();
}

#endif
