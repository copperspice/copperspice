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

#include <qitemdelegate.h>

#ifndef QT_NO_ITEMVIEWS
#include <qabstractitemmodel.h>
#include <qapplication.h>
#include <qbrush.h>

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
#include <qmetaobject.h>
#include <qtextlayout.h>

#include <qabstractitemdelegate_p.h>
#include <qtextengine_p.h>
#include <qdebug.h>
#include <qlocale.h>
#include <qdialog.h>
#include <qmath.h>
#include <limits.h>

#ifndef DBL_DIG
#  define DBL_DIG 10
#endif

class QItemDelegatePrivate : public QAbstractItemDelegatePrivate
{
   Q_DECLARE_PUBLIC(QItemDelegate)

 public:
   QItemDelegatePrivate()
      : f(nullptr), clipPainting(true)
   {
   }

   inline const QItemEditorFactory *editorFactory() const {
      return f ? f : QItemEditorFactory::defaultFactory();
   }

   inline QIcon::Mode iconMode(QStyle::State state) const {
      if (! (state & QStyle::State_Enabled)) {
         return QIcon::Disabled;
      }

      if (state & QStyle::State_Selected) {
         return QIcon::Selected;
      }

      return QIcon::Normal;
   }

   inline QIcon::State iconState(QStyle::State state) const {
      return state & QStyle::State_Open ? QIcon::On : QIcon::Off;
   }

   static inline QString replaceNewLine(QString text) {
      const QChar ch = QChar::LineSeparator;
      text.replace('\n', ch);

      return text;
   }

   QString valueToText(const QVariant &value, const QStyleOptionViewItem &option) const;

   QItemEditorFactory *f;
   bool clipPainting;

   QRect textLayoutBounds(const QStyleOptionViewItem &options) const;
   QSizeF doTextLayout(int lineWidth) const;
   mutable QTextLayout textLayout;
   mutable QTextOption textOption;

   const QWidget *widget(const QStyleOptionViewItem &option) const {

      return option.widget;
   }

   // ### emerald, temporary until we have QStandardItemDelegate
   mutable struct Icon {
      QIcon icon;
      QIcon::Mode mode;
      QIcon::State state;
   } tmp;
};

QRect QItemDelegatePrivate::textLayoutBounds(const QStyleOptionViewItem &option) const
{
   QRect rect = option.rect;
   const bool wrapText = option.features & QStyleOptionViewItem::WrapText;

   switch (option.decorationPosition) {
      case QStyleOptionViewItem::Left:
      case QStyleOptionViewItem::Right:
         rect.setWidth(wrapText && rect.isValid() ? rect.width() : (QFIXED_MAX));
         break;

      case QStyleOptionViewItem::Top:
      case QStyleOptionViewItem::Bottom:
         rect.setWidth(wrapText ? option.decorationSize.width() : (QFIXED_MAX));
         break;
   }

   return rect;
}

QSizeF QItemDelegatePrivate::doTextLayout(int lineWidth) const
{
   qreal height    = 0;
   qreal widthUsed = 0;

   textLayout.beginLayout();

   while (true) {
      QTextLine line = textLayout.createLine();

      if (! line.isValid()) {
         break;
      }

      line.setLineWidth(lineWidth);
      line.setPosition(QPointF(0, height));
      height += line.height();
      widthUsed = qMax(widthUsed, line.naturalTextWidth());
   }

   textLayout.endLayout();

   return QSizeF(widthUsed, height);
}

QItemDelegate::QItemDelegate(QObject *parent)
   : QAbstractItemDelegate(*new QItemDelegatePrivate(), parent)
{
}

QItemDelegate::~QItemDelegate()
{
}

bool QItemDelegate::hasClipping() const
{
   Q_D(const QItemDelegate);
   return d->clipPainting;
}

void QItemDelegate::setClipping(bool clip)
{
   Q_D(QItemDelegate);
   d->clipPainting = clip;
}

QString QItemDelegatePrivate::valueToText(const QVariant &value, const QStyleOptionViewItem &option) const
{
   return textForRole(Qt::DisplayRole, value, option.locale, DBL_DIG);
}

void QItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
   Q_D(const QItemDelegate);
   Q_ASSERT(index.isValid());

   QStyleOptionViewItem opt = setOptions(index, option);

   // prepare
   painter->save();
   if (d->clipPainting) {
      painter->setClipRect(opt.rect);
   }

   // get the data and the rectangles
   QVariant value;

   QPixmap pixmap;
   QRect decorationRect;
   value = index.data(Qt::DecorationRole);

   if (value.isValid()) {
      // ### need the pixmap to call the virtual function
      pixmap = decoration(opt, value);

      if (value.type() == QVariant::Icon) {
         d->tmp.icon  = value.value<QIcon>();
         d->tmp.mode = d->iconMode(option.state);
         d->tmp.state = d->iconState(option.state);

         const QSize size = d->tmp.icon.actualSize(option.decorationSize, d->tmp.mode, d->tmp.state);

         decorationRect = QRect(QPoint(0, 0), size);
      } else {
         d->tmp.icon = QIcon();
         decorationRect = QRect(QPoint(0, 0), pixmap.size());
      }

   } else {
      d->tmp.icon = QIcon();
      decorationRect = QRect();
   }

   QString text;
   QRect displayRect;
   value = index.data(Qt::DisplayRole);

   if (value.isValid()) {
      text = d->valueToText(value, opt);
      displayRect = textRectangle(painter, d->textLayoutBounds(opt), opt.font, text);
   }

   QRect checkRect;
   Qt::CheckState checkState = Qt::Unchecked;
   value = index.data(Qt::CheckStateRole);

   if (value.isValid()) {
      checkState = static_cast<Qt::CheckState>(value.toInt());
      checkRect  = doCheck(opt, opt.rect, value);
   }

   // do the layout
   doLayout(opt, &checkRect, &decorationRect, &displayRect, false);

   // draw the item
   drawBackground(painter, opt, index);
   drawCheck(painter, opt, checkRect, checkState);
   drawDecoration(painter, opt, decorationRect, pixmap);
   drawDisplay(painter, opt, displayRect, text);
   drawFocus(painter, opt, displayRect);

   // done
   painter->restore();
}

QSize QItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
   QVariant value = index.data(Qt::SizeHintRole);

   if (value.isValid()) {
      return value.value<QSize>();
   }

   QRect decorationRect = rect(option, index, Qt::DecorationRole);
   QRect displayRect    = rect(option, index, Qt::DisplayRole);
   QRect checkRect      = rect(option, index, Qt::CheckStateRole);

   doLayout(option, &checkRect, &decorationRect, &displayRect, true);

   return (decorationRect | displayRect | checkRect).size();
}

QWidget *QItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &,
            const QModelIndex &index) const
{
   Q_D(const QItemDelegate);

   if (! index.isValid()) {
      return nullptr;
   }

   const QItemEditorFactory *factory = d->f;

   if (factory == nullptr) {
      factory = QItemEditorFactory::defaultFactory();
   }

   QVariant::Type t = static_cast<QVariant::Type>(index.data(Qt::EditRole).userType());
   QWidget *w = factory->createEditor(t, parent);

   if (w) {
      w->setFocusPolicy(Qt::WheelFocus);
   }

   return w;
}

void QItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
#ifndef QT_NO_PROPERTIES
   QVariant v = index.data(Qt::EditRole);
   QString n  = editor->metaObject()->userProperty().name();

   if (! n.isEmpty()) {
      if (! v.isValid())  {
         v = QVariant(editor->property(n).userType(), (const void *)nullptr);
      }

      editor->setProperty(n, v);
   }
#endif
}

void QItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
#if ! defined(QT_NO_PROPERTIES)
   Q_D(const QItemDelegate);
   Q_ASSERT(model);
   Q_ASSERT(editor);

   QString n = editor->metaObject()->userProperty().name();

   if (n.isEmpty()) {
      n = d->editorFactory()->valuePropertyName(static_cast<QVariant::Type>(model->data(index, Qt::EditRole).userType()));
   }

   if (!n.isEmpty()) {
      model->setData(index, editor->property(n), Qt::EditRole);
   }
#endif
}

void QItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
   if (! editor) {
      return;
   }

   Q_ASSERT(index.isValid());

   QPixmap pixmap = decoration(option, index.data(Qt::DecorationRole));
   QString text = QItemDelegatePrivate::replaceNewLine(index.data(Qt::DisplayRole).toString());
   QRect pixmapRect = QRect(QPoint(0, 0), option.decorationSize).intersected(pixmap.rect());
   QRect textRect = textRectangle(nullptr, option.rect, option.font, text);
   QRect checkRect = doCheck(option, textRect, index.data(Qt::CheckStateRole));

   QStyleOptionViewItem opt = option;
   opt.showDecorationSelected = true; // let the editor take up all available space
   doLayout(opt, &checkRect, &pixmapRect, &textRect, false);
   editor->setGeometry(textRect);
}

QItemEditorFactory *QItemDelegate::itemEditorFactory() const
{
   Q_D(const QItemDelegate);
   return d->f;
}

void QItemDelegate::setItemEditorFactory(QItemEditorFactory *factory)
{
   Q_D(QItemDelegate);
   d->f = factory;
}

void QItemDelegate::drawDisplay(QPainter *painter, const QStyleOptionViewItem &option,
   const QRect &rect, const QString &text) const
{
   Q_D(const QItemDelegate);

   QPalette::ColorGroup cg = option.state & QStyle::State_Enabled ? QPalette::Normal : QPalette::Disabled;

   if (cg == QPalette::Normal && !(option.state & QStyle::State_Active)) {
      cg = QPalette::Inactive;
   }

   if (option.state & QStyle::State_Selected) {
      painter->fillRect(rect, option.palette.brush(cg, QPalette::Highlight));
      painter->setPen(option.palette.color(cg, QPalette::HighlightedText));

   } else {
      painter->setPen(option.palette.color(cg, QPalette::Text));
   }

   if (text.isEmpty()) {
      return;
   }

   if (option.state & QStyle::State_Editing) {
      painter->save();
      painter->setPen(option.palette.color(cg, QPalette::Text));
      painter->drawRect(rect.adjusted(0, 0, -1, -1));
      painter->restore();
   }

   const QStyleOptionViewItem opt = option;

   const QWidget *widget = d->widget(option);
   QStyle *style = widget ? widget->style() : QApplication::style();

   const int textMargin = style->pixelMetric(QStyle::PM_FocusFrameHMargin, nullptr, widget) + 1;
   QRect textRect = rect.adjusted(textMargin, 0, -textMargin, 0); // remove width padding

   const bool wrapText = opt.features & QStyleOptionViewItem::WrapText;
   d->textOption.setWrapMode(wrapText ? QTextOption::WordWrap : QTextOption::ManualWrap);
   d->textOption.setTextDirection(option.direction);
   d->textOption.setAlignment(QStyle::visualAlignment(option.direction, option.displayAlignment));
   d->textLayout.setTextOption(d->textOption);
   d->textLayout.setFont(option.font);
   d->textLayout.setText(QItemDelegatePrivate::replaceNewLine(text));

   QSizeF textLayoutSize = d->doTextLayout(textRect.width());

   if (textRect.width() < textLayoutSize.width() || textRect.height() < textLayoutSize.height()) {
      QString elided;
      int start = 0;
      int end   = text.indexOf(QChar(QChar::LineSeparator), start);

      if (end == -1) {
         elided += option.fontMetrics.elidedText(text, option.textElideMode, textRect.width());

      } else {
         while (end != -1) {
            elided += option.fontMetrics.elidedText(text.mid(start, end - start), option.textElideMode, textRect.width());
            elided += QChar::LineSeparator;
            start = end + 1;
            end = text.indexOf(QChar(QChar::LineSeparator), start);
         }

         // add the last line (after the last QChar::LineSeparator)
         elided += option.fontMetrics.elidedText(text.mid(start), option.textElideMode, textRect.width());
      }

      d->textLayout.setText(elided);
      textLayoutSize = d->doTextLayout(textRect.width());
   }

   const QSize layoutSize(textRect.width(), int(textLayoutSize.height()));
   const QRect layoutRect = QStyle::alignedRect(option.direction, option.displayAlignment, layoutSize, textRect);

   // if we still overflow even after eliding the text, enable clipping
   if (!hasClipping() && (textRect.width() < textLayoutSize.width() || textRect.height() < textLayoutSize.height())) {
      painter->save();
      painter->setClipRect(layoutRect);
      d->textLayout.draw(painter, layoutRect.topLeft(), QVector<QTextLayout::FormatRange>(), layoutRect);
      painter->restore();

   } else {
      d->textLayout.draw(painter, layoutRect.topLeft(), QVector<QTextLayout::FormatRange>(), layoutRect);
   }
}

