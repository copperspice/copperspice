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

#include <qstyleditemdelegate.h>

#ifndef QT_NO_ITEMVIEWS

#include <qabstractitemmodel.h>
#include <qapplication.h>
#include <qbitmap.h>
#include <qbrush.h>
#include <qdatetime.h>
#include <qevent.h>
#include <qitemeditorfactory.h>
#include <qlineedit.h>
#include <qpainter.h>
#include <qpalette.h>
#include <qpixmap.h>
#include <qpixmapcache.h>
#include <qplaintextedit.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qtextedit.h>

#include <qmetaobject.h>
#include <qtextlayout.h>
#include <qdebug.h>
#include <qlocale.h>
#include <qdialog.h>
#include <qtableview.h>

#include <qabstractitemdelegate_p.h>
#include <qitemeditorfactory_p.h>
#include <qlayoutengine_p.h>
#include <qtextengine_p.h>

#include <limits.h>

class QStyledItemDelegatePrivate : public QAbstractItemDelegatePrivate
{
   Q_DECLARE_PUBLIC(QStyledItemDelegate)

 public:
   QStyledItemDelegatePrivate()
      : factory(nullptr)
   { }

   static const QWidget *widget(const QStyleOptionViewItem &option) {
      return option.widget;
   }

   const QItemEditorFactory *editorFactory() const {

      if (factory == nullptr) {
         return QItemEditorFactory::defaultFactory();

      } else {
         return factory;
      }
   }

   QItemEditorFactory *factory;
};

QStyledItemDelegate::QStyledItemDelegate(QObject *parent)
   : QAbstractItemDelegate(*new QStyledItemDelegatePrivate(), parent)
{
}

QString QStyledItemDelegate::displayText(const QVariant &value, const QLocale &locale) const
{
   return d_func()->textForRole(Qt::DisplayRole, value, locale);
}

void QStyledItemDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
   QVariant value = index.data(Qt::FontRole);

   if (value.isValid()) {
      option->font = value.value<QFont>().resolve(option->font);
      option->fontMetrics = QFontMetrics(option->font);
   }

   value = index.data(Qt::TextAlignmentRole);
   if (value.isValid()) {
      option->displayAlignment = Qt::Alignment(value.toInt());
   }

   value = index.data(Qt::ForegroundRole);
   if (value.canConvert<QBrush>()) {
      option->palette.setBrush(QPalette::Text, value.value<QBrush>());
   }

   option->index = index;
   value = index.data(Qt::CheckStateRole);
   if (value.isValid()) {
      option->features |= QStyleOptionViewItem::HasCheckIndicator;
      option->checkState = static_cast<Qt::CheckState>(value.toInt());
   }

   value = index.data(Qt::DecorationRole);

   if (value.isValid()) {
      option->features |= QStyleOptionViewItem::HasDecoration;

      switch (value.type()) {
         case QVariant::Icon: {
            option->icon = value.value<QIcon>();
            QIcon::Mode mode;

            if (! (option->state & QStyle::State_Enabled)) {
               mode = QIcon::Disabled;

            } else if (option->state & QStyle::State_Selected) {
               mode = QIcon::Selected;

            } else {
               mode = QIcon::Normal;
            }

            QIcon::State state = option->state & QStyle::State_Open ? QIcon::On : QIcon::Off;
            QSize actualSize = option->icon.actualSize(option->decorationSize, mode, state);

            // For highdpi icons actualSize might be larger than decorationSize, which we don't want. Clamp it to decorationSize.
            option->decorationSize = QSize(qMin(option->decorationSize.width(), actualSize.width()),
                  qMin(option->decorationSize.height(), actualSize.height()));
            break;
         }

         case QVariant::Color: {
            QPixmap pixmap(option->decorationSize);
            pixmap.fill(value.value<QColor>());
            option->icon = QIcon(pixmap);
            break;
         }

         case QVariant::Image: {
            QImage image = value.value<QImage>();
            option->icon = QIcon(QPixmap::fromImage(image));
            option->decorationSize = image.size() / image.devicePixelRatio();
            break;
         }

         case QVariant::Pixmap: {
            QPixmap pixmap = value.value<QPixmap>();
            option->icon = QIcon(pixmap);
            option->decorationSize = pixmap.size() / pixmap.devicePixelRatio();
            break;
         }
         default:
            break;
      }
   }

   value = index.data(Qt::DisplayRole);

   if (value.isValid()) {
      option->features |= QStyleOptionViewItem::HasDisplay;
      option->text = displayText(value, option->locale);
   }

   option->backgroundBrush = (index.data(Qt::BackgroundRole)).value<QBrush>();

   // disable style animations for checkboxes etc. within itemviews (QTBUG-30146)
   option->styleObject = nullptr;
}

void QStyledItemDelegate::paint(QPainter *painter,
   const QStyleOptionViewItem &option, const QModelIndex &index) const
{
   Q_ASSERT(index.isValid());

   QStyleOptionViewItem opt = option;
   initStyleOption(&opt, index);

   const QWidget *widget = QStyledItemDelegatePrivate::widget(option);
   QStyle *style = widget ? widget->style() : QApplication::style();
   style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);
}

QSize QStyledItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
   QVariant value = index.data(Qt::SizeHintRole);

   if (value.isValid()) {
      return value.value<QSize>();
   }

   QStyleOptionViewItem opt = option;
   initStyleOption(&opt, index);

   const QWidget *widget = QStyledItemDelegatePrivate::widget(option);
   QStyle *style = widget ? widget->style() : QApplication::style();

   return style->sizeFromContents(QStyle::CT_ItemViewItem, &opt, QSize(), widget);
}

