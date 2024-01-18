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

#ifndef QCOMPLETER_H
#define QCOMPLETER_H

#include <qabstractitemmodel.h>
#include <qitemselection.h>
#include <qobject.h>
#include <qpoint.h>
#include <qrect.h>
#include <qstring.h>
#include <qscopedpointer.h>

#ifndef QT_NO_COMPLETER

class QAbstractItemView;
class QAbstractProxyModel;
class QCompleterPrivate;
class QWidget;

class Q_GUI_EXPORT QCompleter : public QObject
{
   GUI_CS_OBJECT(QCompleter)

   GUI_CS_PROPERTY_READ(completionPrefix, completionPrefix)
   GUI_CS_PROPERTY_WRITE(completionPrefix, setCompletionPrefix)

   GUI_CS_PROPERTY_READ(modelSorting, modelSorting)
   GUI_CS_PROPERTY_WRITE(modelSorting, setModelSorting)

   GUI_CS_PROPERTY_READ(completionMode, completionMode)
   GUI_CS_PROPERTY_WRITE(completionMode, setCompletionMode)

   GUI_CS_PROPERTY_READ(completionColumn, completionColumn)
   GUI_CS_PROPERTY_WRITE(completionColumn, setCompletionColumn)

   GUI_CS_PROPERTY_READ(completionRole, completionRole)
   GUI_CS_PROPERTY_WRITE(completionRole, setCompletionRole)

   GUI_CS_PROPERTY_READ(maxVisibleItems, maxVisibleItems)
   GUI_CS_PROPERTY_WRITE(maxVisibleItems, setMaxVisibleItems)

   GUI_CS_PROPERTY_READ(caseSensitivity, caseSensitivity)
   GUI_CS_PROPERTY_WRITE(caseSensitivity, setCaseSensitivity)

   GUI_CS_PROPERTY_READ(wrapAround, wrapAround)
   GUI_CS_PROPERTY_WRITE(wrapAround, setWrapAround)

 public:
   enum CompletionMode {
      PopupCompletion,
      UnfilteredPopupCompletion,
      InlineCompletion
   };

   enum ModelSorting {
      UnsortedModel = 0,
      CaseSensitivelySortedModel,
      CaseInsensitivelySortedModel
   };

   QCompleter(QObject *parent = nullptr);
   QCompleter(QAbstractItemModel *model, QObject *parent = nullptr);

#ifndef QT_NO_STRINGLISTMODEL
   QCompleter(const QStringList &list, QObject *parent = nullptr);
#endif

   QCompleter(const QCompleter &) = delete;
   QCompleter &operator=(const QCompleter &) = delete;

   ~QCompleter();

   void setWidget(QWidget *widget);
   QWidget *widget() const;

   void setModel(QAbstractItemModel *model);
   QAbstractItemModel *model() const;

   void setCompletionMode(CompletionMode mode);
   CompletionMode completionMode() const;
   void setFilterMode(Qt::MatchFlags filterMode);
   Qt::MatchFlags filterMode() const;

   QAbstractItemView *popup() const;
   void setPopup(QAbstractItemView *popup);

   void setCaseSensitivity(Qt::CaseSensitivity caseSensitivity);
   Qt::CaseSensitivity caseSensitivity() const;

   void setModelSorting(ModelSorting sorting);
   ModelSorting modelSorting() const;

   void setCompletionColumn(int column);
   int  completionColumn() const;

   void setCompletionRole(int role);
   int  completionRole() const;

   bool wrapAround() const;

   int maxVisibleItems() const;
   void setMaxVisibleItems(int maxItems);

   int completionCount() const;
   bool setCurrentRow(int row);
   int currentRow() const;

   QModelIndex currentIndex() const;
   QString currentCompletion() const;

   QAbstractItemModel *completionModel() const;

   QString completionPrefix() const;

   virtual QString pathFromIndex(const QModelIndex &index) const;
   virtual QStringList splitPath(const QString &path) const;

   GUI_CS_SLOT_1(Public, void setCompletionPrefix(const QString &prefix))
   GUI_CS_SLOT_2(setCompletionPrefix)

   GUI_CS_SLOT_1(Public, void complete(const QRect &rect = QRect()))
   GUI_CS_SLOT_2(complete)

   GUI_CS_SLOT_1(Public, void setWrapAround(bool wrap))
   GUI_CS_SLOT_2(setWrapAround)

   GUI_CS_SIGNAL_1(Public, void activated(const QString &text))
   GUI_CS_SIGNAL_OVERLOAD(activated, (const QString &), text)

   GUI_CS_SIGNAL_1(Public, void activated(const QModelIndex &index))
   GUI_CS_SIGNAL_OVERLOAD(activated, (const QModelIndex &), index)

   GUI_CS_SIGNAL_1(Public, void highlighted(const QString &text))
   GUI_CS_SIGNAL_OVERLOAD(highlighted, (const QString &), text)

   GUI_CS_SIGNAL_1(Public, void highlighted(const QModelIndex &index))
   GUI_CS_SIGNAL_OVERLOAD(highlighted, (const QModelIndex &), index)

 protected:
   bool eventFilter(QObject *object, QEvent *event) override;
   bool event(QEvent *event) override;
   QScopedPointer<QCompleterPrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QCompleter)

   GUI_CS_SLOT_1(Private, void _q_complete(const QModelIndex &index))
   GUI_CS_SLOT_2(_q_complete)

   GUI_CS_SLOT_1(Private, void _q_completionSelected(const QItemSelection &selection))
   GUI_CS_SLOT_2(_q_completionSelected)

   GUI_CS_SLOT_1(Private, void _q_autoResizePopup())
   GUI_CS_SLOT_2(_q_autoResizePopup)

   GUI_CS_SLOT_1(Private, void _q_fileSystemModelDirectoryLoaded(const QString &path))
   GUI_CS_SLOT_2(_q_fileSystemModelDirectoryLoaded)
};

#endif // QT_NO_COMPLETER

#endif // QCOMPLETER_H
