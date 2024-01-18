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

#ifndef QDECLARATIVESTATEOPERATIONS_P_H
#define QDECLARATIVESTATEOPERATIONS_P_H

#include <qdeclarativestate_p.h>
#include <qdeclarativeitem.h>
#include <qdeclarativeanchors_p.h>
#include <qdeclarativescriptstring.h>

QT_BEGIN_NAMESPACE

class QDeclarativeParentChangePrivate;

class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeParentChange : public QDeclarativeStateOperation,
   public QDeclarativeActionEvent
{
   DECL_CS_OBJECT(QDeclarativeParentChange)
   Q_DECLARE_PRIVATE(QDeclarativeParentChange)

   DECL_CS_PROPERTY_READ(*target, object)
   DECL_CS_PROPERTY_WRITE(*target, setObject)
   DECL_CS_PROPERTY_READ(*parent, parent)
   DECL_CS_PROPERTY_WRITE(*parent, setParent)
   DECL_CS_PROPERTY_READ(x, x)
   DECL_CS_PROPERTY_WRITE(x, setX)
   DECL_CS_PROPERTY_READ(y, y)
   DECL_CS_PROPERTY_WRITE(y, setY)
   DECL_CS_PROPERTY_READ(width, width)
   DECL_CS_PROPERTY_WRITE(width, setWidth)
   DECL_CS_PROPERTY_READ(height, height)
   DECL_CS_PROPERTY_WRITE(height, setHeight)
   DECL_CS_PROPERTY_READ(scale, scale)
   DECL_CS_PROPERTY_WRITE(scale, setScale)
   DECL_CS_PROPERTY_READ(rotation, rotation)
   DECL_CS_PROPERTY_WRITE(rotation, setRotation)
 public:
   QDeclarativeParentChange(QObject *parent = nullptr);
   ~QDeclarativeParentChange();

   QDeclarativeItem *object() const;
   void setObject(QDeclarativeItem *);

   QDeclarativeItem *parent() const;
   void setParent(QDeclarativeItem *);

   QDeclarativeItem *originalParent() const;

   QDeclarativeScriptString x() const;
   void setX(QDeclarativeScriptString x);
   bool xIsSet() const;

   QDeclarativeScriptString y() const;
   void setY(QDeclarativeScriptString y);
   bool yIsSet() const;

   QDeclarativeScriptString width() const;
   void setWidth(QDeclarativeScriptString width);
   bool widthIsSet() const;

   QDeclarativeScriptString height() const;
   void setHeight(QDeclarativeScriptString height);
   bool heightIsSet() const;

   QDeclarativeScriptString scale() const;
   void setScale(QDeclarativeScriptString scale);
   bool scaleIsSet() const;

   QDeclarativeScriptString rotation() const;
   void setRotation(QDeclarativeScriptString rotation);
   bool rotationIsSet() const;

   virtual ActionList actions();

   virtual void saveOriginals();
   //virtual void copyOriginals(QDeclarativeActionEvent*);
   virtual void execute(Reason reason = ActualChange);
   virtual bool isReversable();
   virtual void reverse(Reason reason = ActualChange);
   virtual QString typeName() const;
   virtual bool override(QDeclarativeActionEvent *other);
   virtual void rewind();
   virtual void saveCurrentValues();
};

class QDeclarativeStateChangeScriptPrivate;
class QDeclarativeStateChangeScript : public QDeclarativeStateOperation, public QDeclarativeActionEvent
{
   DECL_CS_OBJECT(QDeclarativeStateChangeScript)
   Q_DECLARE_PRIVATE(QDeclarativeStateChangeScript)

   DECL_CS_PROPERTY_READ(script, script)
   DECL_CS_PROPERTY_WRITE(script, setScript)
   DECL_CS_PROPERTY_READ(name, name)
   DECL_CS_PROPERTY_WRITE(name, setName)

 public:
   QDeclarativeStateChangeScript(QObject *parent = nullptr);
   ~QDeclarativeStateChangeScript();

