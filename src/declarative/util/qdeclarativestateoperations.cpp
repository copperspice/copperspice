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

#include <qdeclarativestateoperations_p.h>
#include <qdeclarative.h>
#include <qdeclarativecontext.h>
#include <qdeclarativeexpression.h>
#include <qdeclarativeinfo.h>
#include <qdeclarativeanchors_p_p.h>
#include <qdeclarativeitem_p.h>
#include <qdeclarativeguard_p.h>
#include <qdeclarativenullablevalue_p_p.h>
#include <qdeclarativecontext_p.h>
#include <qdeclarativeproperty_p.h>
#include <qdeclarativebinding_p.h>
#include <qdeclarativestate_p_p.h>

#include <QtCore/qdebug.h>
#include <QtGui/qgraphicsitem.h>
#include <QtCore/qmath.h>

QT_BEGIN_NAMESPACE

class QDeclarativeParentChangePrivate : public QDeclarativeStateOperationPrivate
{
   Q_DECLARE_PUBLIC(QDeclarativeParentChange)

 public:
   QDeclarativeParentChangePrivate() : target(0), parent(0), origParent(0), origStackBefore(0),
      rewindParent(0), rewindStackBefore(0) {}

   QDeclarativeItem *target;
   QDeclarativeGuard<QDeclarativeItem> parent;
   QDeclarativeGuard<QDeclarativeItem> origParent;
   QDeclarativeGuard<QDeclarativeItem> origStackBefore;
   QDeclarativeItem *rewindParent;
   QDeclarativeItem *rewindStackBefore;

   QDeclarativeNullableValue<QDeclarativeScriptString> xString;
   QDeclarativeNullableValue<QDeclarativeScriptString> yString;
   QDeclarativeNullableValue<QDeclarativeScriptString> widthString;
   QDeclarativeNullableValue<QDeclarativeScriptString> heightString;
   QDeclarativeNullableValue<QDeclarativeScriptString> scaleString;
   QDeclarativeNullableValue<QDeclarativeScriptString> rotationString;

   QDeclarativeNullableValue<qreal> x;
   QDeclarativeNullableValue<qreal> y;
   QDeclarativeNullableValue<qreal> width;
   QDeclarativeNullableValue<qreal> height;
   QDeclarativeNullableValue<qreal> scale;
   QDeclarativeNullableValue<qreal> rotation;

   void doChange(QDeclarativeItem *targetParent, QDeclarativeItem *stackBefore = 0);
};

void QDeclarativeParentChangePrivate::doChange(QDeclarativeItem *targetParent, QDeclarativeItem *stackBefore)
{
   if (targetParent && target && target->parentItem()) {
      Q_Q(QDeclarativeParentChange);
      bool ok;
      const QTransform &transform = target->parentItem()->itemTransform(targetParent, &ok);
      if (transform.type() >= QTransform::TxShear || !ok) {
         qmlInfo(q) << QDeclarativeParentChange::tr("Unable to preserve appearance under complex transform");
         ok = false;
      }

      qreal scale = 1;
      qreal rotation = 0;
      bool isRotate = (transform.type() == QTransform::TxRotate) || (transform.m11() < 0);
      if (ok && !isRotate) {
         if (transform.m11() == transform.m22()) {
            scale = transform.m11();
         } else {
            qmlInfo(q) << QDeclarativeParentChange::tr("Unable to preserve appearance under non-uniform scale");
            ok = false;
         }
      } else if (ok && isRotate) {
         if (transform.m11() == transform.m22()) {
            scale = qSqrt(transform.m11() * transform.m11() + transform.m12() * transform.m12());
         } else {
            qmlInfo(q) << QDeclarativeParentChange::tr("Unable to preserve appearance under non-uniform scale");
            ok = false;
         }

         if (scale != 0) {
            rotation = atan2(transform.m12() / scale, transform.m11() / scale) * 180 / qreal(M_PI);
         } else {
            qmlInfo(q) << QDeclarativeParentChange::tr("Unable to preserve appearance under scale of 0");
            ok = false;
         }
      }

      const QPointF &point = transform.map(QPointF(target->x(), target->y()));
      qreal x = point.x();
      qreal y = point.y();

      // setParentItem will update the transformOriginPoint if needed
      target->setParentItem(targetParent);

      if (ok && target->transformOrigin() != QDeclarativeItem::TopLeft) {
         qreal tempxt = target->transformOriginPoint().x();
         qreal tempyt = target->transformOriginPoint().y();
         QTransform t;
         t.translate(-tempxt, -tempyt);
         t.rotate(rotation);
         t.scale(scale, scale);
         t.translate(tempxt, tempyt);
         const QPointF &offset = t.map(QPointF(0, 0));
         x += offset.x();
         y += offset.y();
      }

      if (ok) {
         //qDebug() << x << y << rotation << scale;
         target->setX(x);
         target->setY(y);
         target->setRotation(target->rotation() + rotation);
         target->setScale(target->scale() * scale);
      }
   } else if (target) {
      target->setParentItem(targetParent);
   }

   //restore the original stack position.
   //### if stackBefore has also been reparented this won't work
   if (stackBefore) {
      target->stackBefore(stackBefore);
   }
}

/*!
    \preliminary
    \qmlclass ParentChange QDeclarativeParentChange
    \ingroup qml-state-elements
    \brief The ParentChange element allows you to reparent an Item in a state change.

    ParentChange reparents an item while preserving its visual appearance (position, size,
    rotation, and scale) on screen. You can then specify a transition to move/resize/rotate/scale
    the item to its final intended appearance.

    ParentChange can only preserve visual appearance if no complex transforms are involved.
    More specifically, it will not work if the transform property has been set for any
    items involved in the reparenting (i.e. items in the common ancestor tree
    for the original and new parent).

    The example below displays a large red rectangle and a small blue rectangle, side by side.
    When the \c blueRect is clicked, it changes to the "reparented" state: its parent is changed to \c redRect and it is
    positioned at (10, 10) within the red rectangle, as specified in the ParentChange.

    \snippet doc/src/snippets/declarative/parentchange.qml 0

    \image parentchange.png

    You can specify at which point in a transition you want a ParentChange to occur by
    using a ParentAnimation.
*/


QDeclarativeParentChange::QDeclarativeParentChange(QObject *parent)
   : QDeclarativeStateOperation(*(new QDeclarativeParentChangePrivate), parent)
{
}

QDeclarativeParentChange::~QDeclarativeParentChange()
{
}

/*!
    \qmlproperty real ParentChange::x
    \qmlproperty real ParentChange::y
    \qmlproperty real ParentChange::width
    \qmlproperty real ParentChange::height
    \qmlproperty real ParentChange::scale
    \qmlproperty real ParentChange::rotation
    These properties hold the new position, size, scale, and rotation
    for the item in this state.
*/
QDeclarativeScriptString QDeclarativeParentChange::x() const
{
   Q_D(const QDeclarativeParentChange);
   return d->xString.value;
}

void tryReal(QDeclarativeNullableValue<qreal> &value, const QString &string)
{
   bool ok = false;
   qreal realValue = string.toFloat(&ok);
   if (ok) {
      value = realValue;
   } else {
      value.invalidate();
   }
}

void QDeclarativeParentChange::setX(QDeclarativeScriptString x)
{
   Q_D(QDeclarativeParentChange);
   d->xString = x;
   tryReal(d->x, x.script());
}

bool QDeclarativeParentChange::xIsSet() const
{
   Q_D(const QDeclarativeParentChange);
   return d->xString.isValid();
}

