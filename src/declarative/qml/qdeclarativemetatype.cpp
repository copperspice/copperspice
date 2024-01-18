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

#include <QtDeclarative/qdeclarativeprivate.h>
#include "private/qdeclarativemetatype_p.h"

#include "private/qdeclarativeproxymetaobject_p.h"
#include "private/qdeclarativecustomparser_p.h"
#include "private/qdeclarativeguard_p.h"
#include "private/qdeclarativeengine_p.h"
#include "private/qdeclarativeitemsmodule_p.h"
#include "private/qdeclarativeutilmodule_p.h"

#include <QtCore/qdebug.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qmetaobject.h>
#include <QtCore/qbitarray.h>
#include <QtCore/qreadwritelock.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qmetatype.h>
#include <qobjectdefs.h>
#include <qdatetime.h>
#include <qbytearray.h>
#include <qreadwritelock.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qvector.h>
#include <qlocale.h>
#include <QtCore/qcryptographichash.h>
#include <QtScript/qscriptvalue.h>

#include <ctype.h>
#include <qbitarray.h>
#include <qurl.h>
#include <qvariant.h>
#include <qsize.h>
#include <qpoint.h>
#include <qrect.h>
#include <qline.h>
#include <qvector3d.h>

#define NS(x) QT_PREPEND_NAMESPACE(x)

QT_BEGIN_NAMESPACE

struct QDeclarativeMetaTypeData {
   ~QDeclarativeMetaTypeData();
   QList<QDeclarativeType *> types;
   typedef QHash<int, QDeclarativeType *> Ids;
   Ids idToType;
   typedef QHash<QByteArray, QDeclarativeType *> Names;
   Names nameToType;
   typedef QHash<const QMetaObject *, QDeclarativeType *> MetaObjects;
   MetaObjects metaObjectToType;
   typedef QHash<int, QDeclarativeMetaType::StringConverter> StringConverters;
   StringConverters stringConverters;

   struct ModuleInfo {
      ModuleInfo(int major, int minor)
         : vmajor_min(major), vminor_min(minor), vmajor_max(major), vminor_max(minor) {}
      ModuleInfo(int major_min, int minor_min, int major_max, int minor_max)
         : vmajor_min(major_min), vminor_min(minor_min), vmajor_max(major_max), vminor_max(minor_max) {}
      int vmajor_min, vminor_min;
      int vmajor_max, vminor_max;
   };
   typedef QHash<QByteArray, ModuleInfo> ModuleInfoHash;
   ModuleInfoHash modules;

   QBitArray objects;
   QBitArray interfaces;
   QBitArray lists;

   QList<QDeclarativePrivate::AutoParentFunction> parentFunctions;
};
Q_GLOBAL_STATIC(QDeclarativeMetaTypeData, metaTypeData)
Q_GLOBAL_STATIC(QReadWriteLock, metaTypeDataLock)

struct QDeclarativeRegisteredComponentData {
   QMap<QByteArray, QDeclarativeDirComponents *> registeredComponents;
};

Q_GLOBAL_STATIC(QDeclarativeRegisteredComponentData, registeredComponentData)
Q_GLOBAL_STATIC(QReadWriteLock, registeredComponentDataLock)

QDeclarativeMetaTypeData::~QDeclarativeMetaTypeData()
{
   for (int i = 0; i < types.count(); ++i) {
      delete types.at(i);
   }
}

class QDeclarativeTypePrivate
{
 public:
   QDeclarativeTypePrivate();

   void init() const;

   bool m_isInterface : 1;
   const char *m_iid;
   QByteArray m_module;
   QByteArray m_name;
   int m_version_maj;
   int m_version_min;
   int m_typeId;
   int m_listId;
   int m_revision;
   mutable bool m_containsRevisionedAttributes;
   mutable QDeclarativeType *m_superType;

   int m_allocationSize;
   void (*m_newFunc)(void *);
   QString m_noCreationReason;

   const QMetaObject *m_baseMetaObject;
   QDeclarativeAttachedPropertiesFunc m_attachedPropertiesFunc;
   const QMetaObject *m_attachedPropertiesType;
   int m_attachedPropertiesId;
   int m_parserStatusCast;
   int m_propertyValueSourceCast;
   int m_propertyValueInterceptorCast;
   QObject *(*m_extFunc)(QObject *);
   const QMetaObject *m_extMetaObject;
   int m_index;
   QDeclarativeCustomParser *m_customParser;
   mutable volatile bool m_isSetup: 1;
   mutable bool m_haveSuperType : 1;
   mutable QList<QDeclarativeProxyMetaObject::ProxyData> m_metaObjects;

   static QHash<const QMetaObject *, int> m_attachedPropertyIds;
};

QHash<const QMetaObject *, int> QDeclarativeTypePrivate::m_attachedPropertyIds;

QDeclarativeTypePrivate::QDeclarativeTypePrivate()
   : m_isInterface(false), m_iid(0), m_typeId(0), m_listId(0), m_revision(0), m_containsRevisionedAttributes(false),
     m_superType(0), m_allocationSize(0), m_newFunc(0), m_baseMetaObject(0), m_attachedPropertiesFunc(0),
     m_attachedPropertiesType(0), m_parserStatusCast(-1), m_propertyValueSourceCast(-1),
     m_propertyValueInterceptorCast(-1), m_extFunc(0), m_extMetaObject(0), m_index(-1), m_customParser(0),
     m_isSetup(false), m_haveSuperType(false)
{
}


QDeclarativeType::QDeclarativeType(int index, const QDeclarativePrivate::RegisterInterface &interface)
   : d(new QDeclarativeTypePrivate)
{
   d->m_isInterface = true;
   d->m_iid = interface.iid;
   d->m_typeId = interface.typeId;
   d->m_listId = interface.listId;
   d->m_newFunc = 0;
   d->m_index = index;
   d->m_isSetup = true;
   d->m_version_maj = 0;
   d->m_version_min = 0;
}

QDeclarativeType::QDeclarativeType(int index, const QDeclarativePrivate::RegisterType &type)
   : d(new QDeclarativeTypePrivate)
{
   QByteArray name = type.uri;
   if (type.uri) {
      name += '/';
   }
   name += type.elementName;

   d->m_module = type.uri;
   d->m_name = name;
   d->m_version_maj = type.versionMajor;
   d->m_version_min = type.versionMinor;
   if (type.version >= 1) { // revisions added in version 1
      d->m_revision = type.revision;
   }
   d->m_typeId = type.typeId;
   d->m_listId = type.listId;
   d->m_allocationSize = type.objectSize;
   d->m_newFunc = type.create;
   d->m_noCreationReason = type.noCreationReason;
   d->m_baseMetaObject = type.metaObject;
   d->m_attachedPropertiesFunc = type.attachedPropertiesFunction;
   d->m_attachedPropertiesType = type.attachedPropertiesMetaObject;
   if (d->m_attachedPropertiesType) {
      QHash<const QMetaObject *, int>::Iterator iter = d->m_attachedPropertyIds.find(d->m_baseMetaObject);
      if (iter == d->m_attachedPropertyIds.end()) {
         iter = d->m_attachedPropertyIds.insert(d->m_baseMetaObject, index);
      }
      d->m_attachedPropertiesId = *iter;
   } else {
      d->m_attachedPropertiesId = -1;
   }
   d->m_parserStatusCast = type.parserStatusCast;
   d->m_propertyValueSourceCast = type.valueSourceCast;
   d->m_propertyValueInterceptorCast = type.valueInterceptorCast;
   d->m_extFunc = type.extensionObjectCreate;
   d->m_index = index;
   d->m_customParser = type.customParser;

   if (type.extensionMetaObject) {
      d->m_extMetaObject = type.extensionMetaObject;
   }
}

