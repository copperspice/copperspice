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

#ifndef QTEXTOBJECT_P_H
#define QTEXTOBJECT_P_H

#include <qtextobject.h>
#include <qtextdocument.h>



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
 public:
   QTextFramePrivate(QTextDocument *doc)
      : QTextObjectPrivate(doc), fragment_start(0), fragment_end(0), parentFrame(nullptr), layoutData(nullptr)
   {
   }

   virtual void fragmentAdded(QChar type, uint fragment);
   virtual void fragmentRemoved(QChar type, uint fragment);
   void remove_me();

   uint fragment_start;
   uint fragment_end;

   QTextFrame *parentFrame;
   QList<QTextFrame *> childFrames;
   QTextFrameLayoutData *layoutData;

 private:
   Q_DECLARE_PUBLIC(QTextFrame)
   friend class QTextDocumentPrivate;
};

#endif // QTEXTOBJECT_P_H