QDeclarativeScriptString QDeclarativeParentChange::y() const
{
   Q_D(const QDeclarativeParentChange);
   return d->yString.value;
}

void QDeclarativeParentChange::setY(QDeclarativeScriptString y)
{
   Q_D(QDeclarativeParentChange);
   d->yString = y;
   tryReal(d->y, y.script());
}

bool QDeclarativeParentChange::yIsSet() const
{
   Q_D(const QDeclarativeParentChange);
   return d->yString.isValid();
}

QDeclarativeScriptString QDeclarativeParentChange::width() const
{
   Q_D(const QDeclarativeParentChange);
   return d->widthString.value;
}

void QDeclarativeParentChange::setWidth(QDeclarativeScriptString width)
{
   Q_D(QDeclarativeParentChange);
   d->widthString = width;
   tryReal(d->width, width.script());
}

bool QDeclarativeParentChange::widthIsSet() const
{
   Q_D(const QDeclarativeParentChange);
   return d->widthString.isValid();
}

QDeclarativeScriptString QDeclarativeParentChange::height() const
{
   Q_D(const QDeclarativeParentChange);
   return d->heightString.value;
}

void QDeclarativeParentChange::setHeight(QDeclarativeScriptString height)
{
   Q_D(QDeclarativeParentChange);
   d->heightString = height;
   tryReal(d->height, height.script());
}

bool QDeclarativeParentChange::heightIsSet() const
{
   Q_D(const QDeclarativeParentChange);
   return d->heightString.isValid();
}

QDeclarativeScriptString QDeclarativeParentChange::scale() const
{
   Q_D(const QDeclarativeParentChange);
   return d->scaleString.value;
}

void QDeclarativeParentChange::setScale(QDeclarativeScriptString scale)
{
   Q_D(QDeclarativeParentChange);
   d->scaleString = scale;
   tryReal(d->scale, scale.script());
}

bool QDeclarativeParentChange::scaleIsSet() const
{
   Q_D(const QDeclarativeParentChange);
   return d->scaleString.isValid();
}

QDeclarativeScriptString QDeclarativeParentChange::rotation() const
{
   Q_D(const QDeclarativeParentChange);
   return d->rotationString.value;
}

void QDeclarativeParentChange::setRotation(QDeclarativeScriptString rotation)
{
   Q_D(QDeclarativeParentChange);
   d->rotationString = rotation;
   tryReal(d->rotation, rotation.script());
}

bool QDeclarativeParentChange::rotationIsSet() const
{
   Q_D(const QDeclarativeParentChange);
   return d->rotationString.isValid();
}

QDeclarativeItem *QDeclarativeParentChange::originalParent() const
{
   Q_D(const QDeclarativeParentChange);
   return d->origParent;
}

/*!
    \qmlproperty Item ParentChange::target
    This property holds the item to be reparented
*/

QDeclarativeItem *QDeclarativeParentChange::object() const
{
   Q_D(const QDeclarativeParentChange);
   return d->target;
}

void QDeclarativeParentChange::setObject(QDeclarativeItem *target)
{
   Q_D(QDeclarativeParentChange);
   d->target = target;
}

/*!
    \qmlproperty Item ParentChange::parent
    This property holds the new parent for the item in this state.
*/

QDeclarativeItem *QDeclarativeParentChange::parent() const
{
   Q_D(const QDeclarativeParentChange);
   return d->parent;
}

void QDeclarativeParentChange::setParent(QDeclarativeItem *parent)
{
   Q_D(QDeclarativeParentChange);
   d->parent = parent;
}

QDeclarativeStateOperation::ActionList QDeclarativeParentChange::actions()
{
   Q_D(QDeclarativeParentChange);
   if (!d->target || !d->parent) {
      return ActionList();
   }

   ActionList actions;

   QDeclarativeAction a;
   a.event = this;
   actions << a;

   QDeclarativeContext *ctxt = qmlContext(this);

   if (d->xString.isValid()) {
      if (d->x.isValid()) {
         QDeclarativeAction xa(d->target, QLatin1String("x"), ctxt, d->x.value);
         actions << xa;
      } else {
         QDeclarativeBinding *newBinding = new QDeclarativeBinding(d->xString.value.script(), d->target, ctxt);
         newBinding->setTarget(QDeclarativeProperty(d->target, QLatin1String("x"), ctxt));
         QDeclarativeAction xa;
         xa.property = newBinding->property();
         xa.toBinding = newBinding;
         xa.fromValue = xa.property.read();
         xa.deletableToBinding = true;
         actions << xa;
      }
   }

   if (d->yString.isValid()) {
      if (d->y.isValid()) {
         QDeclarativeAction ya(d->target, QLatin1String("y"), ctxt, d->y.value);
         actions << ya;
      } else {
         QDeclarativeBinding *newBinding = new QDeclarativeBinding(d->yString.value.script(), d->target, ctxt);
         newBinding->setTarget(QDeclarativeProperty(d->target, QLatin1String("y"), ctxt));
         QDeclarativeAction ya;
         ya.property = newBinding->property();
         ya.toBinding = newBinding;
         ya.fromValue = ya.property.read();
         ya.deletableToBinding = true;
         actions << ya;
      }
   }

   if (d->scaleString.isValid()) {
      if (d->scale.isValid()) {
         QDeclarativeAction sa(d->target, QLatin1String("scale"), ctxt, d->scale.value);
         actions << sa;
      } else {
         QDeclarativeBinding *newBinding = new QDeclarativeBinding(d->scaleString.value.script(), d->target, ctxt);
         newBinding->setTarget(QDeclarativeProperty(d->target, QLatin1String("scale"), ctxt));
         QDeclarativeAction sa;
         sa.property = newBinding->property();
         sa.toBinding = newBinding;
         sa.fromValue = sa.property.read();
         sa.deletableToBinding = true;
         actions << sa;
      }
   }

   if (d->rotationString.isValid()) {
      if (d->rotation.isValid()) {
         QDeclarativeAction ra(d->target, QLatin1String("rotation"), ctxt, d->rotation.value);
         actions << ra;
      } else {
         QDeclarativeBinding *newBinding = new QDeclarativeBinding(d->rotationString.value.script(), d->target, ctxt);
         newBinding->setTarget(QDeclarativeProperty(d->target, QLatin1String("rotation"), ctxt));
         QDeclarativeAction ra;
         ra.property = newBinding->property();
         ra.toBinding = newBinding;
         ra.fromValue = ra.property.read();
         ra.deletableToBinding = true;
         actions << ra;
      }
   }

   if (d->widthString.isValid()) {
      if (d->width.isValid()) {
         QDeclarativeAction wa(d->target, QLatin1String("width"), ctxt, d->width.value);
         actions << wa;
      } else {
         QDeclarativeBinding *newBinding = new QDeclarativeBinding(d->widthString.value.script(), d->target, ctxt);
         newBinding->setTarget(QDeclarativeProperty(d->target, QLatin1String("width"), ctxt));
         QDeclarativeAction wa;
         wa.property = newBinding->property();
         wa.toBinding = newBinding;
         wa.fromValue = wa.property.read();
         wa.deletableToBinding = true;
         actions << wa;
      }
   }

   if (d->heightString.isValid()) {
      if (d->height.isValid()) {
         QDeclarativeAction ha(d->target, QLatin1String("height"), ctxt, d->height.value);
         actions << ha;
      } else {
         QDeclarativeBinding *newBinding = new QDeclarativeBinding(d->heightString.value.script(), d->target, ctxt);
         newBinding->setTarget(QDeclarativeProperty(d->target, QLatin1String("height"), ctxt));
         QDeclarativeAction ha;
         ha.property = newBinding->property();
         ha.toBinding = newBinding;
         ha.fromValue = ha.property.read();
         ha.deletableToBinding = true;
         actions << ha;
      }
   }

   return actions;
}