QDeclarativeType::~QDeclarativeType()
{
   delete d->m_customParser;
   delete d;
}

QByteArray QDeclarativeType::module() const
{
   return d->m_module;
}

int QDeclarativeType::majorVersion() const
{
   return d->m_version_maj;
}

int QDeclarativeType::minorVersion() const
{
   return d->m_version_min;
}

bool QDeclarativeType::availableInVersion(int vmajor, int vminor) const
{
   return vmajor > d->m_version_maj || (vmajor == d->m_version_maj && vminor >= d->m_version_min);
}

bool QDeclarativeType::availableInVersion(const QByteArray &module, int vmajor, int vminor) const
{
   return module == d->m_module && (vmajor > d->m_version_maj || (vmajor == d->m_version_maj &&
                                    vminor >= d->m_version_min));
}

// returns the nearest _registered_ super class
QDeclarativeType *QDeclarativeType::superType() const
{
   if (!d->m_haveSuperType) {
      const QMetaObject *mo = d->m_baseMetaObject->superClass();
      while (mo && !d->m_superType) {
         d->m_superType = QDeclarativeMetaType::qmlType(mo, d->m_module, d->m_version_maj, d->m_version_min);
         mo = mo->superClass();
      }
      d->m_haveSuperType = true;
   }

   return d->m_superType;
}

static void clone(QMetaObjectBuilder &builder, const QMetaObject *mo,
                  const QMetaObject *ignoreStart, const QMetaObject *ignoreEnd)
{
   // Clone Q_CLASSINFO
   for (int ii = mo->classInfoOffset(); ii < mo->classInfoCount(); ++ii) {
      QMetaClassInfo info = mo->classInfo(ii);

      int otherIndex = ignoreEnd->indexOfClassInfo(info.name());
      if (otherIndex >= ignoreStart->classInfoOffset() + ignoreStart->classInfoCount()) {
         // Skip
      } else {
         builder.addClassInfo(info.name(), info.value());
      }
   }

   // Clone Q_PROPERTY
   for (int ii = mo->propertyOffset(); ii < mo->propertyCount(); ++ii) {
      QMetaProperty property = mo->property(ii);

      int otherIndex = ignoreEnd->indexOfProperty(property.name());
      if (otherIndex >= ignoreStart->propertyOffset() + ignoreStart->propertyCount()) {
         builder.addProperty(QByteArray("__qml_ignore__") + property.name(), QByteArray("void"));
         // Skip
      } else {
         builder.addProperty(property);
      }
   }

   // Clone Q_METHODS
   for (int ii = mo->methodOffset(); ii < mo->methodCount(); ++ii) {
      QMetaMethod method = mo->method(ii);

      // More complex - need to search name
      QByteArray name = method.signature();
      int parenIdx = name.indexOf('(');
      if (parenIdx != -1) {
         name = name.left(parenIdx);
      }


      bool found = false;

      for (int ii = ignoreStart->methodOffset() + ignoreStart->methodCount();
            !found && ii < ignoreEnd->methodOffset() + ignoreEnd->methodCount();
            ++ii) {

         QMetaMethod other = ignoreEnd->method(ii);
         QByteArray othername = other.signature();
         int parenIdx = othername.indexOf('(');
         if (parenIdx != -1) {
            othername = othername.left(parenIdx);
         }

         found = name == othername;
      }

      QMetaMethodBuilder m = builder.addMethod(method);
      if (found) { // SKIP
         m.setAccess(QMetaMethod::Private);
      }
   }

   // Clone Q_ENUMS
   for (int ii = mo->enumeratorOffset(); ii < mo->enumeratorCount(); ++ii) {
      QMetaEnum enumerator = mo->enumerator(ii);

      int otherIndex = ignoreEnd->indexOfEnumerator(enumerator.name());
      if (otherIndex >= ignoreStart->enumeratorOffset() + ignoreStart->enumeratorCount()) {
         // Skip
      } else {
         builder.addEnumerator(enumerator);
      }
   }
}

void QDeclarativeTypePrivate::init() const
{
   if (m_isSetup) {
      return;
   }

   QWriteLocker lock(metaTypeDataLock());
   if (m_isSetup) {
      return;
   }

   // Setup extended meta object
   // XXX - very inefficient
   const QMetaObject *mo = m_baseMetaObject;
   if (m_extFunc) {
      QMetaObject *mmo = new QMetaObject;
      *mmo = *m_extMetaObject;
      mmo->d.superdata = mo;
      QDeclarativeProxyMetaObject::ProxyData data = { mmo, m_extFunc, 0, 0 };
      m_metaObjects << data;
   }

   mo = mo->d.superdata;
   while (mo) {
      QDeclarativeType *t = metaTypeData()->metaObjectToType.value(mo);
      if (t) {
         if (t->d->m_extFunc) {
            QMetaObjectBuilder builder;
            clone(builder, t->d->m_extMetaObject, t->d->m_baseMetaObject, m_baseMetaObject);
            QMetaObject *mmo = builder.toMetaObject();
            mmo->d.superdata = m_baseMetaObject;
            if (!m_metaObjects.isEmpty()) {
               m_metaObjects.last().metaObject->d.superdata = mmo;
            }
            QDeclarativeProxyMetaObject::ProxyData data = { mmo, t->d->m_extFunc, 0, 0 };
            m_metaObjects << data;
         }
      }
      mo = mo->d.superdata;
   }

   for (int ii = 0; ii < m_metaObjects.count(); ++ii) {
      m_metaObjects[ii].propertyOffset =
         m_metaObjects.at(ii).metaObject->propertyOffset();
      m_metaObjects[ii].methodOffset =
         m_metaObjects.at(ii).metaObject->methodOffset();
   }

   // Check for revisioned details
   {
      const QMetaObject *mo = 0;
      if (m_metaObjects.isEmpty()) {
         mo = m_baseMetaObject;
      } else {
         mo = m_metaObjects.first().metaObject;
      }

      for (int ii = 0; !m_containsRevisionedAttributes && ii < mo->propertyCount(); ++ii) {
         if (mo->property(ii).revision() != 0) {
            m_containsRevisionedAttributes = true;
         }
      }

      for (int ii = 0; !m_containsRevisionedAttributes && ii < mo->methodCount(); ++ii) {
         if (mo->method(ii).revision() != 0) {
            m_containsRevisionedAttributes = true;
         }
      }
   }

   m_isSetup = true;
   lock.unlock();
}

QByteArray QDeclarativeType::typeName() const
{
   if (d->m_baseMetaObject) {
      return d->m_baseMetaObject->className();
   } else {
      return QByteArray();
   }
}

