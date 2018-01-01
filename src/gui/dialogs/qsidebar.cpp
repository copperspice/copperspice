/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include <qsidebar_p.h>
#include <qfilesystemmodel.h>

#ifndef QT_NO_FILEDIALOG

#include <qaction.h>
#include <qurl.h>
#include <qmenu.h>
#include <qmimedata.h>
#include <qevent.h>
#include <qdebug.h>
#include <qfileiconprovider.h>
#include <qfiledialog.h>

QT_BEGIN_NAMESPACE

void QSideBarDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
   QStyledItemDelegate::initStyleOption(option, index);
   QVariant value = index.data(QUrlModel::EnabledRole);

   if (value.isValid()) {
      // if the bookmark/entry is not enabled then we paint it in gray
      if (!qvariant_cast<bool>(value)) {
         option->state &= ~QStyle::State_Enabled;
      }
   }
}

QUrlModel::QUrlModel(QObject *parent) : QStandardItemModel(parent), showFullPath(false), fileSystemModel(0)
{
}

/*!
    \reimp
*/
QStringList QUrlModel::mimeTypes() const
{
   return QStringList(QLatin1String("text/uri-list"));
}

/*!
    \reimp
*/
Qt::ItemFlags QUrlModel::flags(const QModelIndex &index) const
{
   Qt::ItemFlags flags = QStandardItemModel::flags(index);
   if (index.isValid()) {
      flags &= ~Qt::ItemIsEditable;
      // ### some future version could support "moving" urls onto a folder
      flags &= ~Qt::ItemIsDropEnabled;
   }

   if (index.data(Qt::DecorationRole).isNull()) {
      flags &= ~Qt::ItemIsEnabled;
   }

   return flags;
}

/*!
    \reimp
*/
QMimeData *QUrlModel::mimeData(const QModelIndexList &indexes) const
{
   QList<QUrl> list;
   for (int i = 0; i < indexes.count(); ++i) {
      if (indexes.at(i).column() == 0) {
         list.append(indexes.at(i).data(UrlRole).toUrl());
      }
   }
   QMimeData *data = new QMimeData();
   data->setUrls(list);
   return data;
}

#ifndef QT_NO_DRAGANDDROP

/*!
    Decide based upon the data if it should be accepted or not
    We only accept dirs and not files
*/
bool QUrlModel::canDrop(QDragEnterEvent *event)
{
   if (!event->mimeData()->formats().contains(mimeTypes().first())) {
      return false;
   }

   const QList<QUrl> list = event->mimeData()->urls();
   for (int i = 0; i < list.count(); ++i) {
      QModelIndex idx = fileSystemModel->index(list.at(0).toLocalFile());
      if (!fileSystemModel->isDir(idx)) {
         return false;
      }
   }
   return true;
}

/*!
    \reimp
*/
bool QUrlModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                             int row, int column, const QModelIndex &parent)
{
   if (!data->formats().contains(mimeTypes().first())) {
      return false;
   }

   Q_UNUSED(action);
   Q_UNUSED(column);
   Q_UNUSED(parent);

   addUrls(data->urls(), row);
   return true;
}

#endif // QT_NO_DRAGANDDROP

/*!
    \reimp

    If the role is the UrlRole then handle otherwise just pass to QStandardItemModel
*/
bool QUrlModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
   if (value.type() == QVariant::Url) {
      QUrl url = value.toUrl();
      QModelIndex dirIndex = fileSystemModel->index(url.toLocalFile());

      //On windows the popup display the "C:\", convert to nativeSeparators
      if (showFullPath)
         QStandardItemModel::setData(index, QDir::toNativeSeparators(fileSystemModel->data(dirIndex,
                                     QFileSystemModel::FilePathRole).toString()));

      else {
         QStandardItemModel::setData(index, QDir::toNativeSeparators(fileSystemModel->data(dirIndex,
                                     QFileSystemModel::FilePathRole).toString()), Qt::ToolTipRole);

         QStandardItemModel::setData(index, fileSystemModel->data(dirIndex).toString());
      }

      QStandardItemModel::setData(index, fileSystemModel->data(dirIndex, Qt::DecorationRole), Qt::DecorationRole);
      QStandardItemModel::setData(index, url, UrlRole);
      return true;
   }
   return QStandardItemModel::setData(index, value, role);
}