class AccessibleFxItem : public QDeclarativeItem
{
   DECL_CS_OBJECT(AccessibleFxItem)
   Q_DECLARE_PRIVATE_D(QGraphicsItem::d_ptr.data(), QDeclarativeItem)

 public:
   int siblingIndex() {
      Q_D(QDeclarativeItem);
      return d->siblingIndex;
   }
};

void QDeclarativeParentChange::saveOriginals()
{
   Q_D(QDeclarativeParentChange);
   saveCurrentValues();
   d->origParent = d->rewindParent;
   d->origStackBefore = d->rewindStackBefore;
}

/*void QDeclarativeParentChange::copyOriginals(QDeclarativeActionEvent *other)
{
    Q_D(QDeclarativeParentChange);
    QDeclarativeParentChange *pc = static_cast<QDeclarativeParentChange*>(other);

    d->origParent = pc->d_func()->rewindParent;
    d->origStackBefore = pc->d_func()->rewindStackBefore;

    saveCurrentValues();
}*/

void QDeclarativeParentChange::execute(Reason)
{
   Q_D(QDeclarativeParentChange);
   d->doChange(d->parent);
}

bool QDeclarativeParentChange::isReversable()
{
   return true;
}

void QDeclarativeParentChange::reverse(Reason)
{
   Q_D(QDeclarativeParentChange);
   d->doChange(d->origParent, d->origStackBefore);
}

QString QDeclarativeParentChange::typeName() const
{
   return QLatin1String("ParentChange");
}

bool QDeclarativeParentChange::override(QDeclarativeActionEvent *other)
{
   Q_D(QDeclarativeParentChange);
   if (other->typeName() != QLatin1String("ParentChange")) {
      return false;
   }
   if (QDeclarativeParentChange *otherPC = static_cast<QDeclarativeParentChange *>(other)) {
      return (d->target == otherPC->object());
   }
   return false;
}

void QDeclarativeParentChange::saveCurrentValues()
{
   Q_D(QDeclarativeParentChange);
   if (!d->target) {
      d->rewindParent = 0;
      d->rewindStackBefore = 0;
      return;
   }

   d->rewindParent = d->target->parentItem();
   d->rewindStackBefore = 0;

   if (!d->rewindParent) {
      return;
   }

   //try to determine the item's original stack position so we can restore it
   int siblingIndex = ((AccessibleFxItem *)d->target)->siblingIndex() + 1;
   QList<QGraphicsItem *> children = d->rewindParent->childItems();
   for (int i = 0; i < children.count(); ++i) {
      QDeclarativeItem *child = qobject_cast<QDeclarativeItem *>(children.at(i));
      if (!child) {
         continue;
      }
      if (((AccessibleFxItem *)child)->siblingIndex() == siblingIndex) {
         d->rewindStackBefore = child;
         break;
      }
   }
}

void QDeclarativeParentChange::rewind()
{
   Q_D(QDeclarativeParentChange);
   d->doChange(d->rewindParent, d->rewindStackBefore);
}

class QDeclarativeStateChangeScriptPrivate : public QDeclarativeStateOperationPrivate
{
 public:
   QDeclarativeStateChangeScriptPrivate() {}

   QDeclarativeScriptString script;
   QString name;
};

/*!
    \qmlclass StateChangeScript QDeclarativeStateChangeScript
    \ingroup qml-state-elements
    \brief The StateChangeScript element allows you to run a script in a state.

    A StateChangeScript is run upon entering a state. You can optionally use
    ScriptAction to specify the point in the transition at which
    the StateChangeScript should to be run.

    \snippet snippets/declarative/states/statechangescript.qml state and transition

    \sa ScriptAction
*/

QDeclarativeStateChangeScript::QDeclarativeStateChangeScript(QObject *parent)
   : QDeclarativeStateOperation(*(new QDeclarativeStateChangeScriptPrivate), parent)
{
}

QDeclarativeStateChangeScript::~QDeclarativeStateChangeScript()
{
}

/*!
    \qmlproperty script StateChangeScript::script
    This property holds the script to run when the state is current.
*/
QDeclarativeScriptString QDeclarativeStateChangeScript::script() const
{
   Q_D(const QDeclarativeStateChangeScript);
   return d->script;
}

void QDeclarativeStateChangeScript::setScript(const QDeclarativeScriptString &s)
{
   Q_D(QDeclarativeStateChangeScript);
   d->script = s;
}

/*!
    \qmlproperty string StateChangeScript::name
    This property holds the name of the script. This name can be used by a
    ScriptAction to target a specific script.

    \sa ScriptAction::scriptName
*/
QString QDeclarativeStateChangeScript::name() const
{
   Q_D(const QDeclarativeStateChangeScript);
   return d->name;
}

void QDeclarativeStateChangeScript::setName(const QString &n)
{
   Q_D(QDeclarativeStateChangeScript);
   d->name = n;
}

void QDeclarativeStateChangeScript::execute(Reason)
{
   Q_D(QDeclarativeStateChangeScript);
   const QString &script = d->script.script();
   if (!script.isEmpty()) {
      QDeclarativeExpression expr(d->script.context(), d->script.scopeObject(), script);
      QDeclarativeData *ddata = QDeclarativeData::get(this);
      if (ddata && ddata->outerContext && !ddata->outerContext->url.isEmpty()) {
         expr.setSourceLocation(ddata->outerContext->url.toString(), ddata->lineNumber);
      }
      expr.evaluate();
      if (expr.hasError()) {
         qmlInfo(this, expr.error());
      }
   }
}

QDeclarativeStateChangeScript::ActionList QDeclarativeStateChangeScript::actions()
{
   ActionList rv;
   QDeclarativeAction a;
   a.event = this;
   rv << a;
   return rv;
}

QString QDeclarativeStateChangeScript::typeName() const
{
   return QLatin1String("StateChangeScript");
}

/*!
    \qmlclass AnchorChanges QDeclarativeAnchorChanges
    \ingroup qml-state-elements
    \brief The AnchorChanges element allows you to change the anchors of an item in a state.

    The AnchorChanges element is used to modify the anchors of an item in a \l State.

    AnchorChanges cannot be used to modify the margins on an item. For this, use
    PropertyChanges intead.

    In the following example we change the top and bottom anchors of an item
    using AnchorChanges, and the top and bottom anchor margins using
    PropertyChanges:

    \snippet doc/src/snippets/declarative/anchorchanges.qml 0

    \image anchorchanges.png

    AnchorChanges can be animated using AnchorAnimation.
    \qml
    //animate our anchor changes
    Transition {
        AnchorAnimation {}
    }
    \endqml

    Margin animations can be animated using NumberAnimation.

    For more information on anchors see \l {anchor-layout}{Anchor Layouts}.
*/

class QDeclarativeAnchorSetPrivate
{
   Q_DECLARE_PUBLIC(QDeclarativeAnchorSet)
 public:
   QDeclarativeAnchorSetPrivate()
      : usedAnchors(0), resetAnchors(0), fill(0),
        centerIn(0)/*, leftMargin(0), rightMargin(0), topMargin(0), bottomMargin(0),
        margins(0), vCenterOffset(0), hCenterOffset(0), baselineOffset(0)*/
   {
   }

