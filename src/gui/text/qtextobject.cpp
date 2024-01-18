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

#include <qtextobject.h>

#include <qdebug.h>
#include <qtextcursor.h>
#include <qtextdocument.h>
#include <qtextlist.h>
#include <qabstracttextdocumentlayout.h>

#include <qtextobject_p.h>
#include <qtextcursor_p.h>
#include <qtextformat_p.h>
#include <qtextdocument_p.h>
#include <qtextengine_p.h>

#include <algorithm>

QTextObject::QTextObject(QTextDocument *doc)
   : QObject(doc), d_ptr(new QTextObjectPrivate(doc))
{
   d_ptr->q_ptr = this;
}

QTextObject::QTextObject(QTextObjectPrivate &dd, QTextDocument *doc)
   : QObject(doc), d_ptr(&dd)
{
   d_ptr->q_ptr = this;
}

QTextObject::~QTextObject()
{
}

QTextFormat QTextObject::format() const
{
   Q_D(const QTextObject);
   return d->pieceTable->formatCollection()->objectFormat(d->objectIndex);
}

int QTextObject::formatIndex() const
{
   Q_D(const QTextObject);
   return d->pieceTable->formatCollection()->objectFormatIndex(d->objectIndex);
}

void QTextObject::setFormat(const QTextFormat &format)
{
   Q_D(QTextObject);
   int idx = d->pieceTable->formatCollection()->indexForFormat(format);
   d->pieceTable->changeObjectFormat(this, idx);
}

int QTextObject::objectIndex() const
{
   Q_D(const QTextObject);
   return d->objectIndex;
}

QTextDocument *QTextObject::document() const
{
   return static_cast<QTextDocument *>(parent());
}

QTextDocumentPrivate *QTextObject::docHandle() const
{
   return static_cast<const QTextDocument *>(parent())->docHandle();
}

void QTextBlockGroupPrivate::markBlocksDirty()
{
   for (int i = 0; i < blocks.count(); ++i) {
      const QTextBlock &block = blocks.at(i);
      pieceTable->documentChange(block.position(), block.length());
   }
}

QTextBlockGroup::QTextBlockGroup(QTextDocument *doc)
   : QTextObject(*new QTextBlockGroupPrivate(doc), doc)
{
}

QTextBlockGroup::QTextBlockGroup(QTextBlockGroupPrivate &p, QTextDocument *doc)
   : QTextObject(p, doc)
{
}

QTextBlockGroup::~QTextBlockGroup()
{
}

// Should this be insertBlock()?
void QTextBlockGroup::blockInserted(const QTextBlock &block)
{
   Q_D(QTextBlockGroup);
   QTextBlockGroupPrivate::BlockList::iterator it = std::lower_bound(d->blocks.begin(), d->blocks.end(), block);
   d->blocks.insert(it, block);
   d->markBlocksDirty();
}

void QTextBlockGroup::blockRemoved(const QTextBlock &block)
{
   Q_D(QTextBlockGroup);
   d->blocks.removeAll(block);
   d->markBlocksDirty();
   if (d->blocks.isEmpty()) {
      document()->docHandle()->deleteObject(this);
      return;
   }
}

void QTextBlockGroup::blockFormatChanged(const QTextBlock &)
{
}

QList<QTextBlock> QTextBlockGroup::blockList() const
{
   Q_D(const QTextBlockGroup);
   return d->blocks;
}

QTextFrameLayoutData::~QTextFrameLayoutData()
{
}

QTextFrame::QTextFrame(QTextDocument *doc)
   : QTextObject(*new QTextFramePrivate(doc), doc)
{
}

QTextFrame::~QTextFrame()
{
   Q_D(QTextFrame);
   delete d->layoutData;
}

QTextFrame::QTextFrame(QTextFramePrivate &p, QTextDocument *doc)
   : QTextObject(p, doc)
{
}

