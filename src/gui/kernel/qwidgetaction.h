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

#ifndef QWIDGETACTION_H
#define QWIDGETACTION_H

#include <qaction.h>



#ifndef QT_NO_ACTION

class QWidgetActionPrivate;

class Q_GUI_EXPORT QWidgetAction : public QAction
{
   GUI_CS_OBJECT(QWidgetAction)
   Q_DECLARE_PRIVATE(QWidgetAction)

 public:
   explicit QWidgetAction(QObject *parent);
   virtual ~QWidgetAction();

   void setDefaultWidget(QWidget *w);
   QWidget *defaultWidget() const;

   QWidget *requestWidget(QWidget *parent);
   void releaseWidget(QWidget *widget);

 protected:
   virtual bool event(QEvent *) override;
   virtual bool eventFilter(QObject *, QEvent *) override;
   virtual QWidget *createWidget(QWidget *parent);
   virtual void deleteWidget(QWidget *widget);
   QList<QWidget *> createdWidgets() const;

 private:
   Q_DISABLE_COPY(QWidgetAction)

   GUI_CS_SLOT_1(Private, void _q_widgetDestroyed(QObject *obj))
   GUI_CS_SLOT_2(_q_widgetDestroyed)

   friend class QToolBar;
};

#endif // QT_NO_ACTION



#endif // QWIDGETACTION_H