   QDeclarativeAnchors::Anchors usedAnchors;
   QDeclarativeAnchors::Anchors resetAnchors;

   QDeclarativeItem *fill;
   QDeclarativeItem *centerIn;

   QDeclarativeScriptString leftScript;
   QDeclarativeScriptString rightScript;
   QDeclarativeScriptString topScript;
   QDeclarativeScriptString bottomScript;
   QDeclarativeScriptString hCenterScript;
   QDeclarativeScriptString vCenterScript;
   QDeclarativeScriptString baselineScript;

   /*qreal leftMargin;
   qreal rightMargin;
   qreal topMargin;
   qreal bottomMargin;
   qreal margins;
   qreal vCenterOffset;
   qreal hCenterOffset;
   qreal baselineOffset;*/
};

QDeclarativeAnchorSet::QDeclarativeAnchorSet(QObject *parent)
   : QObject(*new QDeclarativeAnchorSetPrivate, parent)
{
}

QDeclarativeAnchorSet::~QDeclarativeAnchorSet()
{
}

QDeclarativeScriptString QDeclarativeAnchorSet::top() const
{
   Q_D(const QDeclarativeAnchorSet);
   return d->topScript;
}

void QDeclarativeAnchorSet::setTop(const QDeclarativeScriptString &edge)
{
   Q_D(QDeclarativeAnchorSet);
   d->usedAnchors |= QDeclarativeAnchors::TopAnchor;
   d->topScript = edge;
   if (edge.script() == QLatin1String("undefined")) {
      resetTop();
   }
}

void QDeclarativeAnchorSet::resetTop()
{
   Q_D(QDeclarativeAnchorSet);
   d->usedAnchors &= ~QDeclarativeAnchors::TopAnchor;
   d->topScript = QDeclarativeScriptString();
   d->resetAnchors |= QDeclarativeAnchors::TopAnchor;
}

QDeclarativeScriptString QDeclarativeAnchorSet::bottom() const
{
   Q_D(const QDeclarativeAnchorSet);
   return d->bottomScript;
}

void QDeclarativeAnchorSet::setBottom(const QDeclarativeScriptString &edge)
{
   Q_D(QDeclarativeAnchorSet);
   d->usedAnchors |= QDeclarativeAnchors::BottomAnchor;
   d->bottomScript = edge;
   if (edge.script() == QLatin1String("undefined")) {
      resetBottom();
   }
}

void QDeclarativeAnchorSet::resetBottom()
{
   Q_D(QDeclarativeAnchorSet);
   d->usedAnchors &= ~QDeclarativeAnchors::BottomAnchor;
   d->bottomScript = QDeclarativeScriptString();
   d->resetAnchors |= QDeclarativeAnchors::BottomAnchor;
}

QDeclarativeScriptString QDeclarativeAnchorSet::verticalCenter() const
{
   Q_D(const QDeclarativeAnchorSet);
   return d->vCenterScript;
}

void QDeclarativeAnchorSet::setVerticalCenter(const QDeclarativeScriptString &edge)
{
   Q_D(QDeclarativeAnchorSet);
   d->usedAnchors |= QDeclarativeAnchors::VCenterAnchor;
   d->vCenterScript = edge;
   if (edge.script() == QLatin1String("undefined")) {
      resetVerticalCenter();
   }
}

void QDeclarativeAnchorSet::resetVerticalCenter()
{
   Q_D(QDeclarativeAnchorSet);
   d->usedAnchors &= ~QDeclarativeAnchors::VCenterAnchor;
   d->vCenterScript = QDeclarativeScriptString();
   d->resetAnchors |= QDeclarativeAnchors::VCenterAnchor;
}

QDeclarativeScriptString QDeclarativeAnchorSet::baseline() const
{
   Q_D(const QDeclarativeAnchorSet);
   return d->baselineScript;
}

void QDeclarativeAnchorSet::setBaseline(const QDeclarativeScriptString &edge)
{
   Q_D(QDeclarativeAnchorSet);
   d->usedAnchors |= QDeclarativeAnchors::BaselineAnchor;
   d->baselineScript = edge;
   if (edge.script() == QLatin1String("undefined")) {
      resetBaseline();
   }
}

void QDeclarativeAnchorSet::resetBaseline()
{
   Q_D(QDeclarativeAnchorSet);
   d->usedAnchors &= ~QDeclarativeAnchors::BaselineAnchor;
   d->baselineScript = QDeclarativeScriptString();
   d->resetAnchors |= QDeclarativeAnchors::BaselineAnchor;
}

QDeclarativeScriptString QDeclarativeAnchorSet::left() const
{
   Q_D(const QDeclarativeAnchorSet);
   return d->leftScript;
}

void QDeclarativeAnchorSet::setLeft(const QDeclarativeScriptString &edge)
{
   Q_D(QDeclarativeAnchorSet);
   d->usedAnchors |= QDeclarativeAnchors::LeftAnchor;
   d->leftScript = edge;
   if (edge.script() == QLatin1String("undefined")) {
      resetLeft();
   }
}

void QDeclarativeAnchorSet::resetLeft()
{
   Q_D(QDeclarativeAnchorSet);
   d->usedAnchors &= ~QDeclarativeAnchors::LeftAnchor;
   d->leftScript = QDeclarativeScriptString();
   d->resetAnchors |= QDeclarativeAnchors::LeftAnchor;
}

QDeclarativeScriptString QDeclarativeAnchorSet::right() const
{
   Q_D(const QDeclarativeAnchorSet);
   return d->rightScript;
}

void QDeclarativeAnchorSet::setRight(const QDeclarativeScriptString &edge)
{
   Q_D(QDeclarativeAnchorSet);
   d->usedAnchors |= QDeclarativeAnchors::RightAnchor;
   d->rightScript = edge;
   if (edge.script() == QLatin1String("undefined")) {
      resetRight();
   }
}

void QDeclarativeAnchorSet::resetRight()
{
   Q_D(QDeclarativeAnchorSet);
   d->usedAnchors &= ~QDeclarativeAnchors::RightAnchor;
   d->rightScript = QDeclarativeScriptString();
   d->resetAnchors |= QDeclarativeAnchors::RightAnchor;
}

QDeclarativeScriptString QDeclarativeAnchorSet::horizontalCenter() const
{
   Q_D(const QDeclarativeAnchorSet);
   return d->hCenterScript;
}

void QDeclarativeAnchorSet::setHorizontalCenter(const QDeclarativeScriptString &edge)
{
   Q_D(QDeclarativeAnchorSet);
   d->usedAnchors |= QDeclarativeAnchors::HCenterAnchor;
   d->hCenterScript = edge;
   if (edge.script() == QLatin1String("undefined")) {
      resetHorizontalCenter();
   }
}

void QDeclarativeAnchorSet::resetHorizontalCenter()
{
   Q_D(QDeclarativeAnchorSet);
   d->usedAnchors &= ~QDeclarativeAnchors::HCenterAnchor;
   d->hCenterScript = QDeclarativeScriptString();
   d->resetAnchors |= QDeclarativeAnchors::HCenterAnchor;
}

QDeclarativeItem *QDeclarativeAnchorSet::fill() const
{
   Q_D(const QDeclarativeAnchorSet);
   return d->fill;
}