void QItemDelegate::drawDecoration(QPainter *painter, const QStyleOptionViewItem &option,
   const QRect &rect, const QPixmap &pixmap) const
{
   Q_D(const QItemDelegate);

   // if we have an icon, we ignore the pixmap
   if (!d->tmp.icon.isNull()) {
      d->tmp.icon.paint(painter, rect, option.decorationAlignment, d->tmp.mode, d->tmp.state);
      return;
   }

   if (pixmap.isNull() || !rect.isValid()) {
      return;
   }
   QPoint p = QStyle::alignedRect(option.direction, option.decorationAlignment, pixmap.size(), rect).topLeft();

   if (option.state & QStyle::State_Selected) {
      QPixmap *pm = selected(pixmap, option.palette, option.state & QStyle::State_Enabled);
      painter->drawPixmap(p, *pm);
   } else {
      painter->drawPixmap(p, pixmap);
   }
}

void QItemDelegate::drawFocus(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect) const
{
   Q_D(const QItemDelegate);

   if ((option.state & QStyle::State_HasFocus) == 0 || !rect.isValid()) {
      return;
   }

   QStyleOptionFocusRect o;
   o.QStyleOption::operator=(option);
   o.rect = rect;
   o.state |= QStyle::State_KeyboardFocusChange;
   o.state |= QStyle::State_Item;
   QPalette::ColorGroup cg = (option.state & QStyle::State_Enabled)
      ? QPalette::Normal : QPalette::Disabled;
   o.backgroundColor = option.palette.color(cg, (option.state & QStyle::State_Selected)
         ? QPalette::Highlight : QPalette::Window);
   const QWidget *widget = d->widget(option);
   QStyle *style = widget ? widget->style() : QApplication::style();
   style->drawPrimitive(QStyle::PE_FrameFocusRect, &o, painter, widget);
}

void QItemDelegate::drawCheck(QPainter *painter, const QStyleOptionViewItem &option,
                  const QRect &rect, Qt::CheckState state) const
{
   Q_D(const QItemDelegate);
   if (!rect.isValid()) {
      return;
   }

   QStyleOptionViewItem opt(option);
   opt.rect = rect;
   opt.state = opt.state & ~QStyle::State_HasFocus;

   switch (state) {
      case Qt::Unchecked:
         opt.state |= QStyle::State_Off;
         break;
      case Qt::PartiallyChecked:
         opt.state |= QStyle::State_NoChange;
         break;
      case Qt::Checked:
         opt.state |= QStyle::State_On;
         break;
   }

   const QWidget *widget = d->widget(option);
   QStyle *style = widget ? widget->style() : QApplication::style();
   style->drawPrimitive(QStyle::PE_IndicatorViewItemCheck, &opt, painter, widget);
}

void QItemDelegate::drawBackground(QPainter *painter, const QStyleOptionViewItem &option,
   const QModelIndex &index) const
{
   if (option.showDecorationSelected && (option.state & QStyle::State_Selected)) {
      QPalette::ColorGroup cg = option.state & QStyle::State_Enabled
         ? QPalette::Normal : QPalette::Disabled;

      if (cg == QPalette::Normal && !(option.state & QStyle::State_Active)) {
         cg = QPalette::Inactive;
      }

      painter->fillRect(option.rect, option.palette.brush(cg, QPalette::Highlight));
   } else {
      QVariant value = index.data(Qt::BackgroundRole);
      if (value.canConvert<QBrush>()) {
         QPointF oldBO = painter->brushOrigin();
         painter->setBrushOrigin(option.rect.topLeft());
         painter->fillRect(option.rect, value.value<QBrush>());
         painter->setBrushOrigin(oldBO);
      }
   }
}