QByteArray QDeclarativeType::qmlTypeName() const
{
   return d->m_name;
}

QObject *QDeclarativeType::create() const
{
   d->init();

   QObject *rv = (QObject *)operator new(d->m_allocationSize);
   d->m_newFunc(rv);

   if (rv && !d->m_metaObjects.isEmpty()) {
      (void)new QDeclarativeProxyMetaObject(rv, &d->m_metaObjects);
   }

   return rv;
}

void QDeclarativeType::create(QObject **out, void **memory, size_t additionalMemory) const
{
   d->init();

   QObject *rv = (QObject *)operator new(d->m_allocationSize + additionalMemory);
   d->m_newFunc(rv);

   if (rv && !d->m_metaObjects.isEmpty()) {
      (void)new QDeclarativeProxyMetaObject(rv, &d->m_metaObjects);
   }

   *out = rv;
   *memory = ((char *)rv) + d->m_allocationSize;
}

QDeclarativeCustomParser *QDeclarativeType::customParser() const
{
   return d->m_customParser;
}

QDeclarativeType::CreateFunc QDeclarativeType::createFunction() const
{
   return d->m_newFunc;
}

QString QDeclarativeType::noCreationReason() const
{
   return d->m_noCreationReason;
}

int QDeclarativeType::createSize() const
{
   return d->m_allocationSize;
}

bool QDeclarativeType::isCreatable() const
{
   return d->m_newFunc != 0;
}

bool QDeclarativeType::isExtendedType() const
{
   d->init();

   return !d->m_metaObjects.isEmpty();
}

bool QDeclarativeType::isInterface() const
{
   return d->m_isInterface;
}

int QDeclarativeType::typeId() const
{
   return d->m_typeId;
}

int QDeclarativeType::qListTypeId() const
{
   return d->m_listId;
}

const QMetaObject *QDeclarativeType::metaObject() const
{
   d->init();

   if (d->m_metaObjects.isEmpty()) {
      return d->m_baseMetaObject;
   } else {
      return d->m_metaObjects.first().metaObject;
   }

}

const QMetaObject *QDeclarativeType::baseMetaObject() const
{
   return d->m_baseMetaObject;
}

bool QDeclarativeType::containsRevisionedAttributes() const
{
   d->init();

   return d->m_containsRevisionedAttributes;
}

int QDeclarativeType::metaObjectRevision() const
{
   return d->m_revision;
}

QDeclarativeAttachedPropertiesFunc QDeclarativeType::attachedPropertiesFunction() const
{
   return d->m_attachedPropertiesFunc;
}

const QMetaObject *QDeclarativeType::attachedPropertiesType() const
{
   return d->m_attachedPropertiesType;
}

/*
This is the id passed to qmlAttachedPropertiesById().  This is different from the index
for the case that a single class is registered under two or more names (eg. Item in
Qt 4.7 and QtQuick 1.0).
*/
int QDeclarativeType::attachedPropertiesId() const
{
   return d->m_attachedPropertiesId;
}

int QDeclarativeType::parserStatusCast() const
{
   return d->m_parserStatusCast;
}

int QDeclarativeType::propertyValueSourceCast() const
{
   return d->m_propertyValueSourceCast;
}

int QDeclarativeType::propertyValueInterceptorCast() const
{
   return d->m_propertyValueInterceptorCast;
}

const char *QDeclarativeType::interfaceIId() const
{
   return d->m_iid;
}

int QDeclarativeType::index() const
{
   return d->m_index;
}

int registerAutoParentFunction(QDeclarativePrivate::RegisterAutoParent &autoparent)
{
   QWriteLocker lock(metaTypeDataLock());
   QDeclarativeMetaTypeData *data = metaTypeData();

   data->parentFunctions.append(autoparent.function);

   return data->parentFunctions.count() - 1;
}

int registerInterface(const QDeclarativePrivate::RegisterInterface &interface)
{
   if (interface.version > 0) {
      qFatal("qmlRegisterType(): Cannot mix incompatible QML versions.");
   }

   QWriteLocker lock(metaTypeDataLock());
   QDeclarativeMetaTypeData *data = metaTypeData();

   int index = data->types.count();

   QDeclarativeType *type = new QDeclarativeType(index, interface);

   data->types.append(type);
   data->idToType.insert(type->typeId(), type);
   data->idToType.insert(type->qListTypeId(), type);
   // XXX No insertMulti, so no multi-version interfaces?
   if (!type->qmlTypeName().isEmpty()) {
      data->nameToType.insert(type->qmlTypeName(), type);
   }

   if (data->interfaces.size() <= interface.typeId) {
      data->interfaces.resize(interface.typeId + 16);
   }
   if (data->lists.size() <= interface.listId) {
      data->lists.resize(interface.listId + 16);
   }
   data->interfaces.setBit(interface.typeId, true);
   data->lists.setBit(interface.listId, true);

   return index;
}

int registerType(const QDeclarativePrivate::RegisterType &type)
{
   if (type.elementName) {
      for (int ii = 0; type.elementName[ii]; ++ii) {
         if (!isalnum(type.elementName[ii])) {
            qWarning("qmlRegisterType(): Invalid QML element name \"%s\"", type.elementName);
            return -1;
         }
      }
   }

   QWriteLocker lock(metaTypeDataLock());
   QDeclarativeMetaTypeData *data = metaTypeData();
   int index = data->types.count();

   QDeclarativeType *dtype = new QDeclarativeType(index, type);

   data->types.append(dtype);
   data->idToType.insert(dtype->typeId(), dtype);
   if (dtype->qListTypeId()) {
      data->idToType.insert(dtype->qListTypeId(), dtype);
   }

   if (!dtype->qmlTypeName().isEmpty()) {
      data->nameToType.insertMulti(dtype->qmlTypeName(), dtype);
   }

   data->metaObjectToType.insertMulti(dtype->baseMetaObject(), dtype);

   if (data->objects.size() <= type.typeId) {
      data->objects.resize(type.typeId + 16);
   }
   if (data->lists.size() <= type.listId) {
      data->lists.resize(type.listId + 16);
   }
   data->objects.setBit(type.typeId, true);
   if (type.listId) {
      data->lists.setBit(type.listId, true);
   }

   if (type.uri) {
      QByteArray mod(type.uri);
      QDeclarativeMetaTypeData::ModuleInfoHash::Iterator it = data->modules.find(mod);
      if (it == data->modules.end()) {
         // New module
         data->modules.insert(mod, QDeclarativeMetaTypeData::ModuleInfo(type.versionMajor, type.versionMinor));
      } else if ((*it).vmajor_max < type.versionMajor || ((*it).vmajor_max == type.versionMajor &&
                 (*it).vminor_max < type.versionMinor)) {
         // Newer module
         data->modules.insert(mod, QDeclarativeMetaTypeData::ModuleInfo((*it).vmajor_min, (*it).vminor_min, type.versionMajor,
                              type.versionMinor));
      } else if ((*it).vmajor_min > type.versionMajor || ((*it).vmajor_min == type.versionMajor &&
                 (*it).vminor_min > type.versionMinor)) {
         // Older module
         data->modules.insert(mod, QDeclarativeMetaTypeData::ModuleInfo(type.versionMajor, type.versionMinor, (*it).vmajor_min,
                              (*it).vminor_min));
      }
   }

   return index;
}

