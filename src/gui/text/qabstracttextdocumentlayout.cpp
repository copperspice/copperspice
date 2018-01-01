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

#include <qabstracttextdocumentlayout.h>
#include <qtextformat.h>
#include <qtextdocument_p.h>
#include <qtextengine_p.h>

#include <qabstracttextdocumentlayout_p.h>

QT_BEGIN_NAMESPACE

QAbstractTextDocumentLayout::QAbstractTextDocumentLayout(QTextDocument *document)
   : QObject(document), d_ptr(new QAbstractTextDocumentLayoutPrivate)
{
   d_ptr->q_ptr = this;
   Q_D(QAbstractTextDocumentLayout);

   d->setDocument(document);
}

/*!
    \internal
*/
QAbstractTextDocumentLayout::QAbstractTextDocumentLayout(QAbstractTextDocumentLayoutPrivate &dd,
      QTextDocument *document)
   : QObject(document), d_ptr(&dd)
{
   d_ptr->q_ptr = this;
   Q_D(QAbstractTextDocumentLayout);

   d->setDocument(document);
}

/*!
    \internal
*/
QAbstractTextDocumentLayout::~QAbstractTextDocumentLayout()
{
}

void QAbstractTextDocumentLayout::registerHandler(int formatType, QObject *component)
{
   Q_D(QAbstractTextDocumentLayout);

   QTextObjectInterface *iface = qobject_cast<QTextObjectInterface *>(component);
   if (!iface) {
      return;   // ### print error message on terminal?
   }

   connect(component, SIGNAL(destroyed(QObject *)), this, SLOT(_q_handlerDestroyed(QObject *)));

   QTextObjectHandler h;
   h.iface = iface;
   h.component = component;
   d->handlers.insert(formatType, h);
}

/*!
    Returns a handler for objects of the given \a objectType.
*/
QTextObjectInterface *QAbstractTextDocumentLayout::handlerForObject(int objectType) const
{
   Q_D(const QAbstractTextDocumentLayout);

   QTextObjectHandler handler = d->handlers.value(objectType);
   if (!handler.component) {
      return 0;
   }

   return handler.iface;
}

/*!
    Sets the size of the inline object \a item corresponding to the text
    \a format.

    \a posInDocument specifies the position of the object within the document.

    The default implementation resizes the \a item to the size returned by
    the object handler's intrinsicSize() function. This function is called only
    within Qt. Subclasses can reimplement this function to customize the
    resizing of inline objects.
*/
void QAbstractTextDocumentLayout::resizeInlineObject(QTextInlineObject item, int posInDocument,
      const QTextFormat &format)
{
   Q_D(QAbstractTextDocumentLayout);

   QTextCharFormat f = format.toCharFormat();
   Q_ASSERT(f.isValid());
   QTextObjectHandler handler = d->handlers.value(f.objectType());
   if (!handler.component) {
      return;
   }

   QSizeF s = handler.iface->intrinsicSize(document(), posInDocument, format);
   item.setWidth(s.width());
   item.setAscent(s.height());
   item.setDescent(0);
}

/*!
    Lays out the inline object \a item using the given text \a format.

    \a posInDocument specifies the position of the object within the document.

    The default implementation does nothing. This function is called only
    within Qt. Subclasses can reimplement this function to customize the
    position of inline objects.

    \sa drawInlineObject()
*/
void QAbstractTextDocumentLayout::positionInlineObject(QTextInlineObject item, int posInDocument,
      const QTextFormat &format)
{
   Q_UNUSED(item);
   Q_UNUSED(posInDocument);
   Q_UNUSED(format);
}

/*!
    \fn void QAbstractTextDocumentLayout::drawInlineObject(QPainter *painter, const QRectF &rect, QTextInlineObject object, int posInDocument, const QTextFormat &format)

    This function is called to draw the inline object, \a object, with the
    given \a painter within the rectangle specified by \a rect using the
    specified text \a format.

    \a posInDocument specifies the position of the object within the document.

    The default implementation calls drawObject() on the object handlers. This
    function is called only within Qt. Subclasses can reimplement this function
    to customize the drawing of inline objects.

    \sa draw()
*/
void QAbstractTextDocumentLayout::drawInlineObject(QPainter *p, const QRectF &rect, QTextInlineObject item,
      int posInDocument, const QTextFormat &format)
{
   Q_UNUSED(item);
   Q_D(QAbstractTextDocumentLayout);

   QTextCharFormat f = format.toCharFormat();
   Q_ASSERT(f.isValid());
   QTextObjectHandler handler = d->handlers.value(f.objectType());
   if (!handler.component) {
      return;
   }

   handler.iface->drawObject(p, rect, document(), posInDocument, format);
}

