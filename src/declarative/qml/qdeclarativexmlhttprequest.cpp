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

#include <qdeclarativexmlhttprequest_p.h>
#include <qdeclarativeengine.h>
#include <qdeclarativeengine_p.h>
#include <qdeclarativerefcount_p.h>
#include <qdeclarativeengine_p.h>
#include <qdeclarativeexpression_p.h>
#include <qdeclarativeglobal_p.h>
#include <QtCore/qobject.h>
#include <QtScript/qscriptvalue.h>
#include <QtScript/qscriptcontext.h>
#include <QtScript/qscriptengine.h>
#include <QtNetwork/qnetworkreply.h>
#include <QtCore/qtextcodec.h>
#include <QtCore/qxmlstream.h>
#include <QtCore/qstack.h>
#include <QtCore/qdebug.h>
#include <QtCore/QStringBuilder>

#ifndef QT_NO_XMLSTREAMREADER

// From DOM-Level-3-Core spec
// http://www.w3.org/TR/DOM-Level-3-Core/core.html
#define INDEX_SIZE_ERR 1
#define DOMSTRING_SIZE_ERR 2
#define HIERARCHY_REQUEST_ERR 3
#define WRONG_DOCUMENT_ERR 4
#define INVALID_CHARACTER_ERR 5
#define NO_DATA_ALLOWED_ERR 6
#define NO_MODIFICATION_ALLOWED_ERR 7
#define NOT_FOUND_ERR 8
#define NOT_SUPPORTED_ERR 9
#define INUSE_ATTRIBUTE_ERR 10
#define INVALID_STATE_ERR 11
#define SYNTAX_ERR 12
#define INVALID_MODIFICATION_ERR 13
#define NAMESPACE_ERR 14
#define INVALID_ACCESS_ERR 15
#define VALIDATION_ERR 16
#define TYPE_MISMATCH_ERR 17

#define THROW_DOM(error, desc) \
{ \
    QScriptValue errorValue = context->throwError(QLatin1String(desc)); \
    errorValue.setProperty(QLatin1String("code"), error); \
    return errorValue; \
}

#define THROW_SYNTAX(desc) \
    return context->throwError(QScriptContext::SyntaxError, QLatin1String(desc));
#define THROW_REFERENCE(desc) \
    return context->throwError(QScriptContext::ReferenceError, QLatin1String(desc));

#define D(arg) (arg)->release()
#define A(arg) (arg)->addref()

QT_BEGIN_NAMESPACE

DEFINE_BOOL_CONFIG_OPTION(xhrDump, QML_XHR_DUMP);

class DocumentImpl;
class NodeImpl
{
 public:
   NodeImpl() : type(Element), document(0), parent(0) {}
   virtual ~NodeImpl() {
      for (int ii = 0; ii < children.count(); ++ii) {
         delete children.at(ii);
      }
      for (int ii = 0; ii < attributes.count(); ++ii) {
         delete attributes.at(ii);
      }
   }

   // These numbers are copied from the Node IDL definition
   enum Type {
      Attr = 2,
      CDATA = 4,
      Comment = 8,
      Document = 9,
      DocumentFragment = 11,
      DocumentType = 10,
      Element = 1,
      Entity = 6,
      EntityReference = 5,
      Notation = 12,
      ProcessingInstruction = 7,
      Text = 3
   };
   Type type;

   QString namespaceUri;
   QString name;

   QString data;

   void addref();
   void release();

   DocumentImpl *document;
   NodeImpl *parent;

   QList<NodeImpl *> children;
   QList<NodeImpl *> attributes;
};

class DocumentImpl : public QDeclarativeRefCount, public NodeImpl
{
 public:
   DocumentImpl() : root(0) {
      type = Document;
   }
   virtual ~DocumentImpl() {
      if (root) {
         delete root;
      }
   }

   QString version;
   QString encoding;
   bool isStandalone;

   NodeImpl *root;

   void addref() {
      QDeclarativeRefCount::addref();
   }
   void release() {
      QDeclarativeRefCount::release();
   }
};

class NamedNodeMap
{
 public:
   // JS API
   static QScriptValue length(QScriptContext *context, QScriptEngine *engine);

   // C++ API
   static QScriptValue prototype(QScriptEngine *);
   static QScriptValue create(QScriptEngine *, NodeImpl *, QList<NodeImpl *> *);

   NamedNodeMap();
   NamedNodeMap(const NamedNodeMap &);
   ~NamedNodeMap();
   bool isNull();

   NodeImpl *d;
   QList<NodeImpl *> *list;
 private:
   NamedNodeMap &operator=(const NamedNodeMap &);
};

class NamedNodeMapClass : public QScriptClass
{
 public:
   NamedNodeMapClass(QScriptEngine *engine) : QScriptClass(engine) {}

   virtual QueryFlags queryProperty(const QScriptValue &object, const QScriptString &name, QueryFlags flags, uint *id);
   virtual QScriptValue property(const QScriptValue &object, const QScriptString &name, uint id);
};

class NodeList
{
 public:
   // JS API
   static QScriptValue length(QScriptContext *context, QScriptEngine *engine);

   // C++ API
   static QScriptValue prototype(QScriptEngine *);
   static QScriptValue create(QScriptEngine *, NodeImpl *);

   NodeList();
   NodeList(const NodeList &);
   ~NodeList();
   bool isNull();

   NodeImpl *d;
 private:
   NodeList &operator=(const NodeList &);
};

class NodeListClass : public QScriptClass
{
 public:
   NodeListClass(QScriptEngine *engine) : QScriptClass(engine) {}
   virtual QueryFlags queryProperty(const QScriptValue &object, const QScriptString &name, QueryFlags flags, uint *id);
   virtual QScriptValue property(const QScriptValue &object, const QScriptString &name, uint id);
};

class Node
{
 public:
   // JS API
   static QScriptValue nodeName(QScriptContext *context, QScriptEngine *engine);
   static QScriptValue nodeValue(QScriptContext *context, QScriptEngine *engine);
   static QScriptValue nodeType(QScriptContext *context, QScriptEngine *engine);

   static QScriptValue parentNode(QScriptContext *context, QScriptEngine *engine);
   static QScriptValue childNodes(QScriptContext *context, QScriptEngine *engine);
   static QScriptValue firstChild(QScriptContext *context, QScriptEngine *engine);
   static QScriptValue lastChild(QScriptContext *context, QScriptEngine *engine);
   static QScriptValue previousSibling(QScriptContext *context, QScriptEngine *engine);
   static QScriptValue nextSibling(QScriptContext *context, QScriptEngine *engine);
   static QScriptValue attributes(QScriptContext *context, QScriptEngine *engine);

   //static QScriptValue ownerDocument(QScriptContext *context, QScriptEngine *engine);
   //static QScriptValue namespaceURI(QScriptContext *context, QScriptEngine *engine);
   //static QScriptValue prefix(QScriptContext *context, QScriptEngine *engine);
   //static QScriptValue localName(QScriptContext *context, QScriptEngine *engine);
   //static QScriptValue baseURI(QScriptContext *context, QScriptEngine *engine);
   //static QScriptValue textContent(QScriptContext *context, QScriptEngine *engine);

   // C++ API
   static QScriptValue prototype(QScriptEngine *);
   static QScriptValue create(QScriptEngine *, NodeImpl *);

   Node();
   Node(const Node &o);
   ~Node();
   bool isNull() const;

   NodeImpl *d;

 private:
   Node &operator=(const Node &);
};

class Element : public Node
{
 public:
   // C++ API
   static QScriptValue prototype(QScriptEngine *);
};

class Attr : public Node
{
 public:
   // JS API
   static QScriptValue name(QScriptContext *context, QScriptEngine *engine);
   static QScriptValue specified(QScriptContext *context, QScriptEngine *engine);
   static QScriptValue value(QScriptContext *context, QScriptEngine *engine);
   static QScriptValue ownerElement(QScriptContext *context, QScriptEngine *engine);
   static QScriptValue schemaTypeInfo(QScriptContext *context, QScriptEngine *engine);
   static QScriptValue isId(QScriptContext *context, QScriptEngine *engine);

   // C++ API
   static QScriptValue prototype(QScriptEngine *);
};