void QItemDelegate::doLayout(const QStyleOptionViewItem &option,
   QRect *checkRect, QRect *pixmapRect, QRect *textRect, bool hint) const
{
   Q_ASSERT(checkRect && pixmapRect && textRect);
   Q_D(const QItemDelegate);

   const QWidget *widget = d->widget(option);
   QStyle *style = widget ? widget->style() : QApplication::style();

   const bool hasCheck    = checkRect->isValid();
   const bool hasPixmap   = pixmapRect->isValid();
   const bool hasText     = textRect->isValid();
   const int textMargin   = hasText   ? style->pixelMetric(QStyle::PM_FocusFrameHMargin, nullptr, widget) + 1 : 0;
   const int pixmapMargin = hasPixmap ? style->pixelMetric(QStyle::PM_FocusFrameHMargin, nullptr, widget) + 1 : 0;
   const int checkMargin  = hasCheck  ? style->pixelMetric(QStyle::PM_FocusFrameHMargin, nullptr, widget) + 1 : 0;

   int x = option.rect.left();
   int y = option.rect.top();
   int w;
   int h;

   textRect->adjust(-textMargin, 0, textMargin, 0); // add width padding

   if (textRect->height() == 0 && (! hasPixmap || ! hint)) {
      // if there is no text, we still want to have a decent height for the item sizeHint and the editor size
      textRect->setHeight(option.fontMetrics.height());
   }

   QSize pm(0, 0);
   if (hasPixmap) {
      pm = pixmapRect->size();
      pm.rwidth() += 2 * pixmapMargin;
   }

   if (hint) {
      h = qMax(checkRect->height(), qMax(textRect->height(), pm.height()));
      if (option.decorationPosition == QStyleOptionViewItem::Left
         || option.decorationPosition == QStyleOptionViewItem::Right) {
         w = textRect->width() + pm.width();
      } else {
         w = qMax(textRect->width(), pm.width());
      }

   } else {
      w = option.rect.width();
      h = option.rect.height();
   }

   int cw = 0;
   QRect check;

   if (hasCheck) {
      cw = checkRect->width() + 2 * checkMargin;

      if (hint) {
         w += cw;
      }

      if (option.direction == Qt::RightToLeft) {
         check.setRect(x + w - cw, y, cw, h);
      } else {
         check.setRect(x + checkMargin, y, cw, h);
      }
   }

   // at this point w should be the *total* width

   QRect display;
   QRect decoration;

   switch (option.decorationPosition) {
      case QStyleOptionViewItem::Top: {
         if (hasPixmap) {
            pm.setHeight(pm.height() + pixmapMargin);   // add space
         }

         h = hint ? textRect->height() : h - pm.height();

         if (option.direction == Qt::RightToLeft) {
            decoration.setRect(x, y, w - cw, pm.height());
            display.setRect(x, y + pm.height(), w - cw, h);
         } else {
            decoration.setRect(x + cw, y, w - cw, pm.height());
            display.setRect(x + cw, y + pm.height(), w - cw, h);
         }
         break;
      }

      case QStyleOptionViewItem::Bottom: {
         if (hasText) {
            textRect->setHeight(textRect->height() + textMargin);   // add space
         }

         h = hint ? textRect->height() + pm.height() : h;

         if (option.direction == Qt::RightToLeft) {
            display.setRect(x, y, w - cw, textRect->height());
            decoration.setRect(x, y + textRect->height(), w - cw, h - textRect->height());
         } else {
            display.setRect(x + cw, y, w - cw, textRect->height());
            decoration.setRect(x + cw, y + textRect->height(), w - cw, h - textRect->height());
         }
         break;
      }

      case QStyleOptionViewItem::Left: {
         if (option.direction == Qt::LeftToRight) {
            decoration.setRect(x + cw, y, pm.width(), h);
            display.setRect(decoration.right() + 1, y, w - pm.width() - cw, h);
         } else {
            display.setRect(x, y, w - pm.width() - cw, h);
            decoration.setRect(display.right() + 1, y, pm.width(), h);
         }
         break;
      }

      case QStyleOptionViewItem::Right: {
         if (option.direction == Qt::LeftToRight) {
            display.setRect(x + cw, y, w - pm.width() - cw, h);
            decoration.setRect(display.right() + 1, y, pm.width(), h);
         } else {
            decoration.setRect(x, y, pm.width(), h);
            display.setRect(decoration.right() + 1, y, w - pm.width() - cw, h);
         }
         break;
      }

      default:
         qWarning("QItemDelegate::doLayout() Decoration position is invalid");
         decoration = *pixmapRect;
         break;
   }

   if (! hint) {
      // only need to do the internal layout if we are going to paint

      *checkRect = QStyle::alignedRect(option.direction, Qt::AlignCenter,
            checkRect->size(), check);

      *pixmapRect = QStyle::alignedRect(option.direction, option.decorationAlignment,
            pixmapRect->size(), decoration);

      // the text takes up all available space, unless the decoration is not shown as selected
      if (option.showDecorationSelected) {
         *textRect = display;
      } else {
         *textRect = QStyle::alignedRect(option.direction, option.displayAlignment,
               textRect->size().boundedTo(display.size()), display);
      }

   } else {
      *checkRect  = check;
      *pixmapRect = decoration;
      *textRect   = display;
   }
}

