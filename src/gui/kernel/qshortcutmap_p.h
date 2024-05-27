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

#ifndef QSHORTCUTMAP_P_H
#define QSHORTCUTMAP_P_H

#include <qkeysequence.h>
#include <qvector.h>
#include <qscopedpointer.h>

#ifndef QT_NO_SHORTCUT

class QKeyEvent;
class QShortcutMapPrivate;
class QObject;

struct QShortcutEntry;

class Q_GUI_EXPORT QShortcutMap
{
   Q_DECLARE_PRIVATE(QShortcutMap)

 public:
   QShortcutMap();
   ~QShortcutMap();

   typedef bool (*ContextMatcher)(QObject *object, Qt::ShortcutContext context);

   int addShortcut(QObject *owner, const QKeySequence &key, Qt::ShortcutContext context, ContextMatcher matcher);
   int removeShortcut(int id, QObject *owner, const QKeySequence &key = QKeySequence());
   int setShortcutEnabled(bool enable, int id, QObject *owner, const QKeySequence &key = QKeySequence());
   int setShortcutAutoRepeat(bool on, int id, QObject *owner, const QKeySequence &key = QKeySequence());

   QKeySequence::SequenceMatch state();

   bool tryShortcut(QKeyEvent *event);
   bool hasShortcutForKeySequence(const QKeySequence &seq) const;

#if defined(CS_SHOW_DEBUG_GUI)
   void dumpMap() const;
#endif

 private:
   void resetState();
   QKeySequence::SequenceMatch nextState(QKeyEvent *e);
   void dispatchEvent(QKeyEvent *e);

   QKeySequence::SequenceMatch find(QKeyEvent *e, int ignoredModifiers = 0);
   QKeySequence::SequenceMatch matches(const QKeySequence &seq1, const QKeySequence &seq2) const;
   QVector<const QShortcutEntry *> matches() const;
   void createNewSequences(QKeyEvent *e, QVector<QKeySequence> &ksl, int ignoredModifiers);
   void clearSequence(QVector<QKeySequence> &ksl);

   int translateModifiers(Qt::KeyboardModifiers modifiers);
   QScopedPointer<QShortcutMapPrivate> d_ptr;
};

#endif // QT_NO_SHORTCUT



#endif
