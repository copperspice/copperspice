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

#include <qdeclarativepropertychanges_p.h>
#include <qdeclarativeopenmetaobject_p.h>
#include <qdeclarativerewrite_p.h>
#include <qdeclarativeengine_p.h>
#include <qdeclarativecompiler_p.h>
#include <qdeclarativeinfo.h>
#include <qdeclarativecustomparser_p.h>
#include <qdeclarativeparser_p.h>
#include <qdeclarativeexpression.h>
#include <qdeclarativebinding_p.h>
#include <qdeclarativecontext.h>
#include <qdeclarativeguard_p.h>
#include <qdeclarativeproperty_p.h>
#include <qdeclarativecontext_p.h>
#include <qdeclarativestate_p_p.h>
#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

class QDeclarativeReplaceSignalHandler : public QDeclarativeActionEvent
{
 public:
   QDeclarativeReplaceSignalHandler() : expression(0), reverseExpression(0),
      rewindExpression(0), ownedExpression(0) {}
   ~QDeclarativeReplaceSignalHandler() {
      delete ownedExpression;
   }

   virtual QString typeName() const {
      return QLatin1String("ReplaceSignalHandler");
   }

   QDeclarativeProperty property;
   QDeclarativeExpression *expression;
   QDeclarativeExpression *reverseExpression;
   QDeclarativeExpression *rewindExpression;
   QDeclarativeGuard<QDeclarativeExpression> ownedExpression;

   virtual void execute(Reason) {
      ownedExpression = QDeclarativePropertyPrivate::setSignalExpression(property, expression);
      if (ownedExpression == expression) {
         ownedExpression = 0;
      }
   }

   virtual bool isReversable() {
      return true;
   }
   virtual void reverse(Reason) {
      ownedExpression = QDeclarativePropertyPrivate::setSignalExpression(property, reverseExpression);
      if (ownedExpression == reverseExpression) {
         ownedExpression = 0;
      }
   }

   virtual void saveOriginals() {
      saveCurrentValues();
      reverseExpression = rewindExpression;
   }

   virtual bool needsCopy() {
      return true;
   }
   virtual void copyOriginals(QDeclarativeActionEvent *other) {
      QDeclarativeReplaceSignalHandler *rsh = static_cast<QDeclarativeReplaceSignalHandler *>(other);
      saveCurrentValues();
      if (rsh == this) {
         return;
      }
      reverseExpression = rsh->reverseExpression;
      if (rsh->ownedExpression == reverseExpression) {
         ownedExpression = rsh->ownedExpression;
         rsh->ownedExpression = 0;
      }
   }

   virtual void rewind() {
      ownedExpression = QDeclarativePropertyPrivate::setSignalExpression(property, rewindExpression);
      if (ownedExpression == rewindExpression) {
         ownedExpression = 0;
      }
   }
   virtual void saveCurrentValues() {
      rewindExpression = QDeclarativePropertyPrivate::signalExpression(property);
   }

   virtual bool override(QDeclarativeActionEvent *other) {
      if (other == this) {
         return true;
      }
      if (other->typeName() != typeName()) {
         return false;
      }
      if (static_cast<QDeclarativeReplaceSignalHandler *>(other)->property == property) {
         return true;
      }
      return false;
   }
};


class QDeclarativePropertyChangesPrivate : public QDeclarativeStateOperationPrivate
{
   Q_DECLARE_PUBLIC(QDeclarativePropertyChanges)
 public:
   QDeclarativePropertyChangesPrivate() : decoded(true), restore(true),
      isExplicit(false) {}

   QDeclarativeGuard<QObject> object;
   QByteArray data;

   bool decoded : 1;
   bool restore : 1;
   bool isExplicit : 1;

   void decode();

   class ExpressionChange
   {
    public:
      ExpressionChange(const QString &_name,
                       QDeclarativeBinding::Identifier _id,
                       QDeclarativeExpression *_expr)
         : name(_name), id(_id), expression(_expr) {}
      QString name;
      QDeclarativeBinding::Identifier id;
      QDeclarativeExpression *expression;
   };