QPixmap QItemDelegate::decoration(const QStyleOptionViewItem &option, const QVariant &variant) const
{
   Q_D(const QItemDelegate);

   switch (variant.type()) {
      case QVariant::Icon: {
         QIcon::Mode mode = d->iconMode(option.state);
         QIcon::State state = d->iconState(option.state);
         return variant.value<QIcon>().pixmap(option.decorationSize, mode, state);
      }

      case QVariant::Color: {
         static QPixmap pixmap(option.decorationSize);
         pixmap.fill(variant.value<QColor>());
         return pixmap;
      }

      default:
         break;
   }

   return variant.value<QPixmap>();
}

// hacky but faster version of "QString::sprintf("%d-%d", i, enabled)"
static QString cs_internal_PixmapSerial(quint64 i, bool enabled)
{
   char32_t arr[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', char32_t('0' + enabled) };
   char32_t *ptr  = std::end(arr);

   while (i > 0) {
      // use ascii character after '9' instead of 'a' for hex
      --ptr;

      *ptr = '0' + i % 16;
      i >>= 4;
   }

   return QString(ptr, std::end(arr));
}

QPixmap *QItemDelegate::selected(const QPixmap &pixmap, const QPalette &palette, bool enabled) const
{
   QString key = cs_internal_PixmapSerial(pixmap.cacheKey(), enabled);
   QPixmap *pm = QPixmapCache::find(key);

   if (! pm) {
      QImage img = pixmap.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);

      QColor color = palette.color(enabled ? QPalette::Normal : QPalette::Disabled, QPalette::Highlight);
      color.setAlphaF((qreal)0.3);

      QPainter painter(&img);
      painter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
      painter.fillRect(0, 0, img.width(), img.height(), color);
      painter.end();

      QPixmap selected = QPixmap(QPixmap::fromImage(img));
      int n = (img.byteCount() >> 10) + 1;
      if (QPixmapCache::cacheLimit() < n) {
         QPixmapCache::setCacheLimit(n);
      }

      QPixmapCache::insert(key, selected);
      pm = QPixmapCache::find(key);
   }

   return pm;
}

QRect QItemDelegate::rect(const QStyleOptionViewItem &option, const QModelIndex &index, int role) const
{
   Q_D(const QItemDelegate);
   QVariant value = index.data(role);

   if (role == Qt::CheckStateRole) {
      return doCheck(option, option.rect, value);
   }

   if (value.isValid()) {
      switch (value.type()) {
         case QVariant::Invalid:
            break;

         case QVariant::Pixmap: {
            const QPixmap &pixmap = value.value<QPixmap>();
            return QRect(QPoint(0, 0), pixmap.size() / pixmap.devicePixelRatio() );
         }

         case QVariant::Image: {
            const QImage &image = value.value<QImage>();
            return QRect(QPoint(0, 0), image.size() /  image.devicePixelRatio() );
         }

         case QVariant::Icon: {
            QIcon::Mode mode = d->iconMode(option.state);
            QIcon::State state = d->iconState(option.state);
            QIcon icon = value.value<QIcon>();
            QSize size = icon.actualSize(option.decorationSize, mode, state);
            return QRect(QPoint(0, 0), size);
         }

         case QVariant::Color:
            return QRect(QPoint(0, 0), option.decorationSize);

         case QVariant::String:
         default: {
            const QString text = d->valueToText(value, option);
            value = index.data(Qt::FontRole);

            QFont fnt = value.value<QFont>().resolve(option.font);

            return textRectangle(nullptr, d->textLayoutBounds(option), fnt, text);
         }
      }
   }

   return QRect();
}