void QUrlModel::setUrl(const QModelIndex &index, const QUrl &url, const QModelIndex &dirIndex)
{
   setData(index, url, UrlRole);
   if (url.path().isEmpty()) {
      setData(index, fileSystemModel->myComputer());
      setData(index, fileSystemModel->myComputer(Qt::DecorationRole), Qt::DecorationRole);

   } else {
      QString newName;
      if (showFullPath) {
         //On windows the popup display the "C:\", convert to nativeSeparators
         newName = QDir::toNativeSeparators(dirIndex.data(QFileSystemModel::FilePathRole).toString());
      } else {
         newName = dirIndex.data().toString();
      }

      QIcon newIcon = qvariant_cast<QIcon>(dirIndex.data(Qt::DecorationRole));
      if (!dirIndex.isValid()) {
         newIcon = fileSystemModel->iconProvider()->icon(QFileIconProvider::Folder);
         newName = QFileInfo(url.toLocalFile()).fileName();

         if (!invalidUrls.contains(url)) {
            invalidUrls.append(url);
         }

         // bookmark is invalid then we set to false the EnabledRole
         setData(index, false, EnabledRole);

      } else {
         // bookmark is valid then we set to true the EnabledRole
         setData(index, true, EnabledRole);
      }

      // Make sure that we have at least 32x32 images
      const QSize size = newIcon.actualSize(QSize(32, 32));
      if (size.width() < 32) {
         QPixmap smallPixmap = newIcon.pixmap(QSize(32, 32));
         newIcon.addPixmap(smallPixmap.scaledToWidth(32, Qt::SmoothTransformation));
      }

      if (index.data().toString() != newName) {
         setData(index, newName);
      }
      QIcon oldIcon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
      if (oldIcon.cacheKey() != newIcon.cacheKey()) {
         setData(index, newIcon, Qt::DecorationRole);
      }
   }
}

void QUrlModel::setUrls(const QList<QUrl> &list)
{
   removeRows(0, rowCount());
   invalidUrls.clear();
   watching.clear();
   addUrls(list, 0);
}

void QUrlModel::addUrls(const QList<QUrl> &list, int row, bool move)
{
   if (row == -1) {
      row = rowCount();
   }

   row = qMin(row, rowCount());
   for (int i = list.count() - 1; i >= 0; --i) {
      QUrl url = list.at(i);
      if (!url.isValid() || url.scheme() != QLatin1String("file")) {
         continue;
      }

      // this makes sure the url is clean
      const QString cleanUrl = QDir::cleanPath(url.toLocalFile());
      url = QUrl::fromLocalFile(cleanUrl);

      for (int j = 0; move && j < rowCount(); ++j) {
         QString local = index(j, 0).data(UrlRole).toUrl().toLocalFile();

#if defined(Q_OS_WIN)
         if (index(j, 0).data(UrlRole).toUrl().toLocalFile().toLower() == cleanUrl.toLower()) {
#else
         if (index(j, 0).data(UrlRole).toUrl().toLocalFile() == cleanUrl) {
#endif
            removeRow(j);
            if (j <= row) {
               row--;
            }
            break;
         }
      }

      row = qMax(row, 0);
      QModelIndex idx = fileSystemModel->index(cleanUrl);

      if (!fileSystemModel->isDir(idx)) {
         continue;
      }

      insertRows(row, 1);
      setUrl(index(row, 0), url, idx);
      watching.append(qMakePair(idx, cleanUrl));
   }
}

QList<QUrl> QUrlModel::urls() const
{
   QList<QUrl> list;
   for (int i = 0; i < rowCount(); ++i) {
      list.append(data(index(i, 0), UrlRole).toUrl());
   }
   return list;
}

void QUrlModel::setFileSystemModel(QFileSystemModel *model)
{
   if (model == fileSystemModel) {
      return;
   }

   if (fileSystemModel != 0) {
      disconnect(model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
                 this, SLOT(dataChanged(const QModelIndex &, const QModelIndex &)));

      disconnect(model, SIGNAL(layoutChanged()), this, SLOT(layoutChanged()));
      disconnect(model, SIGNAL(rowsRemoved(const QModelIndex &, int, int)), this, SLOT(layoutChanged()));
   }

   fileSystemModel = model;
   if (fileSystemModel != 0) {
      connect(model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
              this, SLOT(dataChanged(const QModelIndex &, const QModelIndex &)));

      connect(model, SIGNAL(layoutChanged()), this, SLOT(layoutChanged()));
      connect(model, SIGNAL(rowsRemoved(const QModelIndex &, int, int)), this, SLOT(layoutChanged()));
   }

   clear();
   insertColumns(0, 1);
}

/*
    If one of the index's we are watching has changed update our internal data
*/
void QUrlModel::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
   QModelIndex parent = topLeft.parent();

   for (int i = 0; i < watching.count(); ++i) {
      QModelIndex index = watching.at(i).first;
      if (index.model() && topLeft.model()) {
         Q_ASSERT(index.model() == topLeft.model());
      }

      if (   index.row() >= topLeft.row()
             && index.row() <= bottomRight.row()
             && index.column() >= topLeft.column()
             && index.column() <= bottomRight.column()
             && index.parent() == parent) {
         changed(watching.at(i).second);
      }
   }
}

/*!
    Re-get all of our data, anything could have changed
 */
