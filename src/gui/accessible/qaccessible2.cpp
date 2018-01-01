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

#include <qaccessible2.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qtextboundaryfinder.h>

#ifndef QT_NO_ACCESSIBILITY

QT_BEGIN_NAMESPACE

/*!
    \namespace QAccessible2
    \ingroup accessibility
    \internal
    \preliminary

    \brief The QAccessible2 namespace defines constants relating to
    IAccessible2-based interfaces

    \link http://www.linux-foundation.org/en/Accessibility/IAccessible2 IAccessible2 Specification \endlink
*/

/*!
    \class QAccessibleTextInterface

    \ingroup accessibility
    \internal
    \preliminary

    \brief The QAccessibleTextInterface class implements support for
    the IAccessibleText interface.

    \link http://www.linux-foundation.org/en/Accessibility/IAccessible2 IAccessible2 Specification \endlink
*/

/*!
    \class QAccessibleEditableTextInterface
    \ingroup accessibility
    \internal
    \preliminary

    \brief The QAccessibleEditableTextInterface class implements support for
    the IAccessibleEditableText interface.

    \link http://www.linux-foundation.org/en/Accessibility/IAccessible2 IAccessible2 Specification \endlink
*/

/*!
    \class QAccessibleSimpleEditableTextInterface
    \ingroup accessibility
    \internal
    \preliminary

    \brief The QAccessibleSimpleEditableTextInterface class is a convenience class for
    text-based widgets.

    \link http://www.linux-foundation.org/en/Accessibility/IAccessible2 IAccessible2 Specification \endlink
*/

/*!
    \class QAccessibleValueInterface
    \ingroup accessibility
    \internal
    \preliminary

    \brief The QAccessibleValueInterface class implements support for
    the IAccessibleValue interface.

    \link http://www.linux-foundation.org/en/Accessibility/IAccessible2 IAccessible2 Specification \endlink
*/

/*!
    \class QAccessibleActionInterface
    \ingroup accessibility
    \internal
    \preliminary

    \brief The QAccessibleActionInterface class implements support for
    the IAccessibleAction interface.

    \link http://www.linux-foundation.org/en/Accessibility/IAccessible2 IAccessible2 Specification \endlink
*/

/*!
    \class QAccessibleImageInterface
    \ingroup accessibility
    \internal
    \preliminary

    \brief The QAccessibleImageInterface class implements support for
    the IAccessibleImage interface.

    \link http://www.linux-foundation.org/en/Accessibility/IAccessible2 IAccessible2 Specification \endlink
*/


/*!
  \internal
*/
QString Q_GUI_EXPORT qTextBeforeOffsetFromString(int offset, QAccessible2::BoundaryType boundaryType,
      int *startOffset, int *endOffset, const QString &text)
{
   QTextBoundaryFinder::BoundaryType type;
   switch (boundaryType) {
      case QAccessible2::CharBoundary:
         type = QTextBoundaryFinder::Grapheme;
         break;
      case QAccessible2::WordBoundary:
         type = QTextBoundaryFinder::Word;
         break;
      case QAccessible2::SentenceBoundary:
         type = QTextBoundaryFinder::Sentence;
         break;
      default:
         // in any other case return the whole line
         *startOffset = 0;
         *endOffset = text.length();
         return text;
   }

   QTextBoundaryFinder boundary(type, text);
   boundary.setPosition(offset);

   if (!boundary.isAtBoundary()) {
      boundary.toPreviousBoundary();
   }
   boundary.toPreviousBoundary();
   *startOffset = boundary.position();
   boundary.toNextBoundary();
   *endOffset = boundary.position();

   return text.mid(*startOffset, *endOffset - *startOffset);
}

