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

#include <qdeclarativescriptparser_p.h>
#include <qdeclarativeparser_p.h>
#include <qdeclarativejsengine_p.h>
#include <qdeclarativejsparser_p.h>
#include <qdeclarativejslexer_p.h>
#include <qdeclarativejsnodepool_p.h>
#include <qdeclarativejsastvisitor_p.h>
#include <qdeclarativejsast_p.h>
#include <qdeclarativerewrite_p.h>

#include <QStack>
#include <QCoreApplication>
#include <QDebug>

QT_BEGIN_NAMESPACE

using namespace QDeclarativeJS;
using namespace QDeclarativeParser;

namespace {

class ProcessAST: protected AST::Visitor
{
   struct State {
      State() : object(0), property(0) {}
      State(QDeclarativeParser::Object *o) : object(o), property(0) {}
      State(QDeclarativeParser::Object *o, Property *p) : object(o), property(p) {}

      QDeclarativeParser::Object *object;
      Property *property;
   };

   struct StateStack : public QStack<State> {
      void pushObject(QDeclarativeParser::Object *obj) {
         push(State(obj));
      }

      void pushProperty(const QString &name, const LocationSpan &location) {
         const State &state = top();
         if (state.property) {
            State s(state.property->getValue(location),
                    state.property->getValue(location)->getProperty(name.toUtf8()));
            s.property->location = location;
            push(s);
         } else {
            State s(state.object,
                    state.object->getProperty(name.toUtf8()));

            s.property->location = location;
            push(s);
         }
      }
   };

 public:
   ProcessAST(QDeclarativeScriptParser *parser);
   virtual ~ProcessAST();

   void operator()(const QString &code, AST::Node *node);

 protected:

   QDeclarativeParser::Object *defineObjectBinding(AST::UiQualifiedId *propertyName, bool onAssignment,
         const QString &objectType,
         AST::SourceLocation typeLocation,
         LocationSpan location,
         AST::UiObjectInitializer *initializer = 0);

   QDeclarativeParser::Variant getVariant(AST::ExpressionNode *expr);

   LocationSpan location(AST::SourceLocation start, AST::SourceLocation end);
   LocationSpan location(AST::UiQualifiedId *);

   using AST::Visitor::visit;
   using AST::Visitor::endVisit;

   virtual bool visit(AST::UiProgram *node);
   virtual bool visit(AST::UiImport *node);
   virtual bool visit(AST::UiObjectDefinition *node);
   virtual bool visit(AST::UiPublicMember *node);
   virtual bool visit(AST::UiObjectBinding *node);

   virtual bool visit(AST::UiScriptBinding *node);
   virtual bool visit(AST::UiArrayBinding *node);
   virtual bool visit(AST::UiSourceElement *node);

   void accept(AST::Node *node);

   QString asString(AST::UiQualifiedId *node) const;

   const State state() const;
   QDeclarativeParser::Object *currentObject() const;
   Property *currentProperty() const;

   QString qualifiedNameId() const;

   QString textAt(const AST::SourceLocation &loc) const {
      return _contents.mid(loc.offset, loc.length);
   }


   QString textAt(const AST::SourceLocation &first,
                  const AST::SourceLocation &last) const {
      return _contents.mid(first.offset, last.offset + last.length - first.offset);
   }

   QString asString(AST::ExpressionNode *expr) {
      if (! expr) {
         return QString();
      }

      return textAt(expr->firstSourceLocation(), expr->lastSourceLocation());
   }

   QString asString(AST::Statement *stmt) {
      if (! stmt) {
         return QString();
      }

      QString s = textAt(stmt->firstSourceLocation(), stmt->lastSourceLocation());
      s += QLatin1Char('\n');
      return s;
   }

