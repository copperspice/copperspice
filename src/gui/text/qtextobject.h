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

#ifndef QTEXTOBJECT_H
#define QTEXTOBJECT_H

#include <qobject.h>
#include <qtextformat.h>
#include <qglyphrun.h>
#include <QScopedPointer>

class QTextObjectPrivate;
class QTextDocument;
class QTextDocumentPrivate;
class QTextCursor;
class QTextBlock;
class QTextFragment;
class QTextLayout;
class QTextList;
class QTextBlockGroupPrivate;
class QTextFramePrivate;

class Q_GUI_EXPORT QTextObject : public QObject
{
   GUI_CS_OBJECT(QTextObject)

 public:
   QTextFormat format() const;
   int formatIndex() const;

   QTextDocument *document() const;

   int objectIndex() const;

   QTextDocumentPrivate *docHandle() const;

 protected:
  explicit QTextObject(QTextDocument *doc);
   ~QTextObject();

   void setFormat(const QTextFormat &format);

   QTextObject(QTextObjectPrivate &p, QTextDocument *doc);
   QScopedPointer<QTextObjectPrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QTextObject)
   Q_DISABLE_COPY(QTextObject)

   friend class QTextDocumentPrivate;
};


class Q_GUI_EXPORT QTextBlockGroup : public QTextObject
{
   GUI_CS_OBJECT(QTextBlockGroup)

 protected:
   explicit QTextBlockGroup(QTextDocument *doc);
   ~QTextBlockGroup();

   virtual void blockInserted(const QTextBlock &block);
   virtual void blockRemoved(const QTextBlock &block);
   virtual void blockFormatChanged(const QTextBlock &block);

   QList<QTextBlock> blockList() const;

   QTextBlockGroup(QTextBlockGroupPrivate &p, QTextDocument *doc);

 private:
   Q_DECLARE_PRIVATE(QTextBlockGroup)
   Q_DISABLE_COPY(QTextBlockGroup)
   friend class QTextDocumentPrivate;
};

class Q_GUI_EXPORT QTextFrameLayoutData
{

 public:
   virtual ~QTextFrameLayoutData();
};

class Q_GUI_EXPORT QTextFrame : public QTextObject
{
   GUI_CS_OBJECT(QTextFrame)

 public:
   explicit QTextFrame(QTextDocument *doc);
   ~QTextFrame();

   inline void setFrameFormat(const QTextFrameFormat &format);
   QTextFrameFormat frameFormat() const {
      return QTextObject::format().toFrameFormat();
   }

   QTextCursor firstCursorPosition() const;
   QTextCursor lastCursorPosition() const;
   int firstPosition() const;
   int lastPosition() const;

   QTextFrameLayoutData *layoutData() const;
   void setLayoutData(QTextFrameLayoutData *data);

   QList<QTextFrame *> childFrames() const;
   QTextFrame *parentFrame() const;

   class Q_GUI_EXPORT iterator
   {
      QTextFrame *f;
      int b;
      int e;
      QTextFrame *cf;
      int cb;

      friend class QTextFrame;
      friend class QTextTableCell;
      friend class QTextDocumentLayoutPrivate;
      iterator(QTextFrame *frame, int block, int begin, int end);

    public:
      iterator();
      iterator(const iterator &o);
      iterator &operator=(const iterator &o);

      QTextFrame *parentFrame() const {
         return f;
      }

      QTextFrame *currentFrame() const;
      QTextBlock currentBlock() const;

      bool atEnd() const {
         return !cf && cb == e;
      }

      inline bool operator==(const iterator &o) const {
         return f == o.f && cf == o.cf && cb == o.cb;
      }
      inline bool operator!=(const iterator &o) const {
         return f != o.f || cf != o.cf || cb != o.cb;
      }
      iterator &operator++();
      inline iterator operator++(int) {
         iterator tmp = *this;
         operator++();
         return tmp;
      }
      iterator &operator--();
      inline iterator operator--(int) {
         iterator tmp = *this;
         operator--();
         return tmp;
      }
   };

   friend class iterator;

   typedef iterator Iterator;

   iterator begin() const;
   iterator end() const;

 protected:
   QTextFrame(QTextFramePrivate &p, QTextDocument *doc);

 private:
   friend class QTextDocumentPrivate;
   Q_DECLARE_PRIVATE(QTextFrame)
   Q_DISABLE_COPY(QTextFrame)
};
Q_DECLARE_TYPEINFO(QTextFrame::iterator, Q_MOVABLE_TYPE);

inline void QTextFrame::setFrameFormat(const QTextFrameFormat &aformat)
{
   QTextObject::setFormat(aformat);
}

