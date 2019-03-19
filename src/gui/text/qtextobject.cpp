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

#include <algorithm>

#include <qtextobject.h>
#include <qtextobject_p.h>
#include <qtextdocument.h>
#include <qtextformat_p.h>
#include <qtextdocument_p.h>
#include <qtextcursor.h>
#include <qtextlist.h>
#include <qabstracttextdocumentlayout.h>
#include <qtextengine_p.h>
#include <qdebug.h>

QTextObject::QTextObject(QTextDocument *doc)
   : QObject(doc), d_ptr(new QTextObjectPrivate(doc))
{
   d_ptr->q_ptr = this;
}

/*!
  \fn QTextObject::QTextObject(QTextObjectPrivate &p, QTextDocument *document)

  \internal
*/
QTextObject::QTextObject(QTextObjectPrivate &dd, QTextDocument *doc)
   : QObject(doc), d_ptr(&dd)
{
   d_ptr->q_ptr = this;
}

/*!
    Destroys the text object.

    \warning Text objects are owned by the document, so you should
    never destroy them yourself.
*/
QTextObject::~QTextObject()
{
}

/*!
    Returns the text object's format.

    \sa setFormat() document()
*/
QTextFormat QTextObject::format() const
{
   Q_D(const QTextObject);
   return d->pieceTable->formatCollection()->objectFormat(d->objectIndex);
}

/*!
    Returns the index of the object's format in the document's internal
    list of formats.

    \sa QTextDocument::allFormats()
*/
int QTextObject::formatIndex() const
{
   Q_D(const QTextObject);
   return d->pieceTable->formatCollection()->objectFormatIndex(d->objectIndex);
}


/*!
    Sets the text object's \a format.

    \sa format()
*/
void QTextObject::setFormat(const QTextFormat &format)
{
   Q_D(QTextObject);
   int idx = d->pieceTable->formatCollection()->indexForFormat(format);
   d->pieceTable->changeObjectFormat(this, idx);
}

/*!
    Returns the object index of this object. This can be used together with
    QTextFormat::setObjectIndex().
*/
int QTextObject::objectIndex() const
{
   Q_D(const QTextObject);
   return d->objectIndex;
}

/*!
    Returns the document this object belongs to.

    \sa format()
*/
QTextDocument *QTextObject::document() const
{
   return static_cast<QTextDocument *>(parent());
}

/*!
  \internal
*/
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

/*!
    \fn QTextBlockGroup::QTextBlockGroup(QTextDocument *document)

    Creates a new new block group for the given \a document.

    \warning This function should only be called from
    QTextDocument::createObject().
*/
QTextBlockGroup::QTextBlockGroup(QTextDocument *doc)
   : QTextObject(*new QTextBlockGroupPrivate(doc), doc)
{
}

/*!
  \internal
*/
QTextBlockGroup::QTextBlockGroup(QTextBlockGroupPrivate &p, QTextDocument *doc)
   : QTextObject(p, doc)
{
}

/*!
    Destroys this block group; the blocks are not deleted, they simply
    don't belong to this block anymore.
*/
QTextBlockGroup::~QTextBlockGroup()
{
}

// ### DOC: Shouldn't this be insertBlock()?
/*!
    Appends the given \a block to the end of the group.

    \warning If you reimplement this function you must call the base
    class implementation.
*/
void QTextBlockGroup::blockInserted(const QTextBlock &block)
{
   Q_D(QTextBlockGroup);
   QTextBlockGroupPrivate::BlockList::iterator it = std::lower_bound(d->blocks.begin(), d->blocks.end(), block);
   d->blocks.insert(it, block);
   d->markBlocksDirty();
}

// ### DOC: Shouldn't this be removeBlock()?
/*!
    Removes the given \a block from the group; the block itself is not
    deleted, it simply isn't a member of this group anymore.
*/
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

/*!
    This function is called whenever the specified \a block of text is changed.
    The text block is a member of this group.

    The base class implementation does nothing.
*/
void QTextBlockGroup::blockFormatChanged(const QTextBlock &)
{
}

/*!
    Returns a (possibly empty) list of all the blocks that are part of
    the block group.
*/
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

/*!
    \internal
*/
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