void QUrlModel::layoutChanged()
{
   QStringList paths;

   for (int i = 0; i < watching.count(); ++i) {
      paths.append(watching.at(i).second);
   }

   watching.clear();

   for (int i = 0; i < paths.count(); ++i) {
      QString path = paths.at(i);
      QModelIndex newIndex = fileSystemModel->index(path);
      watching.append(QPair<QModelIndex, QString>(newIndex, path));
      if (newIndex.isValid()) {
         changed(path);
      }
   }
}

void QUrlModel::changed(const QString &path)
{
   for (int i = 0; i < rowCount(); ++i) {
      QModelIndex idx = index(i, 0);
      if (idx.data(UrlRole).toUrl().toLocalFile() == path) {
         setData(idx, idx.data(UrlRole).toUrl());
      }
   }
}

QSidebar::QSidebar(QWidget *parent) : QListView(parent)
{
   m_selectUrl_processing = 0;
}

void QSidebar::setModelAndUrls(QFileSystemModel *model, const QList<QUrl> &newUrls)
{
   // ### TODO make icon size dynamic
   setIconSize(QSize(24, 24));
   setUniformItemSizes(true);
   urlModel = new QUrlModel(this);
   urlModel->setFileSystemModel(model);
   setModel(urlModel);
   setItemDelegate(new QSideBarDelegate(this));

   connect(selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
           this, SLOT(clicked(const QModelIndex &)));

#ifndef QT_NO_DRAGANDDROP
   setDragDropMode(QAbstractItemView::DragDrop);
#endif

   setContextMenuPolicy(Qt::CustomContextMenu);
   connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(showContextMenu(const QPoint &)));
   urlModel->setUrls(newUrls);
   setCurrentIndex(this->model()->index(0, 0));
}

QSidebar::~QSidebar()
{
}

#ifndef QT_NO_DRAGANDDROP
void QSidebar::dragEnterEvent(QDragEnterEvent *event)
{
   if (urlModel->canDrop(event)) {
      QListView::dragEnterEvent(event);
   }
}
#endif

QSize QSidebar::sizeHint() const
{
   if (model()) {
      return QListView::sizeHintForIndex(model()->index(0, 0)) + QSize(2 * frameWidth(), 2 * frameWidth());
   }
   return QListView::sizeHint();
}

void QSidebar::selectUrl(const QUrl &url)
{
   ++ m_selectUrl_processing;

   selectionModel()->clear();

   for (int i = 0; i < model()->rowCount(); ++i) {
      QModelIndex tempIndex = model()->index(i, 0);

      if (tempIndex.data(QUrlModel::UrlRole).toUrl() == url) {

         // select() emits a signal which is connected to clicked()
         selectionModel()->select(tempIndex, QItemSelectionModel::Select);
         break;
      }
   }

   -- m_selectUrl_processing;
}

#ifndef QT_NO_MENU
// internal
void QSidebar::showContextMenu(const QPoint &position)
{
   QList<QAction *> actions;
   if (indexAt(position).isValid()) {
      QAction *action = new QAction(QFileDialog::tr("Remove"), this);

      if (indexAt(position).data(QUrlModel::UrlRole).toUrl().path().isEmpty()) {
         action->setEnabled(false);
      }

      connect(action, SIGNAL(triggered()), this, SLOT(removeEntry()));

      actions.append(action);
   }

   if (actions.count() > 0) {
      QMenu::exec(actions, mapToGlobal(position));
   }
}
#endif

// internal
void QSidebar::removeEntry()
{
   QList<QModelIndex> idxs = selectionModel()->selectedIndexes();
   QList<QPersistentModelIndex> indexes;

   for (int i = 0; i < idxs.count(); i++) {
      indexes.append(idxs.at(i));
   }

   for (int i = 0; i < indexes.count(); ++i)
      if (!indexes.at(i).data(QUrlModel::UrlRole).toUrl().path().isEmpty()) {
         model()->removeRow(indexes.at(i).row());
      }
}

// internal
void QSidebar::clicked(const QModelIndex &index)
{
   if (m_selectUrl_processing == 0) {
      QUrl url = model()->index(index.row(), 0).data(QUrlModel::UrlRole).toUrl();

      emit goToUrl(url);
      selectUrl(url);
   }
}

/*!
    \reimp
    Don't automatically select something
 */
void QSidebar::focusInEvent(QFocusEvent *event)
{
   QAbstractScrollArea::focusInEvent(event);
   viewport()->update();
}

/*!
    \reimp
 */
bool QSidebar::event(QEvent *event)
{
   if (event->type() == QEvent::KeyRelease) {
      QKeyEvent *ke = (QKeyEvent *) event;
      if (ke->key() == Qt::Key_Delete) {
         removeEntry();
         return true;
      }
   }
   return QListView::event(event);
}

QT_END_NAMESPACE

#endif
