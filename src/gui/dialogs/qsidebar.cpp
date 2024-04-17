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

#include <qsidebar_p.h>

#ifndef QT_NO_FILEDIALOG

#include <qaction.h>
#include <qdebug.h>
#include <qevent.h>
#include <qfileiconprovider.h>
#include <qfiledialog.h>
#include <qfilesystemmodel.h>
#include <qmenu.h>
#include <qmimedata.h>
#include <qurl.h>

void QSideBarDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
   QStyledItemDelegate::initStyleOption(option, index);
   QVariant value = index.data(QUrlModel::EnabledRole);

   if (value.isValid()) {
      // if the bookmark/entry is not enabled then we paint it in gray
      if (! value.value<bool>()) {
         option->state &= ~QStyle::State_Enabled;
      }
   }
}

QUrlModel::QUrlModel(QObject *parent)
   : QStandardItemModel(parent), showFullPath(false), fileSystemModel(nullptr)
{
}

QStringList QUrlModel::mimeTypes() const
{
   return QStringList("text/uri-list");
}

Qt::ItemFlags QUrlModel::flags(const QModelIndex &index) const
{
   Qt::ItemFlags flags = QStandardItemModel::flags(index);

   if (index.isValid()) {
      flags &= ~Qt::ItemIsEditable;
      // ### some future version could support "moving" urls onto a folder
      flags &= ~Qt::ItemIsDropEnabled;
   }

   if (! index.data(Qt::DecorationRole).isValid()) {
      flags &= ~Qt::ItemIsEnabled;
   }

   return flags;
}

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
bool QUrlModel::canDrop(QDragEnterEvent *event)
{
   if (! event->mimeData()->formats().contains(mimeTypes().first())) {
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

bool QUrlModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
   int row, int column, const QModelIndex &parent)
{
   (void) action;
   (void) column;
   (void) parent;

   if (! data->formats().contains(mimeTypes().first())) {
      return false;
   }

   addUrls(data->urls(), row);

   return true;
}
#endif

bool QUrlModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
   if (value.type() == QVariant::Url) {
      QUrl url = value.toUrl();
      QModelIndex dirIndex = fileSystemModel->index(url.toLocalFile());

      //On windows the popup display the "C:\", convert to nativeSeparators
      if (showFullPath) {
         QStandardItemModel::setData(index, QDir::toNativeSeparators(fileSystemModel->data(dirIndex,
               QFileSystemModel::FilePathRole).toString()));

      } else {
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

      QIcon newIcon = (dirIndex.data(Qt::DecorationRole)).value<QIcon>();
      if (!dirIndex.isValid()) {
         const QFileIconProvider *provider = fileSystemModel->iconProvider();
         if (provider) {
            newIcon = provider->icon(QFileIconProvider::Folder);
         }
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

      QIcon oldIcon = (index.data(Qt::DecorationRole)).value<QIcon>();
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
      if (! url.isValid() || url.scheme() != "file") {
         continue;
      }

      // this makes sure the url is clean
      const QString cleanUrl = QDir::cleanPath(url.toLocalFile());
      if (!cleanUrl.isEmpty()) {
         url = QUrl::fromLocalFile(cleanUrl);
      }

      for (int j = 0; move && j < rowCount(); ++j) {
         QString local = index(j, 0).data(UrlRole).toUrl().toLocalFile();

#if defined(Q_OS_WIN)
         const Qt::CaseSensitivity cs = Qt::CaseInsensitive;
#else
         const Qt::CaseSensitivity cs = Qt::CaseSensitive;
#endif
         if (!cleanUrl.compare(local, cs)) {
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
   const int numRows = rowCount();


   for (int i = 0; i < numRows; ++i) {
      list.append(data(index(i, 0), UrlRole).toUrl());
   }

   return list;
}

void QUrlModel::setFileSystemModel(QFileSystemModel *model)
{
   if (model == fileSystemModel) {
      return;
   }

   if (fileSystemModel != nullptr) {
      disconnect(model, &QFileSystemModel::dataChanged,   this, &QUrlModel::dataChanged);
      disconnect(model, &QFileSystemModel::layoutChanged, this, &QUrlModel::layoutChanged);
      disconnect(model, &QFileSystemModel::rowsRemoved,   this, &QUrlModel::layoutChanged);
   }

   fileSystemModel = model;
   if (fileSystemModel != nullptr) {
      connect(model, &QFileSystemModel::dataChanged,   this, &QUrlModel::dataChanged);
      connect(model, &QFileSystemModel::layoutChanged, this, &QUrlModel::layoutChanged);
      connect(model, &QFileSystemModel::rowsRemoved,   this, &QUrlModel::layoutChanged);
   }

   clear();
   insertColumns(0, 1);
}

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

void QUrlModel::layoutChanged()
{
   QStringList paths;
   const int numPaths = watching.count();

   for (int i = 0; i < numPaths; ++i) {
      paths.append(watching.at(i).second);
   }

   watching.clear();

   for (int i = 0; i < numPaths; ++i) {
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

   connect(selectionModel(), &QItemSelectionModel::currentChanged, this, &QSidebar::clicked);

#ifndef QT_NO_DRAGANDDROP
   setDragDropMode(QAbstractItemView::DragDrop);
#endif

   setContextMenuPolicy(Qt::CustomContextMenu);
   connect(this, &QSidebar::customContextMenuRequested, this, &QSidebar::showContextMenu);
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
   disconnect(selectionModel(), &QItemSelectionModel::currentChanged, this, &QSidebar::clicked);

   selectionModel()->clear();

   for (int i = 0; i < model()->rowCount(); ++i) {
      QModelIndex tempIndex = model()->index(i, 0);

      if (tempIndex.data(QUrlModel::UrlRole).toUrl() == url) {

         // select() emits a signal which is connected to clicked()
         selectionModel()->select(tempIndex, QItemSelectionModel::Select);
         break;
      }
   }

   connect(selectionModel(), &QItemSelectionModel::currentChanged, this, &QSidebar::clicked);
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

      connect(action, &QAction::triggered, this, &QSidebar::removeEntry);
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

   const int numIndexes = idxs.count();
   for (int i = 0; i < numIndexes; i++) {
      indexes.append(idxs.at(i));
   }

   for (int i = 0; i < numIndexes; ++i) {
      if (!indexes.at(i).data(QUrlModel::UrlRole).toUrl().path().isEmpty()) {
         model()->removeRow(indexes.at(i).row());
      }
   }
}

void QSidebar::clicked(const QModelIndex &index)
{
   QUrl url = model()->index(index.row(), 0).data(QUrlModel::UrlRole).toUrl();

   emit goToUrl(url);
   selectUrl(url);
}

void QSidebar::focusInEvent(QFocusEvent *event)
{
   QAbstractScrollArea::focusInEvent(event);
   viewport()->update();
}

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

#endif