class CharacterData : public Node
{
 public:
   // JS API
   static QScriptValue length(QScriptContext *context, QScriptEngine *engine);

   // C++ API
   static QScriptValue prototype(QScriptEngine *);
};

class Text : public CharacterData
{
 public:
   // JS API
   static QScriptValue isElementContentWhitespace(QScriptContext *context, QScriptEngine *engine);
   static QScriptValue wholeText(QScriptContext *context, QScriptEngine *engine);

   // C++ API
   static QScriptValue prototype(QScriptEngine *);
};

class CDATA : public Text
{
 public:
   // C++ API
   static QScriptValue prototype(QScriptEngine *);
};

class Document : public Node
{
 public:
   // JS API
   static QScriptValue xmlVersion(QScriptContext *context, QScriptEngine *engine);
   static QScriptValue xmlEncoding(QScriptContext *context, QScriptEngine *engine);
   static QScriptValue xmlStandalone(QScriptContext *context, QScriptEngine *engine);
   static QScriptValue documentElement(QScriptContext *context, QScriptEngine *engine);

   // C++ API
   static QScriptValue prototype(QScriptEngine *);
   static QScriptValue load(QScriptEngine *engine, const QByteArray &data);
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(Node)
Q_DECLARE_METATYPE(NodeList)
Q_DECLARE_METATYPE(NamedNodeMap)

QT_BEGIN_NAMESPACE

void NodeImpl::addref()
{
   A(document);
}

void NodeImpl::release()
{
   D(document);
}

QScriptValue Node::nodeName(QScriptContext *context, QScriptEngine *engine)
{
   Node node = qscriptvalue_cast<Node>(context->thisObject());
   if (node.isNull()) {
      return engine->undefinedValue();
   }

   switch (node.d->type) {
      case NodeImpl::Document:
         return QScriptValue(QLatin1String("#document"));
      case NodeImpl::CDATA:
         return QScriptValue(QLatin1String("#cdata-section"));
      case NodeImpl::Text:
         return QScriptValue(QLatin1String("#text"));
      default:
         return QScriptValue(node.d->name);
   }
}

QScriptValue Node::nodeValue(QScriptContext *context, QScriptEngine *engine)
{
   Node node = qscriptvalue_cast<Node>(context->thisObject());
   if (node.isNull()) {
      return engine->undefinedValue();
   }

   if (node.d->type == NodeImpl::Document ||
         node.d->type == NodeImpl::DocumentFragment ||
         node.d->type == NodeImpl::DocumentType ||
         node.d->type == NodeImpl::Element ||
         node.d->type == NodeImpl::Entity ||
         node.d->type == NodeImpl::EntityReference ||
         node.d->type == NodeImpl::Notation) {
      return engine->nullValue();
   }

   return QScriptValue(node.d->data);
}

QScriptValue Node::nodeType(QScriptContext *context, QScriptEngine *engine)
{
   Node node = qscriptvalue_cast<Node>(context->thisObject());
   if (node.isNull()) {
      return engine->undefinedValue();
   }
   return QScriptValue(node.d->type);
}

QScriptValue Node::parentNode(QScriptContext *context, QScriptEngine *engine)
{
   Node node = qscriptvalue_cast<Node>(context->thisObject());
   if (node.isNull()) {
      return engine->undefinedValue();
   }

   if (node.d->parent) {
      return Node::create(engine, node.d->parent);
   } else {
      return engine->nullValue();
   }
}

QScriptValue Node::childNodes(QScriptContext *context, QScriptEngine *engine)
{
   Node node = qscriptvalue_cast<Node>(context->thisObject());
   if (node.isNull()) {
      return engine->undefinedValue();
   }

   return NodeList::create(engine, node.d);
}

QScriptValue Node::firstChild(QScriptContext *context, QScriptEngine *engine)
{
   Node node = qscriptvalue_cast<Node>(context->thisObject());
   if (node.isNull()) {
      return engine->undefinedValue();
   }

   if (node.d->children.isEmpty()) {
      return engine->nullValue();
   } else {
      return Node::create(engine, node.d->children.first());
   }
}

QScriptValue Node::lastChild(QScriptContext *context, QScriptEngine *engine)
{
   Node node = qscriptvalue_cast<Node>(context->thisObject());
   if (node.isNull()) {
      return engine->undefinedValue();
   }

   if (node.d->children.isEmpty()) {
      return engine->nullValue();
   } else {
      return Node::create(engine, node.d->children.last());
   }
}

QScriptValue Node::previousSibling(QScriptContext *context, QScriptEngine *engine)
{
   Node node = qscriptvalue_cast<Node>(context->thisObject());
   if (node.isNull()) {
      return engine->undefinedValue();
   }

   if (!node.d->parent) {
      return engine->nullValue();
   }

   for (int ii = 0; ii < node.d->parent->children.count(); ++ii) {
      if (node.d->parent->children.at(ii) == node.d) {
         if (ii == 0) {
            return engine->nullValue();
         } else {
            return Node::create(engine, node.d->parent->children.at(ii - 1));
         }
      }
   }

   return engine->nullValue();
}

QScriptValue Node::nextSibling(QScriptContext *context, QScriptEngine *engine)
{
   Node node = qscriptvalue_cast<Node>(context->thisObject());
   if (node.isNull()) {
      return engine->undefinedValue();
   }

   if (!node.d->parent) {
      return engine->nullValue();
   }

   for (int ii = 0; ii < node.d->parent->children.count(); ++ii) {
      if (node.d->parent->children.at(ii) == node.d) {
         if ((ii + 1) == node.d->parent->children.count()) {
            return engine->nullValue();
         } else {
            return Node::create(engine, node.d->parent->children.at(ii + 1));
         }
      }
   }

   return engine->nullValue();
}

QScriptValue Node::attributes(QScriptContext *context, QScriptEngine *engine)
{
   Node node = qscriptvalue_cast<Node>(context->thisObject());
   if (node.isNull()) {
      return engine->undefinedValue();
   }

   if (node.d->type != NodeImpl::Element) {
      return engine->nullValue();
   } else {
      return NamedNodeMap::create(engine, node.d, &node.d->attributes);
   }
}

QScriptValue Node::prototype(QScriptEngine *engine)
{
   QScriptValue proto = engine->newObject();

   proto.setProperty(QLatin1String("nodeName"), engine->newFunction(nodeName),
                     QScriptValue::ReadOnly | QScriptValue::PropertyGetter);
   proto.setProperty(QLatin1String("nodeValue"), engine->newFunction(nodeValue),
                     QScriptValue::ReadOnly | QScriptValue::PropertyGetter | QScriptValue::PropertySetter);
   proto.setProperty(QLatin1String("nodeType"), engine->newFunction(nodeType),
                     QScriptValue::ReadOnly | QScriptValue::PropertyGetter);
   proto.setProperty(QLatin1String("parentNode"), engine->newFunction(parentNode),
                     QScriptValue::ReadOnly | QScriptValue::PropertyGetter);
   proto.setProperty(QLatin1String("childNodes"), engine->newFunction(childNodes),
                     QScriptValue::ReadOnly | QScriptValue::PropertyGetter);
   proto.setProperty(QLatin1String("firstChild"), engine->newFunction(firstChild),
                     QScriptValue::ReadOnly | QScriptValue::PropertyGetter);
   proto.setProperty(QLatin1String("lastChild"), engine->newFunction(lastChild),
                     QScriptValue::ReadOnly | QScriptValue::PropertyGetter);
   proto.setProperty(QLatin1String("previousSibling"), engine->newFunction(previousSibling),
                     QScriptValue::ReadOnly | QScriptValue::PropertyGetter);
   proto.setProperty(QLatin1String("nextSibling"), engine->newFunction(nextSibling),
                     QScriptValue::ReadOnly | QScriptValue::PropertyGetter);
   proto.setProperty(QLatin1String("attributes"), engine->newFunction(attributes),
                     QScriptValue::ReadOnly | QScriptValue::PropertyGetter);

   return proto;
}

QScriptValue Node::create(QScriptEngine *engine, NodeImpl *data)
{
   QScriptValue instance = engine->newObject();

   switch (data->type) {
      case NodeImpl::Attr:
         instance.setPrototype(Attr::prototype(engine));
         break;
      case NodeImpl::Comment:
      case NodeImpl::Document:
      case NodeImpl::DocumentFragment:
      case NodeImpl::DocumentType:
      case NodeImpl::Entity:
      case NodeImpl::EntityReference:
      case NodeImpl::Notation:
      case NodeImpl::ProcessingInstruction:
         return QScriptValue();
      case NodeImpl::CDATA:
         instance.setPrototype(CDATA::prototype(engine));
         break;
      case NodeImpl::Text:
         instance.setPrototype(Text::prototype(engine));
         break;
      case NodeImpl::Element:
         instance.setPrototype(Element::prototype(engine));
         break;
   }

   Node node;
   node.d = data;
   if (data) {
      A(data);
   }

   return engine->newVariant(instance, QVariant::fromValue(node));
}

QScriptValue Element::prototype(QScriptEngine *engine)
{
   QScriptValue proto = engine->newObject();
   proto.setPrototype(Node::prototype(engine));

   proto.setProperty(QLatin1String("tagName"), engine->newFunction(nodeName),
                     QScriptValue::ReadOnly | QScriptValue::PropertyGetter);

   return proto;
}

QScriptValue Attr::prototype(QScriptEngine *engine)
{
   QScriptValue proto = engine->newObject();
   proto.setPrototype(Node::prototype(engine));

   proto.setProperty(QLatin1String("name"), engine->newFunction(name),
                     QScriptValue::ReadOnly | QScriptValue::PropertyGetter);
   proto.setProperty(QLatin1String("value"), engine->newFunction(value),
                     QScriptValue::ReadOnly | QScriptValue::PropertyGetter);
   proto.setProperty(QLatin1String("ownerElement"), engine->newFunction(ownerElement),
                     QScriptValue::ReadOnly | QScriptValue::PropertyGetter);

   return proto;
}

QScriptValue Attr::name(QScriptContext *context, QScriptEngine *engine)
{
   Node node = qscriptvalue_cast<Node>(context->thisObject());
   if (node.isNull()) {
      return engine->undefinedValue();
   }

   return QScriptValue(node.d->name);
}

QScriptValue Attr::value(QScriptContext *context, QScriptEngine *engine)
{
   Node node = qscriptvalue_cast<Node>(context->thisObject());
   if (node.isNull()) {
      return engine->undefinedValue();
   }

   return QScriptValue(node.d->data);
}

QScriptValue Attr::ownerElement(QScriptContext *context, QScriptEngine *engine)
{
   Node node = qscriptvalue_cast<Node>(context->thisObject());
   if (node.isNull()) {
      return engine->undefinedValue();
   }

   return Node::create(engine, node.d->parent);
}

QScriptValue CharacterData::length(QScriptContext *context, QScriptEngine *engine)
{
   Node node = qscriptvalue_cast<Node>(context->thisObject());
   if (node.isNull()) {
      return engine->undefinedValue();
   }

   return QScriptValue(node.d->data.length());
}

QScriptValue CharacterData::prototype(QScriptEngine *engine)
{
   QScriptValue proto = engine->newObject();
   proto.setPrototype(Node::prototype(engine));

   proto.setProperty(QLatin1String("data"), engine->newFunction(nodeValue),
                     QScriptValue::ReadOnly | QScriptValue::PropertyGetter | QScriptValue::PropertySetter);
   proto.setProperty(QLatin1String("length"), engine->newFunction(length),
                     QScriptValue::ReadOnly | QScriptValue::PropertyGetter);

   return proto;
}

QScriptValue Text::isElementContentWhitespace(QScriptContext *context, QScriptEngine *engine)
{
   Node node = qscriptvalue_cast<Node>(context->thisObject());
   if (node.isNull()) {
      return engine->undefinedValue();
   }

   return node.d->data.trimmed().isEmpty();
}

QScriptValue Text::wholeText(QScriptContext *context, QScriptEngine *engine)
{
   Node node = qscriptvalue_cast<Node>(context->thisObject());
   if (node.isNull()) {
      return engine->undefinedValue();
   }

   return node.d->data;
}

QScriptValue Text::prototype(QScriptEngine *engine)
{
   QScriptValue proto = engine->newObject();
   proto.setPrototype(CharacterData::prototype(engine));

   proto.setProperty(QLatin1String("isElementContentWhitespace"), engine->newFunction(isElementContentWhitespace),
                     QScriptValue::ReadOnly | QScriptValue::PropertyGetter);
   proto.setProperty(QLatin1String("wholeText"), engine->newFunction(wholeText),
                     QScriptValue::ReadOnly | QScriptValue::PropertyGetter);

   return proto;
}

QScriptValue CDATA::prototype(QScriptEngine *engine)
{
   QScriptValue proto = engine->newObject();
   proto.setPrototype(Text::prototype(engine));
   return proto;
}

QScriptValue Document::prototype(QScriptEngine *engine)
{
   QScriptValue proto = engine->newObject();
   proto.setPrototype(Node::prototype(engine));

   proto.setProperty(QLatin1String("xmlVersion"), engine->newFunction(xmlVersion),
                     QScriptValue::ReadOnly | QScriptValue::PropertyGetter | QScriptValue::PropertySetter);
   proto.setProperty(QLatin1String("xmlEncoding"), engine->newFunction(xmlEncoding),
                     QScriptValue::ReadOnly | QScriptValue::PropertyGetter | QScriptValue::PropertySetter);
   proto.setProperty(QLatin1String("xmlStandalone"), engine->newFunction(xmlStandalone),
                     QScriptValue::ReadOnly | QScriptValue::PropertyGetter | QScriptValue::PropertySetter);
   proto.setProperty(QLatin1String("documentElement"), engine->newFunction(documentElement),
                     QScriptValue::ReadOnly | QScriptValue::PropertyGetter);

   return proto;
}

QScriptValue Document::load(QScriptEngine *engine, const QByteArray &data)
{
   Q_ASSERT(engine);

   DocumentImpl *document = 0;
   QStack<NodeImpl *> nodeStack;

   QXmlStreamReader reader(data);

   while (!reader.atEnd()) {
      switch (reader.readNext()) {
         case QXmlStreamReader::NoToken:
            break;
         case QXmlStreamReader::Invalid:
            break;
         case QXmlStreamReader::StartDocument:
            Q_ASSERT(!document);
            document = new DocumentImpl;
            document->document = document;
            document->version = reader.documentVersion().toString();
            document->encoding = reader.documentEncoding().toString();
            document->isStandalone = reader.isStandaloneDocument();
            break;
         case QXmlStreamReader::EndDocument:
            break;
         case QXmlStreamReader::StartElement: {
            Q_ASSERT(document);
            NodeImpl *node = new NodeImpl;
            node->document = document;
            node->namespaceUri = reader.namespaceUri().toString();
            node->name = reader.name().toString();
            if (nodeStack.isEmpty()) {
               document->root = node;
            } else {
               node->parent = nodeStack.top();
               node->parent->children.append(node);
            }
            nodeStack.append(node);

            foreach (const QXmlStreamAttribute & a, reader.attributes()) {
               NodeImpl *attr = new NodeImpl;
               attr->document = document;
               attr->type = NodeImpl::Attr;
               attr->namespaceUri = a.namespaceUri().toString();
               attr->name = a.name().toString();
               attr->data = a.value().toString();
               attr->parent = node;
               node->attributes.append(attr);
            }
         }
         break;
         case QXmlStreamReader::EndElement:
            nodeStack.pop();
            break;
         case QXmlStreamReader::Characters: {
            NodeImpl *node = new NodeImpl;
            node->document = document;
            node->type = reader.isCDATA() ? NodeImpl::CDATA : NodeImpl::Text;
            node->parent = nodeStack.top();
            node->parent->children.append(node);
            node->data = reader.text().toString();
         }
         break;
         case QXmlStreamReader::Comment:
            break;
         case QXmlStreamReader::DTD:
            break;
         case QXmlStreamReader::EntityReference:
            break;
         case QXmlStreamReader::ProcessingInstruction:
            break;
      }
   }

   if (!document || reader.hasError()) {
      if (document) {
         D(document);
      }
      return engine->nullValue();
   }

   QScriptValue instance = engine->newObject();
   instance.setPrototype(Document::prototype(engine));
   Node documentNode;
   documentNode.d = document;
   return engine->newVariant(instance, QVariant::fromValue(documentNode));
}

Node::Node()
   : d(0)
{
}

Node::Node(const Node &o)
   : d(o.d)
{
   if (d) {
      A(d);
   }
}

Node::~Node()
{
   if (d) {
      D(d);
   }
}

bool Node::isNull() const
{
   return d == 0;
}

QScriptValue NamedNodeMap::length(QScriptContext *context, QScriptEngine *engine)
{
   NamedNodeMap map = qscriptvalue_cast<NamedNodeMap>(context->thisObject().data());
   if (map.isNull()) {
      return engine->undefinedValue();
   }

   return QScriptValue(map.list->count());
}

QScriptValue NamedNodeMap::prototype(QScriptEngine *engine)
{
   QScriptValue proto = engine->newObject();

   proto.setProperty(QLatin1String("length"), engine->newFunction(length),
                     QScriptValue::ReadOnly | QScriptValue::PropertyGetter);

   return proto;
}

QScriptValue NamedNodeMap::create(QScriptEngine *engine, NodeImpl *data, QList<NodeImpl *> *list)
{
   QScriptValue instance = engine->newObject();
   instance.setPrototype(NamedNodeMap::prototype(engine));

   NamedNodeMap map;
   map.d = data;
   map.list = list;
   if (data) {
      A(data);
   }

   instance.setData(engine->newVariant(QVariant::fromValue(map)));

   if (!QDeclarativeScriptEngine::get(engine)->namedNodeMapClass) {
      QDeclarativeScriptEngine::get(engine)->namedNodeMapClass = new NamedNodeMapClass(engine);
   }

   instance.setScriptClass(QDeclarativeScriptEngine::get(engine)->namedNodeMapClass);

   return instance;
}

NamedNodeMap::NamedNodeMap()
   : d(0), list(0)
{
}

NamedNodeMap::NamedNodeMap(const NamedNodeMap &o)
   : d(o.d), list(o.list)
{
   if (d) {
      A(d);
   }
}

NamedNodeMap::~NamedNodeMap()
{
   if (d) {
      D(d);
   }
}

bool NamedNodeMap::isNull()
{
   return d == 0;
}

QScriptValue NodeList::length(QScriptContext *context, QScriptEngine *engine)
{
   NodeList list = qscriptvalue_cast<NodeList>(context->thisObject().data());
   if (list.isNull()) {
      return engine->undefinedValue();
   }

   return QScriptValue(list.d->children.count());
}

QScriptValue NodeList::prototype(QScriptEngine *engine)
{
   QScriptValue proto = engine->newObject();

   proto.setProperty(QLatin1String("length"), engine->newFunction(length),
                     QScriptValue::ReadOnly | QScriptValue::PropertyGetter);

   return proto;
}

QScriptValue NodeList::create(QScriptEngine *engine, NodeImpl *data)
{
   QScriptValue instance = engine->newObject();
   instance.setPrototype(NodeList::prototype(engine));

   NodeList list;
   list.d = data;
   if (data) {
      A(data);
   }

   instance.setData(engine->newVariant(QVariant::fromValue(list)));

   if (!QDeclarativeScriptEngine::get(engine)->nodeListClass) {
      QDeclarativeScriptEngine::get(engine)->nodeListClass = new NodeListClass(engine);
   }

   instance.setScriptClass(QDeclarativeScriptEngine::get(engine)->nodeListClass);

   return instance;
}

NodeList::NodeList()
   : d(0)
{
}

NodeList::NodeList(const NodeList &o)
   : d(o.d)
{
   if (d) {
      A(d);
   }
}

NodeList::~NodeList()
{
   if (d) {
      D(d);
   }
}

bool NodeList::isNull()
{
   return d == 0;
}

NamedNodeMapClass::QueryFlags NamedNodeMapClass::queryProperty(const QScriptValue &object, const QScriptString &name,
      QueryFlags flags, uint *id)
{
   if (!(flags & HandlesReadAccess)) {
      return 0;
   }

   NamedNodeMap map = qscriptvalue_cast<NamedNodeMap>(object.data());
   Q_ASSERT(!map.isNull());

   bool ok = false;
   QString nameString = name.toString();
   uint index = nameString.toUInt(&ok);
   if (ok) {
      if ((uint)map.list->count() <= index) {
         return 0;
      }

      *id = index;
      return HandlesReadAccess;
   } else {
      for (int ii = 0; ii < map.list->count(); ++ii) {
         if (map.list->at(ii) && map.list->at(ii)->name == nameString) {
            *id = ii;
            return HandlesReadAccess;
         }
      }
   }

   return 0;
}

QScriptValue NamedNodeMapClass::property(const QScriptValue &object, const QScriptString &, uint id)
{
   NamedNodeMap map = qscriptvalue_cast<NamedNodeMap>(object.data());
   return Node::create(engine(), map.list->at(id));
}

NodeListClass::QueryFlags NodeListClass::queryProperty(const QScriptValue &object, const QScriptString &name,
      QueryFlags flags, uint *id)
{
   if (!(flags & HandlesReadAccess)) {
      return 0;
   }

   bool ok = false;
   uint index = name.toString().toUInt(&ok);
   if (!ok) {
      return 0;
   }

   NodeList list = qscriptvalue_cast<NodeList>(object.data());
   if (list.isNull() || (uint)list.d->children.count() <= index) {
      return 0;   // ### I think we're meant to raise an exception
   }

   *id = index;
   return HandlesReadAccess;
}

QScriptValue NodeListClass::property(const QScriptValue &object, const QScriptString &, uint id)
{
   NodeList list = qscriptvalue_cast<NodeList>(object.data());
   return Node::create(engine(), list.d->children.at(id));
}

QScriptValue Document::documentElement(QScriptContext *context, QScriptEngine *engine)
{
   Node document = qscriptvalue_cast<Node>(context->thisObject());
   if (document.isNull() || document.d->type != NodeImpl::Document) {
      return engine->undefinedValue();
   }

   return Node::create(engine, static_cast<DocumentImpl *>(document.d)->root);
}

QScriptValue Document::xmlStandalone(QScriptContext *context, QScriptEngine *engine)
{
   Node document = qscriptvalue_cast<Node>(context->thisObject());
   if (document.isNull() || document.d->type != NodeImpl::Document) {
      return engine->undefinedValue();
   }

   return QScriptValue(static_cast<DocumentImpl *>(document.d)->isStandalone);
}

QScriptValue Document::xmlVersion(QScriptContext *context, QScriptEngine *engine)
{
   Node document = qscriptvalue_cast<Node>(context->thisObject());
   if (document.isNull() || document.d->type != NodeImpl::Document) {
      return engine->undefinedValue();
   }

   return QScriptValue(static_cast<DocumentImpl *>(document.d)->version);
}

QScriptValue Document::xmlEncoding(QScriptContext *context, QScriptEngine *engine)
{
   Node document = qscriptvalue_cast<Node>(context->thisObject());
   if (document.isNull() || document.d->type != NodeImpl::Document) {
      return engine->undefinedValue();
   }

   return QScriptValue(static_cast<DocumentImpl *>(document.d)->encoding);
}

class QDeclarativeXMLHttpRequest : public QObject
{
   DECL_CS_OBJECT(QDeclarativeXMLHttpRequest)

