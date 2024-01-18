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

#include <qtoolbox.h>

#ifndef QT_NO_TOOLBOX

#include <qapplication.h>
#include <qeventloop.h>
#include <qlayout.h>
#include <qlist.h>
#include <qpainter.h>
#include <qscrollarea.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qtooltip.h>
#include <qabstractbutton.h>

#include <qframe_p.h>

class QToolBoxButton : public QAbstractButton
{
   GUI_CS_OBJECT(QToolBoxButton)

 public:
   QToolBoxButton(QWidget *parent)
      : QAbstractButton(parent), selected(false), indexInPage(-1) {
      setBackgroundRole(QPalette::Window);
      setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
      setFocusPolicy(Qt::NoFocus);
   }

   inline void setSelected(bool b) {
      selected = b;
      update();
   }

   inline void setIndex(int newIndex) {
      indexInPage = newIndex;
   }

   QSize sizeHint() const override;
   QSize minimumSizeHint() const override;

 protected:
   void initStyleOption(QStyleOptionToolBox *opt) const;
   void paintEvent(QPaintEvent *) override;

 private:
   bool selected;
   int indexInPage;
};


class QToolBoxPrivate : public QFramePrivate
{
   Q_DECLARE_PUBLIC(QToolBox)
 public:
   struct Page {
      QToolBoxButton *button;
      QScrollArea *sv;
      QWidget *widget;

      inline void setText(const QString &text) {
         button->setText(text);
      }
      inline void setIcon(const QIcon &is) {
         button->setIcon(is);
      }

#ifndef QT_NO_TOOLTIP
      inline void setToolTip(const QString &tip) {
         button->setToolTip(tip);
      }
      inline QString toolTip() const {
         return button->toolTip();
      }
#endif
      inline QString text() const {
         return button->text();
      }
      inline QIcon icon() const {
         return button->icon();
      }

      inline bool operator==(const Page &other) const {
         return widget == other.widget;
      }
   };
   typedef QList<Page> PageList;

   inline QToolBoxPrivate()
      : currentPage(nullptr) {
   }

   void _q_buttonClicked();
   void _q_widgetDestroyed(QObject *);

   const Page *page(const QObject *widget) const;
   const Page *page(int index) const;
   Page *page(int index);

   void updateTabs();
   void relayout();

   PageList pageList;
   QVBoxLayout *layout;
   Page *currentPage;
};

const QToolBoxPrivate::Page *QToolBoxPrivate::page(const QObject *widget) const
{
   if (! widget) {
      return nullptr;
   }

   for (PageList::const_iterator i = pageList.constBegin(); i != pageList.constEnd(); ++i)
      if ((*i).widget == widget) {
         return (const Page *) & (*i);
      }

   return nullptr;
}

QToolBoxPrivate::Page *QToolBoxPrivate::page(int index)
{
   if (index >= 0 && index < pageList.size()) {
      return &pageList[index];
   }
   return nullptr;
}

const QToolBoxPrivate::Page *QToolBoxPrivate::page(int index) const
{
   if (index >= 0 && index < pageList.size()) {
      return &pageList.at(index);
   }
   return nullptr;
}

void QToolBoxPrivate::updateTabs()
{
   QToolBoxButton *lastButton = currentPage ? currentPage->button : nullptr;
   bool after = false;
   int index  = 0;

   for (index = 0; index < pageList.count(); ++index) {
      const Page &page = pageList.at(index);
      QToolBoxButton *tB = page.button;
      // update indexes, since the updates are delayed, the indexes will be correct
      // when we actually paint.
      tB->setIndex(index);
      QWidget *tW = page.widget;
      if (after) {
         QPalette p = tB->palette();
         p.setColor(tB->backgroundRole(), tW->palette().color(tW->backgroundRole()));
         tB->setPalette(p);
         tB->update();
      } else if (tB->backgroundRole() != QPalette::Window) {
         tB->setBackgroundRole(QPalette::Window);
         tB->update();
      }
      after = tB == lastButton;
   }
}

QSize QToolBoxButton::sizeHint() const
{
   QSize iconSize(8, 8);
   if (!icon().isNull()) {
      int icone = style()->pixelMetric(QStyle::PM_SmallIconSize, nullptr, parentWidget());
      iconSize += QSize(icone + 2, icone);
   }
   QSize textSize = fontMetrics().size(Qt::TextShowMnemonic, text()) + QSize(0, 8);

   QSize total(iconSize.width() + textSize.width(), qMax(iconSize.height(), textSize.height()));
   return total.expandedTo(QApplication::globalStrut());
}

QSize QToolBoxButton::minimumSizeHint() const
{
   if (icon().isNull()) {
      return QSize();
   }
   int icone = style()->pixelMetric(QStyle::PM_SmallIconSize, nullptr, parentWidget());

   return QSize(icone + 8, icone + 8);
}