void QDeclarativeAnchorSet::setFill(QDeclarativeItem *f)
{
   Q_D(QDeclarativeAnchorSet);
   d->fill = f;
}

void QDeclarativeAnchorSet::resetFill()
{
   setFill(0);
}

QDeclarativeItem *QDeclarativeAnchorSet::centerIn() const
{
   Q_D(const QDeclarativeAnchorSet);
   return d->centerIn;
}

void QDeclarativeAnchorSet::setCenterIn(QDeclarativeItem *c)
{
   Q_D(QDeclarativeAnchorSet);
   d->centerIn = c;
}

void QDeclarativeAnchorSet::resetCenterIn()
{
   setCenterIn(0);
}


class QDeclarativeAnchorChangesPrivate : public QDeclarativeStateOperationPrivate
{
 public:
   QDeclarativeAnchorChangesPrivate()
      : target(0), anchorSet(new QDeclarativeAnchorSet),
        leftBinding(0), rightBinding(0), hCenterBinding(0),
        topBinding(0), bottomBinding(0), vCenterBinding(0), baselineBinding(0),
        origLeftBinding(0), origRightBinding(0), origHCenterBinding(0),
        origTopBinding(0), origBottomBinding(0), origVCenterBinding(0),
        origBaselineBinding(0) {

   }
   ~QDeclarativeAnchorChangesPrivate() {
      delete anchorSet;
   }

   QDeclarativeItem *target;
   QDeclarativeAnchorSet *anchorSet;

   QDeclarativeBinding *leftBinding;
   QDeclarativeBinding *rightBinding;
   QDeclarativeBinding *hCenterBinding;
   QDeclarativeBinding *topBinding;
   QDeclarativeBinding *bottomBinding;
   QDeclarativeBinding *vCenterBinding;
   QDeclarativeBinding *baselineBinding;

   QDeclarativeAbstractBinding *origLeftBinding;
   QDeclarativeAbstractBinding *origRightBinding;
   QDeclarativeAbstractBinding *origHCenterBinding;
   QDeclarativeAbstractBinding *origTopBinding;
   QDeclarativeAbstractBinding *origBottomBinding;
   QDeclarativeAbstractBinding *origVCenterBinding;
   QDeclarativeAbstractBinding *origBaselineBinding;

   QDeclarativeAnchorLine rewindLeft;
   QDeclarativeAnchorLine rewindRight;
   QDeclarativeAnchorLine rewindHCenter;
   QDeclarativeAnchorLine rewindTop;
   QDeclarativeAnchorLine rewindBottom;
   QDeclarativeAnchorLine rewindVCenter;
   QDeclarativeAnchorLine rewindBaseline;

   qreal fromX;
   qreal fromY;
   qreal fromWidth;
   qreal fromHeight;

   qreal toX;
   qreal toY;
   qreal toWidth;
   qreal toHeight;

   qreal rewindX;
   qreal rewindY;
   qreal rewindWidth;
   qreal rewindHeight;

   bool applyOrigLeft;
   bool applyOrigRight;
   bool applyOrigHCenter;
   bool applyOrigTop;
   bool applyOrigBottom;
   bool applyOrigVCenter;
   bool applyOrigBaseline;

   QDeclarativeNullableValue<qreal> origWidth;
   QDeclarativeNullableValue<qreal> origHeight;
   qreal origX;
   qreal origY;

   QList<QDeclarativeAbstractBinding *> oldBindings;

   QDeclarativeProperty leftProp;
   QDeclarativeProperty rightProp;
   QDeclarativeProperty hCenterProp;
   QDeclarativeProperty topProp;
   QDeclarativeProperty bottomProp;
   QDeclarativeProperty vCenterProp;
   QDeclarativeProperty baselineProp;
};

/*!
    \qmlproperty Item AnchorChanges::target
    This property holds the \l Item for which the anchor changes will be applied.
*/

QDeclarativeAnchorChanges::QDeclarativeAnchorChanges(QObject *parent)
   : QDeclarativeStateOperation(*(new QDeclarativeAnchorChangesPrivate), parent)
{
}

QDeclarativeAnchorChanges::~QDeclarativeAnchorChanges()
{
}

QDeclarativeAnchorChanges::ActionList QDeclarativeAnchorChanges::actions()
{
   Q_D(QDeclarativeAnchorChanges);
   d->leftBinding = d->rightBinding = d->hCenterBinding = d->topBinding
                                      = d->bottomBinding = d->vCenterBinding = d->baselineBinding = 0;

   d->leftProp = QDeclarativeProperty(d->target, QLatin1String("anchors.left"));
   d->rightProp = QDeclarativeProperty(d->target, QLatin1String("anchors.right"));
   d->hCenterProp = QDeclarativeProperty(d->target, QLatin1String("anchors.horizontalCenter"));
   d->topProp = QDeclarativeProperty(d->target, QLatin1String("anchors.top"));
   d->bottomProp = QDeclarativeProperty(d->target, QLatin1String("anchors.bottom"));
   d->vCenterProp = QDeclarativeProperty(d->target, QLatin1String("anchors.verticalCenter"));
   d->baselineProp = QDeclarativeProperty(d->target, QLatin1String("anchors.baseline"));

   QDeclarativeContext *ctxt = qmlContext(this);

   if (d->anchorSet->d_func()->usedAnchors & QDeclarativeAnchors::LeftAnchor) {
      d->leftBinding = new QDeclarativeBinding(d->anchorSet->d_func()->leftScript.script(), d->target, ctxt);
      d->leftBinding->setTarget(d->leftProp);
   }
   if (d->anchorSet->d_func()->usedAnchors & QDeclarativeAnchors::RightAnchor) {
      d->rightBinding = new QDeclarativeBinding(d->anchorSet->d_func()->rightScript.script(), d->target, ctxt);
      d->rightBinding->setTarget(d->rightProp);
   }
   if (d->anchorSet->d_func()->usedAnchors & QDeclarativeAnchors::HCenterAnchor) {
      d->hCenterBinding = new QDeclarativeBinding(d->anchorSet->d_func()->hCenterScript.script(), d->target, ctxt);
      d->hCenterBinding->setTarget(d->hCenterProp);
   }
   if (d->anchorSet->d_func()->usedAnchors & QDeclarativeAnchors::TopAnchor) {
      d->topBinding = new QDeclarativeBinding(d->anchorSet->d_func()->topScript.script(), d->target, ctxt);
      d->topBinding->setTarget(d->topProp);
   }
   if (d->anchorSet->d_func()->usedAnchors & QDeclarativeAnchors::BottomAnchor) {
      d->bottomBinding = new QDeclarativeBinding(d->anchorSet->d_func()->bottomScript.script(), d->target, ctxt);
      d->bottomBinding->setTarget(d->bottomProp);
   }
   if (d->anchorSet->d_func()->usedAnchors & QDeclarativeAnchors::VCenterAnchor) {
      d->vCenterBinding = new QDeclarativeBinding(d->anchorSet->d_func()->vCenterScript.script(), d->target, ctxt);
      d->vCenterBinding->setTarget(d->vCenterProp);
   }
   if (d->anchorSet->d_func()->usedAnchors & QDeclarativeAnchors::BaselineAnchor) {
      d->baselineBinding = new QDeclarativeBinding(d->anchorSet->d_func()->baselineScript.script(), d->target, ctxt);
      d->baselineBinding->setTarget(d->baselineProp);
   }

   QDeclarativeAction a;
   a.event = this;
   return ActionList() << a;
}

