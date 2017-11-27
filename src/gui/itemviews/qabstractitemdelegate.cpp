/***********************************************************************
*
* Copyright (c) 2012-2017 Barbara Geller
* Copyright (c) 2012-2017 Ansel Sermersheim
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

#include <qabstractitemdelegate.h>

#ifndef QT_NO_ITEMVIEWS
#include <qabstractitemmodel.h>
#include <qabstractitemview.h>
#include <qfontmetrics.h>
#include <qwhatsthis.h>
#include <qtooltip.h>
#include <qevent.h>
#include <qstring.h>
#include <qdebug.h>
#include <qtextengine_p.h>

QT_BEGIN_NAMESPACE


/*!
    Creates a new abstract item delegate with the given \a parent.
*/
QAbstractItemDelegate::QAbstractItemDelegate(QObject *parent)
   : QObject(parent)
{
}


/*!
    Destroys the abstract item delegate.
*/
QAbstractItemDelegate::~QAbstractItemDelegate()
{

}

QWidget *QAbstractItemDelegate::createEditor(QWidget *,
      const QStyleOptionViewItem &, const QModelIndex &) const
{
   return 0;
}


void QAbstractItemDelegate::setEditorData(QWidget *, const QModelIndex &) const
{
   // does nothing
}

/*!
    Sets the data for the item at the given \a index in the \a model
    to the contents of the given \a editor.

    The base implementation does nothing. If you want custom editing
    you will need to reimplement this function.

    \sa setEditorData()
*/
void QAbstractItemDelegate::setModelData(QWidget *,
      QAbstractItemModel *,
      const QModelIndex &) const
{
   // do nothing
}

/*!
    Updates the geometry of the \a editor for the item with the given
    \a index, according to the rectangle specified in the \a option.
    If the item has an internal layout, the editor will be laid out
    accordingly. Note that the index contains information about the
    model being used.

    The base implementation does nothing. If you want custom editing
    you must reimplement this function.
*/
void QAbstractItemDelegate::updateEditorGeometry(QWidget *,
      const QStyleOptionViewItem &,
      const QModelIndex &) const
{
   // do nothing
}

/*!
    When editing of an item starts, this function is called with the
    \a event that triggered the editing, the \a model, the \a index of
    the item, and the \a option used for rendering the item.

    Mouse events are sent to editorEvent() even if they don't start
    editing of the item. This can, for instance, be useful if you wish
    to open a context menu when the right mouse button is pressed on
    an item.

    The base implementation returns false (indicating that it has not
    handled the event).
*/
bool QAbstractItemDelegate::editorEvent(QEvent *,
                                        QAbstractItemModel *,
                                        const QStyleOptionViewItem &,
                                        const QModelIndex &)
{
   // do nothing
   return false;
}

/*!
    \obsolete

    Use QFontMetrics::elidedText() instead.

    \oldcode
        QFontMetrics fm = ...
        QString str = QAbstractItemDelegate::elidedText(fm, width, mode, text);
    \newcode
        QFontMetrics fm = ...
        QString str = fm.elidedText(text, mode, width);
    \endcode
*/

QString QAbstractItemDelegate::elidedText(const QFontMetrics &fontMetrics, int width,
      Qt::TextElideMode mode, const QString &text)
{
   return fontMetrics.elidedText(text, mode, width);
}

/*!
    \since 4.3
    Whenever a help event occurs, this function is called with the \a event
    \a view \a option and the \a index that corresponds to the item where the
    event occurs.

    Returns true if the delegate can handle the event; otherwise returns false.
    A return value of true indicates that the data obtained using the index had
    the required role.

    For QEvent::ToolTip and QEvent::WhatsThis events that were handled successfully,
    the relevant popup may be shown depending on the user's system configuration.

    \sa QHelpEvent
*/
// ### Qt5: Make this a virtual non-slot function
bool QAbstractItemDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view,
                  const QStyleOptionViewItem &option, const QModelIndex &index)
{
   Q_UNUSED(option);

   if (!event || !view) {
      return false;
   }

   switch (event->type()) {
#ifndef QT_NO_TOOLTIP
      case QEvent::ToolTip: {
         QHelpEvent *he = static_cast<QHelpEvent *>(event);
         QVariant tooltip = index.data(Qt::ToolTipRole);
         if (tooltip.canConvert<QString>()) {
            QToolTip::showText(he->globalPos(), tooltip.toString(), view);
            return true;
         }
         break;
      }
#endif
#ifndef QT_NO_WHATSTHIS
      case QEvent::QueryWhatsThis: {
         if (index.data(Qt::WhatsThisRole).isValid()) {
            return true;
         }
         break;
      }
      case QEvent::WhatsThis: {
         QHelpEvent *he = static_cast<QHelpEvent *>(event);
         QVariant whatsthis = index.data(Qt::WhatsThisRole);
         if (whatsthis.canConvert<QString>()) {
            QWhatsThis::showText(he->globalPos(), whatsthis.toString(), view);
            return true;
         }
         break ;
      }
#endif
      default:
         break;
   }
   return false;
}

QT_END_NAMESPACE

#endif // QT_NO_ITEMVIEWS