QList<QTextFrame *> QTextFrame::childFrames() const
{
   Q_D(const QTextFrame);
   return d->childFrames;
}

QTextFrame *QTextFrame::parentFrame() const
{
   Q_D(const QTextFrame);
   return d->parentFrame;
}

QTextCursor QTextFrame::firstCursorPosition() const
{
   Q_D(const QTextFrame);
   return QTextCursorPrivate::fromPosition(d->pieceTable, firstPosition());
}

QTextCursor QTextFrame::lastCursorPosition() const
{
   Q_D(const QTextFrame);
   return QTextCursorPrivate::fromPosition(d->pieceTable, lastPosition());
}

int QTextFrame::firstPosition() const
{
   Q_D(const QTextFrame);
   if (!d->fragment_start) {
      return 0;
   }

   return d->pieceTable->fragmentMap().position(d->fragment_start) + 1;
}

int QTextFrame::lastPosition() const
{
   Q_D(const QTextFrame);
   if (!d->fragment_end) {
      return d->pieceTable->length() - 1;
   }
   return d->pieceTable->fragmentMap().position(d->fragment_end);
}

QTextFrameLayoutData *QTextFrame::layoutData() const
{
   Q_D(const QTextFrame);
   return d->layoutData;
}

void QTextFrame::setLayoutData(QTextFrameLayoutData *data)
{
   Q_D(QTextFrame);
   delete d->layoutData;
   d->layoutData = data;
}

void QTextFramePrivate::fragmentAdded(QChar type, uint fragment)
{
   if (type == QTextBeginningOfFrame) {
      Q_ASSERT(!fragment_start);
      fragment_start = fragment;

   } else if (type == QTextEndOfFrame) {
      Q_ASSERT(!fragment_end);
      fragment_end = fragment;

   } else if (type == QChar::ObjectReplacementCharacter) {
      Q_ASSERT(!fragment_start);
      Q_ASSERT(!fragment_end);
      fragment_start = fragment;
      fragment_end = fragment;

   } else {
      Q_ASSERT(false);
   }
}

void QTextFramePrivate::fragmentRemoved(QChar type, uint fragment)
{
   (void) fragment;

   if (type == QTextBeginningOfFrame) {
      Q_ASSERT(fragment_start == fragment);
      fragment_start = 0;

   } else if (type == QTextEndOfFrame) {
      Q_ASSERT(fragment_end == fragment);
      fragment_end = 0;

   } else if (type == QChar::ObjectReplacementCharacter) {
      Q_ASSERT(fragment_start == fragment);
      Q_ASSERT(fragment_end == fragment);
      fragment_start = 0;
      fragment_end = 0;

   } else {
      Q_ASSERT(false);
   }

   remove_me();
}

void QTextFramePrivate::remove_me()
{
   Q_Q(QTextFrame);

   if (fragment_start == 0 && fragment_end == 0
      && !parentFrame) {
      q->document()->docHandle()->deleteObject(q);
      return;
   }

   if (!parentFrame) {
      return;
   }

   int index = parentFrame->d_func()->childFrames.indexOf(q);

   // iterator over all children and move them to the parent
   for (int i = 0; i < childFrames.size(); ++i) {
      QTextFrame *c = childFrames.at(i);
      parentFrame->d_func()->childFrames.insert(index, c);
      c->d_func()->parentFrame = parentFrame;
      ++index;
   }

   Q_ASSERT(parentFrame->d_func()->childFrames.at(index) == q);
   parentFrame->d_func()->childFrames.removeAt(index);

   childFrames.clear();
   parentFrame = nullptr;
}

QTextFrame::iterator QTextFrame::begin() const
{
   const QTextDocumentPrivate *priv = docHandle();
   int b = priv->blockMap().findNode(firstPosition());
   int e = priv->blockMap().findNode(lastPosition() + 1);
   return iterator(const_cast<QTextFrame *>(this), b, b, e);
}