void QToolBoxButton::initStyleOption(QStyleOptionToolBox *option) const
{
   if (!option) {
      return;
   }

   option->initFrom(this);

   if (selected) {
      option->state |= QStyle::State_Selected;
   }

   if (isDown()) {
      option->state |= QStyle::State_Sunken;
   }
   option->text = text();
   option->icon = icon();

   QToolBox *toolBox = static_cast<QToolBox *>(parentWidget()); // I know I'm in a tool box.
   const int widgetCount = toolBox->count();
   const int currIndex = toolBox->currentIndex();

   if (widgetCount == 1) {
      option->position = QStyleOptionToolBox::OnlyOneTab;
   } else if (indexInPage == 0) {
      option->position = QStyleOptionToolBox::Beginning;
   } else if (indexInPage == widgetCount - 1) {
      option->position = QStyleOptionToolBox::End;
   } else {
      option->position = QStyleOptionToolBox::Middle;
   }

   if (currIndex == indexInPage - 1) {
      option->selectedPosition = QStyleOptionToolBox::PreviousIsSelected;

   } else if (currIndex == indexInPage + 1) {
      option->selectedPosition = QStyleOptionToolBox::NextIsSelected;
   } else {
      option->selectedPosition = QStyleOptionToolBox::NotAdjacent;
   }
}

void QToolBoxButton::paintEvent(QPaintEvent *)
{
   QPainter paint(this);
   QPainter *p = &paint;

   QStyleOptionToolBox opt;
   initStyleOption(&opt);
   style()->drawControl(QStyle::CE_ToolBoxTab, &opt, p, parentWidget());
}

QToolBox::QToolBox(QWidget *parent, Qt::WindowFlags flags)
   :  QFrame(*new QToolBoxPrivate, parent, flags)
{
   Q_D(QToolBox);
   d->layout = new QVBoxLayout(this);
   d->layout->setMargin(0);
   setBackgroundRole(QPalette::Button);
}

QToolBox::~QToolBox()
{
}

int QToolBox::insertItem(int index, QWidget *widget, const QIcon &icon, const QString &text)
{
   if (! widget) {
      return -1;
   }

   Q_D(QToolBox);
   connect(widget, &QWidget::destroyed, this, &QToolBox::_q_widgetDestroyed);

   QToolBoxPrivate::Page c;
   c.widget = widget;
   c.button = new QToolBoxButton(this);
   c.button->setObjectName(QLatin1String("qt_toolbox_toolboxbutton"));

   connect(c.button, &QToolBoxButton::clicked, this, &QToolBox::_q_buttonClicked);

   c.sv = new QScrollArea(this);
   c.sv->setWidget(widget);
   c.sv->setWidgetResizable(true);
   c.sv->hide();
   c.sv->setFrameStyle(QFrame::NoFrame);

   c.setText(text);
   c.setIcon(icon);

   if (index < 0 || index >= (int)d->pageList.count()) {
      index = d->pageList.count();
      d->pageList.append(c);
      d->layout->addWidget(c.button);
      d->layout->addWidget(c.sv);
      if (index == 0) {
         setCurrentIndex(index);
      }
   } else {
      d->pageList.insert(index, c);
      d->relayout();
      if (d->currentPage) {
         QWidget *current = d->currentPage->widget;
         int oldindex = indexOf(current);
         if (index <= oldindex) {
            d->currentPage = nullptr;       // trigger change
            setCurrentIndex(oldindex);
         }
      }
   }

   c.button->show();

   d->updateTabs();
   itemInserted(index);
   return index;
}

void QToolBoxPrivate::_q_buttonClicked()
{
   Q_Q(QToolBox);

   QToolBoxButton *tb = qobject_cast<QToolBoxButton *>(q->sender());
   QWidget *item = nullptr;

   for (QToolBoxPrivate::PageList::const_iterator i = pageList.constBegin(); i != pageList.constEnd(); ++i)
      if ((*i).button == tb) {
         item = (*i).widget;
         break;
      }

   q->setCurrentIndex(q->indexOf(item));
}

int QToolBox::count() const
{
   Q_D(const QToolBox);
   return d->pageList.count();
}

void QToolBox::setCurrentIndex(int index)
{
   Q_D(QToolBox);
   QToolBoxPrivate::Page *c = d->page(index);
   if (!c || d->currentPage == c) {
      return;
   }

   c->button->setSelected(true);
   if (d->currentPage) {
      d->currentPage->sv->hide();
      d->currentPage->button->setSelected(false);
   }
   d->currentPage = c;
   d->currentPage->sv->show();
   d->updateTabs();
   emit currentChanged(index);
}

void QToolBoxPrivate::relayout()
{
   Q_Q(QToolBox);

   delete layout;
   layout = new QVBoxLayout(q);
   layout->setMargin(0);

   for (QToolBoxPrivate::PageList::const_iterator i = pageList.constBegin(); i != pageList.constEnd(); ++i) {
      layout->addWidget((*i).button);
      layout->addWidget((*i).sv);
   }
}

