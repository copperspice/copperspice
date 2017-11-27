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

#include <qstyleditemdelegate.h>

#ifndef QT_NO_ITEMVIEWS
#include <qabstractitemmodel.h>
#include <qapplication.h>
#include <qbrush.h>
#include <qlineedit.h>
#include <qtextedit.h>
#include <qplaintextedit.h>
#include <qpainter.h>
#include <qpalette.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <qstyle.h>
#include <qdatetime.h>
#include <qstyleoption.h>
#include <qevent.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qpixmapcache.h>
#include <qitemeditorfactory.h>
#include <qitemeditorfactory_p.h>
#include <qmetaobject.h>
#include <qtextlayout.h>
#include <qdnd_p.h>
#include <qtextengine_p.h>
#include <qlayoutengine_p.h>
#include <qdebug.h>
#include <qlocale.h>
#include <qdialog.h>
#include <qtableview.h>
#include <limits.h>

QT_BEGIN_NAMESPACE

class QStyledItemDelegatePrivate
{
   Q_DECLARE_PUBLIC(QStyledItemDelegate)

 public:
   QStyledItemDelegatePrivate() : factory(0) { }
   virtual ~QStyledItemDelegatePrivate() {}

   static const QWidget *widget(const QStyleOptionViewItem &option) {
      if (const QStyleOptionViewItemV3 *v3 = qstyleoption_cast<const QStyleOptionViewItemV3 *>(&option)) {
         return v3->widget;
      }
      return 0;
   }

   const QItemEditorFactory *editorFactory() const {
      return factory ? factory : QItemEditorFactory::defaultFactory();
   }

   void _q_commitDataAndCloseEditor(QWidget *editor) {
      Q_Q(QStyledItemDelegate);
      emit q->commitData(editor);
      emit q->closeEditor(editor, QAbstractItemDelegate::SubmitModelCache);
   }
   QItemEditorFactory *factory;

 protected:
   QStyledItemDelegate *q_ptr;

};

QStyledItemDelegate::QStyledItemDelegate(QObject *parent)
   : QAbstractItemDelegate(parent), d_ptr(new QStyledItemDelegatePrivate)
{
   d_ptr->q_ptr = this;
}

QStyledItemDelegate::~QStyledItemDelegate()
{
}

QString QStyledItemDelegate::displayText(const QVariant &value, const QLocale &locale) const
{
   QString text;

   switch (value.userType()) {
      case QMetaType::Float:
      case QVariant::Double:
         text = locale.toString(value.toReal());
         break;

      case QVariant::Int:
      case QVariant::LongLong:
         text = locale.toString(value.toLongLong());
         break;

      case QVariant::UInt:
      case QVariant::ULongLong:
         text = locale.toString(value.toULongLong());
         break;

      case QVariant::Date:
         text = locale.toString(value.toDate(), QLocale::ShortFormat);
         break;

      case QVariant::Time:
         text = locale.toString(value.toTime(), QLocale::ShortFormat);
         break;

      case QVariant::DateTime:
         text = locale.toString(value.toDateTime().date(), QLocale::ShortFormat);
         text += QLatin1Char(' ');
         text += locale.toString(value.toDateTime().time(), QLocale::ShortFormat);
         break;

      default:
         // convert new lines into line separators
         text = value.toString();
         for (int i = 0; i < text.count(); ++i) {
            if (text.at(i) == QLatin1Char('\n')) {
               text[i] = QChar::LineSeparator;
            }
         }
         break;
   }

   return text;
}

void QStyledItemDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
   QVariant value = index.data(Qt::FontRole);

   if (value.isValid() && ! value.isNull()) {
      option->font = qvariant_cast<QFont>(value).resolve(option->font);
      option->fontMetrics = QFontMetrics(option->font);
   }

   value = index.data(Qt::TextAlignmentRole);
   if (value.isValid() && !value.isNull()) {
      option->displayAlignment = Qt::Alignment(value.toInt());
   }

   value = index.data(Qt::ForegroundRole);
   if (value.canConvert<QBrush>()) {
      option->palette.setBrush(QPalette::Text, qvariant_cast<QBrush>(value));
   }

   if (QStyleOptionViewItemV4 *v4 = qstyleoption_cast<QStyleOptionViewItemV4 *>(option)) {
      v4->index = index;
      QVariant value = index.data(Qt::CheckStateRole);

      if (value.isValid() && ! value.isNull()) {
         v4->features |= QStyleOptionViewItemV2::HasCheckIndicator;
         v4->checkState = static_cast<Qt::CheckState>(value.toInt());
      }

      value = index.data(Qt::DecorationRole);

      if (value.isValid() && ! value.isNull()) {
         v4->features |= QStyleOptionViewItemV2::HasDecoration;

         switch (value.type()) {
            case QVariant::Icon: {
               v4->icon = qvariant_cast<QIcon>(value);
               QIcon::Mode mode;

               if (!(option->state & QStyle::State_Enabled)) {
                  mode = QIcon::Disabled;

               } else if (option->state & QStyle::State_Selected) {
                  mode = QIcon::Selected;

               } else {
                  mode = QIcon::Normal;
               }

               QIcon::State state = option->state & QStyle::State_Open ? QIcon::On : QIcon::Off;
               v4->decorationSize = v4->icon.actualSize(option->decorationSize, mode, state);
               break;
            }

            case QVariant::Color: {
               QPixmap pixmap(option->decorationSize);
               pixmap.fill(qvariant_cast<QColor>(value));
               v4->icon = QIcon(pixmap);
               break;
            }

            case QVariant::Image: {
               QImage image = qvariant_cast<QImage>(value);
               v4->icon = QIcon(QPixmap::fromImage(image));
               v4->decorationSize = image.size();
               break;
            }

            case QVariant::Pixmap: {
               QPixmap pixmap = qvariant_cast<QPixmap>(value);
               v4->icon = QIcon(pixmap);
               v4->decorationSize = pixmap.size();
               break;
            }
            default:
               break;
         }
      }

      value = index.data(Qt::DisplayRole);

      if (value.isValid() && ! value.isNull()) {
         v4->features |= QStyleOptionViewItemV2::HasDisplay;
         v4->text = displayText(value, v4->locale);
      }

      v4->backgroundBrush = qvariant_cast<QBrush>(index.data(Qt::BackgroundRole));
   }
}

void QStyledItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
   Q_ASSERT(index.isValid());

   QStyleOptionViewItemV4 opt = option;
   initStyleOption(&opt, index);

   const QWidget *widget = QStyledItemDelegatePrivate::widget(option);
   QStyle *style = widget ? widget->style() : QApplication::style();
   style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);
}

QSize QStyledItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
   QVariant value = index.data(Qt::SizeHintRole);

   if (value.isValid()) {
      return qvariant_cast<QSize>(value);
   }

   QStyleOptionViewItemV4 opt = option;
   initStyleOption(&opt, index);

   const QWidget *widget = QStyledItemDelegatePrivate::widget(option);
   QStyle *style = widget ? widget->style() : QApplication::style();

   return style->sizeFromContents(QStyle::CT_ItemViewItem, &opt, QSize(), widget);
}

/*!
    Returns the widget used to edit the item specified by \a index
    for editing. The \a parent widget and style \a option are used to
    control how the editor widget appears.

    \sa QAbstractItemDelegate::createEditor()
*/
QWidget *QStyledItemDelegate::createEditor(QWidget *parent,
      const QStyleOptionViewItem &,
      const QModelIndex &index) const
{
   Q_D(const QStyledItemDelegate);
   if (!index.isValid()) {
      return 0;
   }
   QVariant::Type t = static_cast<QVariant::Type>(index.data(Qt::EditRole).userType());
   return d->editorFactory()->createEditor(t, parent);
}