   QList<QPair<QString, QVariant> > properties;
   QList<ExpressionChange> expressions;
   QList<QDeclarativeReplaceSignalHandler *> signalReplacements;

   QDeclarativeProperty property(const QString &);
};

void
QDeclarativePropertyChangesParser::compileList(QList<QPair<QByteArray, QVariant> > &list,
      const QByteArray &pre,
      const QDeclarativeCustomParserProperty &prop)
{
   QByteArray propName = pre + prop.name();

   QList<QVariant> values = prop.assignedValues();
   for (int ii = 0; ii < values.count(); ++ii) {
      const QVariant &value = values.at(ii);

      if (value.userType() == qMetaTypeId<QDeclarativeCustomParserNode>()) {
         error(qvariant_cast<QDeclarativeCustomParserNode>(value),
               QDeclarativePropertyChanges::tr("PropertyChanges does not support creating state-specific objects."));
         continue;
      } else if (value.userType() == qMetaTypeId<QDeclarativeCustomParserProperty>()) {

         QDeclarativeCustomParserProperty prop =
            qvariant_cast<QDeclarativeCustomParserProperty>(value);
         QByteArray pre = propName + '.';
         compileList(list, pre, prop);

      } else {
         list << qMakePair(propName, value);
      }
   }
}

QByteArray
QDeclarativePropertyChangesParser::compile(const QList<QDeclarativeCustomParserProperty> &props)
{
   QList<QPair<QByteArray, QVariant> > data;
   for (int ii = 0; ii < props.count(); ++ii) {
      compileList(data, QByteArray(), props.at(ii));
   }

   QByteArray rv;
   QDataStream ds(&rv, QIODevice::WriteOnly);

   ds << data.count();
   for (int ii = 0; ii < data.count(); ++ii) {
      QDeclarativeParser::Variant v = qvariant_cast<QDeclarativeParser::Variant>(data.at(ii).second);
      QVariant var;
      bool isScript = v.isScript();
      QDeclarativeBinding::Identifier id = 0;
      switch (v.type()) {
         case QDeclarativeParser::Variant::Boolean:
            var = QVariant(v.asBoolean());
            break;
         case QDeclarativeParser::Variant::Number:
            var = QVariant(v.asNumber());
            break;
         case QDeclarativeParser::Variant::String:
            var = QVariant(v.asString());
            break;
         case QDeclarativeParser::Variant::Invalid:
         case QDeclarativeParser::Variant::Script:
            var = QVariant(v.asScript());
            {
               // Pre-rewrite the expression
               QString expression = v.asScript();
               id = rewriteBinding(expression, data.at(ii).first); //### recreates the AST, which is slow
            }
            break;
      }

      ds << QString::fromUtf8(data.at(ii).first) << isScript << var;
      if (isScript) {
         ds << id;
      }
   }

   return rv;
}

void QDeclarativePropertyChangesPrivate::decode()
{
   Q_Q(QDeclarativePropertyChanges);
   if (decoded) {
      return;
   }

   QDataStream ds(&data, QIODevice::ReadOnly);

   int count;
   ds >> count;
   for (int ii = 0; ii < count; ++ii) {
      QString name;
      bool isScript;
      QVariant data;
      QDeclarativeBinding::Identifier id = QDeclarativeBinding::Invalid;
      ds >> name;
      ds >> isScript;
      ds >> data;
      if (isScript) {
         ds >> id;
      }

      QDeclarativeProperty prop = property(name);      //### better way to check for signal property?
      if (prop.type() & QDeclarativeProperty::SignalProperty) {
         QDeclarativeExpression *expression = new QDeclarativeExpression(qmlContext(q), object, data.toString());
         QDeclarativeData *ddata = QDeclarativeData::get(q);
         if (ddata && ddata->outerContext && !ddata->outerContext->url.isEmpty()) {
            expression->setSourceLocation(ddata->outerContext->url.toString(), ddata->lineNumber);
         }
         QDeclarativeReplaceSignalHandler *handler = new QDeclarativeReplaceSignalHandler;
         handler->property = prop;
         handler->expression = expression;
         signalReplacements << handler;
      } else if (isScript) {
         QDeclarativeExpression *expression = new QDeclarativeExpression(qmlContext(q), object, data.toString());
         QDeclarativeData *ddata = QDeclarativeData::get(q);
         if (ddata && ddata->outerContext && !ddata->outerContext->url.isEmpty()) {
            expression->setSourceLocation(ddata->outerContext->url.toString(), ddata->lineNumber);
         }
         expressions << ExpressionChange(name, id, expression);
      } else {
         properties << qMakePair(name, data);
      }
   }

   decoded = true;
   data.clear();
}

