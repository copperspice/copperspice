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

#ifndef QSHORTCUTMAP_P_H
#define QSHORTCUTMAP_P_H

#include <QtGui/qkeysequence.h>
#include <QtCore/qvector.h>
#include <QtCore/qscopedpointer.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_SHORTCUT

// To enable dump output uncomment below
//#define Dump_QShortcutMap

class QKeyEvent;
struct QShortcutEntry;
class QShortcutMapPrivate;
class QGraphicsWidget;
class QWidget;
class QAction;
class QObject;

class QShortcutMap
{
   Q_DECLARE_PRIVATE(QShortcutMap)

 public:
   QShortcutMap();
   ~QShortcutMap();

   int addShortcut(QObject *owner, const QKeySequence &key, Qt::ShortcutContext context);
   int removeShortcut(int id, QObject *owner, const QKeySequence &key = QKeySequence());
   int setShortcutEnabled(bool enable, int id, QObject *owner, const QKeySequence &key = QKeySequence());
   int setShortcutAutoRepeat(bool on, int id, QObject *owner, const QKeySequence &key = QKeySequence());

   void resetState();
   QKeySequence::SequenceMatch nextState(QKeyEvent *e);
   QKeySequence::SequenceMatch state();
   void dispatchEvent(QKeyEvent *e);
   bool tryShortcutEvent(QObject *o, QKeyEvent *e);

#ifdef Dump_QShortcutMap
   void dumpMap() const;
#endif

   bool hasShortcutForKeySequence(const QKeySequence &seq) const;

 private:
   bool correctWidgetContext(Qt::ShortcutContext context, QWidget *w, QWidget *active_window) const;

#ifndef QT_NO_GRAPHICSVIEW
   bool correctGraphicsWidgetContext(Qt::ShortcutContext context, QGraphicsWidget *w, QWidget *active_window) const;
#endif

#ifndef QT_NO_ACTION
   bool correctContext(Qt::ShortcutContext context, QAction *a, QWidget *active_window) const;
#endif

   QScopedPointer<QShortcutMapPrivate> d_ptr;

   QKeySequence::SequenceMatch find(QKeyEvent *e);
   QKeySequence::SequenceMatch matches(const QKeySequence &seq1, const QKeySequence &seq2) const;
   QVector<const QShortcutEntry *> matches() const;
   void createNewSequences(QKeyEvent *e, QVector<QKeySequence> &ksl);
   void clearSequence(QVector<QKeySequence> &ksl);
   bool correctContext(const QShortcutEntry &item) const;
   int translateModifiers(Qt::KeyboardModifiers modifiers);
};

#endif // QT_NO_SHORTCUT

QT_END_NAMESPACE

#endif // QSHORTCUTMAP_P_H