QTextFrame::iterator QTextFrame::end() const
{
   const QTextDocumentPrivate *priv = docHandle();
   int b = priv->blockMap().findNode(firstPosition());
   int e = priv->blockMap().findNode(lastPosition() + 1);
   return iterator(const_cast<QTextFrame *>(this), e, b, e);
}

QTextFrame::iterator::iterator()
{
   f  = nullptr;
   b  = 0;
   e  = 0;
   cf = nullptr;
   cb = 0;
}

QTextFrame::iterator::iterator(QTextFrame *frame, int block, int begin, int end)
{
   f  = frame;
   b  = begin;
   e  = end;
   cf = nullptr;
   cb = block;
}

QTextFrame::iterator::iterator(const iterator &other)
{
   f  = other.f;
   b  = other.b;
   e  = other.e;
   cf = other.cf;
   cb = other.cb;
}

QTextFrame::iterator &QTextFrame::iterator::operator=(const iterator &other)
{
   f = other.f;
   b = other.b;
   e = other.e;
   cf = other.cf;
   cb = other.cb;
   return *this;
}

QTextFrame *QTextFrame::iterator::currentFrame() const
{
   return cf;
}

QTextBlock QTextFrame::iterator::currentBlock() const
{
   if (!f) {
      return QTextBlock();
   }
   return QTextBlock(f->docHandle(), cb);
}

QTextFrame::iterator &QTextFrame::iterator::operator++()
{
   const QTextDocumentPrivate *priv = f->docHandle();
   const QTextDocumentPrivate::BlockMap &map = priv->blockMap();

   if (cf) {
      int end = cf->lastPosition() + 1;
      cb = map.findNode(end);
      cf = nullptr;

   } else if (cb) {
      cb = map.next(cb);
      if (cb == e) {
         return *this;
      }

      if (!f->d_func()->childFrames.isEmpty()) {
         int pos = map.position(cb);

         // check if we entered a frame
         QTextDocumentPrivate::FragmentIterator frag = priv->find(pos - 1);

         if (priv->buffer().at(frag->stringPosition) != QChar::ParagraphSeparator) {
            QTextFrame *nf = qobject_cast<QTextFrame *>(priv->objectForFormat(frag->format));
            if (nf) {
               if (priv->buffer().at(frag->stringPosition) == QTextBeginningOfFrame && nf != f) {
                  cf = nf;
                  cb = 0;
               } else {
                  Q_ASSERT(priv->buffer().at(frag->stringPosition) != QTextEndOfFrame);
               }
            }
         }
      }
   }
   return *this;
}

QTextFrame::iterator &QTextFrame::iterator::operator--()
{
   const QTextDocumentPrivate *priv = f->docHandle();
   const QTextDocumentPrivate::BlockMap &map = priv->blockMap();

   if (cf) {
      int start = cf->firstPosition() - 1;
      cb = map.findNode(start);
      cf = nullptr;

   } else {
      if (cb == b) {
         goto end;
      }

      if (cb != e) {
         int pos = map.position(cb);
         // check if we have to enter a frame
         QTextDocumentPrivate::FragmentIterator frag = priv->find(pos - 1);
         if (priv->buffer().at(frag->stringPosition) != QChar::ParagraphSeparator) {
            QTextFrame *pf = qobject_cast<QTextFrame *>(priv->objectForFormat(frag->format));
            if (pf) {
               if (priv->buffer().at(frag->stringPosition) == QTextBeginningOfFrame) {
                  Q_ASSERT(pf == f);
               } else if (priv->buffer().at(frag->stringPosition) == QTextEndOfFrame) {
                  Q_ASSERT(pf != f);
                  cf = pf;
                  cb = 0;
                  goto end;
               }
            }
         }
      }
      cb = map.previous(cb);
   }
end:
   return *this;
}

QTextBlockUserData::~QTextBlockUserData()
{
}

bool QTextBlock::isValid() const
{
   return p != nullptr && p->blockMap().isValid(n);
}