void QToolBoxPrivate::_q_widgetDestroyed(QObject *object)
{
   Q_Q(QToolBox);

   const QToolBoxPrivate::Page *const c = page(object);

   if (!c) {
      return;
   }

   layout->removeWidget(c->sv);
   layout->removeWidget(c->button);
   c->sv->deleteLater(); // page might still be a child of sv
   delete c->button;

   bool removeCurrent = c == currentPage;
   pageList.removeAll(*c);

   if (!pageList.count()) {
      currentPage = nullptr;
      emit q->currentChanged(-1);
   } else if (removeCurrent) {
      currentPage = nullptr;
      q->setCurrentIndex(0);
   }
}

void QToolBox::removeItem(int index)
{
   Q_D(QToolBox);

   if (QWidget *w = widget(index)) {
      disconnect(w, &QWidget::destroyed, this, &QToolBox::_q_widgetDestroyed);

      w->setParent(this);

      // destroy internal data
      d->_q_widgetDestroyed(w);
      itemRemoved(index);
   }
}

int QToolBox::currentIndex() const
{
   Q_D(const QToolBox);
   return d->currentPage ? indexOf(d->currentPage->widget) : -1;
}

QWidget *QToolBox::currentWidget() const
{
   Q_D(const QToolBox);
   return d->currentPage ? d->currentPage->widget : nullptr;
}

void QToolBox::setCurrentWidget(QWidget *widget)
{
   int i = indexOf(widget);

   if (i >= 0) {
      setCurrentIndex(i);
   } else {
      qWarning("QToolBox::setCurrentWidget() Current widget was not found in this ToolBox");
   }
}

QWidget *QToolBox::widget(int index) const
{
   Q_D(const QToolBox);
   if (index < 0 || index >= (int) d->pageList.size()) {
      return nullptr;
   }

   return d->pageList.at(index).widget;
}

int QToolBox::indexOf(QWidget *widget) const
{
   Q_D(const QToolBox);
   const QToolBoxPrivate::Page *c = (widget ? d->page(widget) : nullptr);

   return c ? d->pageList.indexOf(*c) : -1;
}

void QToolBox::setItemEnabled(int index, bool enabled)
{
   Q_D(QToolBox);
   QToolBoxPrivate::Page *c = d->page(index);

   if (!c) {
      return;
   }

   c->button->setEnabled(enabled);
   if (!enabled && c == d->currentPage) {
      int curIndexUp = index;
      int curIndexDown = curIndexUp;
      const int count = d->pageList.count();

      while (curIndexUp > 0 || curIndexDown < count - 1) {
         if (curIndexDown < count - 1) {
            if (d->page(++curIndexDown)->button->isEnabled()) {
               index = curIndexDown;
               break;
            }
         }
         if (curIndexUp > 0) {
            if (d->page(--curIndexUp)->button->isEnabled()) {
               index = curIndexUp;
               break;
            }
         }
      }
      setCurrentIndex(index);
   }
}

void QToolBox::setItemText(int index, const QString &text)
{
   Q_D(QToolBox);
   QToolBoxPrivate::Page *c = d->page(index);
   if (c) {
      c->setText(text);
   }
}

void QToolBox::setItemIcon(int index, const QIcon &icon)
{
   Q_D(QToolBox);
   QToolBoxPrivate::Page *c = d->page(index);
   if (c) {
      c->setIcon(icon);
   }
}

#ifndef QT_NO_TOOLTIP
void QToolBox::setItemToolTip(int index, const QString &toolTip)
{
   Q_D(QToolBox);
   QToolBoxPrivate::Page *c = d->page(index);
   if (c) {
      c->setToolTip(toolTip);
   }
}
#endif

bool QToolBox::isItemEnabled(int index) const
{
   Q_D(const QToolBox);
   const QToolBoxPrivate::Page *c = d->page(index);
   return c && c->button->isEnabled();
}

QString QToolBox::itemText(int index) const
{
   Q_D(const QToolBox);
   const QToolBoxPrivate::Page *c = d->page(index);
   return (c ? c->text() : QString());
}

QIcon QToolBox::itemIcon(int index) const
{
   Q_D(const QToolBox);
   const QToolBoxPrivate::Page *c = d->page(index);
   return (c ? c->icon() : QIcon());
}

#ifndef QT_NO_TOOLTIP
QString QToolBox::itemToolTip(int index) const
{
   Q_D(const QToolBox);
   const QToolBoxPrivate::Page *c = d->page(index);
   return (c ? c->toolTip() : QString());
}
#endif

void QToolBox::showEvent(QShowEvent *e)
{
   QWidget::showEvent(e);
}

void QToolBox::changeEvent(QEvent *ev)
{
   Q_D(QToolBox);
   if (ev->type() == QEvent::StyleChange) {
      d->updateTabs();
   }
   QFrame::changeEvent(ev);
}

void QToolBox::itemInserted(int index)
{
   (void) index;
}

void QToolBox::itemRemoved(int index)
{
   (void) index;
}

bool QToolBox::event(QEvent *e)
{
   return QFrame::event(e);
}

void QToolBox::_q_buttonClicked()
{
   Q_D(QToolBox);
   d->_q_buttonClicked();
}

void QToolBox::_q_widgetDestroyed(QObject *object)
{
   Q_D(QToolBox);
   d->_q_widgetDestroyed(object);
}

#endif //QT_NO_TOOLBOX
