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

#include <qcompleter_p.h>

#ifndef QT_NO_COMPLETER

#include <qapplication.h>
#include <qdesktopwidget.h>
#include <qdirmodel.h>
#include <qevent.h>
#include <qfilesystemmodel.h>
#include <qheaderview.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qscrollbar.h>
#include <qstringlistmodel.h>

QCompletionModel::QCompletionModel(QCompleterPrivate *obj, QObject *parent)
   : QAbstractProxyModel(*new QCompletionModelPrivate, parent), m_completerPrivate(obj),
     showAll(false), m_completerShutdown(false)
{
   createEngine();
}

int QCompletionModel::columnCount(const QModelIndex &) const
{
   Q_D(const QCompletionModel);
   return d->model->columnCount();
}

void QCompletionModel::setSourceModel(QAbstractItemModel *source)
{
   if (sourceModel() != nullptr) {
      QObject::disconnect(sourceModel(), QString(), this, QString());
   }

   QAbstractProxyModel::setSourceModel(source);

   if (source) {
      // TODO: Optimize updates in the source model
      connect(source, &QAbstractItemModel::modelReset,      this, &QCompletionModel::invalidate);
      connect(source, &QAbstractItemModel::destroyed,       this, &QCompletionModel::modelDestroyed);
      connect(source, &QAbstractItemModel::layoutChanged,   this, &QCompletionModel::invalidate);

      connect(source, &QAbstractItemModel::rowsInserted,    this, &QCompletionModel::rowsInserted);
      connect(source, &QAbstractItemModel::rowsRemoved,     this, &QCompletionModel::invalidate);
      connect(source, &QAbstractItemModel::columnsInserted, this, &QCompletionModel::invalidate);
      connect(source, &QAbstractItemModel::columnsRemoved,  this, &QCompletionModel::invalidate);
      connect(source, &QAbstractItemModel::dataChanged,     this, &QCompletionModel::invalidate);
   }

   invalidate();
}

void QCompletionModel::createEngine()
{
   bool sortedEngine = false;

   if (m_completerPrivate->filterMode == Qt::MatchStartsWith) {
      switch (m_completerPrivate->sorting) {
         case QCompleter::UnsortedModel:
            sortedEngine = false;
            break;

         case QCompleter::CaseSensitivelySortedModel:
            sortedEngine = m_completerPrivate->cs == Qt::CaseSensitive;
            break;

         case QCompleter::CaseInsensitivelySortedModel:
            sortedEngine = m_completerPrivate->cs == Qt::CaseInsensitive;
            break;
      }
   }

   if (sortedEngine) {
      engine.reset(new QSortedModelEngine(m_completerPrivate));
   } else {
      engine.reset(new QUnsortedModelEngine(m_completerPrivate));
   }
}

QModelIndex QCompletionModel::mapToSource(const QModelIndex &index) const
{
   Q_D(const QCompletionModel);

   if (! index.isValid()) {
      return engine->curParent;
   }

   int row;
   QModelIndex parent = engine->curParent;

   if (!showAll) {
      if (!engine->matchCount()) {
         return QModelIndex();
      }

      Q_ASSERT(index.row() < engine->matchCount());

      QIndexMapper &rootIndices = engine->historyMatch.m_indices;

      if (index.row() < rootIndices.count()) {
         row = rootIndices[index.row()];
         parent = QModelIndex();
      } else {
         row = engine->curMatch.m_indices[index.row() - rootIndices.count()];
      }

   } else {
      row = index.row();
   }

   return d->model->index(row, index.column(), parent);
}

QModelIndex QCompletionModel::mapFromSource(const QModelIndex &idx) const
{
   if (! idx.isValid()) {
      return QModelIndex();
   }

   int row = -1;

   if (! showAll) {
      if (!engine->matchCount()) {
         return QModelIndex();
      }

      QIndexMapper &rootIndices = engine->historyMatch.m_indices;

      if (idx.parent().isValid()) {
         if (idx.parent() != engine->curParent) {
            return QModelIndex();
         }

      } else {
         row = rootIndices.indexOf(idx.row());

         if (row == -1 && engine->curParent.isValid()) {
            return QModelIndex();   // source parent and our parent don't match
         }
      }

      if (row == -1) {
         QIndexMapper &indices = engine->curMatch.m_indices;
         engine->filterOnDemand(idx.row() - indices.last());
         row = indices.indexOf(idx.row()) + rootIndices.count();
      }

      if (row == -1) {
         return QModelIndex();
      }

   } else {
      if (idx.parent() != engine->curParent) {
         return QModelIndex();
      }

      row = idx.row();
   }

   return createIndex(row, idx.column());
}

bool QCompletionModel::setCurrentRow(int row)
{
   if (row < 0 || !engine->matchCount()) {
      return false;
   }

   if (row >= engine->matchCount()) {
      engine->filterOnDemand(row + 1 - engine->matchCount());
   }

   if (row >= engine->matchCount()) {
      // invalid row
      return false;
   }

   engine->curRow = row;
   return true;
}

QModelIndex QCompletionModel::currentIndex(bool sourceIndex) const
{
   if (! engine->matchCount()) {
      return QModelIndex();
   }

   int row = engine->curRow;

   if (showAll) {
      row = engine->curMatch.m_indices[engine->curRow];
   }

   QModelIndex idx = createIndex(row, m_completerPrivate->column);

   if (! sourceIndex) {
      return idx;
   }

   return mapToSource(idx);
}

QModelIndex QCompletionModel::index(int row, int column, const QModelIndex &parent) const
{
   Q_D(const QCompletionModel);

   if (row < 0 || column < 0 || column >= columnCount(parent) || parent.isValid()) {
      return QModelIndex();
   }

   if (! showAll) {
      if (!engine->matchCount()) {
         return QModelIndex();
      }

      if (row >= engine->historyMatch.m_indices.count()) {
         int want = row + 1 - engine->matchCount();

         if (want > 0) {
            engine->filterOnDemand(want);
         }

         if (row >= engine->matchCount()) {
            return QModelIndex();
         }
      }

   } else {
      if (row >= d->model->rowCount(engine->curParent)) {
         return QModelIndex();
      }
   }

   return createIndex(row, column);
}