   virtual ActionList actions();

   virtual QString typeName() const;

   QDeclarativeScriptString script() const;
   void setScript(const QDeclarativeScriptString &);

   QString name() const;
   void setName(const QString &);

   virtual void execute(Reason reason = ActualChange);
};

class QDeclarativeAnchorChanges;
class QDeclarativeAnchorSetPrivate;
class QDeclarativeAnchorSet : public QObject
{
   DECL_CS_OBJECT(QDeclarativeAnchorSet)

   DECL_CS_PROPERTY_READ(left, left)
   DECL_CS_PROPERTY_WRITE(left, setLeft)
   DECL_CS_PROPERTY_RESET(left, resetLeft)
   DECL_CS_PROPERTY_READ(right, right)
   DECL_CS_PROPERTY_WRITE(right, setRight)
   DECL_CS_PROPERTY_RESET(right, resetRight)
   DECL_CS_PROPERTY_READ(horizontalCenter, horizontalCenter)
   DECL_CS_PROPERTY_WRITE(horizontalCenter, setHorizontalCenter)
   DECL_CS_PROPERTY_RESET(horizontalCenter, resetHorizontalCenter)
   DECL_CS_PROPERTY_READ(top, top)
   DECL_CS_PROPERTY_WRITE(top, setTop)
   DECL_CS_PROPERTY_RESET(top, resetTop)
   DECL_CS_PROPERTY_READ(bottom, bottom)
   DECL_CS_PROPERTY_WRITE(bottom, setBottom)
   DECL_CS_PROPERTY_RESET(bottom, resetBottom)
   DECL_CS_PROPERTY_READ(verticalCenter, verticalCenter)
   DECL_CS_PROPERTY_WRITE(verticalCenter, setVerticalCenter)
   DECL_CS_PROPERTY_RESET(verticalCenter, resetVerticalCenter)
   DECL_CS_PROPERTY_READ(baseline, baseline)
   DECL_CS_PROPERTY_WRITE(baseline, setBaseline)
   DECL_CS_PROPERTY_RESET(baseline, resetBaseline)
   //Q_PROPERTY(QDeclarativeItem *fill READ fill WRITE setFill RESET resetFill)
   //Q_PROPERTY(QDeclarativeItem *centerIn READ centerIn WRITE setCenterIn RESET resetCenterIn)

   /*Q_PROPERTY(qreal margins READ margins WRITE setMargins NOTIFY marginsChanged)
   Q_PROPERTY(qreal leftMargin READ leftMargin WRITE setLeftMargin NOTIFY leftMarginChanged)
   Q_PROPERTY(qreal rightMargin READ rightMargin WRITE setRightMargin NOTIFY rightMarginChanged)
   Q_PROPERTY(qreal horizontalCenterOffset READ horizontalCenterOffset WRITE setHorizontalCenterOffset NOTIFY horizontalCenterOffsetChanged())
   Q_PROPERTY(qreal topMargin READ topMargin WRITE setTopMargin NOTIFY topMarginChanged)
   Q_PROPERTY(qreal bottomMargin READ bottomMargin WRITE setBottomMargin NOTIFY bottomMarginChanged)
   Q_PROPERTY(qreal verticalCenterOffset READ verticalCenterOffset WRITE setVerticalCenterOffset NOTIFY verticalCenterOffsetChanged())
   Q_PROPERTY(qreal baselineOffset READ baselineOffset WRITE setBaselineOffset NOTIFY baselineOffsetChanged())*/

 public:
   QDeclarativeAnchorSet(QObject *parent = nullptr);
   virtual ~QDeclarativeAnchorSet();

   QDeclarativeScriptString left() const;
   void setLeft(const QDeclarativeScriptString &edge);
   void resetLeft();

   QDeclarativeScriptString right() const;
   void setRight(const QDeclarativeScriptString &edge);
   void resetRight();