QRect QItemDelegate::doCheck(const QStyleOptionViewItem &option,
            const QRect &bounding, const QVariant &value) const
{
   if (value.isValid()) {
      Q_D(const QItemDelegate);

      QStyleOptionButton opt;
      opt.QStyleOption::operator=(option);
      opt.rect = bounding;

      const QWidget *widget = d->widget(option);
      QStyle *style = widget ? widget->style() : QApplication::style();
      return style->subElementRect(QStyle::SE_ItemViewItemCheckIndicator, &opt, widget);
   }

   return QRect();
}

QRect QItemDelegate::textRectangle(QPainter *, const QRect &rect,
   const QFont &font, const QString &text) const
{
   Q_D(const QItemDelegate);

   d->textOption.setWrapMode(QTextOption::WordWrap);
   d->textLayout.setTextOption(d->textOption);
   d->textLayout.setFont(font);
   d->textLayout.setText(QItemDelegatePrivate::replaceNewLine(text));

   QSizeF fpSize = d->doTextLayout(rect.width());
   const QSize size = QSize(qCeil(fpSize.width()), qCeil(fpSize.height()));

   // ###: textRectangle should take style option as argument
   const int textMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;

   return QRect(0, 0, size.width() + 2 * textMargin, size.height());
}

bool QItemDelegate::eventFilter(QObject *object, QEvent *event)
{
   Q_D(QItemDelegate);
   return d->editorEventFilter(object, event);
}

bool QItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,
   const QStyleOptionViewItem &option, const QModelIndex &index)
{
   Q_ASSERT(event);
   Q_ASSERT(model);

   // make sure the item is checkable
   Qt::ItemFlags flags = model->flags(index);
   if (!(flags & Qt::ItemIsUserCheckable) || !(option.state & QStyle::State_Enabled)
      || ! (flags & Qt::ItemIsEnabled)) {

      return false;
   }

   // make sure that we have a check state
   QVariant value = index.data(Qt::CheckStateRole);
   if (!value.isValid()) {
      return false;
   }

   // make sure that we have the right event type
   if ((event->type() == QEvent::MouseButtonRelease)
      || (event->type() == QEvent::MouseButtonDblClick)
      || (event->type() == QEvent::MouseButtonPress)) {
      QRect checkRect = doCheck(option, option.rect, Qt::Checked);
      QRect emptyRect;
      doLayout(option, &checkRect, &emptyRect, &emptyRect, false);
      QMouseEvent *me = static_cast<QMouseEvent *>(event);
      if (me->button() != Qt::LeftButton || !checkRect.contains(me->pos())) {
         return false;
      }

      // eat the double click events inside the check rect
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

QStyleOptionViewItem QItemDelegate::setOptions(const QModelIndex &index,
      const QStyleOptionViewItem &option) const
{
   QStyleOptionViewItem opt = option;

   // set font
   QVariant value = index.data(Qt::FontRole);
   if (value.isValid()) {
      opt.font = value.value<QFont>().resolve(opt.font);
      opt.fontMetrics = QFontMetrics(opt.font);
   }

   // set text alignment
   value = index.data(Qt::TextAlignmentRole);
   if (value.isValid()) {
      opt.displayAlignment = Qt::Alignment(value.toInt());
   }

   // set foreground brush
   value = index.data(Qt::ForegroundRole);
   if (value.canConvert<QBrush>()) {
      opt.palette.setBrush(QPalette::Text, value.value<QBrush>());
   }

   // disable style animations for checkboxes etc. within itemviews (QTBUG-30146)
   opt.styleObject = nullptr;

   return opt;
}

#endif // QT_NO_ITEMVIEWS