void QDeclarativePropertyChangesParser::setCustomData(QObject *object,
      const QByteArray &data)
{
   QDeclarativePropertyChangesPrivate *p =
      static_cast<QDeclarativePropertyChangesPrivate *>(QObjectPrivate::get(object));
   p->data = data;
   p->decoded = false;
}

QDeclarativePropertyChanges::QDeclarativePropertyChanges()
   : QDeclarativeStateOperation(*(new QDeclarativePropertyChangesPrivate))
{
}

QDeclarativePropertyChanges::~QDeclarativePropertyChanges()
{
   Q_D(QDeclarativePropertyChanges);
   for (int ii = 0; ii < d->expressions.count(); ++ii) {
      delete d->expressions.at(ii).expression;
   }
   for (int ii = 0; ii < d->signalReplacements.count(); ++ii) {
      delete d->signalReplacements.at(ii);
   }
}

QObject *QDeclarativePropertyChanges::object() const
{
   Q_D(const QDeclarativePropertyChanges);
   return d->object;
}

void QDeclarativePropertyChanges::setObject(QObject *o)
{
   Q_D(QDeclarativePropertyChanges);
   d->object = o;
}

/*!
    \qmlproperty bool PropertyChanges::restoreEntryValues

    This property holds whether the previous values should be restored when
    leaving the state.

    The default value is \c true. Setting this value to \c false creates a
    temporary state that has permanent effects on property values.
*/
bool QDeclarativePropertyChanges::restoreEntryValues() const
{
   Q_D(const QDeclarativePropertyChanges);
   return d->restore;
}

void QDeclarativePropertyChanges::setRestoreEntryValues(bool v)
{
   Q_D(QDeclarativePropertyChanges);
   d->restore = v;
}

QDeclarativeProperty
QDeclarativePropertyChangesPrivate::property(const QString &property)
{
   Q_Q(QDeclarativePropertyChanges);
   QDeclarativeProperty prop(object, property, qmlContext(q));
   if (!prop.isValid()) {
      qmlInfo(q) << QDeclarativePropertyChanges::tr("Cannot assign to non-existent property \"%1\"").arg(property);
      return QDeclarativeProperty();
   } else if (!(prop.type() & QDeclarativeProperty::SignalProperty) && !prop.isWritable()) {
      qmlInfo(q) << QDeclarativePropertyChanges::tr("Cannot assign to read-only property \"%1\"").arg(property);
      return QDeclarativeProperty();
   }
   return prop;
}