int registerComponent(const QDeclarativePrivate::RegisterComponent &data)
{
   if (data.typeName) {
      for (int ii = 0; data.typeName[ii]; ++ii) {
         if (!isalnum(data.typeName[ii])) {
            qWarning("qmlRegisterType(): Invalid QML type name \"%s\"", data.typeName);
            return 0;
         }
      }
   } else {
      qWarning("qmlRegisterType(): No QML type name for \"%s\"", data.url.toString().toLatin1().constData());
      return 0;
   }

   QWriteLocker lock(registeredComponentDataLock());
   QString path;
   // Relative paths are relative to application working directory
   if (data.url.isRelative() || data.url.scheme() == QLatin1String("file")) { // Workaround QTBUG-11929
      path = QUrl::fromLocalFile(QDir::currentPath() + QLatin1String("/")).resolved(data.url).toString();
   } else {
      path = data.url.toString();
   }
   QDeclarativeRegisteredComponentData *d = registeredComponentData();
   QDeclarativeDirParser::Component comp(
      QString::fromUtf8(data.typeName),
      path,
      data.majorVersion,
      data.minorVersion
   );

   QDeclarativeDirComponents *comps = d->registeredComponents.value(QByteArray(data.uri), 0);
   if (!comps) {
      d->registeredComponents.insert(QByteArray(data.uri), comps = new QDeclarativeDirComponents);
   }

   // Types added later should take precedence, like registerType
   comps->prepend(comp);

   return 1;
}

/*
This method is "over generalized" to allow us to (potentially) register more types of things in
the future without adding exported symbols.
*/
int QDeclarativePrivate::qmlregister(RegistrationType type, void *data)
{
   if (type == TypeRegistration) {
      return registerType(*reinterpret_cast<RegisterType *>(data));
   } else if (type == InterfaceRegistration) {
      return registerInterface(*reinterpret_cast<RegisterInterface *>(data));
   } else if (type == AutoParentRegistration) {
      return registerAutoParentFunction(*reinterpret_cast<RegisterAutoParent *>(data));
   } else if (type == ComponentRegistration) {
      return registerComponent(*reinterpret_cast<RegisterComponent *>(data));
   }
   return -1;
}

/*
    Have any types been registered for \a module with at least versionMajor.versionMinor, and types
    for \a module with at most versionMajor.versionMinor.

    So if only 4.7 and 4.9 have been registered, 4.7,4.8, and 4.9 are valid, but not 4.6 nor 4.10.

    Passing -1 for both \a versionMajor \a versionMinor will return true if any version is installed.
*/
bool QDeclarativeMetaType::isModule(const QByteArray &module, int versionMajor, int versionMinor)
{
#ifndef QT_NO_IMPORT_QT47_QML
   // "import Qt 4.7" should have died off, but unfortunately, it was in a
   // major release. We don't register 4.7 types by default, as it's a
   // performance penalty. Instead, register them on-demand.
   if (strcmp(module.constData(), "Qt") == 0 && versionMajor == 4 && versionMinor == 7) {
      static bool qt47Registered = false;
      if (!qt47Registered) {
         qWarning() << Q_FUNC_INFO <<
                    "Qt 4.7 import detected; please note that Qt 4.7 is directly reusable as QtQuick 1.x with no code changes. Continuing, but startup time will be slower.";
         qt47Registered = true;
         QDeclarativeEnginePrivate::defineModuleCompat();
         QDeclarativeItemModule::defineModuleCompat();
         QDeclarativeValueTypeFactory::registerValueTypesCompat();
         QDeclarativeUtilModule::defineModuleCompat();
      }
   }
#endif

   QDeclarativeMetaTypeData *data = metaTypeData();
   QDeclarativeMetaTypeData::ModuleInfoHash::Iterator it = data->modules.find(module);
   return it != data->modules.end()
          && ((versionMajor < 0 && versionMinor < 0) ||
              (((*it).vmajor_max > versionMajor ||
                ((*it).vmajor_max == versionMajor && (*it).vminor_max >= versionMinor))
               && ((*it).vmajor_min < versionMajor ||
                   ((*it).vmajor_min == versionMajor && (*it).vminor_min <= versionMinor))));
}

QList<QDeclarativePrivate::AutoParentFunction> QDeclarativeMetaType::parentFunctions()
{
   QReadLocker lock(metaTypeDataLock());
   QDeclarativeMetaTypeData *data = metaTypeData();
   return data->parentFunctions;
}

QObject *QDeclarativeMetaType::toQObject(const QVariant &v, bool *ok)
{
   if (!isQObject(v.userType())) {
      if (ok) {
         *ok = false;
      }
      return 0;
   }

   if (ok) {
      *ok = true;
   }

   return *(QObject **)v.constData();
}

bool QDeclarativeMetaType::isQObject(int userType)
{
   if (userType == QMetaType::QObjectStar) {
      return true;
   }

   QReadLocker lock(metaTypeDataLock());
   QDeclarativeMetaTypeData *data = metaTypeData();
   return userType >= 0 && userType < data->objects.size() && data->objects.testBit(userType);
}

/*
    Returns the item type for a list of type \a id.
 */
int QDeclarativeMetaType::listType(int id)
{
   QReadLocker lock(metaTypeDataLock());
   QDeclarativeMetaTypeData *data = metaTypeData();
   QDeclarativeType *type = data->idToType.value(id);
   if (type && type->qListTypeId() == id) {
      return type->typeId();
   } else {
      return 0;
   }
}

int QDeclarativeMetaType::attachedPropertiesFuncId(const QMetaObject *mo)
{
   QReadLocker lock(metaTypeDataLock());
   QDeclarativeMetaTypeData *data = metaTypeData();

   QDeclarativeType *type = data->metaObjectToType.value(mo);
   if (type && type->attachedPropertiesFunction()) {
      return type->attachedPropertiesId();
   } else {
      return -1;
   }
}

QDeclarativeAttachedPropertiesFunc QDeclarativeMetaType::attachedPropertiesFuncById(int id)
{
   if (id < 0) {
      return 0;
   }
   QReadLocker lock(metaTypeDataLock());
   QDeclarativeMetaTypeData *data = metaTypeData();
   return data->types.at(id)->attachedPropertiesFunction();
}

QMetaProperty QDeclarativeMetaType::defaultProperty(const QMetaObject *metaObject)
{
   int idx = metaObject->indexOfClassInfo("DefaultProperty");
   if (-1 == idx) {
      return QMetaProperty();
   }

   QMetaClassInfo info = metaObject->classInfo(idx);
   if (!info.value()) {
      return QMetaProperty();
   }

   idx = metaObject->indexOfProperty(info.value());
   if (-1 == idx) {
      return QMetaProperty();
   }

   return metaObject->property(idx);
}

QMetaProperty QDeclarativeMetaType::defaultProperty(QObject *obj)
{
   if (!obj) {
      return QMetaProperty();
   }

   const QMetaObject *metaObject = obj->metaObject();
   return defaultProperty(metaObject);
}

