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

#ifndef QCOMPLETER_P_H
#define QCOMPLETER_P_H

#ifndef QT_NO_COMPLETER

#include <QtGui/qtreeview.h>
#include <QtGui/qabstractproxymodel.h>
#include <qcompleter.h>
#include <QtGui/qitemdelegate.h>
#include <QtGui/qpainter.h>
#include <qabstractproxymodel_p.h>

QT_BEGIN_NAMESPACE

class QCompletionModel;
class QCompletionModelPrivate;

class QCompleterPrivate
{
   Q_DECLARE_PUBLIC(QCompleter)

 public:
   QCompleterPrivate();
   virtual ~QCompleterPrivate() {
      delete popup;
   }

   void init(QAbstractItemModel *model = 0);

   QPointer<QWidget> widget;
   QCompletionModel *proxy;
   QAbstractItemView *popup;
   QCompleter::CompletionMode mode;

   QString prefix;
   Qt::CaseSensitivity cs;
   int role;
   int column;
   int maxVisibleItems;
   QCompleter::ModelSorting sorting;
   bool wrap;

   bool eatFocusOut;
   QRect popupRect;
   bool hiddenBecauseNoMatch;

   void showPopup(const QRect &);

   void _q_complete(const QModelIndex &, bool = false);
   void _q_completionSelected(const QItemSelection &);
   void _q_autoResizePopup();
   void _q_fileSystemModelDirectoryLoaded(const QString &path);

   void setCurrentIndex(QModelIndex, bool = true);

 protected:
   QCompleter *q_ptr;

};

class QIndexMapper
{
 public:
   QIndexMapper() : v(false), f(0), t(-1) { }
   QIndexMapper(int f, int t) : v(false), f(f), t(t) { }
   QIndexMapper(QVector<int> vec) : v(true), vector(vec), f(-1), t(-1) { }

   inline int count() const {
      return v ? vector.count() : t - f + 1;
   }
   inline int operator[] (int index) const {
      return v ? vector[index] : f + index;
   }
   inline int indexOf(int x) const {
      return v ? vector.indexOf(x) : ((t < f) ? -1 : x - f);
   }
   inline bool isValid() const {
      return !isEmpty();
   }
   inline bool isEmpty() const {
      return v ? vector.isEmpty() : (t < f);
   }
   inline void append(int x) {
      Q_ASSERT(v);
      vector.append(x);
   }
   inline int first() const {
      return v ? vector.first() : f;
   }
   inline int last() const {
      return v ? vector.last() : t;
   }
   inline int from() const {
      Q_ASSERT(!v);
      return f;
   }
   inline int to() const {
      Q_ASSERT(!v);
      return t;
   }
   inline int cost() const {
      return vector.count() + 2;
   }

 private:
   bool v;
   QVector<int> vector;
   int f, t;
};

struct QMatchData {
   QMatchData() : exactMatchIndex(-1) { }
   QMatchData(const QIndexMapper &indices, int em, bool p) :
      indices(indices), exactMatchIndex(em), partial(p) { }
   QIndexMapper indices;
   inline bool isValid() const {
      return indices.isValid();
   }
   int  exactMatchIndex;
   bool partial;
};

class QCompletionEngine
{
 public:
   typedef QMap<QString, QMatchData> CacheItem;
   typedef QMap<QModelIndex, CacheItem> Cache;

   QCompletionEngine(QCompleterPrivate *c) : c(c), curRow(-1), cost(0) { }
   virtual ~QCompletionEngine() { }

   void filter(const QStringList &parts);

   QMatchData filterHistory();
   bool matchHint(QString, const QModelIndex &, QMatchData *);

   void saveInCache(QString, const QModelIndex &, const QMatchData &);
   bool lookupCache(QString part, const QModelIndex &parent, QMatchData *m);

   virtual void filterOnDemand(int) { }
   virtual QMatchData filter(const QString &, const QModelIndex &, int) = 0;

   int matchCount() const {
      return curMatch.indices.count() + historyMatch.indices.count();
   }

   QMatchData curMatch, historyMatch;
   QCompleterPrivate *c;
   QStringList curParts;
   QModelIndex curParent;
   int curRow;

   Cache cache;
   int cost;
};

class QSortedModelEngine : public QCompletionEngine
{
 public:
   QSortedModelEngine(QCompleterPrivate *c) : QCompletionEngine(c) { }
   QMatchData filter(const QString &, const QModelIndex &, int)  override;
   QIndexMapper indexHint(QString, const QModelIndex &, Qt::SortOrder);
   Qt::SortOrder sortOrder(const QModelIndex &) const;
};

class QUnsortedModelEngine : public QCompletionEngine
{
 public:
   QUnsortedModelEngine(QCompleterPrivate *c) : QCompletionEngine(c) { }

   void filterOnDemand(int)  override;
   QMatchData filter(const QString &, const QModelIndex &, int)  override;

 private:
   int buildIndices(const QString &str, const QModelIndex &parent, int n, const QIndexMapper &iv, QMatchData *m);
};

class QCompleterItemDelegate : public QItemDelegate
{
 public:
   QCompleterItemDelegate(QAbstractItemView *view)
      : QItemDelegate(view), view(view) { }

   void paint(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &idx) const  override {
      QStyleOptionViewItem optCopy = opt;
      optCopy.showDecorationSelected = true;

      if (view->currentIndex() == idx) {
         optCopy.state |= QStyle::State_HasFocus;
      }

      QItemDelegate::paint(p, optCopy, idx);
   }

 private:
   QAbstractItemView *view;
};

class QCompletionModel : public QAbstractProxyModel
{
   GUI_CS_OBJECT(QCompletionModel)

 public:
   QCompletionModel(QCompleterPrivate *c, QObject *parent);

   void createEngine();
   void setFiltered(bool);
   void filter(const QStringList &parts);
   int completionCount() const;
   int currentRow() const {
      return engine->curRow;
   }
   bool setCurrentRow(int row);
   QModelIndex currentIndex(bool) const;
   void resetModel();

   QModelIndex index(int row, int column, const QModelIndex & = QModelIndex()) const override;
   int rowCount(const QModelIndex &index = QModelIndex()) const override;
   int columnCount(const QModelIndex &index = QModelIndex()) const override;
   bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;

   QModelIndex parent(const QModelIndex & = QModelIndex()) const override {
      return QModelIndex();
   }

   QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

   void setSourceModel(QAbstractItemModel *sourceModel) override;
   QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;
   QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;

   QCompleterPrivate *c;
   QScopedPointer<QCompletionEngine> engine;
   bool showAll;

   Q_DECLARE_PRIVATE(QCompletionModel)

   GUI_CS_SIGNAL_1(Public, void rowsAdded())
   GUI_CS_SIGNAL_2(rowsAdded)

   GUI_CS_SLOT_1(Public, void invalidate())
   GUI_CS_SLOT_2(invalidate)

   GUI_CS_SLOT_1(Public, void rowsInserted())
   GUI_CS_SLOT_2(rowsInserted)

   GUI_CS_SLOT_1(Public, void modelDestroyed())
   GUI_CS_SLOT_2(modelDestroyed)
};

class QCompletionModelPrivate : public QAbstractProxyModelPrivate
{
   Q_DECLARE_PUBLIC(QCompletionModel)
};

QT_END_NAMESPACE

#endif // QT_NO_COMPLETER

#endif // QCOMPLETER_P_H