QDeclarativePropertyChanges::ActionList QDeclarativePropertyChanges::actions()
{
   Q_D(QDeclarativePropertyChanges);

   d->decode();

   ActionList list;

   for (int ii = 0; ii < d->properties.count(); ++ii) {

      QDeclarativeAction a(d->object, d->properties.at(ii).first,
                           qmlContext(this), d->properties.at(ii).second);

      if (a.property.isValid()) {
         a.restore = restoreEntryValues();
         list << a;
      }
   }

   for (int ii = 0; ii < d->signalReplacements.count(); ++ii) {

      QDeclarativeReplaceSignalHandler *handler = d->signalReplacements.at(ii);

      if (handler->property.isValid()) {
         QDeclarativeAction a;
         a.event = handler;
         list << a;
      }
   }

   for (int ii = 0; ii < d->expressions.count(); ++ii) {

      const QString &property = d->expressions.at(ii).name;
      QDeclarativeProperty prop = d->property(property);

      if (prop.isValid()) {
         QDeclarativeAction a;
         a.restore = restoreEntryValues();
         a.property = prop;
         a.fromValue = a.property.read();
         a.specifiedObject = d->object;
         a.specifiedProperty = property;

         if (d->isExplicit) {
            a.toValue = d->expressions.at(ii).expression->evaluate();
         } else {
            QDeclarativeExpression *e = d->expressions.at(ii).expression;

            QDeclarativeBinding::Identifier id = d->expressions.at(ii).id;
            QDeclarativeBinding *newBinding = id != QDeclarativeBinding::Invalid ? QDeclarativeBinding::createBinding(id, object(),
                                              qmlContext(this), e->sourceFile(), e->lineNumber()) : 0;
            if (!newBinding) {
               newBinding = new QDeclarativeBinding(e->expression(), object(), qmlContext(this));
               newBinding->setSourceLocation(e->sourceFile(), e->lineNumber());
            }
            newBinding->setTarget(prop);
            a.toBinding = newBinding;
            a.deletableToBinding = true;
         }

         list << a;
      }
   }

   return list;
}

/*!
    \qmlproperty bool PropertyChanges::explicit

    If explicit is set to true, any potential bindings will be interpreted as
    once-off assignments that occur when the state is entered.

    In the following example, the addition of explicit prevents \c myItem.width from
    being bound to \c parent.width. Instead, it is assigned the value of \c parent.width
    at the time of the state change.
    \qml
    PropertyChanges {
        target: myItem
        explicit: true
        width: parent.width
    }
    \endqml

    By default, explicit is false.
*/
bool QDeclarativePropertyChanges::isExplicit() const
{
   Q_D(const QDeclarativePropertyChanges);
   return d->isExplicit;
}

void QDeclarativePropertyChanges::setIsExplicit(bool e)
{
   Q_D(QDeclarativePropertyChanges);
   d->isExplicit = e;
}

bool QDeclarativePropertyChanges::containsValue(const QString &name) const
{
   Q_D(const QDeclarativePropertyChanges);
   typedef QPair<QString, QVariant> PropertyEntry;

   QListIterator<PropertyEntry> propertyIterator(d->properties);
   while (propertyIterator.hasNext()) {
      const PropertyEntry &entry = propertyIterator.next();
      if (entry.first == name) {
         return true;
      }
   }

   return false;
}

bool QDeclarativePropertyChanges::containsExpression(const QString &name) const
{
   Q_D(const QDeclarativePropertyChanges);
   typedef QDeclarativePropertyChangesPrivate::ExpressionChange ExpressionEntry;

   QListIterator<ExpressionEntry> expressionIterator(d->expressions);
   while (expressionIterator.hasNext()) {
      const ExpressionEntry &entry = expressionIterator.next();
      if (entry.name == name) {
         return true;
      }
   }

   return false;
}

bool QDeclarativePropertyChanges::containsProperty(const QString &name) const
{
   return containsValue(name) || containsExpression(name);
}

