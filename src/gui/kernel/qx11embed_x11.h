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

#ifndef QX11EMBED_X11_H
#define QX11EMBED_X11_H

#include <QtGui/qwidget.h>

QT_BEGIN_NAMESPACE

class QX11EmbedWidgetPrivate;
class QX11EmbedContainerPrivate;

class Q_GUI_EXPORT QX11EmbedWidget : public QWidget
{
   GUI_CS_OBJECT(QX11EmbedWidget)

 public:
   QX11EmbedWidget(QWidget *parent = nullptr);
   ~QX11EmbedWidget();

   void embedInto(WId id);
   WId containerWinId() const;

   enum Error {
      Unknown,
      Internal,
      InvalidWindowID
   };
   Error error() const;

   GUI_CS_SIGNAL_1(Public, void embedded())
   GUI_CS_SIGNAL_2(embedded)
   GUI_CS_SIGNAL_1(Public, void containerClosed())
   GUI_CS_SIGNAL_2(containerClosed)
   GUI_CS_SIGNAL_1(Public, void error(QX11EmbedWidget::Error error))
   GUI_CS_SIGNAL_OVERLOAD(error, (QX11EmbedWidget::Error), error)

 protected:
   bool x11Event(XEvent *) override;
   bool eventFilter(QObject *, QEvent *) override;
   bool event(QEvent *) override;
   void resizeEvent(QResizeEvent *) override;

 private:
   Q_DECLARE_PRIVATE(QX11EmbedWidget)
   Q_DISABLE_COPY(QX11EmbedWidget)
};


class Q_GUI_EXPORT QX11EmbedContainer : public QWidget
{
   GUI_CS_OBJECT(QX11EmbedContainer)

 public:
   QX11EmbedContainer(QWidget *parent = nullptr);
   ~QX11EmbedContainer();

   void embedClient(WId id);
   void discardClient();

   WId clientWinId() const;

   QSize minimumSizeHint() const override;

   enum Error {
      Unknown,
      Internal,
      InvalidWindowID
   };
   Error error() const;

   GUI_CS_SIGNAL_1(Public, void clientIsEmbedded())
   GUI_CS_SIGNAL_2(clientIsEmbedded)
   GUI_CS_SIGNAL_1(Public, void clientClosed())
   GUI_CS_SIGNAL_2(clientClosed)
   GUI_CS_SIGNAL_1(Public, void error(QX11EmbedContainer::Error un_named_arg1))
   GUI_CS_SIGNAL_OVERLOAD(error, (QX11EmbedContainer::Error), un_named_arg1)

 protected:
   bool x11Event(XEvent *) override;
   bool eventFilter(QObject *, QEvent *) override;
   void paintEvent(QPaintEvent *e) override;
   void resizeEvent(QResizeEvent *) override;
   void showEvent(QShowEvent *) override;
   void hideEvent(QHideEvent *) override;
   bool event(QEvent *) override;

 private:
   Q_DECLARE_PRIVATE(QX11EmbedContainer)
   Q_DISABLE_COPY(QX11EmbedContainer)
};

QT_END_NAMESPACE

#endif // QX11EMBED_X11_H
