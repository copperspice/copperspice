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

#ifndef QDECLARATIVEVIEW_H
#define QDECLARATIVEVIEW_H

#include <QtCore/qdatetime.h>
#include <QtCore/qurl.h>
#include <QtGui/qgraphicssceneevent.h>
#include <QtGui/qgraphicsview.h>
#include <QtGui/qwidget.h>
#include <QtDeclarative/qdeclarativedebug.h>

QT_BEGIN_NAMESPACE

class QGraphicsObject;
class QDeclarativeEngine;
class QDeclarativeContext;
class QDeclarativeError;
class QDeclarativeViewPrivate;

class Q_DECLARATIVE_EXPORT QDeclarativeView : public QGraphicsView
{
   DECL_CS_OBJECT(QDeclarativeView)

   DECL_CS_PROPERTY_READ(resizeMode, resizeMode)
   DECL_CS_PROPERTY_WRITE(resizeMode, setResizeMode)
   DECL_CS_PROPERTY_READ(status, status)
   DECL_CS_PROPERTY_NOTIFY(status, statusChanged)
   DECL_CS_PROPERTY_READ(source, source)
   DECL_CS_PROPERTY_WRITE(source, setSource)
   DECL_CS_PROPERTY_DESIGNABLE(source, true)

   CS_ENUM(ResizeMode)
   CS_ENUM(Status)

 public:
   explicit QDeclarativeView(QWidget *parent = nullptr);
   QDeclarativeView(const QUrl &source, QWidget *parent = nullptr);
   virtual ~QDeclarativeView();

   QUrl source() const;
   void setSource(const QUrl &);

   QDeclarativeEngine *engine() const;
   QDeclarativeContext *rootContext() const;

   QGraphicsObject *rootObject() const;

   enum ResizeMode { SizeViewToRootObject, SizeRootObjectToView };
   ResizeMode resizeMode() const;
   void setResizeMode(ResizeMode);

   enum Status { Null, Ready, Loading, Error };
   Status status() const;

   QList<QDeclarativeError> errors() const;

   QSize sizeHint() const;
   QSize initialSize() const;

 public:
   DECL_CS_SIGNAL_1(Public, void sceneResized(QSize size))
   DECL_CS_SIGNAL_2(sceneResized, size) // ???
   DECL_CS_SIGNAL_1(Public, void statusChanged(QDeclarativeView::Status un_named_arg1))
   DECL_CS_SIGNAL_2(statusChanged, un_named_arg1)

 private :
   DECL_CS_SLOT_1(Private, void continueExecute())
   DECL_CS_SLOT_2(continueExecute)

 protected:
   virtual void resizeEvent(QResizeEvent *);
   virtual void paintEvent(QPaintEvent *event);
   virtual void timerEvent(QTimerEvent *);
   virtual void setRootObject(QObject *obj);
   virtual bool eventFilter(QObject *watched, QEvent *e);

 private:
   Q_DISABLE_COPY(QDeclarativeView)
   Q_DECLARE_PRIVATE(QDeclarativeView)
};

QT_END_NAMESPACE

#endif // QDECLARATIVEVIEW_H