/*!
    Sets the data to be displayed and edited by the \a editor from the
    data model item specified by the model \a index.

    The default implementation stores the data in the \a editor
    widget's \l {Qt's Property System} {user property}.

    \sa QMetaProperty::isUser()
*/
void QStyledItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
#ifdef QT_NO_PROPERTIES
   Q_UNUSED(editor);
   Q_UNUSED(index);
#else
   Q_D(const QStyledItemDelegate);
   QVariant v = index.data(Qt::EditRole);
   QByteArray n = editor->metaObject()->userProperty().name();

   // ### Qt5: remove
   // A work-around for missing "USER true" in qdatetimeedit.h for
   // QTimeEdit's time property and QDateEdit's date property.
   // It only triggers if the default user property "dateTime" is
   // reported for QTimeEdit and QDateEdit.
   if (n == "dateTime") {
      if (editor->inherits("QTimeEdit")) {
         n = "time";
      } else if (editor->inherits("QDateEdit")) {
         n = "date";
      }
   }

   // ### Qt5: give QComboBox a USER property
   if (n.isEmpty() && editor->inherits("QComboBox")) {
      n = d->editorFactory()->valuePropertyName(static_cast<QVariant::Type>(v.userType()));
   }
   if (!n.isEmpty()) {
      if (!v.isValid()) {
         v = QVariant(editor->property(n).userType(), (const void *)0);
      }
      editor->setProperty(n, v);
   }
#endif
}

void QStyledItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
#ifdef QT_NO_PROPERTIES
   Q_UNUSED(model);
   Q_UNUSED(editor);
   Q_UNUSED(index);
#else

   Q_D(const QStyledItemDelegate);
   Q_ASSERT(model);
   Q_ASSERT(editor);

   QByteArray n = editor->metaObject()->userProperty().name();

   if (n.isEmpty()) {
      n = d->editorFactory()->valuePropertyName(static_cast<QVariant::Type>(model->data(index, Qt::EditRole).userType()));
   }

   if (!n.isEmpty()) {
      model->setData(index, editor->property(n), Qt::EditRole);
   }
#endif
}

/*!
    Updates the \a editor for the item specified by \a index
    according to the style \a option given.
*/
void QStyledItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
   if (!editor) {
      return;
   }

   Q_ASSERT(index.isValid());
   const QWidget *widget = QStyledItemDelegatePrivate::widget(option);

   QStyleOptionViewItemV4 opt = option;
   initStyleOption(&opt, index);

   // let the editor take up all available space
   //if the editor is not a QLineEdit or it is in a QTableView