 private:
   QDeclarativeScriptParser *_parser;
   StateStack _stateStack;
   QStringList _scope;
   QString _contents;
};

ProcessAST::ProcessAST(QDeclarativeScriptParser *parser)
   : _parser(parser)
{
}

ProcessAST::~ProcessAST()
{
}

void ProcessAST::operator()(const QString &code, AST::Node *node)
{
   _contents = code;
   accept(node);
}

void ProcessAST::accept(AST::Node *node)
{
   AST::Node::acceptChild(node, this);
}

const ProcessAST::State ProcessAST::state() const
{
   if (_stateStack.isEmpty()) {
      return State();
   }

   return _stateStack.back();
}

QDeclarativeParser::Object *ProcessAST::currentObject() const
{
   return state().object;
}

Property *ProcessAST::currentProperty() const
{
   return state().property;
}

QString ProcessAST::qualifiedNameId() const
{
   return _scope.join(QLatin1String("/"));
}

QString ProcessAST::asString(AST::UiQualifiedId *node) const
{
   QString s;

   for (AST::UiQualifiedId *it = node; it; it = it->next) {
      s.append(it->name->asString());

      if (it->next) {
         s.append(QLatin1Char('.'));
      }
   }

   return s;
}

QDeclarativeParser::Object *
ProcessAST::defineObjectBinding(AST::UiQualifiedId *propertyName,
                                bool onAssignment,
                                const QString &objectType,
                                AST::SourceLocation typeLocation,
                                LocationSpan location,
                                AST::UiObjectInitializer *initializer)
{
   int lastTypeDot = objectType.lastIndexOf(QLatin1Char('.'));
   bool isType = !objectType.isEmpty() &&
                 (objectType.at(0).isUpper() ||
                  (lastTypeDot >= 0 && objectType.at(lastTypeDot + 1).isUpper()));

   int propertyCount = 0;
   for (AST::UiQualifiedId *name = propertyName; name; name = name->next) {
      ++propertyCount;
      _stateStack.pushProperty(name->name->asString(),
                               this->location(name));
   }

   if (!onAssignment && propertyCount && currentProperty() && currentProperty()->values.count()) {
      QDeclarativeError error;
      error.setDescription(QCoreApplication::translate("QDeclarativeParser", "Property value set multiple times"));
      error.setLine(this->location(propertyName).start.line);
      error.setColumn(this->location(propertyName).start.column);
      _parser->_errors << error;
      return 0;
   }

   if (!isType) {

      if (propertyCount || !currentObject()) {
         QDeclarativeError error;
         error.setDescription(QCoreApplication::translate("QDeclarativeParser", "Expected type name"));
         error.setLine(typeLocation.startLine);
         error.setColumn(typeLocation.startColumn);
         _parser->_errors << error;
         return 0;
      }

      LocationSpan loc = ProcessAST::location(typeLocation, typeLocation);
      if (propertyName) {
         loc = ProcessAST::location(propertyName);
      }

      _stateStack.pushProperty(objectType, loc);
      accept(initializer);
      _stateStack.pop();

      return 0;

   } else {
      // Class

      QString resolvableObjectType = objectType;
      if (lastTypeDot >= 0) {
         resolvableObjectType.replace(QLatin1Char('.'), QLatin1Char('/'));
      }

      QDeclarativeParser::Object *obj = new QDeclarativeParser::Object;

      QDeclarativeScriptParser::TypeReference *typeRef = _parser->findOrCreateType(resolvableObjectType);
      obj->type = typeRef->id;

      typeRef->refObjects.append(obj);

      // XXX this doesn't do anything (_scope never builds up)
      _scope.append(resolvableObjectType);
      obj->typeName = qualifiedNameId().toUtf8();
      _scope.removeLast();

      obj->location = location;

      if (propertyCount) {
         Property *prop = currentProperty();
         QDeclarativeParser::Value *v = new QDeclarativeParser::Value;
         v->object = obj;
         v->location = obj->location;
         if (onAssignment) {
            prop->addOnValue(v);
         } else {
            prop->addValue(v);
         }

         while (propertyCount--) {
            _stateStack.pop();
         }

      } else {

         if (! _parser->tree()) {
            _parser->setTree(obj);
         } else {
            const State state = _stateStack.top();
            QDeclarativeParser::Value *v = new QDeclarativeParser::Value;
            v->object = obj;
            v->location = obj->location;
            if (state.property) {
               state.property->addValue(v);
            } else {
               Property *defaultProp = state.object->getDefaultProperty();
               if (defaultProp->location.start.line == -1) {
                  defaultProp->location = v->location;
                  defaultProp->location.end = defaultProp->location.start;
                  defaultProp->location.range.length = 0;
               }
               defaultProp->addValue(v);
            }
         }
      }

      _stateStack.pushObject(obj);
      accept(initializer);
      _stateStack.pop();

      return obj;
   }
}

LocationSpan ProcessAST::location(AST::UiQualifiedId *id)
{
   return location(id->identifierToken, id->identifierToken);
}

LocationSpan ProcessAST::location(AST::SourceLocation start, AST::SourceLocation end)
{
   LocationSpan rv;
   rv.start.line = start.startLine;
   rv.start.column = start.startColumn;
   rv.end.line = end.startLine;
   rv.end.column = end.startColumn + end.length - 1;
   rv.range.offset = start.offset;
   rv.range.length = end.offset + end.length - start.offset;
   return rv;
}

// UiProgram: UiImportListOpt UiObjectMemberList ;
bool ProcessAST::visit(AST::UiProgram *node)
{
   accept(node->imports);
   accept(node->members->member);
   return false;
}

// UiImport: T_IMPORT T_STRING_LITERAL ;
bool ProcessAST::visit(AST::UiImport *node)
{
   QString uri;
   QDeclarativeScriptParser::Import import;

   if (node->fileName) {
      uri = node->fileName->asString();

      if (uri.endsWith(QLatin1String(".js"))) {
         import.type = QDeclarativeScriptParser::Import::Script;
      } else {
         import.type = QDeclarativeScriptParser::Import::File;
      }
   } else {
      import.type = QDeclarativeScriptParser::Import::Library;
      uri = asString(node->importUri);
   }

   AST::SourceLocation startLoc = node->importToken;
   AST::SourceLocation endLoc = node->semicolonToken;

   // Qualifier
   if (node->importId) {
      import.qualifier = node->importId->asString();
      if (!import.qualifier.at(0).isUpper()) {
         QDeclarativeError error;
         error.setDescription(QCoreApplication::translate("QDeclarativeParser", "Invalid import qualifier ID"));
         error.setLine(node->importIdToken.startLine);
         error.setColumn(node->importIdToken.startColumn);
         _parser->_errors << error;
         return false;
      }
      if (import.qualifier == QLatin1String("Qt")) {
         QDeclarativeError error;
         error.setDescription(QCoreApplication::translate("QDeclarativeParser",
                              "Reserved name \"Qt\" cannot be used as an qualifier"));
         error.setLine(node->importIdToken.startLine);
         error.setColumn(node->importIdToken.startColumn);
         _parser->_errors << error;
         return false;
      }

      // Check for script qualifier clashes
      bool isScript = import.type == QDeclarativeScriptParser::Import::Script;
      for (int ii = 0; ii < _parser->_imports.count(); ++ii) {
         const QDeclarativeScriptParser::Import &other = _parser->_imports.at(ii);
         bool otherIsScript = other.type == QDeclarativeScriptParser::Import::Script;

         if ((isScript || otherIsScript) && import.qualifier == other.qualifier) {
            QDeclarativeError error;
            error.setDescription(QCoreApplication::translate("QDeclarativeParser", "Script import qualifiers must be unique."));
            error.setLine(node->importIdToken.startLine);
            error.setColumn(node->importIdToken.startColumn);
            _parser->_errors << error;
            return false;
         }
      }

   } else if (import.type == QDeclarativeScriptParser::Import::Script) {
      QDeclarativeError error;
      error.setDescription(QCoreApplication::translate("QDeclarativeParser", "Script import requires a qualifier"));
      error.setLine(node->fileNameToken.startLine);
      error.setColumn(node->fileNameToken.startColumn);
      _parser->_errors << error;
      return false;
   }

   if (node->versionToken.isValid()) {
      import.version = textAt(node->versionToken);
   } else if (import.type == QDeclarativeScriptParser::Import::Library) {
      QDeclarativeError error;
      error.setDescription(QCoreApplication::translate("QDeclarativeParser", "Library import requires a version"));
      error.setLine(node->importIdToken.startLine);
      error.setColumn(node->importIdToken.startColumn);
      _parser->_errors << error;
      return false;
   }


   import.location = location(startLoc, endLoc);
   import.uri = uri;

   _parser->_imports << import;

   return false;
}

bool ProcessAST::visit(AST::UiPublicMember *node)
{
   const struct TypeNameToType {
      const char *name;
      Object::DynamicProperty::Type type;
      const char *qtName;
   } propTypeNameToTypes[] = {
      { "int", Object::DynamicProperty::Int, "int" },
      { "bool", Object::DynamicProperty::Bool, "bool" },
      { "double", Object::DynamicProperty::Real, "double" },
      { "real", Object::DynamicProperty::Real, "qreal" },
      { "string", Object::DynamicProperty::String, "QString" },
      { "url", Object::DynamicProperty::Url, "QUrl" },
      { "color", Object::DynamicProperty::Color, "QColor" },
      // Internally QTime, QDate and QDateTime are all supported.
      // To be more consistent with JavaScript we expose only
      // QDateTime as it matches closely with the Date JS type.
      // We also call it "date" to match.
      // { "time", Object::DynamicProperty::Time, "QTime" },
      // { "date", Object::DynamicProperty::Date, "QDate" },
      { "date", Object::DynamicProperty::DateTime, "QDateTime" },
      { "variant", Object::DynamicProperty::Variant, "QVariant" }
   };
   const int propTypeNameToTypesCount = sizeof(propTypeNameToTypes) /
                                        sizeof(propTypeNameToTypes[0]);

   if (node->type == AST::UiPublicMember::Signal) {
      const QString name = node->name->asString();

      Object::DynamicSignal signal;
      signal.name = name.toUtf8();

      AST::UiParameterList *p = node->parameters;
      while (p) {
         const QString memberType = p->type->asString();
         const char *qtType = 0;
         for (int ii = 0; !qtType && ii < propTypeNameToTypesCount; ++ii) {
            if (QLatin1String(propTypeNameToTypes[ii].name) == memberType) {
               qtType = propTypeNameToTypes[ii].qtName;
            }
         }

         if (!qtType) {
            QDeclarativeError error;
            error.setDescription(QCoreApplication::translate("QDeclarativeParser", "Expected parameter type"));
            error.setLine(node->typeToken.startLine);
            error.setColumn(node->typeToken.startColumn);
            _parser->_errors << error;
            return false;
         }

         signal.parameterTypes << qtType;
         signal.parameterNames << p->name->asString().toUtf8();
         p = p->finish();
      }

      _stateStack.top().object->dynamicSignals << signal;
   } else {
      const QString memberType = node->memberType->asString();
      const QString name = node->name->asString();

      bool typeFound = false;
      Object::DynamicProperty::Type type;

      if (memberType == QLatin1String("alias")) {
         type = Object::DynamicProperty::Alias;
         typeFound = true;
      }

      for (int ii = 0; !typeFound && ii < propTypeNameToTypesCount; ++ii) {
         if (QLatin1String(propTypeNameToTypes[ii].name) == memberType) {
            type = propTypeNameToTypes[ii].type;
            typeFound = true;
         }
      }

      if (!typeFound && memberType.at(0).isUpper()) {
         QString typemodifier;
         if (node->typeModifier) {
            typemodifier = node->typeModifier->asString();
         }
         if (typemodifier.isEmpty()) {
            type = Object::DynamicProperty::Custom;
         } else if (typemodifier == QLatin1String("list")) {
            type = Object::DynamicProperty::CustomList;
         } else {
            QDeclarativeError error;
            error.setDescription(QCoreApplication::translate("QDeclarativeParser", "Invalid property type modifier"));
            error.setLine(node->typeModifierToken.startLine);
            error.setColumn(node->typeModifierToken.startColumn);
            _parser->_errors << error;
            return false;
         }
         typeFound = true;
      } else if (node->typeModifier) {
         QDeclarativeError error;
         error.setDescription(QCoreApplication::translate("QDeclarativeParser", "Unexpected property type modifier"));
         error.setLine(node->typeModifierToken.startLine);
         error.setColumn(node->typeModifierToken.startColumn);
         _parser->_errors << error;
         return false;
      }

      if (!typeFound) {
         QDeclarativeError error;
         error.setDescription(QCoreApplication::translate("QDeclarativeParser", "Expected property type"));
         error.setLine(node->typeToken.startLine);
         error.setColumn(node->typeToken.startColumn);
         _parser->_errors << error;
         return false;
      }

      if (node->isReadonlyMember) {
         QDeclarativeError error;
         error.setDescription(QCoreApplication::translate("QDeclarativeParser", "Readonly not yet supported"));
         error.setLine(node->readonlyToken.startLine);
         error.setColumn(node->readonlyToken.startColumn);
         _parser->_errors << error;
         return false;

      }
      Object::DynamicProperty property;
      property.isDefaultProperty = node->isDefaultMember;
      property.type = type;
      if (type >= Object::DynamicProperty::Custom) {
         QDeclarativeScriptParser::TypeReference *typeRef =
            _parser->findOrCreateType(memberType);
         typeRef->refObjects.append(_stateStack.top().object);
      }
      property.customType = memberType.toUtf8();
      property.name = name.toUtf8();
      property.location = location(node->firstSourceLocation(),
                                   node->lastSourceLocation());

      if (node->expression) { // default value
         property.defaultValue = new Property;
         property.defaultValue->parent = _stateStack.top().object;
         property.defaultValue->location =
            location(node->expression->firstSourceLocation(),
                     node->expression->lastSourceLocation());
         QDeclarativeParser::Value *value = new QDeclarativeParser::Value;
         value->location = location(node->expression->firstSourceLocation(),
                                    node->expression->lastSourceLocation());
         value->value = getVariant(node->expression);
         property.defaultValue->values << value;
      }

      _stateStack.top().object->dynamicProperties << property;

      // process QML-like initializers (e.g. property Object o: Object {})
      accept(node->binding);
   }

   return false;
}


// UiObjectMember: UiQualifiedId UiObjectInitializer ;
bool ProcessAST::visit(AST::UiObjectDefinition *node)
{
   LocationSpan l = location(node->firstSourceLocation(),
                             node->lastSourceLocation());

   const QString objectType = asString(node->qualifiedTypeNameId);
   const AST::SourceLocation typeLocation = node->qualifiedTypeNameId->identifierToken;

   defineObjectBinding(/*propertyName = */ 0, false, objectType,
                                           typeLocation, l, node->initializer);

   return false;
}


// UiObjectMember: UiQualifiedId T_COLON UiQualifiedId UiObjectInitializer ;
bool ProcessAST::visit(AST::UiObjectBinding *node)
{
   LocationSpan l = location(node->qualifiedTypeNameId->identifierToken,
                             node->initializer->rbraceToken);

   const QString objectType = asString(node->qualifiedTypeNameId);
   const AST::SourceLocation typeLocation = node->qualifiedTypeNameId->identifierToken;

   defineObjectBinding(node->qualifiedId, node->hasOnToken, objectType,
                       typeLocation, l, node->initializer);

   return false;
}

QDeclarativeParser::Variant ProcessAST::getVariant(AST::ExpressionNode *expr)
{
   if (AST::StringLiteral *lit = AST::cast<AST::StringLiteral *>(expr)) {
      return QDeclarativeParser::Variant(lit->value->asString());
   } else if (expr->kind == AST::Node::Kind_TrueLiteral) {
      return QDeclarativeParser::Variant(true);
   } else if (expr->kind == AST::Node::Kind_FalseLiteral) {
      return QDeclarativeParser::Variant(false);
   } else if (AST::NumericLiteral *lit = AST::cast<AST::NumericLiteral *>(expr)) {
      return QDeclarativeParser::Variant(lit->value, asString(expr));
   } else {

      if (AST::UnaryMinusExpression *unaryMinus = AST::cast<AST::UnaryMinusExpression *>(expr)) {
         if (AST::NumericLiteral *lit = AST::cast<AST::NumericLiteral *>(unaryMinus->expression)) {
            return QDeclarativeParser::Variant(-lit->value, asString(expr));
         }
      }

      return  QDeclarativeParser::Variant(asString(expr), expr);
   }
}


// UiObjectMember: UiQualifiedId T_COLON Statement ;
bool ProcessAST::visit(AST::UiScriptBinding *node)
{
   int propertyCount = 0;
   AST::UiQualifiedId *propertyName = node->qualifiedId;
   for (AST::UiQualifiedId *name = propertyName; name; name = name->next) {
      ++propertyCount;
      _stateStack.pushProperty(name->name->asString(),
                               location(name));
   }

   Property *prop = currentProperty();

   if (prop->values.count()) {
      QDeclarativeError error;
      error.setDescription(QCoreApplication::translate("QDeclarativeParser", "Property value set multiple times"));
      error.setLine(this->location(propertyName).start.line);
      error.setColumn(this->location(propertyName).start.column);
      _parser->_errors << error;
      return 0;
   }

   QDeclarativeParser::Variant primitive;

   if (AST::ExpressionStatement *stmt = AST::cast<AST::ExpressionStatement *>(node->statement)) {
      primitive = getVariant(stmt->expression);
   } else { // do binding
      primitive = QDeclarativeParser::Variant(asString(node->statement),
                                              node->statement);
   }

   prop->location.range.length = prop->location.range.offset + prop->location.range.length -
                                 node->qualifiedId->identifierToken.offset;
   prop->location.range.offset = node->qualifiedId->identifierToken.offset;
   QDeclarativeParser::Value *v = new QDeclarativeParser::Value;
   v->value = primitive;
   v->location = location(node->statement->firstSourceLocation(),
                          node->statement->lastSourceLocation());

   prop->addValue(v);

   while (propertyCount--) {
      _stateStack.pop();
   }

   return true;
}

static QList<int> collectCommas(AST::UiArrayMemberList *members)
{
   QList<int> commas;

   if (members) {
      for (AST::UiArrayMemberList *it = members->next; it; it = it->next) {
         commas.append(it->commaToken.offset);
      }
   }

   return commas;
}

// UiObjectMember: UiQualifiedId T_COLON T_LBRACKET UiArrayMemberList T_RBRACKET ;
bool ProcessAST::visit(AST::UiArrayBinding *node)
{
   int propertyCount = 0;
   AST::UiQualifiedId *propertyName = node->qualifiedId;
   for (AST::UiQualifiedId *name = propertyName; name; name = name->next) {
      ++propertyCount;
      _stateStack.pushProperty(name->name->asString(),
                               location(name));
   }

   Property *prop = currentProperty();

   if (prop->values.count()) {
      QDeclarativeError error;
      error.setDescription(QCoreApplication::translate("QDeclarativeParser", "Property value set multiple times"));
      error.setLine(this->location(propertyName).start.line);
      error.setColumn(this->location(propertyName).start.column);
      _parser->_errors << error;
      return 0;
   }

   accept(node->members);

   // For the DOM, store the position of the T_LBRACKET upto the T_RBRACKET as the range:
   prop->listValueRange.offset = node->lbracketToken.offset;
   prop->listValueRange.length = node->rbracketToken.offset + node->rbracketToken.length - node->lbracketToken.offset;

   // Store the positions of the comma token too, again for the DOM to be able to retrieve it.
   prop->listCommaPositions = collectCommas(node->members);

   while (propertyCount--) {
      _stateStack.pop();
   }

   return false;
}

bool ProcessAST::visit(AST::UiSourceElement *node)
{
   QDeclarativeParser::Object *obj = currentObject();

   if (AST::FunctionDeclaration *funDecl = AST::cast<AST::FunctionDeclaration *>(node->sourceElement)) {

      Object::DynamicSlot slot;
      slot.location = location(funDecl->firstSourceLocation(), funDecl->lastSourceLocation());

      AST::FormalParameterList *f = funDecl->formals;
      while (f) {
         slot.parameterNames << f->name->asString().toUtf8();
         f = f->finish();
      }

      AST::SourceLocation loc = funDecl->rparenToken;
      loc.offset = loc.end();
      loc.startColumn += 1;
      QString body = textAt(loc, funDecl->rbraceToken);
      slot.name = funDecl->name->asString().toUtf8();
      slot.body = body;
      obj->dynamicSlots << slot;

   } else {
      QDeclarativeError error;
      error.setDescription(QCoreApplication::translate("QDeclarativeParser",
                           "JavaScript declaration outside Script element"));
      error.setLine(node->firstSourceLocation().startLine);
      error.setColumn(node->firstSourceLocation().startColumn);
      _parser->_errors << error;
   }
   return false;
}

} // end of anonymous namespace


