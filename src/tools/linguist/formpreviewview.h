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

#ifndef FORMPREVIEWVIEW_H
#define FORMPREVIEWVIEW_H

#include <quiloader_p.h>

#include <QtCore/QHash>
#include <QtCore/QList>

#include <QtGui/QMainWindow>

QT_BEGIN_NAMESPACE

class MultiDataModel;
class FormFrame;
class MessageItem;

class QComboBox;
class QListWidgetItem;
class QGridLayout;
class QMdiArea;
class QMdiSubWindow;
class QToolBox;
class QTableWidgetItem;
class QTreeWidgetItem;

enum TranslatableEntryType {
   TranslatableProperty,
   TranslatableToolItemText,
   TranslatableToolItemToolTip,
   TranslatableTabPageText,
   TranslatableTabPageToolTip,
   TranslatableTabPageWhatsThis,
   TranslatableListWidgetItem,
   TranslatableTableWidgetItem,
   TranslatableTreeWidgetItem,
   TranslatableComboBoxItem
};

struct TranslatableEntry {
   TranslatableEntryType type;
   union {
      QObject *object;
      QComboBox *comboBox;
      QTabWidget *tabWidget;
      QToolBox *toolBox;
      QListWidgetItem *listWidgetItem;
      QTableWidgetItem *tableWidgetItem;
      QTreeWidgetItem *treeWidgetItem;
   } target;
   union {
      char *name;
      int index;
      struct {
         short index; // Known to be below 1000
         short column;
      } treeIndex;
   } prop;
};

typedef QHash<QUiTranslatableStringValue, QList<TranslatableEntry> > TargetsHash;

class FormPreviewView : public QMainWindow
{
   Q_OBJECT
 public:
   FormPreviewView(QWidget *parent, MultiDataModel *dataModel);

   void setSourceContext(int model, MessageItem *messageItem);

 private:
   bool m_isActive;
   QString m_currentFileName;
   QMdiArea *m_mdiArea;
   QMdiSubWindow *m_mdiSubWindow;
   QWidget *m_form;
   TargetsHash m_targets;
   QList<TranslatableEntry> m_highlights;
   MultiDataModel *m_dataModel;

   QString m_lastFormName;
   QString m_lastClassName;
   int m_lastModel;
};

QT_END_NAMESPACE

#endif // FORMPREVIEWVIEW_H
