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

#ifndef QCOMPLETER_P_H
#define QCOMPLETER_P_H

#ifndef QT_NO_COMPLETER

#include <qabstractproxymodel.h>
#include <qcompleter.h>
#include <qitemdelegate.h>
#include <qpainter.h>
#include <qtreeview.h>

#include <qabstractproxymodel_p.h>

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

   void init(QAbstractItemModel *model = nullptr);

   QPointer<QWidget> widget;
   QCompletionModel *proxy;
   QAbstractItemView *popup;
   QCompleter::CompletionMode mode;
   Qt::MatchFlags filterMode;

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

   void _q_complete(QModelIndex, bool = false);
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
   QIndexMapper()
      : v(false), m_from(0), m_to(-1)
   {
   }

   QIndexMapper(int f, int t)
      : v(false), m_from(f), m_to(t)
   {
   }

   QIndexMapper(const QVector<int> &newVector)
      : m_vector(newVector), v(true), m_from(-1), m_to(-1)
   {
   }

   int count() const {
      return v ? m_vector.count() : m_to - m_from + 1;
   }

   int operator[] (int index) const {
      return v ? m_vector[index] : m_from + index;
   }

   int indexOf(int x) const {
      return v ? m_vector.indexOf(x) : ((m_to < m_from) ? -1 : x - m_from);
   }

   bool isValid() const {
      return ! isEmpty();
   }

   bool isEmpty() const {
      return v ? m_vector.isEmpty() : (m_to < m_from);
   }

   void append(int x) {
      Q_ASSERT(v);
      m_vector.append(x);
   }

   int first() const {
      return v ? m_vector.first() : m_from;
   }

   int last() const {
      return v ? m_vector.last() : m_to;
   }

   int from() const {
      Q_ASSERT(! v);
      return m_from;
   }

   int to() const {
      Q_ASSERT(! v);
      return m_to;
   }

   int cost() const {
      return m_vector.count() + 2;
   }

 private:
   QVector<int> m_vector;

   bool v;
   int m_from;
   int m_to;
};

struct QMatchData {
   QMatchData()
      : exactMatchIndex(-1), partial(false)
   {
   }

   QMatchData(const QIndexMapper &indices, int em, bool p)
      : m_indices(indices), exactMatchIndex(em), partial(p)
   {
   }

   bool isValid() const {
      return m_indices.isValid();
   }

   QIndexMapper m_indices;

   int  exactMatchIndex;
   bool partial;
};

class QCompletionEngine
{
 public:
   typedef QMap<QString, QMatchData> CacheItem;
   typedef QMap<QModelIndex, CacheItem> Cache;

   QCompletionEngine(QCompleterPrivate *obj)
      : m_completerPrivate(obj), curRow(-1), cost(0)
   {
   }

   virtual ~QCompletionEngine()
   {
   }

   void filter(const QStringList &parts);

   QMatchData filterHistory();
   bool matchHint(QString, const QModelIndex &, QMatchData *);

   void saveInCache(QString, const QModelIndex &, const QMatchData &);
   bool lookupCache(QString part, const QModelIndex &parent, QMatchData *m);

   virtual void filterOnDemand(int) { }
   virtual QMatchData filter(const QString &, const QModelIndex &, int) = 0;

   int matchCount() const {
      return curMatch.m_indices.count() + historyMatch.m_indices.count();
   }

   QMatchData curMatch, historyMatch;
   QCompleterPrivate *m_completerPrivate;
   QStringList curParts;
   QModelIndex curParent;
   int curRow;

   Cache cache;
   int cost;
};

class QSortedModelEngine : public QCompletionEngine
{
 public:
   QSortedModelEngine(QCompleterPrivate *obj)
      : QCompletionEngine(obj)
   {
   }

   QMatchData filter(const QString &, const QModelIndex &, int) override;
   QIndexMapper indexHint(QString, const QModelIndex &, Qt::SortOrder);
   Qt::SortOrder sortOrder(const QModelIndex &) const;
};

class QUnsortedModelEngine : public QCompletionEngine
{
 public:
   QUnsortedModelEngine(QCompleterPrivate *obj)
      : QCompletionEngine(obj)
   {
   }

   void filterOnDemand(int) override;
   QMatchData filter(const QString &, const QModelIndex &, int) override;

 private:
   int buildIndices(const QString &str, const QModelIndex &parent, int n, const QIndexMapper &iv, QMatchData *m);
};

class QCompleterItemDelegate : public QItemDelegate
{
 public:
   QCompleterItemDelegate(QAbstractItemView *view)
      : QItemDelegate(view), m_view(view)
   {
   }

   void paint(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &idx) const  override {
      QStyleOptionViewItem optCopy = opt;
      optCopy.showDecorationSelected = true;

      if (m_view->currentIndex() == idx) {
         optCopy.state |= QStyle::State_HasFocus;
      }

      QItemDelegate::paint(p, optCopy, idx);
   }

 private:
   QAbstractItemView *m_view;
};

class QCompletionModel : public QAbstractProxyModel
{
   GUI_CS_OBJECT(QCompletionModel)
   Q_DECLARE_PRIVATE(QCompletionModel)

 public:
   QCompletionModel(QCompleterPrivate *obj, QObject *parent);

   void createEngine();
   void setFiltered(bool);
   void filter(const QStringList &parts);
   int completionCount() const;

   int currentRow() const {
      return engine->curRow;
   }

   bool setCurrentRow(int row);
   QModelIndex currentIndex(bool) const;

   QModelIndex index(int row, int column, const QModelIndex & = QModelIndex()) const override;

   int rowCount(const QModelIndex &index      = QModelIndex()) const override;
   int columnCount(const QModelIndex &index   = QModelIndex()) const override;
   bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;

   QModelIndex parent(const QModelIndex & = QModelIndex()) const override {
      return QModelIndex();
   }

   QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

   void setSourceModel(QAbstractItemModel *sourceModel) override;
   QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;
   QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;

   QCompleterPrivate *m_completerPrivate;
   QScopedPointer<QCompletionEngine> engine;

   bool showAll;
   bool m_completerShutdown;

   GUI_CS_SIGNAL_1(Public, void rowsAdded())
   GUI_CS_SIGNAL_2(rowsAdded)

   // slots
   void invalidate();
   void rowsInserted();
   void modelDestroyed();
};

class QCompletionModelPrivate : public QAbstractProxyModelPrivate
{
   Q_DECLARE_PUBLIC(QCompletionModel)
};

#endif // QT_NO_COMPLETER

#endif