QDeclarativeScriptParser::QDeclarativeScriptParser()
   : root(0), data(0)
{

}

QDeclarativeScriptParser::~QDeclarativeScriptParser()
{
   clear();
}

class QDeclarativeScriptParserJsASTData
{
 public:
   QDeclarativeScriptParserJsASTData(const QString &filename)
      : nodePool(filename, &engine) {}

   Engine engine;
   NodePool nodePool;
};

bool QDeclarativeScriptParser::parse(const QByteArray &qmldata, const QUrl &url)
{
   clear();

   const QString fileName = url.toString();
   _scriptFile = fileName;

   QTextStream stream(qmldata, QIODevice::ReadOnly);
#ifndef QT_NO_TEXTCODEC
   stream.setCodec("UTF-8");
#endif
   const QString code = stream.readAll();

   data = new QDeclarativeScriptParserJsASTData(fileName);

   Lexer lexer(&data->engine);
   lexer.setCode(code, /*line = */ 1);

   Parser parser(&data->engine);

   if (! parser.parse() || !_errors.isEmpty()) {

      // Extract errors from the parser
      foreach (const DiagnosticMessage & m, parser.diagnosticMessages()) {

         if (m.isWarning()) {
            continue;
         }

         QDeclarativeError error;
         error.setUrl(url);
         error.setDescription(m.message);
         error.setLine(m.loc.startLine);
         error.setColumn(m.loc.startColumn);
         _errors << error;

      }
   }

   if (_errors.isEmpty()) {
      ProcessAST process(this);
      process(code, parser.ast());

      // Set the url for process errors
      for (int ii = 0; ii < _errors.count(); ++ii) {
         _errors[ii].setUrl(url);
      }
   }

   return _errors.isEmpty();
}