QMetaMethod QDeclarativeMetaType::defaultMethod(const QMetaObject *metaObject)
{
   int idx = metaObject->indexOfClassInfo("DefaultMethod");
   if (-1 == idx) {
      return QMetaMethod();
   }

   QMetaClassInfo info = metaObject->classInfo(idx);
   if (!info.value()) {
      return QMetaMethod();
   }

   idx = metaObject->indexOfMethod(info.value());
   if (-1 == idx) {
      return QMetaMethod();
   }

   return metaObject->method(idx);
}

QMetaMethod QDeclarativeMetaType::defaultMethod(QObject *obj)
{
   if (!obj) {
      return QMetaMethod();
   }

   const QMetaObject *metaObject = obj->metaObject();
   return defaultMethod(metaObject);
}

QDeclarativeMetaType::TypeCategory QDeclarativeMetaType::typeCategory(int userType)
{
   if (userType < 0) {
      return Unknown;
   }
   if (userType == QMetaType::QObjectStar) {
      return Object;
   }

   QReadLocker lock(metaTypeDataLock());
   QDeclarativeMetaTypeData *data = metaTypeData();
   if (userType < data->objects.size() && data->objects.testBit(userType)) {
      return Object;
   } else if (userType < data->lists.size() && data->lists.testBit(userType)) {
      return List;
   } else {
      return Unknown;
   }
}

bool QDeclarativeMetaType::isInterface(int userType)
{
   QReadLocker lock(metaTypeDataLock());
   QDeclarativeMetaTypeData *data = metaTypeData();
   return userType >= 0 && userType < data->interfaces.size() && data->interfaces.testBit(userType);
}

const char *QDeclarativeMetaType::interfaceIId(int userType)
{
   QReadLocker lock(metaTypeDataLock());
   QDeclarativeMetaTypeData *data = metaTypeData();
   QDeclarativeType *type = data->idToType.value(userType);
   lock.unlock();
   if (type && type->isInterface() && type->typeId() == userType) {
      return type->interfaceIId();
   } else {
      return 0;
   }
}

bool QDeclarativeMetaType::isList(int userType)
{
   QReadLocker lock(metaTypeDataLock());
   QDeclarativeMetaTypeData *data = metaTypeData();
   return userType >= 0 && userType < data->lists.size() && data->lists.testBit(userType);
}

/*!
    A custom string convertor allows you to specify a function pointer that
    returns a variant of \a type. For example, if you have written your own icon
    class that you want to support as an object property assignable in QML:

    \code
    int type = qRegisterMetaType<SuperIcon>("SuperIcon");
    QML::addCustomStringConvertor(type, &SuperIcon::pixmapFromString);
    \endcode

    The function pointer must be of the form:
    \code
    QVariant (*StringConverter)(const QString &);
    \endcode
 */
void QDeclarativeMetaType::registerCustomStringConverter(int type, StringConverter converter)
{
   QWriteLocker lock(metaTypeDataLock());

   QDeclarativeMetaTypeData *data = metaTypeData();
   if (data->stringConverters.contains(type)) {
      return;
   }
   data->stringConverters.insert(type, converter);
}

/*!
    Return the custom string converter for \a type, previously installed through
    registerCustomStringConverter()
 */
QDeclarativeMetaType::StringConverter QDeclarativeMetaType::customStringConverter(int type)
{
   QReadLocker lock(metaTypeDataLock());

   QDeclarativeMetaTypeData *data = metaTypeData();
   return data->stringConverters.value(type);
}

/*!
    Returns the type (if any) of URI-qualified named \a name in version specified
    by \a version_major and \a version_minor.
*/
QDeclarativeType *QDeclarativeMetaType::qmlType(const QByteArray &name, int version_major, int version_minor)
{
   QReadLocker lock(metaTypeDataLock());
   QDeclarativeMetaTypeData *data = metaTypeData();

   QList<QDeclarativeType *> types = data->nameToType.values(name);
   foreach (QDeclarativeType * t, types) {
      // XXX version_major<0 just a kludge for QDeclarativePropertyPrivate::initProperty
      if (version_major < 0 || t->availableInVersion(version_major, version_minor)) {
         return t;
      }
   }
   return 0;
}

/*!
    Returns the type (if any) that corresponds to the \a metaObject.  Returns null if no
    type is registered.
*/
QDeclarativeType *QDeclarativeMetaType::qmlType(const QMetaObject *metaObject)
{
   QReadLocker lock(metaTypeDataLock());
   QDeclarativeMetaTypeData *data = metaTypeData();

   return data->metaObjectToType.value(metaObject);
}

/*!
    Returns the type (if any) that corresponds to the \a metaObject in version specified
    by \a version_major and \a version_minor in module specified by \a uri.  Returns null if no
    type is registered.
*/
QDeclarativeType *QDeclarativeMetaType::qmlType(const QMetaObject *metaObject, const QByteArray &module,
      int version_major, int version_minor)
{
   QReadLocker lock(metaTypeDataLock());
   QDeclarativeMetaTypeData *data = metaTypeData();

   QDeclarativeMetaTypeData::MetaObjects::const_iterator it = data->metaObjectToType.find(metaObject);
   while (it != data->metaObjectToType.end() && it.key() == metaObject) {
      QDeclarativeType *t = *it;
      if (version_major < 0 || t->availableInVersion(module, version_major, version_minor)) {
         return t;
      }
      ++it;
   }

   return 0;
}

/*!
    Returns the type (if any) that corresponds to the QVariant::Type \a userType.
    Returns null if no type is registered.
*/
QDeclarativeType *QDeclarativeMetaType::qmlType(int userType)
{
   QReadLocker lock(metaTypeDataLock());
   QDeclarativeMetaTypeData *data = metaTypeData();

   QDeclarativeType *type = data->idToType.value(userType);
   if (type && type->typeId() == userType) {
      return type;
   } else {
      return 0;
   }
}

/*!
    Returns the component(s) that have been registered for the module specified by \a uri and the version specified
    by \a version_major and \a version_minor.  Returns an empty list if no such components were registered.
*/
QDeclarativeDirComponents QDeclarativeMetaType::qmlComponents(const QByteArray &module, int version_major,
      int version_minor)
{
   QReadLocker lock(registeredComponentDataLock());
   QDeclarativeRegisteredComponentData *data = registeredComponentData();

   QDeclarativeDirComponents *comps = data->registeredComponents.value(module, 0);
   if (!comps) {
      return QDeclarativeDirComponents();
   }
   QDeclarativeDirComponents ret = *comps;
   for (int i = ret.count() - 1; i >= 0; i--) {
      QDeclarativeDirParser::Component &c = ret[i];
      if (version_major >= 0 && (c.majorVersion != version_major || c.minorVersion > version_minor)) {
         ret.removeAt(i);
      }
   }

   return ret;
}


/*!
    Returns the list of registered QML type names.
*/
QList<QByteArray> QDeclarativeMetaType::qmlTypeNames()
{
   QReadLocker lock(metaTypeDataLock());
   QDeclarativeMetaTypeData *data = metaTypeData();

   return data->nameToType.keys();
}

/*!
    Returns the list of registered QML types.
*/
QList<QDeclarativeType *> QDeclarativeMetaType::qmlTypes()
{
   QReadLocker lock(metaTypeDataLock());
   QDeclarativeMetaTypeData *data = metaTypeData();

   return data->nameToType.values();
}