/*!
    Returns the first cursor position inside the frame.

    \sa lastCursorPosition() firstPosition() lastPosition()
*/
QTextCursor QTextFrame::firstCursorPosition() const
{
   Q_D(const QTextFrame);
   return QTextCursor(d->pieceTable, firstPosition());
}

/*!
    Returns the last cursor position inside the frame.

    \sa firstCursorPosition() firstPosition() lastPosition()
*/
QTextCursor QTextFrame::lastCursorPosition() const
{
   Q_D(const QTextFrame);
   return QTextCursor(d->pieceTable, lastPosition());
}

/*!
    Returns the first document position inside the frame.

    \sa lastPosition() firstCursorPosition() lastCursorPosition()
*/
int QTextFrame::firstPosition() const
{
   Q_D(const QTextFrame);
   if (!d->fragment_start) {
      return 0;
   }

   return d->pieceTable->fragmentMap().position(d->fragment_start) + 1;
}

/*!
    Returns the last document position inside the frame.

    \sa firstPosition() firstCursorPosition() lastCursorPosition()
*/
int QTextFrame::lastPosition() const
{
   Q_D(const QTextFrame);
   if (!d->fragment_end) {
      return d->pieceTable->length() - 1;
   }
   return d->pieceTable->fragmentMap().position(d->fragment_end);
}

/*!
  \internal
*/
QTextFrameLayoutData *QTextFrame::layoutData() const
{
   Q_D(const QTextFrame);
   return d->layoutData;
}

/*!
  \internal
*/
void QTextFrame::setLayoutData(QTextFrameLayoutData *data)
{
   Q_D(QTextFrame);
   delete d->layoutData;
   d->layoutData = data;
}



void QTextFramePrivate::fragmentAdded(const QChar &type, uint fragment)
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

void QTextFramePrivate::fragmentRemoved(const QChar &type, uint fragment)
{
   Q_UNUSED(fragment); // --release warning
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
   parentFrame = 0;
}

/*!
    \class QTextFrame::iterator
    \reentrant

    \brief The iterator class provides an iterator for reading
    the contents of a QTextFrame.

    \ingroup richtext-processing

    A frame consists of an arbitrary sequence of \l{QTextBlock}s and
    child \l{QTextFrame}s. This class provides a way to iterate over the
    child objects of a frame, and read their contents. It does not provide
    a way to modify the contents of the frame.

*/

/*!
    \fn bool QTextFrame::iterator::atEnd() const

    Returns true if the current item is the last item in the text frame.
*/

/*!
    Returns an iterator pointing to the first document element inside the frame.
    Please see the document \l{STL-style-Iterators} for more information.

    \sa end()
*/
QTextFrame::iterator QTextFrame::begin() const
{
   const QTextDocumentPrivate *priv = docHandle();
   int b = priv->blockMap().findNode(firstPosition());
   int e = priv->blockMap().findNode(lastPosition() + 1);
   return iterator(const_cast<QTextFrame *>(this), b, b, e);
}

/*!
    Returns an iterator pointing to the position past the last document element inside the frame.
    Please see the document \l{STL-Style Iterators} for more information.
    \sa begin()
*/
QTextFrame::iterator QTextFrame::end() const
{
   const QTextDocumentPrivate *priv = docHandle();
   int b = priv->blockMap().findNode(firstPosition());
   int e = priv->blockMap().findNode(lastPosition() + 1);
   return iterator(const_cast<QTextFrame *>(this), e, b, e);
}

/*!
    Constructs an invalid iterator.
*/
QTextFrame::iterator::iterator()
{
   f = 0;
   b = 0;
   e = 0;
   cf = 0;
   cb = 0;
}

/*!
  \internal
*/
QTextFrame::iterator::iterator(QTextFrame *frame, int block, int begin, int end)
{
   f = frame;
   b = begin;
   e = end;
   cf = 0;
   cb = block;
}

/*!
    Copy constructor. Constructs a copy of the \a other iterator.
*/
QTextFrame::iterator::iterator(const iterator &other)
{
   f = other.f;
   b = other.b;
   e = other.e;
   cf = other.cf;
   cb = other.cb;
}

/*!
    Assigns \a other to this iterator and returns a reference to
    this iterator.
*/
QTextFrame::iterator &QTextFrame::iterator::operator=(const iterator &other)
{
   f = other.f;
   b = other.b;
   e = other.e;
   cf = other.cf;
   cb = other.cb;
   return *this;
}