void QDeclarativePropertyChanges::changeValue(const QString &name, const QVariant &value)
{
   Q_D(QDeclarativePropertyChanges);
   typedef QPair<QString, QVariant> PropertyEntry;
   typedef QDeclarativePropertyChangesPrivate::ExpressionChange ExpressionEntry;

   QMutableListIterator<ExpressionEntry> expressionIterator(d->expressions);
   while (expressionIterator.hasNext()) {
      const ExpressionEntry &entry = expressionIterator.next();
      if (entry.name == name) {
         expressionIterator.remove();
         if (state() && state()->isStateActive()) {
            QDeclarativeAbstractBinding *oldBinding = QDeclarativePropertyPrivate::binding(d->property(name));
            if (oldBinding) {
               QDeclarativePropertyPrivate::setBinding(d->property(name), 0);
               oldBinding->destroy();
            }
            d->property(name).write(value);
         }

         d->properties.append(PropertyEntry(name, value));
         return;
      }
   }

   QMutableListIterator<PropertyEntry> propertyIterator(d->properties);
   while (propertyIterator.hasNext()) {
      PropertyEntry &entry = propertyIterator.next();
      if (entry.first == name) {
         entry.second = value;
         if (state() && state()->isStateActive()) {
            d->property(name).write(value);
         }
         return;
      }
   }

   QDeclarativeAction action;
   action.restore = restoreEntryValues();
   action.property = d->property(name);
   action.fromValue = action.property.read();
   action.specifiedObject = object();
   action.specifiedProperty = name;
   action.toValue = value;

   propertyIterator.insert(PropertyEntry(name, value));
   if (state() && state()->isStateActive()) {
      state()->addEntryToRevertList(action);
      QDeclarativeAbstractBinding *oldBinding = QDeclarativePropertyPrivate::binding(action.property);
      if (oldBinding) {
         oldBinding->setEnabled(false, QDeclarativePropertyPrivate::DontRemoveBinding |
                                QDeclarativePropertyPrivate::BypassInterceptor);
      }
      d->property(name).write(value);
   }
}

void QDeclarativePropertyChanges::changeExpression(const QString &name, const QString &expression)
{
   Q_D(QDeclarativePropertyChanges);
   typedef QPair<QString, QVariant> PropertyEntry;
   typedef QDeclarativePropertyChangesPrivate::ExpressionChange ExpressionEntry;

   bool hadValue = false;

   QMutableListIterator<PropertyEntry> propertyIterator(d->properties);
   while (propertyIterator.hasNext()) {
      PropertyEntry &entry = propertyIterator.next();
      if (entry.first == name) {
         propertyIterator.remove();
         hadValue = true;
         break;
      }
   }

   QMutableListIterator<ExpressionEntry> expressionIterator(d->expressions);
   while (expressionIterator.hasNext()) {
      const ExpressionEntry &entry = expressionIterator.next();
      if (entry.name == name) {
         entry.expression->setExpression(expression);
         if (state() && state()->isStateActive()) {
            QDeclarativeAbstractBinding *oldBinding = QDeclarativePropertyPrivate::binding(d->property(name));
            if (oldBinding) {
               QDeclarativePropertyPrivate::setBinding(d->property(name), 0);
               oldBinding->destroy();
            }

            QDeclarativeBinding *newBinding = new QDeclarativeBinding(expression, object(), qmlContext(this));
            newBinding->setTarget(d->property(name));
            QDeclarativePropertyPrivate::setBinding(d->property(name), newBinding,
                                                    QDeclarativePropertyPrivate::DontRemoveBinding | QDeclarativePropertyPrivate::BypassInterceptor);
         }
         return;
      }
   }

   QDeclarativeExpression *newExpression = new QDeclarativeExpression(qmlContext(this), d->object, expression);
   expressionIterator.insert(ExpressionEntry(name, QDeclarativeBinding::Invalid, newExpression));

   if (state() && state()->isStateActive()) {
      if (hadValue) {
         QDeclarativeAbstractBinding *oldBinding = QDeclarativePropertyPrivate::binding(d->property(name));
         if (oldBinding) {
            oldBinding->setEnabled(false, QDeclarativePropertyPrivate::DontRemoveBinding |
                                   QDeclarativePropertyPrivate::BypassInterceptor);
            state()->changeBindingInRevertList(object(), name, oldBinding);
         }

         QDeclarativeBinding *newBinding = new QDeclarativeBinding(expression, object(), qmlContext(this));
         newBinding->setTarget(d->property(name));
         QDeclarativePropertyPrivate::setBinding(d->property(name), newBinding,
                                                 QDeclarativePropertyPrivate::DontRemoveBinding | QDeclarativePropertyPrivate::BypassInterceptor);
      } else {
         QDeclarativeAction action;
         action.restore = restoreEntryValues();
         action.property = d->property(name);
         action.fromValue = action.property.read();
         action.specifiedObject = object();
         action.specifiedProperty = name;


         if (d->isExplicit) {
            action.toValue = newExpression->evaluate();
         } else {
            QDeclarativeBinding *newBinding = new QDeclarativeBinding(newExpression->expression(), object(), qmlContext(this));
            newBinding->setTarget(d->property(name));
            action.toBinding = newBinding;
            action.deletableToBinding = true;

            state()->addEntryToRevertList(action);
            QDeclarativeAbstractBinding *oldBinding = QDeclarativePropertyPrivate::binding(action.property);
            if (oldBinding) {
               oldBinding->setEnabled(false, QDeclarativePropertyPrivate::DontRemoveBinding |
                                      QDeclarativePropertyPrivate::BypassInterceptor);
            }

            QDeclarativePropertyPrivate::setBinding(action.property, newBinding,
                                                    QDeclarativePropertyPrivate::DontRemoveBinding | QDeclarativePropertyPrivate::BypassInterceptor);
         }
      }
   }
   // what about the signal handler?
}