QList<QDeclarativeScriptParser::TypeReference *> QDeclarativeScriptParser::referencedTypes() const
{
   return _refTypes;
}

QDeclarativeParser::Object *QDeclarativeScriptParser::tree() const
{
   return root;
}

QList<QDeclarativeScriptParser::Import> QDeclarativeScriptParser::imports() const
{
   return _imports;
}

QList<QDeclarativeError> QDeclarativeScriptParser::errors() const
{
   return _errors;
}

static void replaceWithSpace(QString &str, int idx, int n)
{
   QChar *data = str.data() + idx;
   const QChar space(QLatin1Char(' '));
   for (int ii = 0; ii < n; ++ii) {
      *data++ = space;
   }
}

/*
Searches for ".pragma <value>" declarations within \a script.  Currently supported pragmas
are:
    library
*/
QDeclarativeParser::Object::ScriptBlock::Pragmas QDeclarativeScriptParser::extractPragmas(QString &script)
{
   QDeclarativeParser::Object::ScriptBlock::Pragmas rv = QDeclarativeParser::Object::ScriptBlock::None;

   const QString pragma(QLatin1String("pragma"));
   const QString library(QLatin1String("library"));

   QDeclarativeJS::Lexer l(0);
   l.setCode(script, 0);

   int token = l.lex();

   while (true) {
      if (token != QDeclarativeJSGrammar::T_DOT) {
         return rv;
      }

      int startOffset = l.tokenOffset();
      int startLine = l.currentLineNo();

      token = l.lex();

      if (token != QDeclarativeJSGrammar::T_IDENTIFIER ||
            l.currentLineNo() != startLine ||
            script.mid(l.tokenOffset(), l.tokenLength()) != pragma) {
         return rv;
      }

      token = l.lex();

      if (token != QDeclarativeJSGrammar::T_IDENTIFIER ||
            l.currentLineNo() != startLine) {
         return rv;
      }

      QString pragmaValue = script.mid(l.tokenOffset(), l.tokenLength());
      int endOffset = l.tokenLength() + l.tokenOffset();

      token = l.lex();
      if (l.currentLineNo() == startLine) {
         return rv;
      }

      if (pragmaValue == library) {
         rv |= QDeclarativeParser::Object::ScriptBlock::Shared;
         replaceWithSpace(script, startOffset, endOffset - startOffset);
      } else {
         return rv;
      }
   }
   return rv;
}