int QTextBlock::position() const
{
   if (! p || !n) {
      return 0;
   }

   return p->blockMap().position(n);
}

int QTextBlock::length() const
{
   if (!p || !n) {
      return 0;
   }

   return p->blockMap().size(n);
}

bool QTextBlock::contains(int position) const
{
   if (!p || !n) {
      return false;
   }

   int pos = p->blockMap().position(n);
   int len = p->blockMap().size(n);
   return position >= pos && position < pos + len;
}

QTextLayout *QTextBlock::layout() const
{
   if (!p || !n) {
      return nullptr;
   }

   const QTextBlockData *b = p->blockMap().fragment(n);
   if (!b->layout) {
      b->layout = new QTextLayout(*this);
   }
   return b->layout;
}

void QTextBlock::clearLayout()
{
   if (!p || !n) {
      return;
   }

   const QTextBlockData *b = p->blockMap().fragment(n);
   if (b->layout) {
      b->layout->clearLayout();
   }
}

QTextBlockFormat QTextBlock::blockFormat() const
{
   if (!p || !n) {
      return QTextFormat().toBlockFormat();
   }

   return p->formatCollection()->blockFormat(p->blockMap().fragment(n)->format);
}

int QTextBlock::blockFormatIndex() const
{
   if (!p || !n) {
      return -1;
   }

   return p->blockMap().fragment(n)->format;
}

QTextCharFormat QTextBlock::charFormat() const
{
   if (!p || ! n) {
      return QTextFormat().toCharFormat();
   }

   return p->formatCollection()->charFormat(charFormatIndex());
}

int QTextBlock::charFormatIndex() const
{
   if (! p || ! n) {
      return -1;
   }

   return p->blockCharFormatIndex(n);
}

Qt::LayoutDirection QTextBlock::textDirection() const
{
   Qt::LayoutDirection dir = blockFormat().layoutDirection();
   if (dir != Qt::LayoutDirectionAuto) {
      return dir;
   }

   dir = p->defaultTextOption.textDirection();
   if (dir != Qt::LayoutDirectionAuto) {
      return dir;
   }

   const QString buffer = p->buffer();
   const int pos = position();

   QTextDocumentPrivate::FragmentIterator frag_iter = p->find(pos);
   QTextDocumentPrivate::FragmentIterator frag_end  = p->find(pos + length() - 1); // -1 to omit the block separator char

   for (; frag_iter != frag_end; ++frag_iter) {
      const QTextFragmentData *const frag = frag_iter.value();

      QString::const_iterator iter_beg = buffer.begin() + frag->stringPosition;
      QString::const_iterator iter_end = iter_beg + frag->size_array[0];

      while (iter_beg < iter_end) {
         switch (iter_beg->direction()) {

            case QChar::DirL:
               return Qt::LeftToRight;

            case QChar::DirR:
            case QChar::DirAL:
               return Qt::RightToLeft;

            default:
               break;
         }

         ++iter_beg;
      }
   }

   return Qt::LeftToRight;
}

QString QTextBlock::text() const
{
   QString text;

   if (! p || ! n) {
      return text;
   }

   const QString &buffer = p->buffer();

   const int pos = position();
   QTextDocumentPrivate::FragmentIterator iter_end = p->find(pos + length() - 1);    // -1 to omit the block separator char

   for (auto iter = p->find(pos); iter != iter_end; ++iter) {
      const QTextFragmentData *frag = iter.value();
      text += buffer.midView(frag->stringPosition, frag->size_array[0]);
   }

   return text;
}