/*!
    Returns the current frame pointed to by the iterator, or 0 if the
    iterator currently points to a block.

    \sa currentBlock()
*/
QTextFrame *QTextFrame::iterator::currentFrame() const
{
   return cf;
}

/*!
    Returns the current block the iterator points to. If the iterator
    points to a child frame, the returned block is invalid.

    \sa currentFrame()
*/
QTextBlock QTextFrame::iterator::currentBlock() const
{
   if (!f) {
      return QTextBlock();
   }
   return QTextBlock(f->docHandle(), cb);
}

/*!
    Moves the iterator to the next frame or block.

    \sa currentBlock() currentFrame()
*/
QTextFrame::iterator &QTextFrame::iterator::operator++()
{
   const QTextDocumentPrivate *priv = f->docHandle();
   const QTextDocumentPrivate::BlockMap &map = priv->blockMap();
   if (cf) {
      int end = cf->lastPosition() + 1;
      cb = map.findNode(end);
      cf = 0;
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

/*!
    Moves the iterator to the previous frame or block.

    \sa currentBlock() currentFrame()
*/
QTextFrame::iterator &QTextFrame::iterator::operator--()
{
   const QTextDocumentPrivate *priv = f->docHandle();
   const QTextDocumentPrivate::BlockMap &map = priv->blockMap();
   if (cf) {
      int start = cf->firstPosition() - 1;
      cb = map.findNode(start);
      cf = 0;
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
      return 0;
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
   if (!p || !n) {
      return QTextFormat().toCharFormat();
   }

   return p->formatCollection()->charFormat(charFormatIndex());
}

int QTextBlock::charFormatIndex() const
{
   if (! p || !n) {
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

const QTextDocument *QTextBlock::document() const
{
   return p ? p->document() : 0;
}

QTextList *QTextBlock::textList() const
{
   if (! isValid()) {
      return 0;
   }

   const QTextBlockFormat fmt = blockFormat();
   QTextObject *obj = p->document()->objectForFormat(fmt);

   return qobject_cast<QTextList *>(obj);
}

QTextBlockUserData *QTextBlock::userData() const
{
   if (! p || ! n) {
      return 0;
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

/*!
    \since 4.1

    Returns the integer value previously set with setUserState() or -1.
*/
int QTextBlock::userState() const
{
   if (!p || !n) {
      return -1;
   }

   const QTextBlockData *b = p->blockMap().fragment(n);
   return b->userState;
}

/*!
    \since 4.1

    Stores the specified \a state integer value in the text block. This may be
    useful for example in a syntax highlighter to store a text parsing state.
*/
void QTextBlock::setUserState(int state)
{
   if (!p || !n) {
      return;
   }

   const QTextBlockData *b = p->blockMap().fragment(n);
   b->userState = state;
}

/*!
    \since 4.4

    Returns the blocks revision.

    \sa setRevision(), QTextDocument::revision()
*/
int QTextBlock::revision() const
{
   if (!p || !n) {
      return -1;
   }

   const QTextBlockData *b = p->blockMap().fragment(n);
   return b->revision;
}

/*!
    \since 4.4

    Sets a blocks revision to \a rev.

    \sa revision(), QTextDocument::revision()
*/
void QTextBlock::setRevision(int rev)
{
   if (!p || !n) {
      return;
   }

   const QTextBlockData *b = p->blockMap().fragment(n);
   b->revision = rev;
}

/*!
    \since 4.4

    Returns true if the block is visible; otherwise returns false.

    \sa setVisible()
*/
bool QTextBlock::isVisible() const
{
   if (!p || !n) {
      return true;
   }

   const QTextBlockData *b = p->blockMap().fragment(n);
   return !b->hidden;
}

/*!
    \since 4.4

    Sets the block's visibility to \a visible.

    \sa isVisible()
*/
void QTextBlock::setVisible(bool visible)
{
   if (!p || !n) {
      return;
   }

   const QTextBlockData *b = p->blockMap().fragment(n);
   b->hidden = !visible;
}


/*!
\since 4.4

    Returns the number of this block, or -1 if the block is invalid.

    \sa QTextCursor::blockNumber()

*/
int QTextBlock::blockNumber() const
{
   if (!p || !n) {
      return -1;
   }
   return p->blockMap().position(n, 1);
}

/*!
\since 4.5

    Returns the first line number of this block, or -1 if the block is invalid.
    Unless the layout supports it, the line number is identical to the block number.

    \sa QTextBlock::blockNumber()

*/
int QTextBlock::firstLineNumber() const
{
   if (!p || !n) {
      return -1;
   }
   return p->blockMap().position(n, 2);
}


/*!
\since 4.5

Sets the line count to \a count.

\sa lineCount()
*/
void QTextBlock::setLineCount(int count)
{
   if (!p || !n) {
      return;
   }
   p->blockMap().setSize(n, count, 2);
}
/*!
\since 4.5

Returns the line count. Not all document layouts support this feature.

\sa setLineCount()
 */
int QTextBlock::lineCount() const
{
   if (!p || !n) {
      return -1;
   }
   return p->blockMap().size(n, 2);
}


/*!
    Returns a text block iterator pointing to the beginning of the
    text block.

    \sa end()
*/
QTextBlock::iterator QTextBlock::begin() const
{
   if (!p || !n) {
      return iterator();
   }

   int pos = position();
   int len = length() - 1; // exclude the fragment that holds the paragraph separator
   int b = p->fragmentMap().findNode(pos);
   int e = p->fragmentMap().findNode(pos + len);
   return iterator(p, b, e, b);
}

/*!
    Returns a text block iterator pointing to the end of the text
    block.

    \sa begin() next() previous()
*/
QTextBlock::iterator QTextBlock::end() const
{
   if (!p || !n) {
      return iterator();
   }

   int pos = position();
   int len = length() - 1; // exclude the fragment that holds the paragraph separator
   int b = p->fragmentMap().findNode(pos);
   int e = p->fragmentMap().findNode(pos + len);
   return iterator(p, b, e, e);
}


/*!
    Returns the text block in the document after this block, or an empty
    text block if this is the last one.

    Note that the next block may be in a different frame or table to this block.

    \sa previous() begin() end()
*/
QTextBlock QTextBlock::next() const
{
   if (!isValid() || !p->blockMap().isValid(n)) {
      return QTextBlock();
   }

   return QTextBlock(p, p->blockMap().next(n));
}

/*!
    Returns the text block in the document before this block, or an empty text
    block if this is the first one.

    Note that the next block may be in a different frame or table to this block.

    \sa next() begin() end()
*/
QTextBlock QTextBlock::previous() const
{
   if (!p) {
      return QTextBlock();
   }

   return QTextBlock(p, p->blockMap().previous(n));
}


/*!
    Returns the text fragment the iterator currently points to.
*/
QTextFragment QTextBlock::iterator::fragment() const
{
   int ne = n;
   int formatIndex = p->fragmentMap().fragment(n)->format;
   do {
      ne = p->fragmentMap().next(ne);
   } while (ne != e && p->fragmentMap().fragment(ne)->format == formatIndex);
   return QTextFragment(p, n, ne);
}

/*!
    The prefix ++ operator (\c{++i}) advances the iterator to the
    next item in the hash and returns an iterator to the new current
    item.
*/

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

/*!
    The prefix -- operator (\c{--i}) makes the preceding item
    current and returns an iterator pointing to the new current item.
*/

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

#if !defined(QT_NO_RAWFONT)
QList<QGlyphRun> QTextFragment::glyphRuns() const
{
   if (!p || !n) {
      return QList<QGlyphRun>();
   }

   int pos = position();
   int len = length();
   if (len == 0) {
      return QList<QGlyphRun>();
   }

   int blockNode = p->blockMap().findNode(pos);

   const QTextBlockData *blockData = p->blockMap().fragment(blockNode);
   QTextLayout *layout = blockData->layout;

   QList<QGlyphRun> ret;

   for (int i = 0; i < layout->lineCount(); ++i) {
      QTextLine textLine = layout->lineAt(i);
      ret += textLine.glyphs(pos, len);
   }

   return ret;
}
#endif // QT_NO_RAWFONT

/*!
    Returns the position of this text fragment in the document.
*/
int QTextFragment::position() const
{
   if (!p || !n) {
      return 0;   // ### -1 instead?
   }

   return p->fragmentMap().position(n);
}

/*!
    Returns the number of characters in the text fragment.

    \sa text()
*/
int QTextFragment::length() const
{
   if (!p || !n) {
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
   if (!p || !n) {
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