#define CHECK_LINE if(l.currentLineNo() != startLine) return rv;
#define CHECK_TOKEN(t) if (token != QDeclarativeJSGrammar:: t) return rv;

static const int uriTokens[] = {
   QDeclarativeJSGrammar::T_IDENTIFIER,
   QDeclarativeJSGrammar::T_PROPERTY,
   QDeclarativeJSGrammar::T_SIGNAL,
   QDeclarativeJSGrammar::T_READONLY,
   QDeclarativeJSGrammar::T_ON,
   QDeclarativeJSGrammar::T_BREAK,
   QDeclarativeJSGrammar::T_CASE,
   QDeclarativeJSGrammar::T_CATCH,
   QDeclarativeJSGrammar::T_CONTINUE,
   QDeclarativeJSGrammar::T_DEFAULT,
   QDeclarativeJSGrammar::T_DELETE,
   QDeclarativeJSGrammar::T_DO,
   QDeclarativeJSGrammar::T_ELSE,
   QDeclarativeJSGrammar::T_FALSE,
   QDeclarativeJSGrammar::T_FINALLY,
   QDeclarativeJSGrammar::T_FOR,
   QDeclarativeJSGrammar::T_FUNCTION,
   QDeclarativeJSGrammar::T_IF,
   QDeclarativeJSGrammar::T_IN,
   QDeclarativeJSGrammar::T_INSTANCEOF,
   QDeclarativeJSGrammar::T_NEW,
   QDeclarativeJSGrammar::T_NULL,
   QDeclarativeJSGrammar::T_RETURN,
   QDeclarativeJSGrammar::T_SWITCH,
   QDeclarativeJSGrammar::T_THIS,
   QDeclarativeJSGrammar::T_THROW,
   QDeclarativeJSGrammar::T_TRUE,
   QDeclarativeJSGrammar::T_TRY,
   QDeclarativeJSGrammar::T_TYPEOF,
   QDeclarativeJSGrammar::T_VAR,
   QDeclarativeJSGrammar::T_VOID,
   QDeclarativeJSGrammar::T_WHILE,
   QDeclarativeJSGrammar::T_CONST,
   QDeclarativeJSGrammar::T_DEBUGGER,
   QDeclarativeJSGrammar::T_RESERVED_WORD,
   QDeclarativeJSGrammar::T_WITH,

   QDeclarativeJSGrammar::EOF_SYMBOL
};
static inline bool isUriToken(int token)
{
   const int *current = uriTokens;
   while (*current != QDeclarativeJSGrammar::EOF_SYMBOL) {
      if (*current == token) {
         return true;
      }
      ++current;
   }
   return false;
}

