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

#ifndef QTEXTLIST_H
#define QTEXTLIST_H

#include <QtGui/qtextobject.h>
#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

class QTextListPrivate;
class QTextCursor;

class Q_GUI_EXPORT QTextList : public QTextBlockGroup
{
   GUI_CS_OBJECT(QTextList)

 public:
   explicit QTextList(QTextDocument *doc);
   ~QTextList();

   int count() const;

   inline bool isEmpty() const {
      return count() == 0;
   }

   QTextBlock item(int i) const;

   int itemNumber(const QTextBlock &) const;
   QString itemText(const QTextBlock &) const;

   void removeItem(int i);
   void remove(const QTextBlock &);

   void add(const QTextBlock &block);

   inline void setFormat(const QTextListFormat &format);
   QTextListFormat format() const {
      return QTextObject::format().toListFormat();
   }

 private:
   Q_DISABLE_COPY(QTextList)
   Q_DECLARE_PRIVATE(QTextList)
};

inline void QTextList::setFormat(const QTextListFormat &aformat)
{
   QTextObject::setFormat(aformat);
}

QT_END_NAMESPACE

#endif // QTEXTLIST_H