#if !defined(QT_NO_TABLEVIEW) && !defined(QT_NO_LINEEDIT)
   if (qobject_cast<QExpandingLineEdit *>(editor) && !qobject_cast<const QTableView *>(widget)) {
      opt.showDecorationSelected = editor->style()->styleHint(QStyle::SH_ItemView_ShowDecorationSelected, 0, editor);
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

/*!
  Returns the editor factory used by the item delegate.
  If no editor factory is set, the function will return null.

  \sa setItemEditorFactory()
*/
QItemEditorFactory *QStyledItemDelegate::itemEditorFactory() const
{
   Q_D(const QStyledItemDelegate);
   return d->factory;
}

/*!
  Sets the editor factory to be used by the item delegate to be the \a factory
  specified. If no editor factory is set, the item delegate will use the
  default editor factory.

  \sa itemEditorFactory()
*/
void QStyledItemDelegate::setItemEditorFactory(QItemEditorFactory *factory)
{
   Q_D(QStyledItemDelegate);
   d->factory = factory;
}

bool QStyledItemDelegate::eventFilter(QObject *object, QEvent *event)
{
   QWidget *editor = qobject_cast<QWidget *>(object);
   if (!editor) {
      return false;
   }

   if (event->type() == QEvent::KeyPress) {
      switch (static_cast<QKeyEvent *>(event)->key()) {
         case Qt::Key_Tab:
            emit commitData(editor);
            emit closeEditor(editor, QAbstractItemDelegate::EditNextItem);
            return true;

         case Qt::Key_Backtab:
            emit commitData(editor);
            emit closeEditor(editor, QAbstractItemDelegate::EditPreviousItem);
            return true;

         case Qt::Key_Enter:
         case Qt::Key_Return:
#ifndef QT_NO_TEXTEDIT
            if (qobject_cast<QTextEdit *>(editor) || qobject_cast<QPlainTextEdit *>(editor)) {
               return false;   // don't filter enter key events for QTextEdit
            }
            // We want the editor to be able to process the key press
            // before committing the data (e.g. so it can do
            // validation/fixup of the input).
#endif

#ifndef QT_NO_LINEEDIT
            if (QLineEdit *e = qobject_cast<QLineEdit *>(editor))
               if (!e->hasAcceptableInput()) {
                  return false;
               }
#endif
            QMetaObject::invokeMethod(this, "_q_commitDataAndCloseEditor", Qt::QueuedConnection, Q_ARG(QWidget *, editor));
            return false;

         case Qt::Key_Escape:
            // don't commit data
            emit closeEditor(editor, QAbstractItemDelegate::RevertModelCache);
            break;
         default:
            return false;
      }

      if (editor->parentWidget()) {
         editor->parentWidget()->setFocus();
      }
      return true;

   } else if (event->type() == QEvent::FocusOut || (event->type() == QEvent::Hide && editor->isWindow())) {
      //the Hide event will take care of he editors that are in fact complete dialogs
      if (!editor->isActiveWindow() || (QApplication::focusWidget() != editor)) {
         QWidget *w = QApplication::focusWidget();
         while (w) { // don't worry about focus changes internally in the editor
            if (w == editor) {
               return false;
            }
            w = w->parentWidget();
         }
#ifndef QT_NO_DRAGANDDROP
         // The window may lose focus during an drag operation.
         // i.e when dragging involves the taskbar on Windows.
         if (QDragManager::self() && QDragManager::self()->object != 0) {
            return false;
         }
#endif

         emit commitData(editor);
         emit closeEditor(editor, NoHint);
      }
   } else if (event->type() == QEvent::ShortcutOverride) {
      if (static_cast<QKeyEvent *>(event)->key() == Qt::Key_Escape) {
         event->accept();
         return true;
      }
   }
   return false;
}

/*!
  \reimp
*/
bool QStyledItemDelegate::editorEvent(QEvent *event,
                                      QAbstractItemModel *model,
                                      const QStyleOptionViewItem &option,
                                      const QModelIndex &index)
{
   Q_ASSERT(event);
   Q_ASSERT(model);

   // make sure that the item is checkable
   Qt::ItemFlags flags = model->flags(index);
   if (!(flags & Qt::ItemIsUserCheckable) || !(option.state & QStyle::State_Enabled)
         || !(flags & Qt::ItemIsEnabled)) {
      return false;
   }

   // make sure that we have a check state
   QVariant value = index.data(Qt::CheckStateRole);
   if (!value.isValid()) {
      return false;
   }

   const QWidget *widget = QStyledItemDelegatePrivate::widget(option);
   QStyle *style = widget ? widget->style() : QApplication::style();

   // make sure that we have the right event type
   if ((event->type() == QEvent::MouseButtonRelease)
         || (event->type() == QEvent::MouseButtonDblClick)
         || (event->type() == QEvent::MouseButtonPress)) {
      QStyleOptionViewItemV4 viewOpt(option);
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

   Qt::CheckState state = (static_cast<Qt::CheckState>(value.toInt()) == Qt::Checked
                           ? Qt::Unchecked : Qt::Checked);
   return model->setData(index, state, Qt::CheckStateRole);
}

void QStyledItemDelegate::_q_commitDataAndCloseEditor(QWidget *un_named_arg1)
{
   Q_D(QStyledItemDelegate);
   d->_q_commitDataAndCloseEditor(un_named_arg1);
}

QT_END_NAMESPACE

#endif // QT_NO_ITEMVIEWS
