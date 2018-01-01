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

#ifndef QTEXTOBJECT_P_H
#define QTEXTOBJECT_P_H

#include <QtGui/qtextobject.h>
#include <QtGui/qtextdocument.h>

QT_BEGIN_NAMESPACE

class QTextDocumentPrivate;
class QTextFrameLayoutData;

class QTextObjectPrivate
{
   Q_DECLARE_PUBLIC(QTextObject)

 public:
   QTextObjectPrivate(QTextDocument *doc)
      : pieceTable(doc->d_func()), objectIndex(-1) {
   }

   virtual ~QTextObjectPrivate() {}

   QTextDocumentPrivate *pieceTable;
   int objectIndex;

 protected:
   QTextObject *q_ptr;

};

class QTextBlockGroupPrivate : public QTextObjectPrivate
{
   Q_DECLARE_PUBLIC(QTextBlockGroup)

 public:
   QTextBlockGroupPrivate(QTextDocument *doc)
      : QTextObjectPrivate(doc) {
   }
   typedef QList<QTextBlock> BlockList;
   BlockList blocks;
   void markBlocksDirty();
};

class QTextFramePrivate : public QTextObjectPrivate
{
   friend class QTextDocumentPrivate;
   Q_DECLARE_PUBLIC(QTextFrame)

 public:
   QTextFramePrivate(QTextDocument *doc)
      : QTextObjectPrivate(doc), fragment_start(0), fragment_end(0), parentFrame(0), layoutData(0) {
   }
   virtual void fragmentAdded(const QChar &type, uint fragment);
   virtual void fragmentRemoved(const QChar &type, uint fragment);
   void remove_me();

   uint fragment_start;
   uint fragment_end;

   QTextFrame *parentFrame;
   QList<QTextFrame *> childFrames;
   QTextFrameLayoutData *layoutData;
};

QT_END_NAMESPACE

#endif // QTEXTOBJECT_P_H