 public:
   enum State { Unsent = 0,
                Opened = 1, HeadersReceived = 2,
                Loading = 3, Done = 4
              };

   QDeclarativeXMLHttpRequest(QNetworkAccessManager *manager);
   virtual ~QDeclarativeXMLHttpRequest();

   bool sendFlag() const;
   bool errorFlag() const;
   quint32 readyState() const;
   int replyStatus() const;
   QString replyStatusText() const;

   QScriptValue open(QScriptValue *me, const QString &, const QUrl &);

   void addHeader(const QString &, const QString &);
   QString header(const QString &name);
   QString headers();
   QScriptValue send(QScriptValue *me, const QByteArray &);
   QScriptValue abort(QScriptValue *me);

   QString responseBody();
   const QByteArray &rawResponseBody() const;
   bool receivedXml() const;

 private:
   DECL_CS_SLOT_1(Private, void downloadProgress(qint64 un_named_arg1))
   DECL_CS_SLOT_2(downloadProgress)

   DECL_CS_SLOT_1(Private, void error(QNetworkReply::NetworkError un_named_arg1))
   DECL_CS_SLOT_2(error)

   DECL_CS_SLOT_1(Private, void finished())
   DECL_CS_SLOT_2(finished)

   void requestFromUrl(const QUrl &url);