QVariant QDeclarativePropertyChanges::property(const QString &name) const
{
   Q_D(const QDeclarativePropertyChanges);
   typedef QPair<QString, QVariant> PropertyEntry;
   typedef QDeclarativePropertyChangesPrivate::ExpressionChange ExpressionEntry;

   QListIterator<PropertyEntry> propertyIterator(d->properties);
   while (propertyIterator.hasNext()) {
      const PropertyEntry &entry = propertyIterator.next();
      if (entry.first == name) {
         return entry.second;
      }
   }

   QListIterator<ExpressionEntry> expressionIterator(d->expressions);
   while (expressionIterator.hasNext()) {
      const ExpressionEntry &entry = expressionIterator.next();
      if (entry.name == name) {
         return QVariant(entry.expression->expression());
      }
   }

   return QVariant();
}

void QDeclarativePropertyChanges::removeProperty(const QString &name)
{
   Q_D(QDeclarativePropertyChanges);
   typedef QPair<QString, QVariant> PropertyEntry;
   typedef QDeclarativePropertyChangesPrivate::ExpressionChange ExpressionEntry;

   QMutableListIterator<ExpressionEntry> expressionIterator(d->expressions);
   while (expressionIterator.hasNext()) {
      const ExpressionEntry &entry = expressionIterator.next();
      if (entry.name == name) {
         expressionIterator.remove();
         state()->removeEntryFromRevertList(object(), name);
         return;
      }
   }

   QMutableListIterator<PropertyEntry> propertyIterator(d->properties);
   while (propertyIterator.hasNext()) {
      const PropertyEntry &entry = propertyIterator.next();
      if (entry.first == name) {
         propertyIterator.remove();
         state()->removeEntryFromRevertList(object(), name);
         return;
      }
   }
}

QVariant QDeclarativePropertyChanges::value(const QString &name) const
{
   Q_D(const QDeclarativePropertyChanges);
   typedef QPair<QString, QVariant> PropertyEntry;

   QListIterator<PropertyEntry> propertyIterator(d->properties);
   while (propertyIterator.hasNext()) {
      const PropertyEntry &entry = propertyIterator.next();
      if (entry.first == name) {
         return entry.second;
      }
   }

   return QVariant();
}

QString QDeclarativePropertyChanges::expression(const QString &name) const
{
   Q_D(const QDeclarativePropertyChanges);
   typedef QDeclarativePropertyChangesPrivate::ExpressionChange ExpressionEntry;

   QListIterator<ExpressionEntry> expressionIterator(d->expressions);
   while (expressionIterator.hasNext()) {
      const ExpressionEntry &entry = expressionIterator.next();
      if (entry.name == name) {
         return entry.expression->expression();
      }
   }

   return QString();
}

void QDeclarativePropertyChanges::detachFromState()
{
   if (state()) {
      state()->removeAllEntriesFromRevertList(object());
   }
}

void QDeclarativePropertyChanges::attachToState()
{
   if (state()) {
      state()->addEntriesToRevertList(actions());
   }
}

QT_END_NAMESPACE