QDeclarativeAnchorSet *QDeclarativeAnchorChanges::anchors()
{
   Q_D(QDeclarativeAnchorChanges);
   return d->anchorSet;
}

QDeclarativeItem *QDeclarativeAnchorChanges::object() const
{
   Q_D(const QDeclarativeAnchorChanges);
   return d->target;
}

void QDeclarativeAnchorChanges::setObject(QDeclarativeItem *target)
{
   Q_D(QDeclarativeAnchorChanges);
   d->target = target;
}

/*!
    \qmlproperty AnchorLine AnchorChanges::anchors.left
    \qmlproperty AnchorLine AnchorChanges::anchors.right
    \qmlproperty AnchorLine AnchorChanges::anchors.horizontalCenter
    \qmlproperty AnchorLine AnchorChanges::anchors.top
    \qmlproperty AnchorLine AnchorChanges::anchors.bottom
    \qmlproperty AnchorLine AnchorChanges::anchors.verticalCenter
    \qmlproperty AnchorLine AnchorChanges::anchors.baseline

    These properties change the respective anchors of the item.

    To reset an anchor you can assign \c undefined:
    \qml
    AnchorChanges {
        target: myItem
        anchors.left: undefined          //remove myItem's left anchor
        anchors.right: otherItem.right
    }
    \endqml
*/

void QDeclarativeAnchorChanges::execute(Reason reason)
{
   Q_D(QDeclarativeAnchorChanges);
   if (!d->target) {
      return;
   }

   QDeclarativeItemPrivate *targetPrivate = QDeclarativeItemPrivate::get(d->target);
   //incorporate any needed "reverts"
   if (d->applyOrigLeft) {
      if (!d->origLeftBinding) {
         targetPrivate->anchors()->resetLeft();
      }
      QDeclarativePropertyPrivate::setBinding(d->leftProp, d->origLeftBinding);
   }
   if (d->applyOrigRight) {
      if (!d->origRightBinding) {
         targetPrivate->anchors()->resetRight();
      }
      QDeclarativePropertyPrivate::setBinding(d->rightProp, d->origRightBinding);
   }
   if (d->applyOrigHCenter) {
      if (!d->origHCenterBinding) {
         targetPrivate->anchors()->resetHorizontalCenter();
      }
      QDeclarativePropertyPrivate::setBinding(d->hCenterProp, d->origHCenterBinding);
   }
   if (d->applyOrigTop) {
      if (!d->origTopBinding) {
         targetPrivate->anchors()->resetTop();
      }
      QDeclarativePropertyPrivate::setBinding(d->topProp, d->origTopBinding);
   }
   if (d->applyOrigBottom) {
      if (!d->origBottomBinding) {
         targetPrivate->anchors()->resetBottom();
      }
      QDeclarativePropertyPrivate::setBinding(d->bottomProp, d->origBottomBinding);
   }
   if (d->applyOrigVCenter) {
      if (!d->origVCenterBinding) {
         targetPrivate->anchors()->resetVerticalCenter();
      }
      QDeclarativePropertyPrivate::setBinding(d->vCenterProp, d->origVCenterBinding);
   }
   if (d->applyOrigBaseline) {
      if (!d->origBaselineBinding) {
         targetPrivate->anchors()->resetBaseline();
      }
      QDeclarativePropertyPrivate::setBinding(d->baselineProp, d->origBaselineBinding);
   }

   //destroy old bindings
   if (reason == ActualChange) {
      for (int i = 0; i < d->oldBindings.size(); ++i) {
         QDeclarativeAbstractBinding *binding = d->oldBindings.at(i);
         if (binding) {
            binding->destroy();
         }
      }
      d->oldBindings.clear();
   }

   //reset any anchors that have been specified as "undefined"
   if (d->anchorSet->d_func()->resetAnchors & QDeclarativeAnchors::LeftAnchor) {
      targetPrivate->anchors()->resetLeft();
      QDeclarativePropertyPrivate::setBinding(d->leftProp, 0);
   }
   if (d->anchorSet->d_func()->resetAnchors & QDeclarativeAnchors::RightAnchor) {
      targetPrivate->anchors()->resetRight();
      QDeclarativePropertyPrivate::setBinding(d->rightProp, 0);
   }
   if (d->anchorSet->d_func()->resetAnchors & QDeclarativeAnchors::HCenterAnchor) {
      targetPrivate->anchors()->resetHorizontalCenter();
      QDeclarativePropertyPrivate::setBinding(d->hCenterProp, 0);
   }
   if (d->anchorSet->d_func()->resetAnchors & QDeclarativeAnchors::TopAnchor) {
      targetPrivate->anchors()->resetTop();
      QDeclarativePropertyPrivate::setBinding(d->topProp, 0);
   }
   if (d->anchorSet->d_func()->resetAnchors & QDeclarativeAnchors::BottomAnchor) {
      targetPrivate->anchors()->resetBottom();
      QDeclarativePropertyPrivate::setBinding(d->bottomProp, 0);
   }
   if (d->anchorSet->d_func()->resetAnchors & QDeclarativeAnchors::VCenterAnchor) {
      targetPrivate->anchors()->resetVerticalCenter();
      QDeclarativePropertyPrivate::setBinding(d->vCenterProp, 0);
   }
   if (d->anchorSet->d_func()->resetAnchors & QDeclarativeAnchors::BaselineAnchor) {
      targetPrivate->anchors()->resetBaseline();
      QDeclarativePropertyPrivate::setBinding(d->baselineProp, 0);
   }

   //set any anchors that have been specified
   if (d->leftBinding) {
      QDeclarativePropertyPrivate::setBinding(d->leftBinding->property(), d->leftBinding);
   }
   if (d->rightBinding) {
      QDeclarativePropertyPrivate::setBinding(d->rightBinding->property(), d->rightBinding);
   }
   if (d->hCenterBinding) {
      QDeclarativePropertyPrivate::setBinding(d->hCenterBinding->property(), d->hCenterBinding);
   }
   if (d->topBinding) {
      QDeclarativePropertyPrivate::setBinding(d->topBinding->property(), d->topBinding);
   }
   if (d->bottomBinding) {
      QDeclarativePropertyPrivate::setBinding(d->bottomBinding->property(), d->bottomBinding);
   }
   if (d->vCenterBinding) {
      QDeclarativePropertyPrivate::setBinding(d->vCenterBinding->property(), d->vCenterBinding);
   }
   if (d->baselineBinding) {
      QDeclarativePropertyPrivate::setBinding(d->baselineBinding->property(), d->baselineBinding);
   }
}

bool QDeclarativeAnchorChanges::isReversable()
{
   return true;
}