QVector<QTextLayout::FormatRange> QTextBlock::textFormats() const
{
   QVector<QTextLayout::FormatRange> formats;
   if (!p || !n) {
      return formats;
   }

   const QTextFormatCollection *formatCollection = p->formatCollection();

   int start = 0;
   int cur = start;
   int format = -1;

   const int pos = position();
   QTextDocumentPrivate::FragmentIterator it = p->find(pos);
   QTextDocumentPrivate::FragmentIterator end = p->find(pos + length() - 1); // -1 to omit the block separator char
   for (; it != end; ++it) {
      const QTextFragmentData *const frag = it.value();
      if (format != it.value()->format) {
         if (cur - start > 0) {
            QTextLayout::FormatRange range;
            range.start = start;
            range.length = cur - start;
            range.format = formatCollection->charFormat(format);
            formats.append(range);
         }

         format = frag->format;
         start = cur;
      }
      cur += frag->size_array[0];
   }
   if (cur - start > 0) {
      QTextLayout::FormatRange range;
      range.start = start;
      range.length = cur - start;
      range.format = formatCollection->charFormat(format);
      formats.append(range);
   }

   return formats;
}

const QTextDocument *QTextBlock::document() const
{
   return p ? p->document() : nullptr;
}

QTextList *QTextBlock::textList() const
{
   if (! isValid()) {
      return nullptr;
   }

   const QTextBlockFormat fmt = blockFormat();
   QTextObject *obj = p->document()->objectForFormat(fmt);

   return qobject_cast<QTextList *>(obj);
}

QTextBlockUserData *QTextBlock::userData() const
{
   if (! p || ! n) {
      return nullptr;
   }

   const QTextBlockData *b = p->blockMap().fragment(n);

   return b->userData;
}

void QTextBlock::setUserData(QTextBlockUserData *data)
{
   if (!p || !n) {
      return;
   }

   const QTextBlockData *b = p->blockMap().fragment(n);
   if (data != b->userData) {
      delete b->userData;
   }

   b->userData = data;
}

int QTextBlock::userState() const
{
   if (!p || !n) {
      return -1;
   }

   const QTextBlockData *b = p->blockMap().fragment(n);
   return b->userState;
}

void QTextBlock::setUserState(int state)
{
   if (!p || !n) {
      return;
   }

   const QTextBlockData *b = p->blockMap().fragment(n);
   b->userState = state;
}

int QTextBlock::revision() const
{
   if (!p || !n) {
      return -1;
   }

   const QTextBlockData *b = p->blockMap().fragment(n);
   return b->revision;
}

void QTextBlock::setRevision(int rev)
{
   if (!p || !n) {
      return;
   }

   const QTextBlockData *b = p->blockMap().fragment(n);
   b->revision = rev;
}

bool QTextBlock::isVisible() const
{
   if (!p || !n) {
      return true;
   }

   const QTextBlockData *b = p->blockMap().fragment(n);
   return !b->hidden;
}

void QTextBlock::setVisible(bool visible)
{
   if (!p || !n) {
      return;
   }

   const QTextBlockData *b = p->blockMap().fragment(n);
   b->hidden = !visible;
}

int QTextBlock::blockNumber() const
{
   if (!p || !n) {
      return -1;
   }
   return p->blockMap().position(n, 1);
}

int QTextBlock::firstLineNumber() const
{
   if (!p || !n) {
      return -1;
   }
   return p->blockMap().position(n, 2);
}

void QTextBlock::setLineCount(int count)
{
   if (!p || !n) {
      return;
   }
   p->blockMap().setSize(n, count, 2);
}

int QTextBlock::lineCount() const
{
   if (! p || !n) {
      return -1;
   }
   return p->blockMap().size(n, 2);
}

QTextBlock::iterator QTextBlock::begin() const
{
   if (! p || !n) {
      return iterator();
   }

   int pos = position();
   int len = length() - 1; // exclude the fragment that holds the paragraph separator
   int b = p->fragmentMap().findNode(pos);
   int e = p->fragmentMap().findNode(pos + len);
   return iterator(p, b, e, b);
}

QTextBlock::iterator QTextBlock::end() const
{
   if (! p || !n) {
      return iterator();
   }

   int pos = position();
   int len = length() - 1; // exclude the fragment that holds the paragraph separator
   int b = p->fragmentMap().findNode(pos);
   int e = p->fragmentMap().findNode(pos + len);
   return iterator(p, b, e, e);
}