void QAbstractTextDocumentLayoutPrivate::_q_handlerDestroyed(QObject *obj)
{
   HandlerHash::Iterator it = handlers.begin();
   while (it != handlers.end())
      if ((*it).component == obj) {
         it = handlers.erase(it);
      } else {
         ++it;
      }
}

/*!
    \internal

    Returns the index of the format at position \a pos.
*/
int QAbstractTextDocumentLayout::formatIndex(int pos)
{
   QTextDocumentPrivate *pieceTable = qobject_cast<QTextDocument *>(parent())->docHandle();
   return pieceTable->find(pos).value()->format;
}

/*!
    \fn QTextCharFormat QAbstractTextDocumentLayout::format(int position)

    Returns the character format that is applicable at the given \a position.
*/
QTextCharFormat QAbstractTextDocumentLayout::format(int pos)
{
   QTextDocumentPrivate *pieceTable = qobject_cast<QTextDocument *>(parent())->docHandle();
   int idx = pieceTable->find(pos).value()->format;
   return pieceTable->formatCollection()->charFormat(idx);
}



/*!
    Returns the text document that this layout is operating on.
*/
QTextDocument *QAbstractTextDocumentLayout::document() const
{
   Q_D(const QAbstractTextDocumentLayout);
   return d->document;
}

/*!
    \fn QString QAbstractTextDocumentLayout::anchorAt(const QPointF &position) const

    Returns the reference of the anchor the given \a position, or an empty
    string if no anchor exists at that point.
*/
QString QAbstractTextDocumentLayout::anchorAt(const QPointF &pos) const
{
   int cursorPos = hitTest(pos, Qt::ExactHit);
   if (cursorPos == -1) {
      return QString();
   }

   QTextDocumentPrivate *pieceTable = qobject_cast<const QTextDocument *>(parent())->docHandle();
   QTextDocumentPrivate::FragmentIterator it = pieceTable->find(cursorPos);
   QTextCharFormat fmt = pieceTable->formatCollection()->charFormat(it->format);
   return fmt.anchorHref();
}

/*!
    \fn QRectF QAbstractTextDocumentLayout::frameBoundingRect(QTextFrame *frame) const

    Returns the bounding rectangle of \a frame.
*/

/*!
    \fn QRectF QAbstractTextDocumentLayout::blockBoundingRect(const QTextBlock &block) const

    Returns the bounding rectangle of \a block.
*/

/*!
    Sets the paint device used for rendering the document's layout to the given
    \a device.

    \sa paintDevice()
*/
void QAbstractTextDocumentLayout::setPaintDevice(QPaintDevice *device)
{
   Q_D(QAbstractTextDocumentLayout);
   d->paintDevice = device;
}

/*!
    Returns the paint device used to render the document's layout.

    \sa setPaintDevice()
*/
QPaintDevice *QAbstractTextDocumentLayout::paintDevice() const
{
   Q_D(const QAbstractTextDocumentLayout);
   return d->paintDevice;
}

void QAbstractTextDocumentLayout::_q_handlerDestroyed(QObject *obj)
{
   Q_D(QAbstractTextDocumentLayout);
   d->_q_handlerDestroyed(obj);
}

int QAbstractTextDocumentLayout::_q_dynamicPageCountSlot()
{
   Q_D(QAbstractTextDocumentLayout);
   return d->_q_dynamicPageCountSlot();
}

QSizeF QAbstractTextDocumentLayout::_q_dynamicDocumentSizeSlot()
{
   Q_D(QAbstractTextDocumentLayout);
   return d->_q_dynamicDocumentSizeSlot();
}

QT_END_NAMESPACE