void QDeclarativeAnchorChanges::reverse(Reason reason)
{
   Q_D(QDeclarativeAnchorChanges);
   if (!d->target) {
      return;
   }

   QDeclarativeItemPrivate *targetPrivate = QDeclarativeItemPrivate::get(d->target);
   //reset any anchors set by the state
   if (d->leftBinding) {
      targetPrivate->anchors()->resetLeft();
      QDeclarativePropertyPrivate::setBinding(d->leftBinding->property(), 0);
      if (reason == ActualChange) {
         d->leftBinding->destroy();
         d->leftBinding = 0;
      }
   }
   if (d->rightBinding) {
      targetPrivate->anchors()->resetRight();
      QDeclarativePropertyPrivate::setBinding(d->rightBinding->property(), 0);
      if (reason == ActualChange) {
         d->rightBinding->destroy();
         d->rightBinding = 0;
      }
   }
   if (d->hCenterBinding) {
      targetPrivate->anchors()->resetHorizontalCenter();
      QDeclarativePropertyPrivate::setBinding(d->hCenterBinding->property(), 0);
      if (reason == ActualChange) {
         d->hCenterBinding->destroy();
         d->hCenterBinding = 0;
      }
   }
   if (d->topBinding) {
      targetPrivate->anchors()->resetTop();
      QDeclarativePropertyPrivate::setBinding(d->topBinding->property(), 0);
      if (reason == ActualChange) {
         d->topBinding->destroy();
         d->topBinding = 0;
      }
   }
   if (d->bottomBinding) {
      targetPrivate->anchors()->resetBottom();
      QDeclarativePropertyPrivate::setBinding(d->bottomBinding->property(), 0);
      if (reason == ActualChange) {
         d->bottomBinding->destroy();
         d->bottomBinding = 0;
      }
   }
   if (d->vCenterBinding) {
      targetPrivate->anchors()->resetVerticalCenter();
      QDeclarativePropertyPrivate::setBinding(d->vCenterBinding->property(), 0);
      if (reason == ActualChange) {
         d->vCenterBinding->destroy();
         d->vCenterBinding = 0;
      }
   }
   if (d->baselineBinding) {
      targetPrivate->anchors()->resetBaseline();
      QDeclarativePropertyPrivate::setBinding(d->baselineBinding->property(), 0);
      if (reason == ActualChange) {
         d->baselineBinding->destroy();
         d->baselineBinding = 0;
      }
   }

   //restore previous anchors
   if (d->origLeftBinding) {
      QDeclarativePropertyPrivate::setBinding(d->leftProp, d->origLeftBinding);
   }
   if (d->origRightBinding) {
      QDeclarativePropertyPrivate::setBinding(d->rightProp, d->origRightBinding);
   }
   if (d->origHCenterBinding) {
      QDeclarativePropertyPrivate::setBinding(d->hCenterProp, d->origHCenterBinding);
   }
   if (d->origTopBinding) {
      QDeclarativePropertyPrivate::setBinding(d->topProp, d->origTopBinding);
   }
   if (d->origBottomBinding) {
      QDeclarativePropertyPrivate::setBinding(d->bottomProp, d->origBottomBinding);
   }
   if (d->origVCenterBinding) {
      QDeclarativePropertyPrivate::setBinding(d->vCenterProp, d->origVCenterBinding);
   }
   if (d->origBaselineBinding) {
      QDeclarativePropertyPrivate::setBinding(d->baselineProp, d->origBaselineBinding);
   }

   //restore any absolute geometry changed by the state's anchors
   QDeclarativeAnchors::Anchors stateVAnchors = d->anchorSet->d_func()->usedAnchors & QDeclarativeAnchors::Vertical_Mask;
   QDeclarativeAnchors::Anchors origVAnchors = targetPrivate->anchors()->usedAnchors() &
         QDeclarativeAnchors::Vertical_Mask;
   QDeclarativeAnchors::Anchors stateHAnchors = d->anchorSet->d_func()->usedAnchors & QDeclarativeAnchors::Horizontal_Mask;
   QDeclarativeAnchors::Anchors origHAnchors = targetPrivate->anchors()->usedAnchors() &
         QDeclarativeAnchors::Horizontal_Mask;

   bool stateSetWidth = (stateHAnchors &&
                         stateHAnchors != QDeclarativeAnchors::LeftAnchor &&
                         stateHAnchors != QDeclarativeAnchors::RightAnchor &&
                         stateHAnchors != QDeclarativeAnchors::HCenterAnchor);
   bool origSetWidth = (origHAnchors &&
                        origHAnchors != QDeclarativeAnchors::LeftAnchor &&
                        origHAnchors != QDeclarativeAnchors::RightAnchor &&
                        origHAnchors != QDeclarativeAnchors::HCenterAnchor);
   if (d->origWidth.isValid() && stateSetWidth && !origSetWidth) {
      d->target->setWidth(d->origWidth.value);
   }

   bool stateSetHeight = (stateVAnchors &&
                          stateVAnchors != QDeclarativeAnchors::TopAnchor &&
                          stateVAnchors != QDeclarativeAnchors::BottomAnchor &&
                          stateVAnchors != QDeclarativeAnchors::VCenterAnchor &&
                          stateVAnchors != QDeclarativeAnchors::BaselineAnchor);
   bool origSetHeight = (origVAnchors &&
                         origVAnchors != QDeclarativeAnchors::TopAnchor &&
                         origVAnchors != QDeclarativeAnchors::BottomAnchor &&
                         origVAnchors != QDeclarativeAnchors::VCenterAnchor &&
                         origVAnchors != QDeclarativeAnchors::BaselineAnchor);
   if (d->origHeight.isValid() && stateSetHeight && !origSetHeight) {
      d->target->setHeight(d->origHeight.value);
   }

   if (stateHAnchors && !origHAnchors) {
      d->target->setX(d->origX);
   }

   if (stateVAnchors && !origVAnchors) {
      d->target->setY(d->origY);
   }
}

QString QDeclarativeAnchorChanges::typeName() const
{
   return QLatin1String("AnchorChanges");
}

QList<QDeclarativeAction> QDeclarativeAnchorChanges::additionalActions()
{
   Q_D(QDeclarativeAnchorChanges);
   QList<QDeclarativeAction> extra;

   QDeclarativeAnchors::Anchors combined = d->anchorSet->d_func()->usedAnchors | d->anchorSet->d_func()->resetAnchors;
   bool hChange = combined & QDeclarativeAnchors::Horizontal_Mask;
   bool vChange = combined & QDeclarativeAnchors::Vertical_Mask;

   if (d->target) {
      QDeclarativeContext *ctxt = qmlContext(this);
      QDeclarativeAction a;
      if (hChange && d->fromX != d->toX) {
         a.property = QDeclarativeProperty(d->target, QLatin1String("x"), ctxt);
         a.toValue = d->toX;
         extra << a;
      }
      if (vChange && d->fromY != d->toY) {
         a.property = QDeclarativeProperty(d->target, QLatin1String("y"), ctxt);
         a.toValue = d->toY;
         extra << a;
      }
      if (hChange && d->fromWidth != d->toWidth) {
         a.property = QDeclarativeProperty(d->target, QLatin1String("width"), ctxt);
         a.toValue = d->toWidth;
         extra << a;
      }
      if (vChange && d->fromHeight != d->toHeight) {
         a.property = QDeclarativeProperty(d->target, QLatin1String("height"), ctxt);
         a.toValue = d->toHeight;
         extra << a;
      }
   }

   return extra;
}

bool QDeclarativeAnchorChanges::changesBindings()
{
   return true;
}

