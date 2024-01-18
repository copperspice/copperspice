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

#ifndef QSIDEBAR_P_H
#define QSIDEBAR_P_H

#include <qlistwidget.h>
#include <qstandarditemmodel.h>
#include <qstyleditemdelegate.h>
#include <qurl.h>

#ifndef QT_NO_FILEDIALOG

class QFileSystemModel;

class QSideBarDelegate : public QStyledItemDelegate
{
 public:
   QSideBarDelegate(QWidget *parent = nullptr) : QStyledItemDelegate(parent) {}
   void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override;
};

class QUrlModel : public QStandardItemModel
{
   GUI_CS_OBJECT(QUrlModel)

 public:
   enum Roles {
      UrlRole = Qt::UserRole + 1,
      EnabledRole = Qt::UserRole + 2
   };

   QUrlModel(QObject *parent = nullptr);

   QStringList mimeTypes() const override;
   QMimeData *mimeData(const QModelIndexList &indexes) const override;

#ifndef QT_NO_DRAGANDDROP
   bool canDrop(QDragEnterEvent *event);
   bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
#endif

   Qt::ItemFlags flags(const QModelIndex &index) const override;
   bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

   void setUrls(const QList<QUrl> &list);
   void addUrls(const QList<QUrl> &urls, int row = -1, bool move = true);
   QList<QUrl> urls() const;
   void setFileSystemModel(QFileSystemModel *model);
   bool showFullPath;

 private:
   GUI_CS_SLOT_1(Private, void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight))
   GUI_CS_SLOT_2(dataChanged)

   GUI_CS_SLOT_1(Private, void layoutChanged())
   GUI_CS_SLOT_2(layoutChanged)

   void setUrl(const QModelIndex &index, const QUrl &url, const QModelIndex &dirIndex);
   void changed(const QString &path);
   void addIndexToWatch(const QString &path, const QModelIndex &index);
   QFileSystemModel *fileSystemModel;
   QList<QPair<QModelIndex, QString>> watching;
   QList<QUrl> invalidUrls;
};

class QSidebar : public QListView
{
   GUI_CS_OBJECT(QSidebar)

 public:
   GUI_CS_SIGNAL_1(Public, void goToUrl(const QUrl &url))
   GUI_CS_SIGNAL_2(goToUrl, url)

   QSidebar(QWidget *parent = nullptr);
   void setModelAndUrls(QFileSystemModel *model, const QList<QUrl> &newUrls);
   ~QSidebar();

   QSize sizeHint() const override;

   void setUrls(const QList<QUrl> &list) {
      urlModel->setUrls(list);
   }
   void addUrls(const QList<QUrl> &list, int row) {
      urlModel->addUrls(list, row);
   }
   QList<QUrl> urls() const {
      return urlModel->urls();
   }

   void selectUrl(const QUrl &url);

 protected:
   bool event(QEvent *event) override;
   void focusInEvent(QFocusEvent *event) override;

#ifndef QT_NO_DRAGANDDROP
   void dragEnterEvent(QDragEnterEvent *event) override;
#endif

 private:
   GUI_CS_SLOT_1(Private, void clicked(const QModelIndex &index))
   GUI_CS_SLOT_2(clicked)

#ifndef QT_NO_MENU
   GUI_CS_SLOT_1(Private, void showContextMenu(const QPoint &position))
   GUI_CS_SLOT_2(showContextMenu)
#endif

   GUI_CS_SLOT_1(Private, void removeEntry())
   GUI_CS_SLOT_2(removeEntry)

   QUrlModel *urlModel;
};

#endif // QT_NO_FILEDIALOG

#endif