int QCompletionModel::completionCount() const
{
   if (! engine->matchCount()) {
      return 0;
   }

   engine->filterOnDemand(INT_MAX);

   return engine->matchCount();
}

int QCompletionModel::rowCount(const QModelIndex &parent) const
{
   Q_D(const QCompletionModel);

   if (parent.isValid()) {
      return 0;
   }

   if (showAll) {
      // Show all items below current parent, even if we have no valid matches
      if (engine->curParts.count() != 1  && !engine->matchCount()
            && ! engine->curParent.isValid()) {
         return 0;
      }

      return d->model->rowCount(engine->curParent);
   }

   return completionCount();
}

void QCompletionModel::setFiltered(bool filtered)
{
   if (showAll == !filtered) {
      return;
   }

   beginResetModel();
   showAll = !filtered;
   endResetModel();
}

bool QCompletionModel::hasChildren(const QModelIndex &parent) const
{
   Q_D(const QCompletionModel);

   if (parent.isValid()) {
      return false;
   }

   if (showAll) {
      return d->model->hasChildren(mapToSource(parent));
   }

   if (! engine->matchCount()) {
      return false;
   }

   return true;
}

QVariant QCompletionModel::data(const QModelIndex &index, int role) const
{
   Q_D(const QCompletionModel);
   return d->model->data(mapToSource(index), role);
}

void QCompletionModel::modelDestroyed()
{
   QAbstractProxyModel::setSourceModel(nullptr);    // switch to static empty model

   if (! m_completerShutdown) {
      invalidate();
   }
}

void QCompletionModel::rowsInserted()
{
   invalidate();
   emit rowsAdded();
}

void QCompletionModel::invalidate()
{
   engine->cache.clear();
   filter(engine->curParts);
}

void QCompletionModel::filter(const QStringList &parts)
{
   Q_D(QCompletionModel);

   beginResetModel();
   engine->filter(parts);
   endResetModel();

   if (d->model->canFetchMore(engine->curParent)) {
      d->model->fetchMore(engine->curParent);
   }
}

void QCompletionEngine::filter(const QStringList &parts)
{
   const QAbstractItemModel *model = m_completerPrivate->proxy->sourceModel();
   curParts = parts;

   if (curParts.isEmpty()) {
      curParts.append(QString());
   }

   curRow       = -1;
   curParent    = QModelIndex();
   curMatch     = QMatchData();
   historyMatch = filterHistory();

   if (model == nullptr) {
      return;
   }

   QModelIndex parent;

   for (int i = 0; i < curParts.count() - 1; i++) {
      QString part = curParts[i];
      int emi = filter(part, parent, -1).exactMatchIndex;

      if (emi == -1) {
         return;
      }

      parent = model->index(emi, m_completerPrivate->column, parent);
   }

   // Note that we set the curParent to a valid parent, even if we have no matches
   // When filtering is disabled, we show all the items under this parent
   curParent = parent;

   if (curParts.last().isEmpty()) {
      curMatch = QMatchData(QIndexMapper(0, model->rowCount(curParent) - 1), -1, false);

   } else {
      curMatch = filter(curParts.last(), curParent, 1);   // build at least one

   }

   curRow = curMatch.isValid() ? 0 : -1;
}