void QDeclarativeAnchorChanges::saveOriginals()
{
   Q_D(QDeclarativeAnchorChanges);
   if (!d->target) {
      return;
   }

   d->origLeftBinding = QDeclarativePropertyPrivate::binding(d->leftProp);
   d->origRightBinding = QDeclarativePropertyPrivate::binding(d->rightProp);
   d->origHCenterBinding = QDeclarativePropertyPrivate::binding(d->hCenterProp);
   d->origTopBinding = QDeclarativePropertyPrivate::binding(d->topProp);
   d->origBottomBinding = QDeclarativePropertyPrivate::binding(d->bottomProp);
   d->origVCenterBinding = QDeclarativePropertyPrivate::binding(d->vCenterProp);
   d->origBaselineBinding = QDeclarativePropertyPrivate::binding(d->baselineProp);

   QDeclarativeItemPrivate *targetPrivate = QDeclarativeItemPrivate::get(d->target);
   if (targetPrivate->widthValid) {
      d->origWidth = d->target->width();
   }
   if (targetPrivate->heightValid) {
      d->origHeight = d->target->height();
   }
   d->origX = d->target->x();
   d->origY = d->target->y();

   d->applyOrigLeft = d->applyOrigRight = d->applyOrigHCenter = d->applyOrigTop
                                          = d->applyOrigBottom = d->applyOrigVCenter = d->applyOrigBaseline = false;

   saveCurrentValues();
}

void QDeclarativeAnchorChanges::copyOriginals(QDeclarativeActionEvent *other)
{
   Q_D(QDeclarativeAnchorChanges);
   QDeclarativeAnchorChanges *ac = static_cast<QDeclarativeAnchorChanges *>(other);
   QDeclarativeAnchorChangesPrivate *acp = ac->d_func();

   QDeclarativeAnchors::Anchors combined = acp->anchorSet->d_func()->usedAnchors |
                                           acp->anchorSet->d_func()->resetAnchors;

   //probably also need to revert some things
   d->applyOrigLeft = (combined & QDeclarativeAnchors::LeftAnchor);
   d->applyOrigRight = (combined & QDeclarativeAnchors::RightAnchor);
   d->applyOrigHCenter = (combined & QDeclarativeAnchors::HCenterAnchor);
   d->applyOrigTop = (combined & QDeclarativeAnchors::TopAnchor);
   d->applyOrigBottom = (combined & QDeclarativeAnchors::BottomAnchor);
   d->applyOrigVCenter = (combined & QDeclarativeAnchors::VCenterAnchor);
   d->applyOrigBaseline = (combined & QDeclarativeAnchors::BaselineAnchor);

   d->origLeftBinding = acp->origLeftBinding;
   d->origRightBinding = acp->origRightBinding;
   d->origHCenterBinding = acp->origHCenterBinding;
   d->origTopBinding = acp->origTopBinding;
   d->origBottomBinding = acp->origBottomBinding;
   d->origVCenterBinding = acp->origVCenterBinding;
   d->origBaselineBinding = acp->origBaselineBinding;

   d->origWidth = acp->origWidth;
   d->origHeight = acp->origHeight;
   d->origX = acp->origX;
   d->origY = acp->origY;

   d->oldBindings.clear();
   d->oldBindings << acp->leftBinding << acp->rightBinding << acp->hCenterBinding
                  << acp->topBinding << acp->bottomBinding << acp->baselineBinding;

   saveCurrentValues();
}

void QDeclarativeAnchorChanges::clearBindings()
{
   Q_D(QDeclarativeAnchorChanges);
   if (!d->target) {
      return;
   }

   //### should this (saving "from" values) be moved to saveCurrentValues()?
   d->fromX = d->target->x();
   d->fromY = d->target->y();
   d->fromWidth = d->target->width();
   d->fromHeight = d->target->height();

   QDeclarativeItemPrivate *targetPrivate = QDeclarativeItemPrivate::get(d->target);
   //reset any anchors with corresponding reverts
   //reset any anchors that have been specified as "undefined"
   //reset any anchors that we'll be setting in the state
   QDeclarativeAnchors::Anchors combined = d->anchorSet->d_func()->resetAnchors |
                                           d->anchorSet->d_func()->usedAnchors;
   if (d->applyOrigLeft || (combined & QDeclarativeAnchors::LeftAnchor)) {
      targetPrivate->anchors()->resetLeft();
      QDeclarativePropertyPrivate::setBinding(d->leftProp, 0);
   }
   if (d->applyOrigRight || (combined & QDeclarativeAnchors::RightAnchor)) {
      targetPrivate->anchors()->resetRight();
      QDeclarativePropertyPrivate::setBinding(d->rightProp, 0);
   }
   if (d->applyOrigHCenter || (combined & QDeclarativeAnchors::HCenterAnchor)) {
      targetPrivate->anchors()->resetHorizontalCenter();
      QDeclarativePropertyPrivate::setBinding(d->hCenterProp, 0);
   }
   if (d->applyOrigTop || (combined & QDeclarativeAnchors::TopAnchor)) {
      targetPrivate->anchors()->resetTop();
      QDeclarativePropertyPrivate::setBinding(d->topProp, 0);
   }
   if (d->applyOrigBottom || (combined & QDeclarativeAnchors::BottomAnchor)) {
      targetPrivate->anchors()->resetBottom();
      QDeclarativePropertyPrivate::setBinding(d->bottomProp, 0);
   }
   if (d->applyOrigVCenter || (combined & QDeclarativeAnchors::VCenterAnchor)) {
      targetPrivate->anchors()->resetVerticalCenter();
      QDeclarativePropertyPrivate::setBinding(d->vCenterProp, 0);
   }
   if (d->applyOrigBaseline || (combined & QDeclarativeAnchors::BaselineAnchor)) {
      targetPrivate->anchors()->resetBaseline();
      QDeclarativePropertyPrivate::setBinding(d->baselineProp, 0);
   }
}

bool QDeclarativeAnchorChanges::override(QDeclarativeActionEvent *other)
{
   if (other->typeName() != QLatin1String("AnchorChanges")) {
      return false;
   }
   if (static_cast<QDeclarativeActionEvent *>(this) == other) {
      return true;
   }
   if (static_cast<QDeclarativeAnchorChanges *>(other)->object() == object()) {
      return true;
   }
   return false;
}

void QDeclarativeAnchorChanges::rewind()
{
   Q_D(QDeclarativeAnchorChanges);
   if (!d->target) {
      return;
   }

   QDeclarativeItemPrivate *targetPrivate = QDeclarativeItemPrivate::get(d->target);

   //restore previous values (but not previous bindings, i.e. anchors)
   d->target->setX(d->rewindX);
   d->target->setY(d->rewindY);
   if (targetPrivate->widthValid) {
      d->target->setWidth(d->rewindWidth);
   }
   if (targetPrivate->heightValid) {
      d->target->setHeight(d->rewindHeight);
   }
}

void QDeclarativeAnchorChanges::saveCurrentValues()
{
   Q_D(QDeclarativeAnchorChanges);
   if (!d->target) {
      return;
   }

   QDeclarativeItemPrivate *targetPrivate = QDeclarativeItemPrivate::get(d->target);
   d->rewindLeft = targetPrivate->anchors()->left();
   d->rewindRight = targetPrivate->anchors()->right();
   d->rewindHCenter = targetPrivate->anchors()->horizontalCenter();
   d->rewindTop = targetPrivate->anchors()->top();
   d->rewindBottom = targetPrivate->anchors()->bottom();
   d->rewindVCenter = targetPrivate->anchors()->verticalCenter();
   d->rewindBaseline = targetPrivate->anchors()->baseline();

   d->rewindX = d->target->x();
   d->rewindY = d->target->y();
   d->rewindWidth = d->target->width();
   d->rewindHeight = d->target->height();
}

void QDeclarativeAnchorChanges::saveTargetValues()
{
   Q_D(QDeclarativeAnchorChanges);
   if (!d->target) {
      return;
   }

   d->toX = d->target->x();
   d->toY = d->target->y();
   d->toWidth = d->target->width();
   d->toHeight = d->target->height();
}

QT_END_NAMESPACE