QT_END_NAMESPACE

#include <QtGui/qfont.h>
#include <QtGui/qpixmap.h>
#include <QtGui/qbrush.h>
#include <QtGui/qcolor.h>
#include <QtGui/qpalette.h>
#include <QtGui/qicon.h>
#include <QtGui/qimage.h>
#include <QtGui/qpolygon.h>
#include <QtGui/qregion.h>
#include <QtGui/qbitmap.h>
#include <QtGui/qcursor.h>
#include <QtGui/qsizepolicy.h>
#include <QtGui/qkeysequence.h>
#include <QtGui/qpen.h>

//#include <QtGui/qtextlength.h>
#include <QtGui/qtextformat.h>
#include <QtGui/qmatrix.h>
#include <QtGui/qtransform.h>
#include <QtGui/qmatrix4x4.h>
#include <QtGui/qvector2d.h>
#include <QtGui/qvector3d.h>
#include <QtGui/qvector4d.h>
#include <QtGui/qquaternion.h>

Q_DECLARE_METATYPE(QScriptValue);

QT_BEGIN_NAMESPACE

bool QDeclarativeMetaType::canCopy(int type)
{
   switch (type) {
      case QMetaType::VoidStar:
      case QMetaType::QObjectStar:
      case QMetaType::QWidgetStar:
      case QMetaType::Long:
      case QMetaType::Int:
      case QMetaType::Short:
      case QMetaType::Char:
      case QMetaType::ULong:
      case QMetaType::UInt:
      case QMetaType::LongLong:
      case QMetaType::ULongLong:
      case QMetaType::UShort:
      case QMetaType::UChar:
      case QMetaType::Bool:
      case QMetaType::Float:
      case QMetaType::Double:
      case QMetaType::QChar:
      case QMetaType::QVariantMap:
      case QMetaType::QVariantHash:
      case QMetaType::QVariantList:
      case QMetaType::QByteArray:
      case QMetaType::QString:
      case QMetaType::QStringList:
      case QMetaType::QBitArray:
      case QMetaType::QDate:
      case QMetaType::QTime:
      case QMetaType::QDateTime:
      case QMetaType::QUrl:
      case QMetaType::QLocale:
      case QMetaType::QRect:
      case QMetaType::QRectF:
      case QMetaType::QSize:
      case QMetaType::QSizeF:
      case QMetaType::QLine:
      case QMetaType::QLineF:
      case QMetaType::QPoint:
      case QMetaType::QPointF:
      case QMetaType::QVector3D:
#ifndef QT_NO_REGEXP
      case QMetaType::QRegExp:
#endif
      case QMetaType::Void:
      case QMetaType::QFont:
      case QMetaType::QPixmap:
      case QMetaType::QBrush:
      case QMetaType::QColor:
      case QMetaType::QPalette:
      case QMetaType::QIcon:
      case QMetaType::QImage:
      case QMetaType::QPolygon:
      case QMetaType::QRegion:
      case QMetaType::QBitmap:
#ifndef QT_NO_CURSOR
      case QMetaType::QCursor:
#endif
      case QMetaType::QSizePolicy:
      case QMetaType::QKeySequence:
      case QMetaType::QPen:
      case QMetaType::QTextLength:
      case QMetaType::QTextFormat:
      case QMetaType::QMatrix:
      case QMetaType::QTransform:
      case QMetaType::QMatrix4x4:
      case QMetaType::QVector2D:
      case QMetaType::QVector4D:
      case QMetaType::QQuaternion:
         return true;

      default:
         if (type == qMetaTypeId<QVariant>() ||
               type == qMetaTypeId<QScriptValue>() ||
               typeCategory(type) != Unknown) {
            return true;
         }
         break;
   }

   return false;
}