/*!
  \internal
*/
QString Q_GUI_EXPORT qTextAfterOffsetFromString(int offset, QAccessible2::BoundaryType boundaryType,
      int *startOffset, int *endOffset, const QString &text)
{
   QTextBoundaryFinder::BoundaryType type;
   switch (boundaryType) {
      case QAccessible2::CharBoundary:
         type = QTextBoundaryFinder::Grapheme;
         break;
      case QAccessible2::WordBoundary:
         type = QTextBoundaryFinder::Word;
         break;
      case QAccessible2::SentenceBoundary:
         type = QTextBoundaryFinder::Sentence;
         break;
      default:
         // in any other case return the whole line
         *startOffset = 0;
         *endOffset = text.length();
         return text;
   }

   QTextBoundaryFinder boundary(type, text);
   boundary.setPosition(offset);

   boundary.toNextBoundary();
   *startOffset = boundary.position();
   boundary.toNextBoundary();
   *endOffset = boundary.position();

   return text.mid(*startOffset, *endOffset - *startOffset);
}

/*!
  \internal
*/
QString Q_GUI_EXPORT qTextAtOffsetFromString(int offset, QAccessible2::BoundaryType boundaryType,
      int *startOffset, int *endOffset, const QString &text)
{
   QTextBoundaryFinder::BoundaryType type;
   switch (boundaryType) {
      case QAccessible2::CharBoundary:
         type = QTextBoundaryFinder::Grapheme;
         break;
      case QAccessible2::WordBoundary:
         type = QTextBoundaryFinder::Word;
         break;
      case QAccessible2::SentenceBoundary:
         type = QTextBoundaryFinder::Sentence;
         break;
      default:
         // in any other case return the whole line
         *startOffset = 0;
         *endOffset = text.length();
         return text;
   }

   QTextBoundaryFinder boundary(type, text);
   boundary.setPosition(offset);

   if (!boundary.isAtBoundary()) {
      boundary.toPreviousBoundary();
   }
   *startOffset = boundary.position();
   boundary.toNextBoundary();
   *endOffset = boundary.position();

   return text.mid(*startOffset, *endOffset - *startOffset);
}

QAccessibleSimpleEditableTextInterface::QAccessibleSimpleEditableTextInterface(
   QAccessibleInterface *accessibleInterface)
   : iface(accessibleInterface)
{
   Q_ASSERT(iface);
}

#ifndef QT_NO_CLIPBOARD
static QString textForRange(QAccessibleInterface *iface, int startOffset, int endOffset)
{
   return iface->text(QAccessible::Value, 0).mid(startOffset, endOffset - startOffset);
}
#endif

void QAccessibleSimpleEditableTextInterface::copyText(int startOffset, int endOffset)
{
#ifdef QT_NO_CLIPBOARD
   Q_UNUSED(startOffset);
   Q_UNUSED(endOffset);
#else
   QApplication::clipboard()->setText(textForRange(iface, startOffset, endOffset));
#endif
}

void QAccessibleSimpleEditableTextInterface::deleteText(int startOffset, int endOffset)
{
   QString txt = iface->text(QAccessible::Value, 0);
   txt.remove(startOffset, endOffset - startOffset);
   iface->setText(QAccessible::Value, 0, txt);
}

void QAccessibleSimpleEditableTextInterface::insertText(int offset, const QString &text)
{
   QString txt = iface->text(QAccessible::Value, 0);
   txt.insert(offset, text);
   iface->setText(QAccessible::Value, 0, txt);
}

void QAccessibleSimpleEditableTextInterface::cutText(int startOffset, int endOffset)
{
#ifdef QT_NO_CLIPBOARD
   Q_UNUSED(startOffset);
   Q_UNUSED(endOffset);
#else
   QString sub = textForRange(iface, startOffset, endOffset);
   deleteText(startOffset, endOffset);
   QApplication::clipboard()->setText(sub);
#endif
}

void QAccessibleSimpleEditableTextInterface::pasteText(int offset)
{
#ifdef QT_NO_CLIPBOARD
   Q_UNUSED(offset);
#else
   QString txt = iface->text(QAccessible::Value, 0);
   txt.insert(offset, QApplication::clipboard()->text());
   iface->setText(QAccessible::Value, 0, txt);
#endif
}

void QAccessibleSimpleEditableTextInterface::replaceText(int startOffset, int endOffset, const QString &text)
{
   QString txt = iface->text(QAccessible::Value, 0);
   txt.replace(startOffset, endOffset - startOffset, text);
   iface->setText(QAccessible::Value, 0, txt);
}

QT_END_NAMESPACE

#endif // QT_NO_ACCESSIBILITY