QWidget *QStyledItemDelegate::createEditor(QWidget *parent,
      const QStyleOptionViewItem &, const QModelIndex &index) const
{
   Q_D(const QStyledItemDelegate);

   if (! index.isValid()) {
      return nullptr;
   }

   QVariant::Type t = static_cast<QVariant::Type>(index.data(Qt::EditRole).userType());

   return d->editorFactory()->createEditor(t, parent);
}

void QStyledItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
   QVariant data    = index.data(Qt::EditRole);
   QString propName = editor->metaObject()->userProperty().name();

   if (! propName.isEmpty()) {
      if (! data.isValid()) {
         data = QVariant(editor->property(propName).userType(), nullptr);
      }

      editor->setProperty(propName, data);
   }
}

void QStyledItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
#ifndef QT_NO_PROPERTIES

   Q_D(const QStyledItemDelegate);

   Q_ASSERT(model);
   Q_ASSERT(editor);

   QString n = editor->metaObject()->userProperty().name();

   if (n.isEmpty()) {
      n = d->editorFactory()->valuePropertyName(static_cast<QVariant::Type>(model->data(index, Qt::EditRole).userType()));
   }

   if (! n.isEmpty()) {
      model->setData(index, editor->property(n), Qt::EditRole);

   }
#endif
}

void QStyledItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
   if (! editor) {
      return;
   }

   Q_ASSERT(index.isValid());
   const QWidget *widget = QStyledItemDelegatePrivate::widget(option);

   QStyleOptionViewItem opt = option;
   initStyleOption(&opt, index);

   // let the editor take up all available space
   //if the editor is not a QLineEdit or it is in a QTableView

#if ! defined(QT_NO_TABLEVIEW) && !defined(QT_NO_LINEEDIT)
   if (qobject_cast<QExpandingLineEdit *>(editor) && !qobject_cast<const QTableView *>(widget)) {
      opt.showDecorationSelected = editor->style()->styleHint(QStyle::SH_ItemView_ShowDecorationSelected, nullptr, editor);
   } else
#endif
      opt.showDecorationSelected = true;

   QStyle *style = widget ? widget->style() : QApplication::style();
   QRect geom = style->subElementRect(QStyle::SE_ItemViewItemText, &opt, widget);

   if ( editor->layoutDirection() == Qt::RightToLeft) {
      const int delta = qSmartMinSize(editor).width() - geom.width();
      if (delta > 0) {
         //we need to widen the geometry
         geom.adjust(-delta, 0, 0, 0);
      }
   }

   editor->setGeometry(geom);
}

QItemEditorFactory *QStyledItemDelegate::itemEditorFactory() const
{
   Q_D(const QStyledItemDelegate);
   return d->factory;
}

void QStyledItemDelegate::setItemEditorFactory(QItemEditorFactory *factory)
{
   Q_D(QStyledItemDelegate);
   d->factory = factory;
}

bool QStyledItemDelegate::eventFilter(QObject *object, QEvent *event)
{
   Q_D(QStyledItemDelegate);
   return d->editorEventFilter(object, event);
}

bool QStyledItemDelegate::editorEvent(QEvent *event,
      QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
   Q_ASSERT(event);
   Q_ASSERT(model);

   // make sure that the item is checkable
   Qt::ItemFlags flags = model->flags(index);

   if (! (flags & Qt::ItemIsUserCheckable) || !(option.state & QStyle::State_Enabled)
         || ! (flags & Qt::ItemIsEnabled)) {
      return false;
   }

   // make sure that we have a check state
   QVariant value = index.data(Qt::CheckStateRole);
   if (! value.isValid()) {
      return false;
   }

   const QWidget *widget = QStyledItemDelegatePrivate::widget(option);
   QStyle *style = widget ? widget->style() : QApplication::style();

   // make sure that we have the right event type
   if ((event->type() == QEvent::MouseButtonRelease)
         || (event->type() == QEvent::MouseButtonDblClick) || (event->type() == QEvent::MouseButtonPress)) {

      QStyleOptionViewItem viewOpt(option);
      initStyleOption(&viewOpt, index);

      QRect checkRect = style->subElementRect(QStyle::SE_ItemViewItemCheckIndicator, &viewOpt, widget);
      QMouseEvent *me = static_cast<QMouseEvent *>(event);

      if (me->button() != Qt::LeftButton || !checkRect.contains(me->pos())) {
         return false;
      }

      if ((event->type() == QEvent::MouseButtonPress)
            || (event->type() == QEvent::MouseButtonDblClick)) {
         return true;
      }

   } else if (event->type() == QEvent::KeyPress) {
      if (static_cast<QKeyEvent *>(event)->key() != Qt::Key_Space
            && static_cast<QKeyEvent *>(event)->key() != Qt::Key_Select) {
         return false;
      }
   } else {
      return false;
   }

   Qt::CheckState state = static_cast<Qt::CheckState>(value.toInt());

   if (flags & Qt::ItemIsUserTristate) {
      state = ((Qt::CheckState)((state + 1) % 3));
   } else {
      state = (state == Qt::Checked) ? Qt::Unchecked : Qt::Checked;
   }

   return model->setData(index, state, Qt::CheckStateRole);
}

#endif // QT_NO_ITEMVIEWS
