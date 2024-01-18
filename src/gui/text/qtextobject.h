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

#ifndef QTEXTOBJECT_H
#define QTEXTOBJECT_H

#include <qobject.h>
#include <qtextformat.h>
#include <qtextlayout.h>
#include <qglyphrun.h>
#include <qscopedpointer.h>

class QTextObjectPrivate;
class QTextDocument;
class QTextDocumentPrivate;
class QTextCursor;
class QTextBlock;
class QTextFragment;
class QTextList;
class QTextBlockGroupPrivate;
class QTextFramePrivate;

class Q_GUI_EXPORT QTextObject : public QObject
{
   GUI_CS_OBJECT(QTextObject)

 public:
   QTextObject(const QTextObject &) = delete;
   QTextObject &operator=(const QTextObject &) = delete;

   QTextDocument *document() const;
   QTextDocumentPrivate *docHandle() const;

   QTextFormat format() const;
   int formatIndex() const;

   int objectIndex() const;

 protected:
   explicit QTextObject(QTextDocument *document);
   ~QTextObject();

   void setFormat(const QTextFormat &format);

   QTextObject(QTextObjectPrivate &p, QTextDocument *document);
   QScopedPointer<QTextObjectPrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QTextObject)

   friend class QTextDocumentPrivate;
};

class Q_GUI_EXPORT QTextBlockGroup : public QTextObject
{
   GUI_CS_OBJECT(QTextBlockGroup)

 protected:
   explicit QTextBlockGroup(QTextDocument *document);

   QTextBlockGroup(const QTextBlockGroup &) = delete;
   QTextBlockGroup &operator=(const QTextBlockGroup &) = delete;

   ~QTextBlockGroup();

   virtual void blockInserted(const QTextBlock &block);
   virtual void blockRemoved(const QTextBlock &block);
   virtual void blockFormatChanged(const QTextBlock &block);

   QList<QTextBlock> blockList() const;

   QTextBlockGroup(QTextBlockGroupPrivate &p, QTextDocument *document);

 private:
   Q_DECLARE_PRIVATE(QTextBlockGroup)

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
   explicit QTextFrame(QTextDocument *document);

   QTextFrame(const QTextFrame &) = delete;
   QTextFrame &operator=(const QTextFrame &) = delete;

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
    public:
      iterator();
      iterator(const iterator &other);
      iterator &operator=(const iterator &other);

      QTextFrame *parentFrame() const {
         return f;
      }

      QTextFrame *currentFrame() const;
      QTextBlock currentBlock() const;

      bool atEnd() const {
         return !cf && cb == e;
      }

      bool operator==(const iterator &other) const {
         return f == other.f && cf == other.cf && cb == other.cb;
      }

      bool operator!=(const iterator &other) const {
         return f != other.f || cf != other.cf || cb != other.cb;
      }

      iterator &operator++();
      iterator operator++(int) {
         iterator tmp = *this;
         operator++();
         return tmp;
      }

      iterator &operator--();
      iterator operator--(int) {
         iterator tmp = *this;
         operator--();
         return tmp;
      }

    private:
      QTextFrame *f;
      int b;
      int e;
      QTextFrame *cf;
      int cb;

      friend class QTextFrame;
      friend class QTextTableCell;
      friend class QTextDocumentLayoutPrivate;
      iterator(QTextFrame *frame, int block, int begin, int end);
   };

   friend class iterator;

   iterator begin() const;
   iterator end() const;

 protected:
   QTextFrame(QTextFramePrivate &p, QTextDocument *document);

 private:
   Q_DECLARE_PRIVATE(QTextFrame)
   friend class QTextDocumentPrivate;
};

inline void QTextFrame::setFrameFormat(const QTextFrameFormat &format)
{
   QTextObject::setFormat(format);
}

class Q_GUI_EXPORT QTextBlockUserData
{
 public:
   virtual ~QTextBlockUserData();
};

class Q_GUI_EXPORT QTextBlock
{
 public:
   QTextBlock(QTextDocumentPrivate *priv, int b)
      : p(priv), n(b)
   {
   }

   QTextBlock()
      : p(nullptr), n(0)
   {
   }

   inline QTextBlock(const QTextBlock &other) = default;
   inline QTextBlock &operator=(const QTextBlock &other) = default;

   bool isValid() const;

   bool operator==(const QTextBlock &other) const {
      return p == other.p && n == other.n;
   }

   bool operator!=(const QTextBlock &other) const {
      return p != other.p || n != other.n;
   }

   bool operator<(const QTextBlock &other) const {
      return position() < other.position();
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
   QVector<QTextLayout::FormatRange> textFormats() const;

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
    public:
      iterator()
         : p(nullptr), b(0), e(0), n(0)
      {
      }

      iterator(const iterator &other) = default;

      QTextFragment fragment() const;

      bool atEnd() const {
         return n == e;
      }

      inline bool operator==(const iterator &other) const {
         return p == other.p && n == other.n;
      }

      inline bool operator!=(const iterator &other) const {
         return p != other.p || n != other.n;
      }

      iterator &operator++();

      iterator operator++(int) {
         iterator tmp = *this;
         operator++();
         return tmp;
      }

      iterator &operator--();

      iterator operator--(int) {
         iterator tmp = *this;
         operator--();
         return tmp;
      }

    private:
      friend class QTextBlock;

      const QTextDocumentPrivate *p;

      int b;
      int e;
      int n;

      iterator(const QTextDocumentPrivate *priv, int begin, int end, int f)
         : p(priv), b(begin), e(end), n(f)
      {
      }
   };

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

class Q_GUI_EXPORT QTextFragment
{
 public:
   QTextFragment(const QTextDocumentPrivate *priv, int f, int fe)
      : p(priv), n(f), ne(fe)
   {
   }

   QTextFragment()
      : p(nullptr), n(0), ne(0)
   {
   }

   QTextFragment(const QTextFragment &other)
      : p(other.p), n(other.n), ne(other.ne)
   {
   }

   QTextFragment &operator=(const QTextFragment &other) {
      p  = other.p;
      n  = other.n;
      ne = other.ne;
      return *this;
   }

   bool isValid() const {
      return p && n;
   }

   bool operator==(const QTextFragment &other) const {
      return p == other.p && n == other.n;
   }

   bool operator!=(const QTextFragment &other) const {
      return p != other.p || n != other.n;
   }

   bool operator<(const QTextFragment &other) const {
      return position() < other.position();
   }

   int position() const;
   int length() const;
   bool contains(int position) const;

   QTextCharFormat charFormat() const;
   int charFormatIndex() const;
   QString text() const;

   QList<QGlyphRun> glyphRuns(int from = -1, int length = -1) const;

 private:
   const QTextDocumentPrivate *p;
   int n;
   int ne;
};

#endif