QMatchData QCompletionEngine::filterHistory()
{
   QAbstractItemModel *source = m_completerPrivate->proxy->sourceModel();

   if (curParts.count() <= 1 || m_completerPrivate->proxy->showAll || ! source) {
      return QMatchData();
   }

   bool isDirModel = false;
   bool isFsModel  = false;

#ifndef QT_NO_DIRMODEL
   isDirModel = (qobject_cast<QDirModel *>(source) != nullptr);
#endif

#ifndef QT_NO_FILESYSTEMMODEL
   isFsModel = (qobject_cast<QFileSystemModel *>(source) != nullptr);

#endif

   QVector<int> v;
   QIndexMapper im(v);
   QMatchData match(im, -1, true);

   for (int i = 0; i < source->rowCount(); i++) {
      QString str = source->index(i, m_completerPrivate->column).data().toString();

#if defined(Q_OS_WIN)
      (void) isDirModel;
      (void) isFsModel;

      if (str.startsWith(m_completerPrivate->prefix, m_completerPrivate->cs)) {

#else

      if (str.startsWith(m_completerPrivate->prefix, m_completerPrivate->cs) &&
            ((! isFsModel && ! isDirModel) || (QDir::toNativeSeparators(str) != QString(QDir::separator())))) {
#endif

         match.m_indices.append(i);
      }
   }

   return match;
}

// Returns a match hint from the cache by chopping the search string
bool QCompletionEngine::matchHint(QString part, const QModelIndex &parent, QMatchData *hint)
{
   if (m_completerPrivate->cs == Qt::CaseInsensitive) {
      part = part.toLower();
   }

   const CacheItem &map = cache[parent];

   QString key = part;

   while (! key.isEmpty()) {
      key.chop(1);

      if (map.contains(key)) {
         *hint = map[key];
         return true;
      }
   }

   return false;
}

bool QCompletionEngine::lookupCache(QString part, const QModelIndex &parent, QMatchData *match)
{
   if (m_completerPrivate->cs == Qt::CaseInsensitive) {
      part = part.toLower();
   }

   const CacheItem &map = cache[parent];

   if (! map.contains(part)) {
      return false;
   }

   *match = map[part];

   return true;
}

// When the cache size exceeds 1MB, it clears out about 1/2 of the cache.
void QCompletionEngine::saveInCache(QString part, const QModelIndex &parent, const QMatchData &match)
{
   if (m_completerPrivate->filterMode == Qt::MatchEndsWith) {
      return;
   }

   QMatchData old = cache[parent].take(part);
   cost = cost + match.m_indices.cost() - old.m_indices.cost();

   if (cost * sizeof(int) > 1024 * 1024) {
      QMap<QModelIndex, CacheItem>::iterator it1 = cache.begin();

      while (it1 != cache.end()) {
         CacheItem &ci = it1.value();
         int sz = ci.count() / 2;

         QMap<QString, QMatchData>::iterator it2 = ci.begin();
         int i = 0;

         while (it2 != ci.end() && i < sz) {
            cost -= it2.value().m_indices.cost();
            it2 = ci.erase(it2);
            i++;
         }

         if (ci.count() == 0) {
            it1 = cache.erase(it1);
         } else {
            ++it1;
         }
      }
   }

   if (m_completerPrivate->cs == Qt::CaseInsensitive) {
      part = part.toLower();
   }

   cache[parent][part] = match;
}

QIndexMapper QSortedModelEngine::indexHint(QString part, const QModelIndex &parent, Qt::SortOrder order)
{
   const QAbstractItemModel *model = m_completerPrivate->proxy->sourceModel();

   if (m_completerPrivate->cs == Qt::CaseInsensitive) {
      part = part.toLower();
   }

   const CacheItem &map = cache[parent];

   // Try to find a lower and upper bound for the search from previous results
   int to = model->rowCount(parent) - 1;
   int from = 0;
   const CacheItem::const_iterator it = map.lowerBound(part);

   // look backward for first valid hint
   for (CacheItem::const_iterator it1 = it; it1-- != map.constBegin();) {
      const QMatchData &value = it1.value();

      if (value.isValid()) {
         if (order == Qt::AscendingOrder) {
            from = value.m_indices.last() + 1;
         } else {
            to = value.m_indices.first() - 1;
         }

         break;
      }
   }

   // look forward for first valid hint
   for (CacheItem::const_iterator it2 = it; it2 != map.constEnd(); ++it2) {
      const QMatchData &value = it2.value();

      if (value.isValid() && !it2.key().startsWith(part)) {
         if (order == Qt::AscendingOrder) {
            to = value.m_indices.first() - 1;
         } else {
            from = value.m_indices.first() + 1;
         }

         break;
      }
   }

   return QIndexMapper(from, to);
}

Qt::SortOrder QSortedModelEngine::sortOrder(const QModelIndex &parent) const
{
   const QAbstractItemModel *model = m_completerPrivate->proxy->sourceModel();

   int rowCount = model->rowCount(parent);

   if (rowCount < 2) {
      return Qt::AscendingOrder;
   }

   QString first = model->data(model->index(0, m_completerPrivate->column, parent), m_completerPrivate->role).toString();
   QString last  = model->data(model->index(rowCount - 1, m_completerPrivate->column, parent),
         m_completerPrivate->role).toString();

   return QString::compare(first, last, m_completerPrivate->cs) <= 0 ? Qt::AscendingOrder : Qt::DescendingOrder;
}

QMatchData QSortedModelEngine::filter(const QString &part, const QModelIndex &parent, int)
{
   const QAbstractItemModel *model = m_completerPrivate->proxy->sourceModel();

   QMatchData hint;

   if (lookupCache(part, parent, &hint)) {
      return hint;
   }

   QIndexMapper indices;
   Qt::SortOrder order = sortOrder(parent);

   if (matchHint(part, parent, &hint)) {
      if (! hint.isValid()) {
         return QMatchData();
      }

      indices = hint.m_indices;

   } else {
      indices = indexHint(part, parent, order);
   }

   // binary search the model within 'indices' for 'part' under 'parent'
   int high = indices.to() + 1;
   int low  = indices.from() - 1;
   int probe;

   QModelIndex probeIndex;
   QString probeData;

   while (high - low > 1) {
      probe = (high + low) / 2;
      probeIndex = model->index(probe, m_completerPrivate->column, parent);
      probeData  = model->data(probeIndex, m_completerPrivate->role).toString();
      const int cmp = QString::compare(probeData, part, m_completerPrivate->cs);

      if ((order == Qt::AscendingOrder && cmp >= 0) || (order == Qt::DescendingOrder && cmp < 0)) {
         high = probe;
      } else {
         low = probe;
      }
   }

   if ((order == Qt::AscendingOrder && low == indices.to()) || (order == Qt::DescendingOrder && high == indices.from())) {
      // not found
      saveInCache(part, parent, QMatchData());
      return QMatchData();
   }

   probeIndex = model->index(order == Qt::AscendingOrder ? low + 1 : high - 1, m_completerPrivate->column, parent);
   probeData  = model->data(probeIndex, m_completerPrivate->role).toString();

   if (!probeData.startsWith(part, m_completerPrivate->cs)) {
      saveInCache(part, parent, QMatchData());
      return QMatchData();
   }

   const bool exactMatch = QString::compare(probeData, part, m_completerPrivate->cs) == 0;
   int emi = exactMatch ? (order == Qt::AscendingOrder ? low + 1 : high - 1) : -1;

   int from = 0;
   int to   = 0;

   if (order == Qt::AscendingOrder) {
      from = low + 1;
      high = indices.to() + 1;
      low  = from;

   } else {
      to   = high - 1;
      low  = indices.from() - 1;
      high = to;
   }

   while (high - low > 1) {
      probe = (high + low) / 2;
      probeIndex = model->index(probe, m_completerPrivate->column, parent);
      probeData  = model->data(probeIndex, m_completerPrivate->role).toString();
      const bool startsWith = probeData.startsWith(part, m_completerPrivate->cs);

      if ((order == Qt::AscendingOrder && startsWith) || (order == Qt::DescendingOrder && ! startsWith)) {
         low = probe;
      } else {
         high = probe;
      }
   }

   QMatchData m(order == Qt::AscendingOrder ? QIndexMapper(from, high - 1) : QIndexMapper(low + 1, to), emi, false);
   saveInCache(part, parent, m);

   return m;
}

int QUnsortedModelEngine::buildIndices(const QString &str, const QModelIndex &parent, int n,
      const QIndexMapper &indices, QMatchData *m)
{
   Q_ASSERT(m->partial);
   Q_ASSERT(n != -1 || m->exactMatchIndex == -1);

   const QAbstractItemModel *model = m_completerPrivate->proxy->sourceModel();
   int i     = 0;
   int count = 0;

   for (i = 0; i < indices.count() && count != n; ++i) {
      QModelIndex idx = model->index(indices[i], m_completerPrivate->column, parent);

      if (! (model->flags(idx) & Qt::ItemIsSelectable)) {
         continue;
      }

      QString data = model->data(idx, m_completerPrivate->role).toString();

      switch (m_completerPrivate->filterMode) {
         case Qt::MatchStartsWith:
            if (! data.startsWith(str, m_completerPrivate->cs)) {
               continue;
            }

            break;

         case Qt::MatchContains:
            if (!data.contains(str, m_completerPrivate->cs)) {
               continue;
            }

            break;

         case Qt::MatchEndsWith:
            if (!data.endsWith(str, m_completerPrivate->cs)) {
               continue;
            }

            break;

         case Qt::MatchExactly:
         case Qt::MatchFixedString:
         case Qt::MatchCaseSensitive:
         case Qt::MatchRegExp:
         case Qt::MatchWildcard:
         case Qt::MatchWrap:
         case Qt::MatchRecursive:
            // error, may want to throw
            break;
      }

      m->m_indices.append(indices[i]);
      ++count;

      if (m->exactMatchIndex == -1 && QString::compare(data, str, m_completerPrivate->cs) == 0) {
         m->exactMatchIndex = indices[i];

         if (n == -1) {
            return indices[i];
         }
      }
   }

   return indices[i - 1];
}

void QUnsortedModelEngine::filterOnDemand(int n)
{
   Q_ASSERT(matchCount());

   if (! curMatch.partial) {
      return;
   }

   Q_ASSERT(n >= -1);

   const QAbstractItemModel *model = m_completerPrivate->proxy->sourceModel();
   int lastRow = model->rowCount(curParent) - 1;
   QIndexMapper im(curMatch.m_indices.last() + 1, lastRow);

   int lastIndex = buildIndices(curParts.last(), curParent, n, im, &curMatch);
   curMatch.partial = (lastRow != lastIndex);
   saveInCache(curParts.last(), curParent, curMatch);
}

QMatchData QUnsortedModelEngine::filter(const QString &part, const QModelIndex &parent, int n)
{
   QMatchData hint;

   QVector<int> v;
   QIndexMapper im(v);
   QMatchData match(im, -1, true);

   const QAbstractItemModel *model = m_completerPrivate->proxy->sourceModel();
   bool foundInCache = lookupCache(part, parent, &match);

   if (!foundInCache) {
      if (matchHint(part, parent, &hint) && ! hint.isValid()) {
         return QMatchData();
      }
   }

   if (!foundInCache && !hint.isValid()) {
      const int lastRow = model->rowCount(parent) - 1;
      QIndexMapper all(0, lastRow);
      int lastIndex = buildIndices(part, parent, n, all, &match);
      match.partial = (lastIndex != lastRow);

   } else {
      if (! foundInCache) {
         // build from hint as much as we can
         buildIndices(part, parent, INT_MAX, hint.m_indices, &match);
         match.partial = hint.partial;
      }

      if (match.partial && ((n == -1 && match.exactMatchIndex == -1) || (match.m_indices.count() < n))) {
         // need more and have more
         const int lastRow = model->rowCount(parent) - 1;

         QIndexMapper rest(hint.m_indices.last() + 1, lastRow);
         int want = n == -1 ? -1 : n - match.m_indices.count();
         int lastIndex = buildIndices(part, parent, want, rest, &match);
         match.partial = (lastRow != lastIndex);
      }
   }

   saveInCache(part, parent, match);

   return match;
}

QCompleterPrivate::QCompleterPrivate()
   : widget(nullptr), proxy(nullptr), popup(nullptr), filterMode(Qt::MatchStartsWith), cs(Qt::CaseSensitive),
     role(Qt::EditRole), column(0), maxVisibleItems(7), sorting(QCompleter::UnsortedModel),
     wrap(true), eatFocusOut(true), hiddenBecauseNoMatch(false)
{
}

void QCompleterPrivate::init(QAbstractItemModel *obj)
{
   Q_Q(QCompleter);

   proxy = new QCompletionModel(this, q);

   QObject::connect(proxy, &QCompletionModel::rowsAdded, q, &QCompleter::_q_autoResizePopup);
   q->setModel(obj);

#ifdef QT_NO_LISTVIEW
   q->setCompletionMode(QCompleter::InlineCompletion);
#else
   q->setCompletionMode(QCompleter::PopupCompletion);
#endif

}

void QCompleterPrivate::setCurrentIndex(QModelIndex index, bool select)
{
   Q_Q(QCompleter);

   if (! q->popup()) {
      return;
   }

   if (! select) {
      popup->selectionModel()->setCurrentIndex(index, QItemSelectionModel::NoUpdate);

   } else {
      if (! index.isValid()) {
         popup->selectionModel()->clear();
      } else {
         popup->selectionModel()->setCurrentIndex(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
      }
   }

   index = popup->selectionModel()->currentIndex();

   if (! index.isValid()) {
      popup->scrollToTop();
   } else {
      popup->scrollTo(index, QAbstractItemView::PositionAtTop);
   }
}

void QCompleterPrivate::_q_completionSelected(const QItemSelection &selection)
{
   QModelIndex index;

   if (!selection.indexes().isEmpty()) {
      index = selection.indexes().first();
   }

   _q_complete(index, true);
}

void QCompleterPrivate::_q_complete(QModelIndex index, bool highlighted)
{
   Q_Q(QCompleter);
   QString completion;

   if (!index.isValid() || (!proxy->showAll && (index.row() >= proxy->engine->matchCount()))) {
      completion = prefix;
      index = QModelIndex();

   } else {
      if (!(index.flags() & Qt::ItemIsEnabled)) {
         return;
      }

      QModelIndex si = proxy->mapToSource(index);
      si = si.sibling(si.row(), column); // for clicked()
      completion = q->pathFromIndex(si);

#ifndef QT_NO_DIRMODEL
      // add a trailing separator in inline
      if (mode == QCompleter::InlineCompletion) {
         if (qobject_cast<QDirModel *>(proxy->sourceModel()) && QFileInfo(completion).isDir()) {
            completion += QDir::separator();
         }
      }
#endif

#ifndef QT_NO_FILESYSTEMMODEL
      // add a trailing separator in inline
      if (mode == QCompleter::InlineCompletion) {
         if (qobject_cast<QFileSystemModel *>(proxy->sourceModel()) && QFileInfo(completion).isDir()) {
            completion += QDir::separator();
         }
      }
#endif

   }

   if (highlighted) {
      emit q->highlighted(index);
      emit q->highlighted(completion);
   } else {
      emit q->activated(index);
      emit q->activated(completion);
   }
}

void QCompleterPrivate::_q_autoResizePopup()
{
   if (! popup || !popup->isVisible()) {
      return;
   }

   showPopup(popupRect);
}

void QCompleterPrivate::showPopup(const QRect &rect)
{
   const QRect screen = QApplication::desktop()->availableGeometry(widget);
   Qt::LayoutDirection dir = widget->layoutDirection();
   QPoint pos;

   int rh;
   int w;

   int h = (popup->sizeHintForRow(0) * qMin(maxVisibleItems, popup->model()->rowCount()) + 3) + 3;
   QScrollBar *hsb = popup->horizontalScrollBar();

   if (hsb && hsb->isVisible()) {
      h += popup->horizontalScrollBar()->sizeHint().height();
   }

   if (rect.isValid()) {
      rh = rect.height();
      w = rect.width();
      pos = widget->mapToGlobal(dir == Qt::RightToLeft ? rect.bottomRight() : rect.bottomLeft());

   } else {
      rh = widget->height();
      pos = widget->mapToGlobal(QPoint(0, widget->height() - 2));
      w = widget->width();
   }

   if (w > screen.width()) {
      w = screen.width();
   }

   if ((pos.x() + w) > (screen.x() + screen.width())) {
      pos.setX(screen.x() + screen.width() - w);
   }

   if (pos.x() < screen.x()) {
      pos.setX(screen.x());
   }

   int top    = pos.y() - rh - screen.top() + 2;
   int bottom = screen.bottom() - pos.y();
   h = qMax(h, popup->minimumHeight());

   if (h > bottom) {
      h = qMin(qMax(top, bottom), h);

      if (top > bottom) {
         pos.setY(pos.y() - h - rh + 2);
      }
   }

   popup->setGeometry(pos.x(), pos.y(), w, h);

   if (! popup->isVisible()) {
      popup->show();
   }
}

void QCompleterPrivate::_q_fileSystemModelDirectoryLoaded(const QString &path)
{
   Q_Q(QCompleter);

   // Slot called when QFileSystemModel has finished loading.
   // If we hide the popup because there was no match because the model was not loaded yet,
   // we re-start the completion when we get the results

   if (hiddenBecauseNoMatch && prefix.startsWith(path) && prefix != (path + '/') && widget) {
      q->complete();
   }
}

QCompleter::QCompleter(QObject *parent)
   : QObject(parent), d_ptr(new QCompleterPrivate)
{
   d_ptr->q_ptr = this;
   Q_D(QCompleter);

   d->init();
}

QCompleter::QCompleter(QAbstractItemModel *model, QObject *parent)
   : QObject(parent), d_ptr(new QCompleterPrivate)
{
   d_ptr->q_ptr = this;
   Q_D(QCompleter);

   d->init(model);
}

#ifndef QT_NO_STRINGLISTMODEL

QCompleter::QCompleter(const QStringList &list, QObject *parent)
   : QObject(parent), d_ptr(new QCompleterPrivate)
{
   d_ptr->q_ptr = this;
   Q_D(QCompleter);

   d->init(new QStringListModel(list, this));
}

#endif

QCompleter::~QCompleter()
{
   Q_D(QCompleter);

   // warn QCompletionModel
   d->proxy->m_completerShutdown = true;
}

void QCompleter::setWidget(QWidget *widget)
{
   Q_D(QCompleter);

   if (d->widget) {
      d->widget->removeEventFilter(this);
   }

   d->widget = widget;

   if (d->widget) {
      d->widget->installEventFilter(this);
   }

   if (d->popup) {
      d->popup->hide();
      d->popup->setFocusProxy(d->widget);
   }
}

QWidget *QCompleter::widget() const
{
   Q_D(const QCompleter);
   return d->widget;
}

void QCompleter::setModel(QAbstractItemModel *model)
{
   Q_D(QCompleter);

   QAbstractItemModel *oldModel = d->proxy->sourceModel();

#ifndef QT_NO_FILESYSTEMMODEL
   if (dynamic_cast<const QFileSystemModel *>(oldModel)) {
      setCompletionRole(Qt::EditRole);   // clear FileNameRole set by QFileSystemModel
   }
#endif

   d->proxy->setSourceModel(model);

   if (d->popup) {
      setPopup(d->popup);   // set the model and make new connections
   }

   if (oldModel && oldModel->QObject::parent() == this) {
      delete oldModel;
   }

#ifndef QT_NO_DIRMODEL

   if (dynamic_cast<QDirModel *>(model)) {
#if defined(Q_OS_WIN)
      setCaseSensitivity(Qt::CaseInsensitive);
#else
      setCaseSensitivity(Qt::CaseSensitive);
#endif
   }

#endif

#ifndef QT_NO_FILESYSTEMMODEL
   QFileSystemModel *fsModel = qobject_cast<QFileSystemModel *>(model);

   if (fsModel) {

#if defined(Q_OS_WIN)
      setCaseSensitivity(Qt::CaseInsensitive);
#else
      setCaseSensitivity(Qt::CaseSensitive);
#endif

      setCompletionRole(QFileSystemModel::FileNameRole);
      connect(fsModel, &QFileSystemModel::directoryLoaded, this, &QCompleter::_q_fileSystemModelDirectoryLoaded);
   }

#endif
}

QAbstractItemModel *QCompleter::model() const
{
   Q_D(const QCompleter);
   return d->proxy->sourceModel();
}

void QCompleter::setCompletionMode(QCompleter::CompletionMode mode)
{
   Q_D(QCompleter);

   d->mode = mode;
   d->proxy->setFiltered(mode != QCompleter::UnfilteredPopupCompletion);

   if (mode == QCompleter::InlineCompletion) {
      if (d->widget) {
         d->widget->removeEventFilter(this);
      }

      if (d->popup) {
         d->popup->deleteLater();
         d->popup = nullptr;
      }

   } else {
      if (d->widget) {
         d->widget->installEventFilter(this);
      }
   }
}

QCompleter::CompletionMode QCompleter::completionMode() const
{
   Q_D(const QCompleter);
   return d->mode;
}

void QCompleter::setFilterMode(Qt::MatchFlags filterMode)
{
   Q_D(QCompleter);

   if (d->filterMode == filterMode) {
      return;
   }

   if (filterMode != Qt::MatchStartsWith && filterMode != Qt::MatchContains && filterMode != Qt::MatchEndsWith) {
      qWarning("QCompleter::setFilterMode() QCompleter::filterMode flag is not supported");
      return;
   }

   d->filterMode = filterMode;
   d->proxy->createEngine();
   d->proxy->invalidate();
}

Qt::MatchFlags QCompleter::filterMode() const
{
   Q_D(const QCompleter);
   return d->filterMode;
}

void QCompleter::setPopup(QAbstractItemView *popup)
{
   Q_D(QCompleter);

   Q_ASSERT(popup != nullptr);

   if (d->popup) {
      QObject::disconnect(d->popup->selectionModel(), QString(), this, QString());
      QObject::disconnect(d->popup, QString(), this, QString());
   }

   if (d->popup != popup) {
      delete d->popup;
   }

   if (popup->model() != d->proxy) {
      popup->setModel(d->proxy);
   }

   popup->hide();

   Qt::FocusPolicy origPolicy = Qt::NoFocus;

   if (d->widget) {
      origPolicy = d->widget->focusPolicy();
   }

   popup->setParent(nullptr, Qt::Popup);
   popup->setFocusPolicy(Qt::NoFocus);

   if (d->widget) {
      d->widget->setFocusPolicy(origPolicy);
   }

   popup->setFocusProxy(d->widget);
   popup->installEventFilter(this);
   popup->setItemDelegate(new QCompleterItemDelegate(popup));

#ifndef QT_NO_LISTVIEW

   if (QListView *listView = qobject_cast<QListView *>(popup)) {
      listView->setModelColumn(d->column);
   }

#endif

   QObject::connect(popup, &QAbstractItemView::clicked, this, &QCompleter::_q_complete);
   QObject::connect(this, cs_mp_cast<const QModelIndex &>(&QCompleter::activated), popup, &QAbstractItemView::hide);

   QObject::connect(popup->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QCompleter::_q_completionSelected);

   d->popup = popup;
}

QAbstractItemView *QCompleter::popup() const
{
   Q_D(const QCompleter);

#ifndef QT_NO_LISTVIEW

   if (! d->popup && completionMode() != QCompleter::InlineCompletion) {
      QListView *listView = new QListView;
      listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
      listView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      listView->setSelectionBehavior(QAbstractItemView::SelectRows);
      listView->setSelectionMode(QAbstractItemView::SingleSelection);
      listView->setModelColumn(d->column);

      QCompleter *that = const_cast<QCompleter *>(this);
      that->setPopup(listView);
   }

#endif

   return d->popup;
}

bool QCompleter::event(QEvent *ev)
{
   return QObject::event(ev);
}

bool QCompleter::eventFilter(QObject *o, QEvent *e)
{
   Q_D(QCompleter);

   if (d->eatFocusOut && o == d->widget && e->type() == QEvent::FocusOut) {
      d->hiddenBecauseNoMatch = false;

      if (d->popup && d->popup->isVisible()) {
         return true;
      }
   }

   if (o != d->popup) {
      return QObject::eventFilter(o, e);
   }

   switch (e->type()) {
      case QEvent::KeyPress: {
         QKeyEvent *ke = static_cast<QKeyEvent *>(e);

         QModelIndex curIndex = d->popup->currentIndex();
         QModelIndexList selList = d->popup->selectionModel()->selectedIndexes();

         const int key = ke->key();

         // In UnFilteredPopup mode, select the current item
         if ((key == Qt::Key_Up || key == Qt::Key_Down) && selList.isEmpty() && curIndex.isValid()
               && d->mode == QCompleter::UnfilteredPopupCompletion) {
            d->setCurrentIndex(curIndex);
            return true;
         }

         // Handle popup navigation keys. These are hardcoded because up/down might make the
         // widget do something else (lineedit cursor moves to home/end on mac, for instance)
         switch (key) {
            case Qt::Key_End:
            case Qt::Key_Home:
               if (ke->modifiers() & Qt::ControlModifier) {
                  return false;
               }

               break;

            case Qt::Key_Up:
               if (! curIndex.isValid()) {
                  int rowCount = d->proxy->rowCount();
                  QModelIndex lastIndex = d->proxy->index(rowCount - 1, d->column);
                  d->setCurrentIndex(lastIndex);
                  return true;

               } else if (curIndex.row() == 0) {
                  if (d->wrap) {
                     d->setCurrentIndex(QModelIndex());
                  }

                  return true;
               }

               return false;

            case Qt::Key_Down:
               if (! curIndex.isValid()) {
                  QModelIndex firstIndex = d->proxy->index(0, d->column);
                  d->setCurrentIndex(firstIndex);
                  return true;

               } else if (curIndex.row() == d->proxy->rowCount() - 1) {
                  if (d->wrap) {
                     d->setCurrentIndex(QModelIndex());
                  }

                  return true;
               }

               return false;

            case Qt::Key_PageUp:
            case Qt::Key_PageDown:
               return false;
         }

         // Send the event to the widget. If the widget accepted the event, do nothing
         // If the widget did not accept the event, provide a default implementation
         d->eatFocusOut = false;
         (static_cast<QObject *>(d->widget))->event(ke);
         d->eatFocusOut = true;

         if (! d->widget || e->isAccepted() || ! d->popup->isVisible()) {
            // widget lost focus, hide the popup

#ifdef QT_KEYPAD_NAVIGATION
            if (d->widget && (!d->widget->hasFocus()
                  || (QApplication::keypadNavigationEnabled() && ! d->widget->hasEditFocus()) )) {
#else

            if (d->widget && (!d->widget->hasFocus())) {
#endif

               d->popup->hide();
            }

            if (e->isAccepted()) {
               return true;
            }
         }

         // default implementation for keys not handled by the widget when popup is open
         if (ke->matches(QKeySequence::Cancel)) {
            d->popup->hide();
            return true;
         }

         // default implementation for keys not handled by the widget when popup is open
         switch (key) {

#ifdef QT_KEYPAD_NAVIGATION

            case Qt::Key_Select:
               if (!QApplication::keypadNavigationEnabled()) {
                  break;
               }

#endif

            case Qt::Key_Return:
            case Qt::Key_Enter:
            case Qt::Key_Tab:
               d->popup->hide();

               if (curIndex.isValid()) {
                  d->_q_complete(curIndex);
               }

               break;

            case Qt::Key_F4:
               if (ke->modifiers() & Qt::AltModifier) {
                  d->popup->hide();
               }

               break;

            case Qt::Key_Backtab:
               d->popup->hide();
               break;

            default:
               break;
         }

         return true;
      }

#ifdef QT_KEYPAD_NAVIGATION

      case QEvent::KeyRelease: {
         QKeyEvent *ke = static_cast<QKeyEvent *>(e);

         if (QApplication::keypadNavigationEnabled() && ke->key() == Qt::Key_Back) {
            // Send the event to the 'widget'. This is what we did for KeyPress, so we need
            // to do the same for KeyRelease, in case the widget's KeyPress event set
            // up something (such as a timer) that is relying on also receiving the
            // key release. I see this as a bug in Qt, and should really set it up for all
            // the affected keys. However, it is difficult to tell how this will affect
            // existing code, and I can't test for every combination!
            d->eatFocusOut = false;
            static_cast<QObject *>(d->widget)->event(ke);
            d->eatFocusOut = true;
         }

         break;
      }

#endif

      case QEvent::MouseButtonPress: {
#ifdef QT_KEYPAD_NAVIGATION

         if (QApplication::keypadNavigationEnabled()) {
            // if we have clicked in the widget (or its descendant), let it handle the click
            QWidget *source = qobject_cast<QWidget *>(o);

            if (source) {
               QPoint pos = source->mapToGlobal((static_cast<QMouseEvent *>(e))->pos());
               QWidget *target = QApplication::widgetAt(pos);

               if (target && (d->widget->isAncestorOf(target) || target == d->widget)) {
                  d->eatFocusOut = false;
                  static_cast<QObject *>(target)->event(e);
                  d->eatFocusOut = true;
                  return true;
               }
            }
         }

#endif

         if (! d->popup->underMouse()) {
            d->popup->hide();
            return true;
         }
      }

      return false;

      case QEvent::InputMethod:
      case QEvent::ShortcutOverride:
         QApplication::sendEvent(d->widget, e);
         break;

      default:
         return false;
   }

   return false;
}

void QCompleter::complete(const QRect &rect)
{
   Q_D(QCompleter);

   QModelIndex idx = d->proxy->currentIndex(false);
   d->hiddenBecauseNoMatch = false;

   if (d->mode == QCompleter::InlineCompletion) {
      if (idx.isValid()) {
         d->_q_complete(idx, true);
      }

      return;
   }

   Q_ASSERT(d->widget != nullptr);

   if ((d->mode == QCompleter::PopupCompletion && !idx.isValid()) ||
         (d->mode == QCompleter::UnfilteredPopupCompletion && d->proxy->rowCount() == 0)) {

      if (d->popup) {
         d->popup->hide();   // no suggestion, hide
      }

      d->hiddenBecauseNoMatch = true;
      return;
   }

   popup();

   if (d->mode == QCompleter::UnfilteredPopupCompletion) {
      d->setCurrentIndex(idx, false);
   }

   d->showPopup(rect);
   d->popupRect = rect;
}

bool QCompleter::setCurrentRow(int row)
{
   Q_D(QCompleter);
   return d->proxy->setCurrentRow(row);
}

int QCompleter::currentRow() const
{
   Q_D(const QCompleter);
   return d->proxy->currentRow();
}

int QCompleter::completionCount() const
{
   Q_D(const QCompleter);
   return d->proxy->completionCount();
}

void QCompleter::setModelSorting(QCompleter::ModelSorting sorting)
{
   Q_D(QCompleter);

   if (d->sorting == sorting) {
      return;
   }

   d->sorting = sorting;
   d->proxy->createEngine();
   d->proxy->invalidate();
}

QCompleter::ModelSorting QCompleter::modelSorting() const
{
   Q_D(const QCompleter);
   return d->sorting;
}

void QCompleter::setCompletionColumn(int column)
{
   Q_D(QCompleter);

   if (d->column == column) {
      return;
   }

#ifndef QT_NO_LISTVIEW
   if (QListView *listView = qobject_cast<QListView *>(d->popup)) {
      listView->setModelColumn(column);
   }
#endif

   d->column = column;
   d->proxy->invalidate();
}

int QCompleter::completionColumn() const
{
   Q_D(const QCompleter);
   return d->column;
}

void QCompleter::setCompletionRole(int role)
{
   Q_D(QCompleter);

   if (d->role == role) {
      return;
   }

   d->role = role;
   d->proxy->invalidate();
}

int QCompleter::completionRole() const
{
   Q_D(const QCompleter);
   return d->role;
}

void QCompleter::setWrapAround(bool wrap)
{
   Q_D(QCompleter);

   if (d->wrap == wrap) {
      return;
   }

   d->wrap = wrap;
}

bool QCompleter::wrapAround() const
{
   Q_D(const QCompleter);
   return d->wrap;
}

int QCompleter::maxVisibleItems() const
{
   Q_D(const QCompleter);
   return d->maxVisibleItems;
}

void QCompleter::setMaxVisibleItems(int maxItems)
{
   Q_D(QCompleter);

   if (maxItems < 0) {
      qWarning("QCompleter::setMaxVisibleItems() Invalid max visible items (%d) must be >= 0", maxItems);
      return;
   }

   d->maxVisibleItems = maxItems;
}

void QCompleter::setCaseSensitivity(Qt::CaseSensitivity cs)
{
   Q_D(QCompleter);

   if (d->cs == cs) {
      return;
   }

   d->cs = cs;
   d->proxy->createEngine();
   d->proxy->invalidate();
}

Qt::CaseSensitivity QCompleter::caseSensitivity() const
{
   Q_D(const QCompleter);
   return d->cs;
}

void QCompleter::setCompletionPrefix(const QString &prefix)
{
   Q_D(QCompleter);
   d->prefix = prefix;
   d->proxy->filter(splitPath(prefix));
}

QString QCompleter::completionPrefix() const
{
   Q_D(const QCompleter);
   return d->prefix;
}

QModelIndex QCompleter::currentIndex() const
{
   Q_D(const QCompleter);
   return d->proxy->currentIndex(false);
}

QString QCompleter::currentCompletion() const
{
   Q_D(const QCompleter);
   return pathFromIndex(d->proxy->currentIndex(true));
}

QAbstractItemModel *QCompleter::completionModel() const
{
   Q_D(const QCompleter);
   return d->proxy;
}

QString QCompleter::pathFromIndex(const QModelIndex &index) const
{
   Q_D(const QCompleter);

   if (!index.isValid()) {
      return QString();
   }

   QAbstractItemModel *sourceModel = d->proxy->sourceModel();

   if (! sourceModel) {
      return QString();
   }

   bool isDirModel = false;
   bool isFsModel = false;

#ifndef QT_NO_DIRMODEL
   isDirModel = qobject_cast<QDirModel *>(d->proxy->sourceModel()) != nullptr;
#endif

#ifndef QT_NO_FILESYSTEMMODEL
   isFsModel = qobject_cast<QFileSystemModel *>(d->proxy->sourceModel()) != nullptr;
#endif

   if (! isDirModel && !isFsModel) {
      return sourceModel->data(index, d->role).toString();
   }

   QModelIndex idx = index;
   QStringList list;

   do {
      QString t;

      if (isDirModel) {
         t = sourceModel->data(idx, Qt::EditRole).toString();
      }

#ifndef QT_NO_FILESYSTEMMODEL
      else {
         t = sourceModel->data(idx, QFileSystemModel::FileNameRole).toString();
      }

#endif

      list.prepend(t);
      QModelIndex parent = idx.parent();
      idx = parent.sibling(parent.row(), index.column());
   } while (idx.isValid());

#if ! defined(Q_OS_WIN)

   if (list.count() == 1) {
      // only the separator or some other text
      return list[0];
   }

   list[0].clear() ; // the join below will provide the separator
#endif

   return list.join(QDir::separator());
}

QStringList QCompleter::splitPath(const QString &path) const
{
   bool isDirModel = false;
   bool isFsModel  = false;

#ifndef QT_NO_DIRMODEL
   Q_D(const QCompleter);
   isDirModel = qobject_cast<QDirModel *>(d->proxy->sourceModel()) != nullptr;
#endif

#ifndef QT_NO_FILESYSTEMMODEL

#ifdef QT_NO_DIRMODEL
   Q_D(const QCompleter);
#endif

   isFsModel = qobject_cast<QFileSystemModel *>(d->proxy->sourceModel()) != nullptr;
#endif

   if ((!isDirModel && !isFsModel) || path.isEmpty()) {
      return QStringList(completionPrefix());
   }

   const QString sep = QDir::separator();
   QString pathCopy  = QDir::toNativeSeparators(path);

#if defined(Q_OS_WIN)

   if (pathCopy == "\\" || pathCopy == "\\\\") {
      return QStringList(pathCopy);
   }

   QString doubleSlash("\\\\");

   if (pathCopy.startsWith(doubleSlash)) {
      pathCopy = pathCopy.mid(2);
   } else {
      doubleSlash.clear();
   }

#endif

   QRegularExpression re('[' + QRegularExpression::escape(sep) + ']');
   QStringList parts = pathCopy.split(re);

#if defined(Q_OS_WIN)

   if (! doubleSlash.isEmpty()) {
      parts[0].prepend(doubleSlash);
   }

#else

   if (pathCopy[0] == sep[0]) {
      // readd the "/" at the beginning as the split removed it
      parts[0] = QDir::fromNativeSeparators(QString(sep[0]));
   }

#endif

   return parts;
}

void QCompleter::_q_complete(const QModelIndex &index)
{
   Q_D(QCompleter);
   d->_q_complete(index);
}

void QCompleter::_q_completionSelected(const QItemSelection &selection)
{
   Q_D(QCompleter);
   d->_q_completionSelected(selection);
}

void QCompleter::_q_autoResizePopup()
{
   Q_D(QCompleter);
   d->_q_autoResizePopup();
}

void QCompleter::_q_fileSystemModelDirectoryLoaded(const QString &path)
{
   Q_D(QCompleter);
   d->_q_fileSystemModelDirectoryLoaded(path);
}

#endif // QT_NO_COMPLETER