/*!
    Copies \a copy into \a data, assuming they both are of type \a type.  If
    \a copy is zero, a default type is copied.  Returns true if the copy was
    successful and false if not.

    \note This should move into QMetaType once complete

*/
bool QDeclarativeMetaType::copy(int type, void *data, const void *copy)
{
   if (copy) {
      switch (type) {
         case QMetaType::VoidStar:
         case QMetaType::QObjectStar:
         case QMetaType::QWidgetStar:
            *static_cast<void **>(data) = *static_cast<void *const *>(copy);
            return true;
         case QMetaType::Long:
            *static_cast<long *>(data) = *static_cast<const long *>(copy);
            return true;
         case QMetaType::Int:
            *static_cast<int *>(data) = *static_cast<const int *>(copy);
            return true;
         case QMetaType::Short:
            *static_cast<short *>(data) = *static_cast<const short *>(copy);
            return true;
         case QMetaType::Char:
            *static_cast<char *>(data) = *static_cast<const char *>(copy);
            return true;
         case QMetaType::ULong:
            *static_cast<ulong *>(data) = *static_cast<const ulong *>(copy);
            return true;
         case QMetaType::UInt:
            *static_cast<uint *>(data) = *static_cast<const uint *>(copy);
            return true;
         case QMetaType::LongLong:
            *static_cast<qint64 *>(data) = *static_cast<const qint64 *>(copy);
            return true;
         case QMetaType::ULongLong:
            *static_cast<quint64 *>(data) = *static_cast<const quint64 *>(copy);
            return true;
         case QMetaType::UShort:
            *static_cast<ushort *>(data) = *static_cast<const ushort *>(copy);
            return true;
         case QMetaType::UChar:
            *static_cast<uchar *>(data) = *static_cast<const uchar *>(copy);
            return true;
         case QMetaType::Bool:
            *static_cast<bool *>(data) = *static_cast<const bool *>(copy);
            return true;
         case QMetaType::Float:
            *static_cast<float *>(data) = *static_cast<const float *>(copy);
            return true;
         case QMetaType::Double:
            *static_cast<double *>(data) = *static_cast<const double *>(copy);
            return true;
         case QMetaType::QChar:
            *static_cast<NS(QChar) *>(data) = *static_cast<const NS(QChar) *>(copy);
            return true;
         case QMetaType::QVariantMap:
            *static_cast<NS(QVariantMap) *>(data) = *static_cast<const NS(QVariantMap) *>(copy);
            return true;
         case QMetaType::QVariantHash:
            *static_cast<NS(QVariantHash) *>(data) = *static_cast<const NS(QVariantHash) *>(copy);
            return true;
         case QMetaType::QVariantList:
            *static_cast<NS(QVariantList) *>(data) = *static_cast<const NS(QVariantList) *>(copy);
            return true;
         case QMetaType::QByteArray:
            *static_cast<NS(QByteArray) *>(data) = *static_cast<const NS(QByteArray) *>(copy);
            return true;
         case QMetaType::QString:
            *static_cast<NS(QString) *>(data) = *static_cast<const NS(QString) *>(copy);
            return true;
         case QMetaType::QStringList:
            *static_cast<NS(QStringList) *>(data) = *static_cast<const NS(QStringList) *>(copy);
            return true;
         case QMetaType::QBitArray:
            *static_cast<NS(QBitArray) *>(data) = *static_cast<const NS(QBitArray) *>(copy);
            return true;
         case QMetaType::QDate:
            *static_cast<NS(QDate) *>(data) = *static_cast<const NS(QDate) *>(copy);
            return true;
         case QMetaType::QTime:
            *static_cast<NS(QTime) *>(data) = *static_cast<const NS(QTime) *>(copy);
            return true;
         case QMetaType::QDateTime:
            *static_cast<NS(QDateTime) *>(data) = *static_cast<const NS(QDateTime) *>(copy);
            return true;
         case QMetaType::QUrl:
            *static_cast<NS(QUrl) *>(data) = *static_cast<const NS(QUrl) *>(copy);
            return true;
         case QMetaType::QLocale:
            *static_cast<NS(QLocale) *>(data) = *static_cast<const NS(QLocale) *>(copy);
            return true;
         case QMetaType::QRect:
            *static_cast<NS(QRect) *>(data) = *static_cast<const NS(QRect) *>(copy);
            return true;
         case QMetaType::QRectF:
            *static_cast<NS(QRectF) *>(data) = *static_cast<const NS(QRectF) *>(copy);
            return true;
         case QMetaType::QSize:
            *static_cast<NS(QSize) *>(data) = *static_cast<const NS(QSize) *>(copy);
            return true;
         case QMetaType::QSizeF:
            *static_cast<NS(QSizeF) *>(data) = *static_cast<const NS(QSizeF) *>(copy);
            return true;
         case QMetaType::QLine:
            *static_cast<NS(QLine) *>(data) = *static_cast<const NS(QLine) *>(copy);
            return true;
         case QMetaType::QLineF:
            *static_cast<NS(QLineF) *>(data) = *static_cast<const NS(QLineF) *>(copy);
            return true;
         case QMetaType::QPoint:
            *static_cast<NS(QPoint) *>(data) = *static_cast<const NS(QPoint) *>(copy);
            return true;
         case QMetaType::QPointF:
            *static_cast<NS(QPointF) *>(data) = *static_cast<const NS(QPointF) *>(copy);
            return true;
         case QMetaType::QVector3D:
            *static_cast<NS(QVector3D) *>(data) = *static_cast<const NS(QVector3D) *>(copy);
            return true;
#ifndef QT_NO_REGEXP
         case QMetaType::QRegExp:
            *static_cast<NS(QRegExp) *>(data) = *static_cast<const NS(QRegExp) *>(copy);
            return true;
#endif
         case QMetaType::Void:
            return true;
         case QMetaType::QFont:
            *static_cast<NS(QFont) *>(data) = *static_cast<const NS(QFont) *>(copy);
            return true;
         case QMetaType::QPixmap:
            *static_cast<NS(QPixmap) *>(data) = *static_cast<const NS(QPixmap) *>(copy);
            return true;
         case QMetaType::QBrush:
            *static_cast<NS(QBrush) *>(data) = *static_cast<const NS(QBrush) *>(copy);
            return true;
         case QMetaType::QColor:
            *static_cast<NS(QColor) *>(data) = *static_cast<const NS(QColor) *>(copy);
            return true;
         case QMetaType::QPalette:
            *static_cast<NS(QPalette) *>(data) = *static_cast<const NS(QPalette) *>(copy);
            return true;
         case QMetaType::QIcon:
            *static_cast<NS(QIcon) *>(data) = *static_cast<const NS(QIcon) *>(copy);
            return true;
         case QMetaType::QImage:
            *static_cast<NS(QImage) *>(data) = *static_cast<const NS(QImage) *>(copy);
            return true;
         case QMetaType::QPolygon:
            *static_cast<NS(QPolygon) *>(data) = *static_cast<const NS(QPolygon) *>(copy);
            return true;
         case QMetaType::QRegion:
            *static_cast<NS(QRegion) *>(data) = *static_cast<const NS(QRegion) *>(copy);
            return true;
         case QMetaType::QBitmap:
            *static_cast<NS(QBitmap) *>(data) = *static_cast<const NS(QBitmap) *>(copy);
            return true;
#ifndef QT_NO_CURSOR
         case QMetaType::QCursor:
            *static_cast<NS(QCursor) *>(data) = *static_cast<const NS(QCursor) *>(copy);
            return true;
#endif
         case QMetaType::QSizePolicy:
            *static_cast<NS(QSizePolicy) *>(data) = *static_cast<const NS(QSizePolicy) *>(copy);
            return true;
         case QMetaType::QKeySequence:
            *static_cast<NS(QKeySequence) *>(data) = *static_cast<const NS(QKeySequence) *>(copy);
            return true;
         case QMetaType::QPen:
            *static_cast<NS(QPen) *>(data) = *static_cast<const NS(QPen) *>(copy);
            return true;
         case QMetaType::QTextLength:
            *static_cast<NS(QTextLength) *>(data) = *static_cast<const NS(QTextLength) *>(copy);
            return true;
         case QMetaType::QTextFormat:
            *static_cast<NS(QTextFormat) *>(data) = *static_cast<const NS(QTextFormat) *>(copy);
            return true;
         case QMetaType::QMatrix:
            *static_cast<NS(QMatrix) *>(data) = *static_cast<const NS(QMatrix) *>(copy);
            return true;
         case QMetaType::QTransform:
            *static_cast<NS(QTransform) *>(data) = *static_cast<const NS(QTransform) *>(copy);
            return true;
         case QMetaType::QMatrix4x4:
            *static_cast<NS(QMatrix4x4) *>(data) = *static_cast<const NS(QMatrix4x4) *>(copy);
            return true;
         case QMetaType::QVector2D:
            *static_cast<NS(QVector2D) *>(data) = *static_cast<const NS(QVector2D) *>(copy);
            return true;
         case QMetaType::QVector4D:
            *static_cast<NS(QVector4D) *>(data) = *static_cast<const NS(QVector4D) *>(copy);
            return true;
         case QMetaType::QQuaternion:
            *static_cast<NS(QQuaternion) *>(data) = *static_cast<const NS(QQuaternion) *>(copy);
            return true;

         default:
            if (type == qMetaTypeId<QVariant>()) {
               *static_cast<NS(QVariant) *>(data) = *static_cast<const NS(QVariant) *>(copy);
               return true;
            } else if (type == qMetaTypeId<QScriptValue>()) {
               *static_cast<NS(QScriptValue) *>(data) = *static_cast<const NS(QScriptValue) *>(copy);
               return true;
            } else if (typeCategory(type) != Unknown) {
               *static_cast<void **>(data) = *static_cast<void *const *>(copy);
               return true;
            }
            break;
      }
   } else {
      switch (type) {
         case QMetaType::VoidStar:
         case QMetaType::QObjectStar:
         case QMetaType::QWidgetStar:
            *static_cast<void **>(data) = 0;
            return true;
         case QMetaType::Long:
            *static_cast<long *>(data) = long(0);
            return true;
         case QMetaType::Int:
            *static_cast<int *>(data) = int(0);
            return true;
         case QMetaType::Short:
            *static_cast<short *>(data) = short(0);
            return true;
         case QMetaType::Char:
            *static_cast<char *>(data) = char(0);
            return true;
         case QMetaType::ULong:
            *static_cast<ulong *>(data) = ulong(0);
            return true;
         case QMetaType::UInt:
            *static_cast<uint *>(data) = uint(0);
            return true;
         case QMetaType::LongLong:
            *static_cast<qint64 *>(data) = qint64(0);
            return true;
         case QMetaType::ULongLong:
            *static_cast<quint64 *>(data) = quint64(0);
            return true;
         case QMetaType::UShort:
            *static_cast<ushort *>(data) = ushort(0);
            return true;
         case QMetaType::UChar:
            *static_cast<uchar *>(data) = uchar(0);
            return true;
         case QMetaType::Bool:
            *static_cast<bool *>(data) = bool(false);
            return true;
         case QMetaType::Float:
            *static_cast<float *>(data) = float(0);
            return true;
         case QMetaType::Double:
            *static_cast<double *>(data) = double(0);
            return true;
         case QMetaType::QChar:
            *static_cast<NS(QChar) *>(data) = NS(QChar)();
            return true;
         case QMetaType::QVariantMap:
            *static_cast<NS(QVariantMap) *>(data) = NS(QVariantMap)();
            return true;
         case QMetaType::QVariantHash:
            *static_cast<NS(QVariantHash) *>(data) = NS(QVariantHash)();
            return true;
         case QMetaType::QVariantList:
            *static_cast<NS(QVariantList) *>(data) = NS(QVariantList)();
            return true;
         case QMetaType::QByteArray:
            *static_cast<NS(QByteArray) *>(data) = NS(QByteArray)();
            return true;
         case QMetaType::QString:
            *static_cast<NS(QString) *>(data) = NS(QString)();
            return true;
         case QMetaType::QStringList:
            *static_cast<NS(QStringList) *>(data) = NS(QStringList)();
            return true;
         case QMetaType::QBitArray:
            *static_cast<NS(QBitArray) *>(data) = NS(QBitArray)();
            return true;
         case QMetaType::QDate:
            *static_cast<NS(QDate) *>(data) = NS(QDate)();
            return true;
         case QMetaType::QTime:
            *static_cast<NS(QTime) *>(data) = NS(QTime)();
            return true;
         case QMetaType::QDateTime:
            *static_cast<NS(QDateTime) *>(data) = NS(QDateTime)();
            return true;
         case QMetaType::QUrl:
            *static_cast<NS(QUrl) *>(data) = NS(QUrl)();
            return true;
         case QMetaType::QLocale:
            *static_cast<NS(QLocale) *>(data) = NS(QLocale)();
            return true;
         case QMetaType::QRect:
            *static_cast<NS(QRect) *>(data) = NS(QRect)();
            return true;
         case QMetaType::QRectF:
            *static_cast<NS(QRectF) *>(data) = NS(QRectF)();
            return true;
         case QMetaType::QSize:
            *static_cast<NS(QSize) *>(data) = NS(QSize)();
            return true;
         case QMetaType::QSizeF:
            *static_cast<NS(QSizeF) *>(data) = NS(QSizeF)();
            return true;
         case QMetaType::QLine:
            *static_cast<NS(QLine) *>(data) = NS(QLine)();
            return true;
         case QMetaType::QLineF:
            *static_cast<NS(QLineF) *>(data) = NS(QLineF)();
            return true;
         case QMetaType::QPoint:
            *static_cast<NS(QPoint) *>(data) = NS(QPoint)();
            return true;
         case QMetaType::QPointF:
            *static_cast<NS(QPointF) *>(data) = NS(QPointF)();
            return true;
         case QMetaType::QVector3D:
            *static_cast<NS(QVector3D) *>(data) = NS(QVector3D)();
            return true;
#ifndef QT_NO_REGEXP
         case QMetaType::QRegExp:
            *static_cast<NS(QRegExp) *>(data) = NS(QRegExp)();
            return true;
#endif
         case QMetaType::Void:
            return true;
         case QMetaType::QFont:
            *static_cast<NS(QFont) *>(data) = NS(QFont)();
            return true;
         case QMetaType::QPixmap:
            *static_cast<NS(QPixmap) *>(data) = NS(QPixmap)();
            return true;
         case QMetaType::QBrush:
            *static_cast<NS(QBrush) *>(data) = NS(QBrush)();
            return true;
         case QMetaType::QColor:
            *static_cast<NS(QColor) *>(data) = NS(QColor)();
            return true;
         case QMetaType::QPalette:
            *static_cast<NS(QPalette) *>(data) = NS(QPalette)();
            return true;
         case QMetaType::QIcon:
            *static_cast<NS(QIcon) *>(data) = NS(QIcon)();
            return true;
         case QMetaType::QImage:
            *static_cast<NS(QImage) *>(data) = NS(QImage)();
            return true;
         case QMetaType::QPolygon:
            *static_cast<NS(QPolygon) *>(data) = NS(QPolygon)();
            return true;
         case QMetaType::QRegion:
            *static_cast<NS(QRegion) *>(data) = NS(QRegion)();
            return true;
         case QMetaType::QBitmap:
            *static_cast<NS(QBitmap) *>(data) = NS(QBitmap)();
            return true;
#ifndef QT_NO_CURSOR
         case QMetaType::QCursor:
            *static_cast<NS(QCursor) *>(data) = NS(QCursor)();
            return true;
#endif
         case QMetaType::QSizePolicy:
            *static_cast<NS(QSizePolicy) *>(data) = NS(QSizePolicy)();
            return true;
         case QMetaType::QKeySequence:
            *static_cast<NS(QKeySequence) *>(data) = NS(QKeySequence)();
            return true;
         case QMetaType::QPen:
            *static_cast<NS(QPen) *>(data) = NS(QPen)();
            return true;
         case QMetaType::QTextLength:
            *static_cast<NS(QTextLength) *>(data) = NS(QTextLength)();
            return true;
         case QMetaType::QTextFormat:
            *static_cast<NS(QTextFormat) *>(data) = NS(QTextFormat)();
            return true;
         case QMetaType::QMatrix:
            *static_cast<NS(QMatrix) *>(data) = NS(QMatrix)();
            return true;
         case QMetaType::QTransform:
            *static_cast<NS(QTransform) *>(data) = NS(QTransform)();
            return true;
         case QMetaType::QMatrix4x4:
            *static_cast<NS(QMatrix4x4) *>(data) = NS(QMatrix4x4)();
            return true;
         case QMetaType::QVector2D:
            *static_cast<NS(QVector2D) *>(data) = NS(QVector2D)();
            return true;
         case QMetaType::QVector4D:
            *static_cast<NS(QVector4D) *>(data) = NS(QVector4D)();
            return true;
         case QMetaType::QQuaternion:
            *static_cast<NS(QQuaternion) *>(data) = NS(QQuaternion)();
            return true;
         default:
            if (type == qMetaTypeId<QVariant>()) {
               *static_cast<NS(QVariant) *>(data) = NS(QVariant)();
               return true;
            } else if (type == qMetaTypeId<QScriptValue>()) {
               *static_cast<NS(QScriptValue) *>(data) = NS(QScriptValue)();
               return true;
            } else if (typeCategory(type) != Unknown) {
               *static_cast<void **>(data) = 0;
               return true;
            }
            break;
      }
   }

   return false;
}

QT_END_NAMESPACE