QDeclarativeScriptParser::JavaScriptMetaData QDeclarativeScriptParser::extractMetaData(QString &script)
{
   JavaScriptMetaData rv;

   QDeclarativeParser::Object::ScriptBlock::Pragmas &pragmas = rv.pragmas;

   const QString pragma(QLatin1String("pragma"));
   const QString js(QLatin1String(".js"));
   const QString library(QLatin1String("library"));

   QDeclarativeJS::Lexer l(0);
   l.setCode(script, 0);

   int token = l.lex();

   while (true) {
      if (token != QDeclarativeJSGrammar::T_DOT) {
         return rv;
      }

      int startOffset = l.tokenOffset();
      int startLine = l.currentLineNo();

      token = l.lex();

      CHECK_LINE;

      if (token == QDeclarativeJSGrammar::T_IMPORT) {

         // .import <URI> <Version> as <Identifier>
         // .import <file.js> as <Identifier>

         token = l.lex();

         CHECK_LINE;

         if (token == QDeclarativeJSGrammar::T_STRING_LITERAL) {

            QString file(l.characterBuffer(), l.characterCount());
            if (!file.endsWith(js)) {
               return rv;
            }

            token = l.lex();

            CHECK_TOKEN(T_AS);
            CHECK_LINE;

            token = l.lex();

            CHECK_TOKEN(T_IDENTIFIER);
            CHECK_LINE;

            int endOffset = l.tokenLength() + l.tokenOffset();

            QString importId = script.mid(l.tokenOffset(), l.tokenLength());

            if (!importId.at(0).isUpper()) {
               return rv;
            }

            token = l.lex();
            if (l.currentLineNo() == startLine) {
               return rv;
            }

            replaceWithSpace(script, startOffset, endOffset - startOffset);

            Import import;
            import.type = Import::Script;
            import.uri = file;
            import.qualifier = importId;

            rv.imports << import;

         } else {
            // URI
            QString uri;
            QString version;

            while (true) {
               if (!isUriToken(token)) {
                  return rv;
               }

               uri.append(QString(l.characterBuffer(), l.characterCount()));

               token = l.lex();
               CHECK_LINE;
               if (token != QDeclarativeJSGrammar::T_DOT) {
                  break;
               }

               uri.append(QLatin1Char('.'));

               token = l.lex();
               CHECK_LINE;
            }

            CHECK_TOKEN(T_NUMERIC_LITERAL);
            version = script.mid(l.tokenOffset(), l.tokenLength());

            token = l.lex();

            CHECK_TOKEN(T_AS);
            CHECK_LINE;

            token = l.lex();

            CHECK_TOKEN(T_IDENTIFIER);
            CHECK_LINE;

            int endOffset = l.tokenLength() + l.tokenOffset();

            QString importId = script.mid(l.tokenOffset(), l.tokenLength());

            if (!importId.at(0).isUpper()) {
               return rv;
            }

            token = l.lex();
            if (l.currentLineNo() == startLine) {
               return rv;
            }

            replaceWithSpace(script, startOffset, endOffset - startOffset);

            Import import;
            import.type = Import::Library;
            import.uri = uri;
            import.version = version;
            import.qualifier = importId;

            rv.imports << import;
         }

      } else if (token == QDeclarativeJSGrammar::T_IDENTIFIER &&
                 script.mid(l.tokenOffset(), l.tokenLength()) == pragma) {

         token = l.lex();

         CHECK_TOKEN(T_IDENTIFIER);
         CHECK_LINE;

         QString pragmaValue = script.mid(l.tokenOffset(), l.tokenLength());
         int endOffset = l.tokenLength() + l.tokenOffset();

         if (pragmaValue == QLatin1String("library")) {
            pragmas |= QDeclarativeParser::Object::ScriptBlock::Shared;
            replaceWithSpace(script, startOffset, endOffset - startOffset);
         } else {
            return rv;
         }

         token = l.lex();
         if (l.currentLineNo() == startLine) {
            return rv;
         }

      } else {
         return rv;
      }
   }
   return rv;
}

void QDeclarativeScriptParser::clear()
{
   if (root) {
      root->release();
      root = 0;
   }
   _imports.clear();
   qDeleteAll(_refTypes);
   _refTypes.clear();
   _errors.clear();

   if (data) {
      delete data;
      data = 0;
   }
}

QDeclarativeScriptParser::TypeReference *QDeclarativeScriptParser::findOrCreateType(const QString &name)
{
   TypeReference *type = 0;
   int i = 0;
   for (; i < _refTypes.size(); ++i) {
      if (_refTypes.at(i)->name == name) {
         type = _refTypes.at(i);
         break;
      }
   }
   if (!type) {
      type = new TypeReference(i, name);
      _refTypes.append(type);
   }

   return type;
}

void QDeclarativeScriptParser::setTree(QDeclarativeParser::Object *tree)
{
   Q_ASSERT(! root);

   root = tree;
}

QT_END_NAMESPACE