QTextBlock QTextBlock::next() const
{
   if (! isValid()) {
      return QTextBlock();
   }

   return QTextBlock(p, p->blockMap().next(n));
}

QTextBlock QTextBlock::previous() const
{
   if (!p) {
      return QTextBlock();
   }

   return QTextBlock(p, p->blockMap().previous(n));
}

QTextFragment QTextBlock::iterator::fragment() const
{
   int ne = n;
   int formatIndex = p->fragmentMap().fragment(n)->format;

   do {
      ne = p->fragmentMap().next(ne);
   } while (ne != e && p->fragmentMap().fragment(ne)->format == formatIndex);

   return QTextFragment(p, n, ne);
}

QTextBlock::iterator &QTextBlock::iterator::operator++()
{
   int ne = n;
   int formatIndex = p->fragmentMap().fragment(n)->format;
   do {
      ne = p->fragmentMap().next(ne);
   } while (ne != e && p->fragmentMap().fragment(ne)->format == formatIndex);
   n = ne;
   return *this;
}

QTextBlock::iterator &QTextBlock::iterator::operator--()
{
   n = p->fragmentMap().previous(n);

   if (n == b) {
      return *this;
   }

   int formatIndex = p->fragmentMap().fragment(n)->format;
   int last = n;

   while (n != b && p->fragmentMap().fragment(n)->format != formatIndex) {
      last = n;
      n = p->fragmentMap().previous(n);
   }

   n = last;

   return *this;
}

QList<QGlyphRun> QTextFragment::glyphRuns(int pos, int len) const
{
   if (!p || !n) {
      return QList<QGlyphRun>();
   }

   int blockNode = p->blockMap().findNode(position());

   const QTextBlockData *blockData = p->blockMap().fragment(blockNode);
   QTextLayout *layout = blockData->layout;

   int blockPosition = p->blockMap().position(blockNode);
   if (pos < 0) {
      pos = position() - blockPosition;
   }

   if (len < 0) {
      len = length();
   }

   if (len == 0) {
      return QList<QGlyphRun>();
   }

   QList<QGlyphRun> ret;
   for (int i = 0; i < layout->lineCount(); ++i) {
      QTextLine textLine = layout->lineAt(i);
      ret += textLine.glyphRuns(pos, len);
   }

   return ret;
}

int QTextFragment::position() const
{
   if (! p || !n) {
      return 0;   // ### -1 instead?
   }

   return p->fragmentMap().position(n);
}

int QTextFragment::length() const
{
   if (! p || !n) {
      return 0;
   }

   int len = 0;
   int f = n;

   while (f != ne) {
      len += p->fragmentMap().size(f);
      f = p->fragmentMap().next(f);
   }

   return len;
}

bool QTextFragment::contains(int position) const
{
   if (! p || !n) {
      return false;
   }

   int pos = this->position();

   return position >= pos && position < pos + length();
}

QTextCharFormat QTextFragment::charFormat() const
{
   if (!p || !n) {
      return QTextCharFormat();
   }

   const QTextFragmentData *data = p->fragmentMap().fragment(n);

   return p->formatCollection()->charFormat(data->format);
}

int QTextFragment::charFormatIndex() const
{
   if (! p || !n) {
      return -1;
   }

   const QTextFragmentData *data = p->fragmentMap().fragment(n);

   return data->format;
}

QString QTextFragment::text() const
{
   if (! p || !n) {
      return QString();
   }

   QString result;
   QString buffer = p->buffer();
   int f = n;

   while (f != ne) {
      const QTextFragmentData *const frag = p->fragmentMap().fragment(f);
      result += buffer.mid(frag->stringPosition, frag->size_array[0]);
      f = p->fragmentMap().next(f);
   }

   return result;
}