   QDeclarativeScriptString horizontalCenter() const;
   void setHorizontalCenter(const QDeclarativeScriptString &edge);
   void resetHorizontalCenter();

   QDeclarativeScriptString top() const;
   void setTop(const QDeclarativeScriptString &edge);
   void resetTop();

   QDeclarativeScriptString bottom() const;
   void setBottom(const QDeclarativeScriptString &edge);
   void resetBottom();

   QDeclarativeScriptString verticalCenter() const;
   void setVerticalCenter(const QDeclarativeScriptString &edge);
   void resetVerticalCenter();

   QDeclarativeScriptString baseline() const;
   void setBaseline(const QDeclarativeScriptString &edge);
   void resetBaseline();

   QDeclarativeItem *fill() const;
   void setFill(QDeclarativeItem *);
   void resetFill();

   QDeclarativeItem *centerIn() const;
   void setCenterIn(QDeclarativeItem *);
   void resetCenterIn();

   /*qreal leftMargin() const;
   void setLeftMargin(qreal);

   qreal rightMargin() const;
   void setRightMargin(qreal);

   qreal horizontalCenterOffset() const;
   void setHorizontalCenterOffset(qreal);

   qreal topMargin() const;
   void setTopMargin(qreal);

   qreal bottomMargin() const;
   void setBottomMargin(qreal);

   qreal margins() const;
   void setMargins(qreal);

   qreal verticalCenterOffset() const;
   void setVerticalCenterOffset(qreal);

   qreal baselineOffset() const;
   void setBaselineOffset(qreal);*/

   QDeclarativeAnchors::Anchors usedAnchors() const;

   /*Q_SIGNALS:
       void leftMarginChanged();
       void rightMarginChanged();
       void topMarginChanged();
       void bottomMarginChanged();
       void marginsChanged();
       void verticalCenterOffsetChanged();
       void horizontalCenterOffsetChanged();
       void baselineOffsetChanged();*/

 private:
   friend class QDeclarativeAnchorChanges;
   Q_DISABLE_COPY(QDeclarativeAnchorSet)
   Q_DECLARE_PRIVATE(QDeclarativeAnchorSet)
};

class QDeclarativeAnchorChangesPrivate;
class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeAnchorChanges : public QDeclarativeStateOperation,
   public QDeclarativeActionEvent
{
   DECL_CS_OBJECT(QDeclarativeAnchorChanges)
   Q_DECLARE_PRIVATE(QDeclarativeAnchorChanges)

   DECL_CS_PROPERTY_READ(*target, object)
   DECL_CS_PROPERTY_WRITE(*target, setObject)
   DECL_CS_PROPERTY_READ(*anchors, anchors)
   DECL_CS_PROPERTY_CONSTANT(*anchors)

 public:
   QDeclarativeAnchorChanges(QObject *parent = nullptr);
   ~QDeclarativeAnchorChanges();

   virtual ActionList actions();

   QDeclarativeAnchorSet *anchors();

   QDeclarativeItem *object() const;
   void setObject(QDeclarativeItem *);

   virtual void execute(Reason reason = ActualChange);
   virtual bool isReversable();
   virtual void reverse(Reason reason = ActualChange);
   virtual QString typeName() const;
   virtual bool override(QDeclarativeActionEvent *other);
   virtual bool changesBindings();
   virtual void saveOriginals();
   virtual bool needsCopy() {
      return true;
   }
   virtual void copyOriginals(QDeclarativeActionEvent *);
   virtual void clearBindings();
   virtual void rewind();
   virtual void saveCurrentValues();

   QList<QDeclarativeAction> additionalActions();
   virtual void saveTargetValues();
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeParentChange)
QML_DECLARE_TYPE(QDeclarativeStateChangeScript)
QML_DECLARE_TYPE(QDeclarativeAnchorSet)
QML_DECLARE_TYPE(QDeclarativeAnchorChanges)

#endif // QDECLARATIVESTATEOPERATIONS_H