class Q_GUI_EXPORT QTextBlockUserData
{
 public:
   virtual ~QTextBlockUserData();
};

class Q_GUI_EXPORT QTextBlock
{
 public:
   inline QTextBlock(QTextDocumentPrivate *priv, int b)
      : p(priv), n(b)  {}

   inline QTextBlock()
      : p(0), n(0)  {}

   inline QTextBlock(const QTextBlock &o) = default;
   inline QTextBlock &operator=(const QTextBlock &o) = default;

   inline bool isValid() const {
      return p != 0 && n != 0;
   }

   inline bool operator==(const QTextBlock &o) const {
      return p == o.p && n == o.n;
   }

   inline bool operator!=(const QTextBlock &o) const {
      return p != o.p || n != o.n;
   }

   inline bool operator<(const QTextBlock &o) const {
      return position() < o.position();
   }

   int position() const;
   int length() const;
   bool contains(int position) const;

   QTextLayout *layout() const;
   void clearLayout();
   QTextBlockFormat blockFormat() const;
   int blockFormatIndex() const;
   QTextCharFormat charFormat() const;
   int charFormatIndex() const;

   Qt::LayoutDirection textDirection() const;

   QString text() const;

   const QTextDocument *document() const;

   QTextList *textList() const;

   QTextBlockUserData *userData() const;
   void setUserData(QTextBlockUserData *data);

   int userState() const;
   void setUserState(int state);

   int revision() const;
   void setRevision(int rev);

   bool isVisible() const;
   void setVisible(bool visible);

   int blockNumber() const;
   int firstLineNumber() const;

   void setLineCount(int count);
   int lineCount() const;

   class Q_GUI_EXPORT iterator
   {
      friend class QTextBlock;

      const QTextDocumentPrivate *p;

      int b;
      int e;
      int n;

      iterator(const QTextDocumentPrivate *priv, int begin, int end, int f)
         : p(priv), b(begin), e(end), n(f) {}

    public:
      iterator()
         : p(0), b(0), e(0), n(0) {}

      iterator(const iterator &o) = default;

      QTextFragment fragment() const;

      bool atEnd() const {
         return n == e;
      }

      inline bool operator==(const iterator &o) const {
         return p == o.p && n == o.n;
      }

      inline bool operator!=(const iterator &o) const {
         return p != o.p || n != o.n;
      }

      iterator &operator++();
      inline iterator operator++(int) {
         iterator tmp = *this;
         operator++();
         return tmp;
      }

      iterator &operator--();
      inline iterator operator--(int) {
         iterator tmp = *this;
         operator--();
         return tmp;
      }
   };

   typedef iterator Iterator;

   iterator begin() const;
   iterator end() const;

   QTextBlock next() const;
   QTextBlock previous() const;

   inline QTextDocumentPrivate *docHandle() const {
      return p;
   }

   inline int fragmentIndex() const {
      return n;
   }

 private:
   QTextDocumentPrivate *p;
   int n;

   friend class QTextDocumentPrivate;
   friend class QTextLayout;
   friend class QSyntaxHighlighter;
};

Q_DECLARE_TYPEINFO(QTextBlock, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(QTextBlock::iterator, Q_MOVABLE_TYPE);


class Q_GUI_EXPORT QTextFragment
{
 public:
   inline QTextFragment(const QTextDocumentPrivate *priv, int f, int fe) : p(priv), n(f), ne(fe) {}
   inline QTextFragment() : p(0), n(0), ne(0) {}
   inline QTextFragment(const QTextFragment &o) : p(o.p), n(o.n), ne(o.ne) {}
   inline QTextFragment &operator=(const QTextFragment &o) {
      p = o.p;
      n = o.n;
      ne = o.ne;
      return *this;
   }

   inline bool isValid() const {
      return p && n;
   }

   inline bool operator==(const QTextFragment &o) const {
      return p == o.p && n == o.n;
   }
   inline bool operator!=(const QTextFragment &o) const {
      return p != o.p || n != o.n;
   }
   inline bool operator<(const QTextFragment &o) const {
      return position() < o.position();
   }

   int position() const;
   int length() const;
   bool contains(int position) const;

   QTextCharFormat charFormat() const;
   int charFormatIndex() const;
   QString text() const;

#if ! defined(QT_NO_RAWFONT)
   QList<QGlyphRun> glyphRuns() const;
#endif

 private:
   const QTextDocumentPrivate *p;
   int n;
   int ne;
};

Q_DECLARE_TYPEINFO(QTextFragment, Q_MOVABLE_TYPE);

#endif