   State m_state;
   bool m_errorFlag;
   bool m_sendFlag;
   QString m_method;
   QUrl m_url;
   QByteArray m_responseEntityBody;
   QByteArray m_data;
   int m_redirectCount;

   typedef QPair<QByteArray, QByteArray> HeaderPair;
   typedef QList<HeaderPair> HeadersList;
   HeadersList m_headersList;
   void fillHeadersList();

   bool m_gotXml;
   QByteArray m_mime;
   QByteArray m_charset;
   QTextCodec *m_textCodec;

#ifndef QT_NO_TEXTCODEC
   QTextCodec *findTextCodec() const;
#endif
   void readEncoding();

   QScriptValue m_me; // Set to the data object while a send() is ongoing (to access the callback)

   QScriptValue dispatchCallback(QScriptValue *me);
   void printError(const QScriptValue &);

   int m_status;
   QString m_statusText;
   QNetworkRequest m_request;
   QDeclarativeGuard<QNetworkReply> m_network;
   void destroyNetwork();

   QNetworkAccessManager *m_nam;
   QNetworkAccessManager *networkAccessManager() {
      return m_nam;
   }
};

QDeclarativeXMLHttpRequest::QDeclarativeXMLHttpRequest(QNetworkAccessManager *manager)
   : m_state(Unsent), m_errorFlag(false), m_sendFlag(false),
     m_redirectCount(0), m_gotXml(false), m_textCodec(0), m_network(0), m_nam(manager)
{
}

QDeclarativeXMLHttpRequest::~QDeclarativeXMLHttpRequest()
{
   destroyNetwork();
}

bool QDeclarativeXMLHttpRequest::sendFlag() const
{
   return m_sendFlag;
}

bool QDeclarativeXMLHttpRequest::errorFlag() const
{
   return m_errorFlag;
}

quint32 QDeclarativeXMLHttpRequest::readyState() const
{
   return m_state;
}

int QDeclarativeXMLHttpRequest::replyStatus() const
{
   return m_status;
}

QString QDeclarativeXMLHttpRequest::replyStatusText() const
{
   return m_statusText;
}

QScriptValue QDeclarativeXMLHttpRequest::open(QScriptValue *me, const QString &method, const QUrl &url)
{
   destroyNetwork();
   m_sendFlag = false;
   m_errorFlag = false;
   m_responseEntityBody = QByteArray();
   m_method = method;
   m_url = url;
   m_state = Opened;
   return dispatchCallback(me);
}

void QDeclarativeXMLHttpRequest::addHeader(const QString &name, const QString &value)
{
   QByteArray utfname = name.toUtf8();

   if (m_request.hasRawHeader(utfname)) {
      m_request.setRawHeader(utfname, m_request.rawHeader(utfname) + ',' + value.toUtf8());
   } else {
      m_request.setRawHeader(utfname, value.toUtf8());
   }
}

QString QDeclarativeXMLHttpRequest::header(const QString &name)
{
   QByteArray utfname = name.toLower().toUtf8();

   foreach (const HeaderPair & header, m_headersList) {
      if (header.first == utfname) {
         return QString::fromUtf8(header.second);
      }
   }
   return QString();
}

QString QDeclarativeXMLHttpRequest::headers()
{
   QString ret;

   foreach (const HeaderPair & header, m_headersList) {
      if (ret.length()) {
         ret.append(QLatin1String("\r\n"));
      }
      ret = ret % QString::fromUtf8(header.first) % QLatin1String(": ")
            % QString::fromUtf8(header.second);
   }
   return ret;
}

void QDeclarativeXMLHttpRequest::fillHeadersList()
{
   QList<QByteArray> headerList = m_network->rawHeaderList();

   m_headersList.clear();
   foreach (const QByteArray & header, headerList) {
      HeaderPair pair (header.toLower(), m_network->rawHeader(header));
      if (pair.first == "set-cookie" ||
            pair.first == "set-cookie2") {
         continue;
      }

      m_headersList << pair;
   }
}

void QDeclarativeXMLHttpRequest::requestFromUrl(const QUrl &url)
{
   QNetworkRequest request = m_request;
   request.setUrl(url);
   if (m_method == QLatin1String("POST") ||
         m_method == QLatin1String("PUT")) {
      QVariant var = request.header(QNetworkRequest::ContentTypeHeader);
      if (var.isValid()) {
         QString str = var.toString();
         int charsetIdx = str.indexOf(QLatin1String("charset="));
         if (charsetIdx == -1) {
            // No charset - append
            if (!str.isEmpty()) {
               str.append(QLatin1Char(';'));
            }
            str.append(QLatin1String("charset=UTF-8"));
         } else {
            charsetIdx += 8;
            int n = 0;
            int semiColon = str.indexOf(QLatin1Char(';'), charsetIdx);
            if (semiColon == -1) {
               n = str.length() - charsetIdx;
            } else {
               n = semiColon - charsetIdx;
            }

            str.replace(charsetIdx, n, QLatin1String("UTF-8"));
         }
         request.setHeader(QNetworkRequest::ContentTypeHeader, str);
      } else {
         request.setHeader(QNetworkRequest::ContentTypeHeader,
                           QLatin1String("text/plain;charset=UTF-8"));
      }
   }

   if (xhrDump()) {
      qWarning().nospace() << "XMLHttpRequest: " << qPrintable(m_method) << " " << qPrintable(url.toString());
      if (!m_data.isEmpty()) {
         qWarning().nospace() << "                "
                              << qPrintable(QString::fromUtf8(m_data));
      }
   }

   if (m_method == QLatin1String("GET")) {
      m_network = networkAccessManager()->get(request);
   } else if (m_method == QLatin1String("HEAD")) {
      m_network = networkAccessManager()->head(request);
   } else if (m_method == QLatin1String("POST")) {
      m_network = networkAccessManager()->post(request, m_data);
   } else if (m_method == QLatin1String("PUT")) {
      m_network = networkAccessManager()->put(request, m_data);
   } else if (m_method == QLatin1String("DELETE")) {
      m_network = networkAccessManager()->deleteResource(request);
   }

   QObject::connect(m_network, SIGNAL(downloadProgress(qint64, qint64)),
                    this, SLOT(downloadProgress(qint64)));
   QObject::connect(m_network, SIGNAL(error(QNetworkReply::NetworkError)),
                    this, SLOT(error(QNetworkReply::NetworkError)));
   QObject::connect(m_network, SIGNAL(finished()),
                    this, SLOT(finished()));
}

QScriptValue QDeclarativeXMLHttpRequest::send(QScriptValue *me, const QByteArray &data)
{
   m_errorFlag = false;
   m_sendFlag = true;
   m_redirectCount = 0;
   m_data = data;
   m_me = *me;

   requestFromUrl(m_url);

   return QScriptValue();
}

QScriptValue QDeclarativeXMLHttpRequest::abort(QScriptValue *me)
{
   destroyNetwork();
   m_responseEntityBody = QByteArray();
   m_errorFlag = true;
   m_request = QNetworkRequest();

   if (!(m_state == Unsent ||
         (m_state == Opened && !m_sendFlag) ||
         m_state == Done)) {

      m_state = Done;
      m_sendFlag = false;
      QScriptValue cbv = dispatchCallback(me);
      if (cbv.isError()) {
         return cbv;
      }
   }

   m_state = Unsent;
   return QScriptValue();
}

void QDeclarativeXMLHttpRequest::downloadProgress(qint64 bytes)
{
   Q_UNUSED(bytes)
   m_status =
      m_network->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
   m_statusText =
      QString::fromUtf8(m_network->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toByteArray());

   // ### We assume if this is called the headers are now available
   if (m_state < HeadersReceived) {
      m_state = HeadersReceived;
      fillHeadersList ();
      QScriptValue cbv = dispatchCallback(&m_me);
      if (cbv.isError()) {
         printError(cbv);
      }
   }

   bool wasEmpty = m_responseEntityBody.isEmpty();
   m_responseEntityBody.append(m_network->readAll());
   if (wasEmpty && !m_responseEntityBody.isEmpty()) {
      m_state = Loading;
      QScriptValue cbv = dispatchCallback(&m_me);
      if (cbv.isError()) {
         printError(cbv);
      }
   }
}

void QDeclarativeXMLHttpRequest::error(QNetworkReply::NetworkError error)
{
   Q_UNUSED(error)
   m_status =
      m_network->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
   m_statusText =
      QString::fromUtf8(m_network->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toByteArray());

   m_responseEntityBody = QByteArray();

   m_request = QNetworkRequest();
   m_data.clear();
   destroyNetwork();

   if (error == QNetworkReply::ContentAccessDenied ||
         error == QNetworkReply::ContentOperationNotPermittedError ||
         error == QNetworkReply::ContentNotFoundError ||
         error == QNetworkReply::AuthenticationRequiredError ||
         error == QNetworkReply::ContentReSendError) {
      m_state = Loading;
      QScriptValue cbv = dispatchCallback(&m_me);
      if (cbv.isError()) {
         printError(cbv);
      }
   } else {
      m_errorFlag = true;
   }

   m_state = Done;
   QScriptValue cbv = dispatchCallback(&m_me);
   if (cbv.isError()) {
      printError(cbv);
   }
}

#define XMLHTTPREQUEST_MAXIMUM_REDIRECT_RECURSION 15
void QDeclarativeXMLHttpRequest::finished()
{
   m_redirectCount++;
   if (m_redirectCount < XMLHTTPREQUEST_MAXIMUM_REDIRECT_RECURSION) {
      QVariant redirect = m_network->attribute(QNetworkRequest::RedirectionTargetAttribute);
      if (redirect.isValid()) {
         QUrl url = m_network->url().resolved(redirect.toUrl());
         if (url.scheme().toLower() != QLatin1String("file")) {
            // See http://www.ietf.org/rfc/rfc2616.txt, section 10.3.4 "303 See Other":
            // Result of 303 redirection should be a new "GET" request.
            const QVariant code = m_network->attribute(QNetworkRequest::HttpStatusCodeAttribute);
            if (code.isValid() && code.toInt() == 303 && m_method != QLatin1String("GET")) {
               m_method = QLatin1String("GET");
            }
            destroyNetwork();
            requestFromUrl(url);
            return;
         }
      }
   }

   m_status =
      m_network->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
   m_statusText =
      QString::fromUtf8(m_network->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toByteArray());

   if (m_state < HeadersReceived) {
      m_state = HeadersReceived;
      fillHeadersList ();
      QScriptValue cbv = dispatchCallback(&m_me);
      if (cbv.isError()) {
         printError(cbv);
      }
   }
   m_responseEntityBody.append(m_network->readAll());
   readEncoding();

   if (xhrDump()) {
      qWarning().nospace() << "XMLHttpRequest: RESPONSE " << qPrintable(m_url.toString());
      if (!m_responseEntityBody.isEmpty()) {
         qWarning().nospace() << "                "
                              << qPrintable(QString::fromUtf8(m_responseEntityBody));
      }
   }


   m_data.clear();
   destroyNetwork();
   if (m_state < Loading) {
      m_state = Loading;
      QScriptValue cbv = dispatchCallback(&m_me);
      if (cbv.isError()) {
         printError(cbv);
      }
   }
   m_state = Done;
   QScriptValue cbv = dispatchCallback(&m_me);
   if (cbv.isError()) {
      printError(cbv);
   }

   m_me = QScriptValue();
}


void QDeclarativeXMLHttpRequest::readEncoding()
{
   foreach (const HeaderPair & header, m_headersList) {
      if (header.first == "content-type") {
         int separatorIdx = header.second.indexOf(';');
         if (separatorIdx == -1) {
            m_mime = header.second;
         } else {
            m_mime = header.second.mid(0, separatorIdx);
            int charsetIdx = header.second.indexOf("charset=");
            if (charsetIdx != -1) {
               charsetIdx += 8;
               separatorIdx = header.second.indexOf(';', charsetIdx);
               m_charset = header.second.mid(charsetIdx, separatorIdx >= 0 ? separatorIdx : header.second.length());
            }
         }
         break;
      }
   }

   if (m_mime.isEmpty() || m_mime == "text/xml" || m_mime == "application/xml" || m_mime.endsWith("+xml")) {
      m_gotXml = true;
   }
}

bool QDeclarativeXMLHttpRequest::receivedXml() const
{
   return m_gotXml;
}


#ifndef QT_NO_TEXTCODEC
QTextCodec *QDeclarativeXMLHttpRequest::findTextCodec() const
{
   QTextCodec *codec = 0;

   if (!m_charset.isEmpty()) {
      codec = QTextCodec::codecForName(m_charset);
   }

   if (!codec && m_gotXml) {
      QXmlStreamReader reader(m_responseEntityBody);
      reader.readNext();
      codec = QTextCodec::codecForName(reader.documentEncoding().toString().toUtf8());
   }

   if (!codec && m_mime == "text/html") {
      codec = QTextCodec::codecForHtml(m_responseEntityBody, 0);
   }

   if (!codec) {
      codec = QTextCodec::codecForUtfText(m_responseEntityBody, 0);
   }

   if (!codec) {
      codec = QTextCodec::codecForName("UTF-8");
   }
   return codec;
}
#endif


QString QDeclarativeXMLHttpRequest::responseBody()
{
#ifndef QT_NO_TEXTCODEC
   if (!m_textCodec) {
      m_textCodec = findTextCodec();
   }
   if (m_textCodec) {
      return m_textCodec->toUnicode(m_responseEntityBody);
   }
#endif

   return QString::fromUtf8(m_responseEntityBody);
}

const QByteArray &QDeclarativeXMLHttpRequest::rawResponseBody() const
{
   return m_responseEntityBody;
}

QScriptValue QDeclarativeXMLHttpRequest::dispatchCallback(QScriptValue *me)
{
   QScriptValue v = me->property(QLatin1String("callback"));
   return v.call();
}

void QDeclarativeXMLHttpRequest::printError(const QScriptValue &sv)
{
   QDeclarativeError error;
   QDeclarativeExpressionPrivate::exceptionToError(sv.engine(), error);
   QDeclarativeEnginePrivate::warning(QDeclarativeEnginePrivate::get(sv.engine()), error);
}

void QDeclarativeXMLHttpRequest::destroyNetwork()
{
   if (m_network) {
      m_network->disconnect();
      m_network->deleteLater();
      m_network = 0;
   }
}

// XMLHttpRequest methods
static QScriptValue qmlxmlhttprequest_open(QScriptContext *context, QScriptEngine *engine)
{
   QScriptValue dataObject = context->thisObject().data();
   QDeclarativeXMLHttpRequest *request = qobject_cast<QDeclarativeXMLHttpRequest *>(dataObject.toQObject());
   if (!request) {
      THROW_REFERENCE("Not an XMLHttpRequest object");
   }

   if (context->argumentCount() < 2 || context->argumentCount() > 5) {
      THROW_DOM(SYNTAX_ERR, "Incorrect argument count");
   }

   // Argument 0 - Method
   QString method = context->argument(0).toString().toUpper();
   if (method != QLatin1String("GET") &&
         method != QLatin1String("PUT") &&
         method != QLatin1String("HEAD") &&
         method != QLatin1String("POST") &&
         method != QLatin1String("DELETE")) {
      THROW_DOM(SYNTAX_ERR, "Unsupported HTTP method type");
   }


   // Argument 1 - URL
   QUrl url = QUrl::fromEncoded(context->argument(1).toString().toUtf8());

   if (url.isRelative()) {
      url = QDeclarativeScriptEngine::get(engine)->resolvedUrl(context, url);
   }

   // Argument 2 - async (optional)
   if (context->argumentCount() > 2 && !context->argument(2).toBoolean()) {
      THROW_DOM(NOT_SUPPORTED_ERR, "Synchronous XMLHttpRequest calls are not supported");
   }


   // Argument 3/4 - user/pass (optional)
   QString username, password;
   if (context->argumentCount() > 3) {
      username = context->argument(3).toString();
   }
   if (context->argumentCount() > 4) {
      password = context->argument(4).toString();
   }


   // Clear the fragment (if any)
   url.setFragment(QString());
   // Set username/password
   if (!username.isNull()) {
      url.setUserName(username);
   }
   if (!password.isNull()) {
      url.setPassword(password);
   }

   return request->open(&dataObject, method, url);
}

static QScriptValue qmlxmlhttprequest_setRequestHeader(QScriptContext *context, QScriptEngine *engine)
{
   QDeclarativeXMLHttpRequest *request = qobject_cast<QDeclarativeXMLHttpRequest *>
                                         (context->thisObject().data().toQObject());
   if (!request) {
      THROW_REFERENCE("Not an XMLHttpRequest object");
   }

   if (context->argumentCount() != 2) {
      THROW_DOM(SYNTAX_ERR, "Incorrect argument count");
   }


   if (request->readyState() != QDeclarativeXMLHttpRequest::Opened ||
         request->sendFlag()) {
      THROW_DOM(INVALID_STATE_ERR, "Invalid state");
   }


   QString name = context->argument(0).toString();
   QString value = context->argument(1).toString();

   // ### Check that name and value are well formed

   QString nameUpper = name.toUpper();
   if (nameUpper == QLatin1String("ACCEPT-CHARSET") ||
         nameUpper == QLatin1String("ACCEPT-ENCODING") ||
         nameUpper == QLatin1String("CONNECTION") ||
         nameUpper == QLatin1String("CONTENT-LENGTH") ||
         nameUpper == QLatin1String("COOKIE") ||
         nameUpper == QLatin1String("COOKIE2") ||
         nameUpper == QLatin1String("CONTENT-TRANSFER-ENCODING") ||
         nameUpper == QLatin1String("DATE") ||
         nameUpper == QLatin1String("EXPECT") ||
         nameUpper == QLatin1String("HOST") ||
         nameUpper == QLatin1String("KEEP-ALIVE") ||
         nameUpper == QLatin1String("REFERER") ||
         nameUpper == QLatin1String("TE") ||
         nameUpper == QLatin1String("TRAILER") ||
         nameUpper == QLatin1String("TRANSFER-ENCODING") ||
         nameUpper == QLatin1String("UPGRADE") ||
         nameUpper == QLatin1String("USER-AGENT") ||
         nameUpper == QLatin1String("VIA") ||
         nameUpper.startsWith(QLatin1String("PROXY-")) ||
         nameUpper.startsWith(QLatin1String("SEC-"))) {
      return engine->undefinedValue();
   }

   request->addHeader(nameUpper, value);

   return engine->undefinedValue();
}

static QScriptValue qmlxmlhttprequest_send(QScriptContext *context, QScriptEngine *)
{
   QScriptValue dataObject = context->thisObject().data();
   QDeclarativeXMLHttpRequest *request = qobject_cast<QDeclarativeXMLHttpRequest *>(dataObject.toQObject());
   if (!request) {
      THROW_REFERENCE("Not an XMLHttpRequest object");
   }

   if (request->readyState() != QDeclarativeXMLHttpRequest::Opened) {
      THROW_DOM(INVALID_STATE_ERR, "Invalid state");
   }

   if (request->sendFlag()) {
      THROW_DOM(INVALID_STATE_ERR, "Invalid state");
   }

   QByteArray data;
   if (context->argumentCount() > 0) {
      data = context->argument(0).toString().toUtf8();
   }

   return request->send(&dataObject, data);
}

static QScriptValue qmlxmlhttprequest_abort(QScriptContext *context, QScriptEngine *)
{
   QScriptValue dataObject = context->thisObject().data();
   QDeclarativeXMLHttpRequest *request = qobject_cast<QDeclarativeXMLHttpRequest *>(dataObject.toQObject());
   if (!request) {
      THROW_REFERENCE("Not an XMLHttpRequest object");
   }

   return request->abort(&dataObject);
}

static QScriptValue qmlxmlhttprequest_getResponseHeader(QScriptContext *context, QScriptEngine *engine)
{
   Q_UNUSED(engine)
   QDeclarativeXMLHttpRequest *request = qobject_cast<QDeclarativeXMLHttpRequest *>
                                         (context->thisObject().data().toQObject());
   if (!request) {
      THROW_REFERENCE("Not an XMLHttpRequest object");
   }

   if (context->argumentCount() != 1) {
      THROW_DOM(SYNTAX_ERR, "Incorrect argument count");
   }

   if (request->readyState() != QDeclarativeXMLHttpRequest::Loading &&
         request->readyState() != QDeclarativeXMLHttpRequest::Done &&
         request->readyState() != QDeclarativeXMLHttpRequest::HeadersReceived) {
      THROW_DOM(INVALID_STATE_ERR, "Invalid state");
   }

   QString headerName = context->argument(0).toString();

   return QScriptValue(request->header(headerName));
}

static QScriptValue qmlxmlhttprequest_getAllResponseHeaders(QScriptContext *context, QScriptEngine *engine)
{
   Q_UNUSED(engine)
   QDeclarativeXMLHttpRequest *request = qobject_cast<QDeclarativeXMLHttpRequest *>
                                         (context->thisObject().data().toQObject());
   if (!request) {
      THROW_REFERENCE("Not an XMLHttpRequest object");
   }

   if (context->argumentCount() != 0) {
      THROW_DOM(SYNTAX_ERR, "Incorrect argument count");
   }

   if (request->readyState() != QDeclarativeXMLHttpRequest::Loading &&
         request->readyState() != QDeclarativeXMLHttpRequest::Done &&
         request->readyState() != QDeclarativeXMLHttpRequest::HeadersReceived) {
      THROW_DOM(INVALID_STATE_ERR, "Invalid state");
   }

   return QScriptValue(request->headers());
}

// XMLHttpRequest properties
static QScriptValue qmlxmlhttprequest_readyState(QScriptContext *context, QScriptEngine *engine)
{
   Q_UNUSED(engine)
   QDeclarativeXMLHttpRequest *request = qobject_cast<QDeclarativeXMLHttpRequest *>
                                         (context->thisObject().data().toQObject());
   if (!request) {
      THROW_REFERENCE("Not an XMLHttpRequest object");
   }

   return QScriptValue(request->readyState());
}

static QScriptValue qmlxmlhttprequest_status(QScriptContext *context, QScriptEngine *engine)
{
   Q_UNUSED(engine)
   QDeclarativeXMLHttpRequest *request = qobject_cast<QDeclarativeXMLHttpRequest *>
                                         (context->thisObject().data().toQObject());
   if (!request) {
      THROW_REFERENCE("Not an XMLHttpRequest object");
   }

   if (request->readyState() == QDeclarativeXMLHttpRequest::Unsent ||
         request->readyState() == QDeclarativeXMLHttpRequest::Opened) {
      THROW_DOM(INVALID_STATE_ERR, "Invalid state");
   }

   if (request->errorFlag()) {
      return QScriptValue(0);
   } else {
      return QScriptValue(request->replyStatus());
   }
}

static QScriptValue qmlxmlhttprequest_statusText(QScriptContext *context, QScriptEngine *engine)
{
   Q_UNUSED(engine)
   QDeclarativeXMLHttpRequest *request = qobject_cast<QDeclarativeXMLHttpRequest *>
                                         (context->thisObject().data().toQObject());
   if (!request) {
      THROW_REFERENCE("Not an XMLHttpRequest object");
   }

   if (request->readyState() == QDeclarativeXMLHttpRequest::Unsent ||
         request->readyState() == QDeclarativeXMLHttpRequest::Opened) {
      THROW_DOM(INVALID_STATE_ERR, "Invalid state");
   }

   if (request->errorFlag()) {
      return QScriptValue(0);
   } else {
      return QScriptValue(request->replyStatusText());
   }
}

static QScriptValue qmlxmlhttprequest_responseText(QScriptContext *context, QScriptEngine *engine)
{
   Q_UNUSED(engine)
   QDeclarativeXMLHttpRequest *request = qobject_cast<QDeclarativeXMLHttpRequest *>
                                         (context->thisObject().data().toQObject());
   if (!request) {
      THROW_REFERENCE("Not an XMLHttpRequest object");
   }

   if (request->readyState() != QDeclarativeXMLHttpRequest::Loading &&
         request->readyState() != QDeclarativeXMLHttpRequest::Done) {
      return QScriptValue(QString());
   } else {
      return QScriptValue(request->responseBody());
   }
}

static QScriptValue qmlxmlhttprequest_responseXML(QScriptContext *context, QScriptEngine *engine)
{
   QDeclarativeXMLHttpRequest *request = qobject_cast<QDeclarativeXMLHttpRequest *>
                                         (context->thisObject().data().toQObject());
   if (!request) {
      THROW_REFERENCE("Not an XMLHttpRequest object");
   }

   if (!request->receivedXml() ||
         (request->readyState() != QDeclarativeXMLHttpRequest::Loading &&
          request->readyState() != QDeclarativeXMLHttpRequest::Done)) {
      return engine->nullValue();
   } else {
      return Document::load(engine, request->rawResponseBody());
   }
}

static QScriptValue qmlxmlhttprequest_onreadystatechange(QScriptContext *context, QScriptEngine *engine)
{
   Q_UNUSED(engine);
   QScriptValue dataObject = context->thisObject().data();
   QDeclarativeXMLHttpRequest *request = qobject_cast<QDeclarativeXMLHttpRequest *>(dataObject.toQObject());
   if (!request) {
      THROW_REFERENCE("Not an XMLHttpRequest object");
   }

   if (context->argumentCount()) {
      QScriptValue v = context->argument(0);
      dataObject.setProperty(QLatin1String("callback"), v);
      return v;
   } else {
      return dataObject.property(QLatin1String("callback"));
   }
}

// Constructor
static QScriptValue qmlxmlhttprequest_new(QScriptContext *context, QScriptEngine *engine)
{
   if (context->isCalledAsConstructor()) {
      context->thisObject().setData(engine->newQObject(new QDeclarativeXMLHttpRequest(QDeclarativeScriptEngine::get(
                                       engine)->networkAccessManager()), QScriptEngine::ScriptOwnership));
   }
   return engine->undefinedValue();
}

void qt_add_qmlxmlhttprequest(QScriptEngine *engine)
{
   QScriptValue prototype = engine->newObject();

   // Methods
   prototype.setProperty(QLatin1String("open"), engine->newFunction(qmlxmlhttprequest_open, 2));
   prototype.setProperty(QLatin1String("setRequestHeader"), engine->newFunction(qmlxmlhttprequest_setRequestHeader, 2));
   prototype.setProperty(QLatin1String("send"), engine->newFunction(qmlxmlhttprequest_send));
   prototype.setProperty(QLatin1String("abort"), engine->newFunction(qmlxmlhttprequest_abort));
   prototype.setProperty(QLatin1String("getResponseHeader"), engine->newFunction(qmlxmlhttprequest_getResponseHeader, 1));
   prototype.setProperty(QLatin1String("getAllResponseHeaders"),
                         engine->newFunction(qmlxmlhttprequest_getAllResponseHeaders));

   // Read-only properties
   prototype.setProperty(QLatin1String("readyState"), engine->newFunction(qmlxmlhttprequest_readyState),
                         QScriptValue::ReadOnly | QScriptValue::PropertyGetter);
   prototype.setProperty(QLatin1String("status"), engine->newFunction(qmlxmlhttprequest_status),
                         QScriptValue::ReadOnly | QScriptValue::PropertyGetter);
   prototype.setProperty(QLatin1String("statusText"), engine->newFunction(qmlxmlhttprequest_statusText),
                         QScriptValue::ReadOnly | QScriptValue::PropertyGetter);
   prototype.setProperty(QLatin1String("responseText"), engine->newFunction(qmlxmlhttprequest_responseText),
                         QScriptValue::ReadOnly | QScriptValue::PropertyGetter);
   prototype.setProperty(QLatin1String("responseXML"), engine->newFunction(qmlxmlhttprequest_responseXML),
                         QScriptValue::ReadOnly | QScriptValue::PropertyGetter);
   prototype.setProperty(QLatin1String("onreadystatechange"), engine->newFunction(qmlxmlhttprequest_onreadystatechange),
                         QScriptValue::PropertyGetter | QScriptValue::PropertySetter);

   // State values
   prototype.setProperty(QLatin1String("UNSENT"), 0,
                         QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
   prototype.setProperty(QLatin1String("OPENED"), 1,
                         QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
   prototype.setProperty(QLatin1String("HEADERS_RECEIVED"), 2,
                         QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
   prototype.setProperty(QLatin1String("LOADING"), 3,
                         QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
   prototype.setProperty(QLatin1String("DONE"), 4,
                         QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);

   // Constructor
   QScriptValue constructor = engine->newFunction(qmlxmlhttprequest_new, prototype);
   constructor.setProperty(QLatin1String("UNSENT"), 0,
                           QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
   constructor.setProperty(QLatin1String("OPENED"), 1,
                           QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
   constructor.setProperty(QLatin1String("HEADERS_RECEIVED"), 2,
                           QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
   constructor.setProperty(QLatin1String("LOADING"), 3,
                           QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
   constructor.setProperty(QLatin1String("DONE"), 4,
                           QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
   engine->globalObject().setProperty(QLatin1String("XMLHttpRequest"), constructor);

   // DOM Exception
   QScriptValue domExceptionPrototype = engine->newObject();
   domExceptionPrototype.setProperty(QLatin1String("INDEX_SIZE_ERR"), INDEX_SIZE_ERR,
                                     QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
   domExceptionPrototype.setProperty(QLatin1String("DOMSTRING_SIZE_ERR"), DOMSTRING_SIZE_ERR,
                                     QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
   domExceptionPrototype.setProperty(QLatin1String("HIERARCHY_REQUEST_ERR"), HIERARCHY_REQUEST_ERR,
                                     QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
   domExceptionPrototype.setProperty(QLatin1String("WRONG_DOCUMENT_ERR"), WRONG_DOCUMENT_ERR,
                                     QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
   domExceptionPrototype.setProperty(QLatin1String("INVALID_CHARACTER_ERR"), INVALID_CHARACTER_ERR,
                                     QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
   domExceptionPrototype.setProperty(QLatin1String("NO_DATA_ALLOWED_ERR"), NO_DATA_ALLOWED_ERR,
                                     QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
   domExceptionPrototype.setProperty(QLatin1String("NO_MODIFICATION_ALLOWED_ERR"), NO_MODIFICATION_ALLOWED_ERR,
                                     QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
   domExceptionPrototype.setProperty(QLatin1String("NOT_FOUND_ERR"), NOT_FOUND_ERR,
                                     QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
   domExceptionPrototype.setProperty(QLatin1String("NOT_SUPPORTED_ERR"), NOT_SUPPORTED_ERR,
                                     QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
   domExceptionPrototype.setProperty(QLatin1String("INUSE_ATTRIBUTE_ERR"), INUSE_ATTRIBUTE_ERR,
                                     QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
   domExceptionPrototype.setProperty(QLatin1String("INVALID_STATE_ERR"), INVALID_STATE_ERR,
                                     QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
   domExceptionPrototype.setProperty(QLatin1String("SYNTAX_ERR"), SYNTAX_ERR,
                                     QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
   domExceptionPrototype.setProperty(QLatin1String("INVALID_MODIFICATION_ERR"), INVALID_MODIFICATION_ERR,
                                     QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
   domExceptionPrototype.setProperty(QLatin1String("NAMESPACE_ERR"), NAMESPACE_ERR,
                                     QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
   domExceptionPrototype.setProperty(QLatin1String("INVALID_ACCESS_ERR"), INVALID_ACCESS_ERR,
                                     QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
   domExceptionPrototype.setProperty(QLatin1String("VALIDATION_ERR"), VALIDATION_ERR,
                                     QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
   domExceptionPrototype.setProperty(QLatin1String("TYPE_MISMATCH_ERR"), TYPE_MISMATCH_ERR,
                                     QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);

   engine->globalObject().setProperty(QLatin1String("DOMException"), domExceptionPrototype);
}

QT_END_NAMESPACE

#endif // QT_NO_XMLSTREAMREADER
