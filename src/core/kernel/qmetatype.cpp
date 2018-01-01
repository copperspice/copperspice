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

#include <qmetatype.h>

#include <qbitarray.h>
#include <qbytearray.h>
#include <qdatetime.h>
#include <qlocale.h>
#include <qreadwritelock.h>
#include <qstring.h>
#include <qstring8.h>
#include <qstring16.h>
#include <qstringlist.h>

#include <qeasingcurve.h>
#include <qurl.h>
#include <qvariant.h>
#include <quuid.h>
#include <qmodelindex.h>
#include <qsize.h>
#include <qpoint.h>
#include <qrect.h>
#include <qline.h>

#include <qmap.h>
#include <qmultimap.h>
#include <qhash.h>
#include <qmultihash.h>
#include <qvector.h>

QT_BEGIN_NAMESPACE

#define NS(x)  QT_PREPEND_NAMESPACE(x)

#define QT_ADD_STATIC_METATYPE(STR, TP)  { STR, sizeof(STR) - 1, TP }

Q_CORE_EXPORT const QMetaTypeGuiHelper *qMetaTypeGuiHelper = 0;

static const struct {
   const char *typeName;
   int typeNameLength;
   int type;
}

types[] = {

   // core types
   QT_ADD_STATIC_METATYPE("void",    QMetaType::Void),
   QT_ADD_STATIC_METATYPE("bool",    QMetaType::Bool),
   QT_ADD_STATIC_METATYPE("int",     QMetaType::Int),
   QT_ADD_STATIC_METATYPE("uint",    QMetaType::UInt),
   QT_ADD_STATIC_METATYPE("qint64",  QMetaType::LongLong),
   QT_ADD_STATIC_METATYPE("quint64", QMetaType::ULongLong),
   QT_ADD_STATIC_METATYPE("double",  QMetaType::Double),

   QT_ADD_STATIC_METATYPE("long", QMetaType::Long),
   QT_ADD_STATIC_METATYPE("short", QMetaType::Short),
   QT_ADD_STATIC_METATYPE("char", QMetaType::Char),
   QT_ADD_STATIC_METATYPE("ulong", QMetaType::ULong),
   QT_ADD_STATIC_METATYPE("ushort", QMetaType::UShort),
   QT_ADD_STATIC_METATYPE("uchar", QMetaType::UChar),
   QT_ADD_STATIC_METATYPE("float", QMetaType::Float),
   QT_ADD_STATIC_METATYPE("signed char", QMetaType::SChar),

   QT_ADD_STATIC_METATYPE("void*", QMetaType::VoidStar),

   QT_ADD_STATIC_METATYPE("QChar",       QMetaType::QChar),
   QT_ADD_STATIC_METATYPE("QString",     QMetaType::QString),
   QT_ADD_STATIC_METATYPE("QStringList", QMetaType::QStringList),
   QT_ADD_STATIC_METATYPE("QByteArray",  QMetaType::QByteArray),
   QT_ADD_STATIC_METATYPE("QBitArray",   QMetaType::QBitArray),

   QT_ADD_STATIC_METATYPE("QChar32",   QMetaType::QChar32),
   QT_ADD_STATIC_METATYPE("QString8",  QMetaType::QString8),
   QT_ADD_STATIC_METATYPE("QString16", QMetaType::QString16),

   QT_ADD_STATIC_METATYPE("QDate", QMetaType::QDate),
   QT_ADD_STATIC_METATYPE("QTime", QMetaType::QTime),
   QT_ADD_STATIC_METATYPE("QDateTime", QMetaType::QDateTime),
   QT_ADD_STATIC_METATYPE("QUrl", QMetaType::QUrl),
   QT_ADD_STATIC_METATYPE("QLocale", QMetaType::QLocale),

   QT_ADD_STATIC_METATYPE("QRect", QMetaType::QRect),
   QT_ADD_STATIC_METATYPE("QRectF", QMetaType::QRectF),
   QT_ADD_STATIC_METATYPE("QSize", QMetaType::QSize),
   QT_ADD_STATIC_METATYPE("QSizeF", QMetaType::QSizeF),
   QT_ADD_STATIC_METATYPE("QLine", QMetaType::QLine),
   QT_ADD_STATIC_METATYPE("QLineF", QMetaType::QLineF),
   QT_ADD_STATIC_METATYPE("QPoint", QMetaType::QPoint),
   QT_ADD_STATIC_METATYPE("QPointF", QMetaType::QPointF),

   QT_ADD_STATIC_METATYPE("QRegExp", QMetaType::QRegExp),
   QT_ADD_STATIC_METATYPE("QEasingCurve", QMetaType::QEasingCurve),
   QT_ADD_STATIC_METATYPE("QUuid", QMetaType::QUuid),
   QT_ADD_STATIC_METATYPE("QVariant", QMetaType::QVariant),
   QT_ADD_STATIC_METATYPE("QModelIndex", QMetaType::QModelIndex),

   QT_ADD_STATIC_METATYPE("QJsonValue", QMetaType:: QJsonValue),
   QT_ADD_STATIC_METATYPE("QJsonObject", QMetaType::QJsonObject),
   QT_ADD_STATIC_METATYPE("QJsonArray", QMetaType::QJsonArray),
   QT_ADD_STATIC_METATYPE("QJsonDocument", QMetaType::QJsonDocument),

   QT_ADD_STATIC_METATYPE("QObject*", QMetaType::QObjectStar),

   QT_ADD_STATIC_METATYPE("QVariantList",      QMetaType::QVariantList),
   QT_ADD_STATIC_METATYPE("QVariantMap",       QMetaType::QVariantMap),
   QT_ADD_STATIC_METATYPE("QVariantMultiMap",  QMetaType::QVariantMultiMap),
   QT_ADD_STATIC_METATYPE("QVariantHash",      QMetaType::QVariantHash),
   QT_ADD_STATIC_METATYPE("QVariantMultiHash", QMetaType::QVariantMultiHash),

   // GUI types
   QT_ADD_STATIC_METATYPE("QFont", QMetaType::QFont),
   QT_ADD_STATIC_METATYPE("QPixmap", QMetaType::QPixmap),
   QT_ADD_STATIC_METATYPE("QBrush", QMetaType::QBrush),
   QT_ADD_STATIC_METATYPE("QColor", QMetaType::QColor),
   QT_ADD_STATIC_METATYPE("QPalette", QMetaType::QPalette),
   QT_ADD_STATIC_METATYPE("QIcon", QMetaType::QIcon),
   QT_ADD_STATIC_METATYPE("QImage", QMetaType::QImage),
   QT_ADD_STATIC_METATYPE("QPolygon", QMetaType::QPolygon),
   QT_ADD_STATIC_METATYPE("QPolygonF", QMetaType::QPolygonF),
   QT_ADD_STATIC_METATYPE("QRegion", QMetaType::QRegion),
   QT_ADD_STATIC_METATYPE("QBitmap", QMetaType::QBitmap),
   QT_ADD_STATIC_METATYPE("QCursor", QMetaType::QCursor),
   QT_ADD_STATIC_METATYPE("QKeySequence", QMetaType::QKeySequence),
   QT_ADD_STATIC_METATYPE("QSizePolicy", QMetaType::QSizePolicy),
   QT_ADD_STATIC_METATYPE("QPen", QMetaType::QPen),
   QT_ADD_STATIC_METATYPE("QTextLength", QMetaType::QTextLength),
   QT_ADD_STATIC_METATYPE("QTextFormat", QMetaType::QTextFormat),
   QT_ADD_STATIC_METATYPE("QMatrix", QMetaType::QMatrix),
   QT_ADD_STATIC_METATYPE("QTransform", QMetaType::QTransform),
   QT_ADD_STATIC_METATYPE("QMatrix4x4", QMetaType::QMatrix4x4),
   QT_ADD_STATIC_METATYPE("QVector2D", QMetaType::QVector2D),
   QT_ADD_STATIC_METATYPE("QVector3D", QMetaType::QVector3D),
   QT_ADD_STATIC_METATYPE("QVector4D", QMetaType::QVector4D),
   QT_ADD_STATIC_METATYPE("QQuaternion", QMetaType::QQuaternion),

   QT_ADD_STATIC_METATYPE("QWidget*", QMetaType::QWidgetStar),

   // type aliases
   QT_ADD_STATIC_METATYPE("unsigned long", QMetaType::ULong),
   QT_ADD_STATIC_METATYPE("unsigned int", QMetaType::UInt),
   QT_ADD_STATIC_METATYPE("unsigned short", QMetaType::UShort),
   QT_ADD_STATIC_METATYPE("unsigned char", QMetaType::UChar),
   QT_ADD_STATIC_METATYPE("long long", QMetaType::LongLong),
   QT_ADD_STATIC_METATYPE("unsigned long long", QMetaType::ULongLong),

   QT_ADD_STATIC_METATYPE("qint8", QMetaType::Char),
   QT_ADD_STATIC_METATYPE("signed char", QMetaType::Char),
   QT_ADD_STATIC_METATYPE("quint8", QMetaType::UChar),

   QT_ADD_STATIC_METATYPE("qint16", QMetaType::Short),
   QT_ADD_STATIC_METATYPE("quint16", QMetaType::UShort),
   QT_ADD_STATIC_METATYPE("qint32", QMetaType::Int),
   QT_ADD_STATIC_METATYPE("quint32", QMetaType::UInt),
   QT_ADD_STATIC_METATYPE("qint64", QMetaType::LongLong),
   QT_ADD_STATIC_METATYPE("quint64", QMetaType::ULongLong),

   QT_ADD_STATIC_METATYPE("QList<QVariant>", QMetaType::QVariantList),
   QT_ADD_STATIC_METATYPE("QMap<QString,QVariant>",      QMetaType::QVariantMap),
   QT_ADD_STATIC_METATYPE("QMultiMap<QString,QVariant>", QMetaType::QVariantMultiMap),
   QT_ADD_STATIC_METATYPE("QHash<QString,QVariant>",     QMetaType::QVariantHash),
   QT_ADD_STATIC_METATYPE("QMultiHash<QString,QVariant>", QMetaType::QVariantMultiHash),

   // let QMetaTypeId2 figure out the type at compile time
   QT_ADD_STATIC_METATYPE("qreal", QMetaTypeId2<qreal>::MetaType),

   // default or unrecognized type
   {0, 0, QMetaType::UnknownType}
};

const QMap<QMetaType::Type, const QMetaTypeGuiHelper *> &QMetaType::dataTypes_Gui()
{
   static QMap<Type, const QMetaTypeGuiHelper *> data;

   if (data.empty() && qMetaTypeGuiHelper)  {

      for (int k = 0; qMetaTypeGuiHelper[k].typeId != QMetaType::UnknownType; ++k )  {
         // add gui data type to static structure
         data.insert(qMetaTypeGuiHelper[k].typeId, qMetaTypeGuiHelper + k);
      }
   }

   return data;
}

class QCustomTypeInfo
{
 public:

#ifndef QT_NO_DATASTREAM
   QCustomTypeInfo()
      : typeName(), constr(0), destr(0), saveOp(0), loadOp(0) {
   }

#else
   QCustomTypeInfo()
      : typeName(), constr(0), destr(0) {
   }

#endif

   QByteArray typeName;
   QMetaType::Constructor constr;
   QMetaType::Destructor destr;

#ifndef QT_NO_DATASTREAM
   QMetaType::SaveOperator saveOp;
   QMetaType::LoadOperator loadOp;
#endif

   int alias;
};

Q_DECLARE_TYPEINFO(QCustomTypeInfo, Q_MOVABLE_TYPE);
Q_GLOBAL_STATIC(QVector<QCustomTypeInfo>, customTypes)
Q_GLOBAL_STATIC(QReadWriteLock, customTypesLock)


#ifndef QT_NO_DATASTREAM

// internal
void QMetaType::registerStreamOperators(const char *typeName, SaveOperator saveOp, LoadOperator loadOp)
{
   int idx = type(typeName);

   if (! idx)  {
      return;
   }
   registerStreamOperators(idx, saveOp, loadOp);
}

// internal
void QMetaType::registerStreamOperators(int idx, SaveOperator saveOp, LoadOperator loadOp)
{
   if (idx < User)  {
      //builtin types should not be registered;
      return;
   }

   QVector<QCustomTypeInfo> *ct = customTypes();
   if (! ct) {
      return;
   }

   QWriteLocker locker(customTypesLock());
   QCustomTypeInfo &inf = (*ct)[idx - User];
   inf.saveOp = saveOp;
   inf.loadOp = loadOp;
}

#endif


// internal
static inline int qMetaTypeStaticType(const char *typeName, int length)
{
   int i = 0;

   while (types[i].typeName && ((length != types[i].typeNameLength) || strcmp(typeName, types[i].typeName))) {
      ++i;
   }

   return types[i].type;
}

// internal
static int qMetaTypeCustomType_unlocked(const char *typeName, int length)
{
   const QVector<QCustomTypeInfo> *const ct = customTypes();
   if (! ct) {
      return 0;
   }

   for (int v = 0; v < ct->count(); ++v) {
      const QCustomTypeInfo &customInfo = ct->at(v);

      if ((length == customInfo.typeName.size())
            && !strcmp(typeName, customInfo.typeName.constData())) {

         if (customInfo.alias >= 0) {
            return customInfo.alias;
         }

         return v + QMetaType::User;
      }
   }

   return 0;
}

// internal
int QMetaType::registerType(const char *typeName, Destructor destructor, Constructor constructor)
{
   QVector<QCustomTypeInfo> *ct = customTypes();

   if (! ct || !typeName || !destructor || !constructor)  {
      return -1;
   }

   NS(QByteArray) normalizedTypeName = QMetaObject::normalizedType(typeName);

   int idx = qMetaTypeStaticType(normalizedTypeName.constData(), normalizedTypeName.size());

   if (! idx) {
      QWriteLocker locker(customTypesLock());
      idx = qMetaTypeCustomType_unlocked(normalizedTypeName.constData(), normalizedTypeName.size());

      if (! idx) {
         QCustomTypeInfo inf;

         inf.typeName = normalizedTypeName;
         inf.constr   = constructor;
         inf.destr    = destructor;
         inf.alias    = -1;

         idx = ct->size() + User;
         ct->append(inf);
      }
   }

   return idx;
}

// internal
int QMetaType::registerTypedef(const char *typeName, int aliasId)
{
   QVector<QCustomTypeInfo> *ct = customTypes();

   if (! ct || ! typeName) {
      return -1;
   }

   NS(QByteArray) normalizedTypeName = QMetaObject::normalizedType(typeName);

   int idx = qMetaTypeStaticType(normalizedTypeName.constData(), normalizedTypeName.size());

   if (idx != QMetaType::UnknownType) {
      Q_ASSERT(idx == aliasId);
      return idx;
   }

   QWriteLocker locker(customTypesLock());
   idx = qMetaTypeCustomType_unlocked(normalizedTypeName.constData(), normalizedTypeName.size());

   if (idx)  {
      return idx;
   }

   QCustomTypeInfo inf;

   inf.typeName = normalizedTypeName;
   inf.alias    = aliasId;
   inf.constr   = 0;
   inf.destr    = 0;

   ct->append(inf);
   return aliasId;
}

void *QMetaType::construct(int type, const void *copy)
{
   if (copy) {

      switch (type) {
         case QMetaType::VoidStar:
         case QMetaType::QObjectStar:
         case QMetaType::QWidgetStar:
            return new void *(*static_cast<void *const *>(copy));

         case QMetaType::Long:
            return new long(*static_cast<const long *>(copy));

         case QMetaType::Int:
            return new int(*static_cast<const int *>(copy));

         case QMetaType::Short:
            return new short(*static_cast<const short *>(copy));

         case QMetaType::Char:
            return new char(*static_cast<const char *>(copy));

         case QMetaType::ULong:
            return new ulong(*static_cast<const ulong *>(copy));

         case QMetaType::UInt:
            return new uint(*static_cast<const uint *>(copy));

         case QMetaType::LongLong:
            return new qint64(*static_cast<const qint64 *>(copy));

         case QMetaType::ULongLong:
            return new quint64(*static_cast<const quint64 *>(copy));

         case QMetaType::UShort:
            return new ushort(*static_cast<const ushort *>(copy));

         case QMetaType::UChar:
            return new uchar(*static_cast<const uchar *>(copy));

         case QMetaType::Bool:
            return new bool(*static_cast<const bool *>(copy));

         case QMetaType::Float:
            return new float(*static_cast<const float *>(copy));

         case QMetaType::Double:
            return new double(*static_cast<const double *>(copy));

         case QMetaType::QChar:
            return new NS(QChar)(*static_cast<const NS(QChar) *>(copy));

         case QMetaType::QChar32:
            return new NS(QChar32)(*static_cast<const NS(QChar32) *>(copy));

         case QMetaType::QVariantMap:
            return new NS(QVariantMap)(*static_cast<const NS(QVariantMap) *>(copy));

         case QMetaType::QVariantMultiMap:
            return new NS(QVariantMultiMap)(*static_cast<const NS(QVariantMultiMap) *>(copy));

         case QMetaType::QVariantHash:
            return new NS(QVariantHash)(*static_cast<const NS(QVariantHash) *>(copy));

         case QMetaType::QVariantMultiHash:
            return new NS(QVariantMultiHash)(*static_cast<const NS(QVariantMultiHash) *>(copy));

         case QMetaType::QVariantList:
            return new NS(QVariantList)(*static_cast<const NS(QVariantList) *>(copy));

         case QMetaType::QVariant:
            return new NS(QVariant)(*static_cast<const NS(QVariant) *>(copy));

         case QMetaType::QByteArray:
            return new NS(QByteArray)(*static_cast<const NS(QByteArray) *>(copy));

         case QMetaType::QString:
            return new NS(QString)(*static_cast<const NS(QString) *>(copy));

         case QMetaType::QString8:
            return new NS(QString8)(*static_cast<const NS(QString8) *>(copy));

          case QMetaType::QString16:
            return new NS(QString16)(*static_cast<const NS(QString16) *>(copy));

         case QMetaType::QStringList:
            return new NS(QStringList)(*static_cast<const NS(QStringList) *>(copy));

         case QMetaType::QBitArray:
            return new NS(QBitArray)(*static_cast<const NS(QBitArray) *>(copy));

         case QMetaType::QDate:
            return new NS(QDate)(*static_cast<const NS(QDate) *>(copy));

         case QMetaType::QTime:
            return new NS(QTime)(*static_cast<const NS(QTime) *>(copy));

         case QMetaType::QDateTime:
            return new NS(QDateTime)(*static_cast<const NS(QDateTime) *>(copy));

         case QMetaType::QUrl:
            return new NS(QUrl)(*static_cast<const NS(QUrl) *>(copy));

         case QMetaType::QLocale:
            return new NS(QLocale)(*static_cast<const NS(QLocale) *>(copy));

         case QMetaType::QRect:
            return new NS(QRect)(*static_cast<const NS(QRect) *>(copy));

         case QMetaType::QRectF:
            return new NS(QRectF)(*static_cast<const NS(QRectF) *>(copy));

         case QMetaType::QSize:
            return new NS(QSize)(*static_cast<const NS(QSize) *>(copy));

         case QMetaType::QSizeF:
            return new NS(QSizeF)(*static_cast<const NS(QSizeF) *>(copy));

         case QMetaType::QLine:
            return new NS(QLine)(*static_cast<const NS(QLine) *>(copy));

         case QMetaType::QLineF:
            return new NS(QLineF)(*static_cast<const NS(QLineF) *>(copy));

         case QMetaType::QPoint:
            return new NS(QPoint)(*static_cast<const NS(QPoint) *>(copy));

         case QMetaType::QPointF:
            return new NS(QPointF)(*static_cast<const NS(QPointF) *>(copy));

#ifndef QT_NO_REGEXP
         case QMetaType::QRegExp:
            return new NS(QRegExp)(*static_cast<const NS(QRegExp) *>(copy));
#endif

         case QMetaType::QEasingCurve:
            return new NS(QEasingCurve)(*static_cast<const NS(QEasingCurve) *>(copy));

         case QMetaType::QUuid:
            return new NS(QUuid)(*static_cast<const NS(QUuid) *>(copy));

         case QMetaType::QModelIndex:
            return new NS(QModelIndex)(*static_cast<const NS(QModelIndex) *>(copy));

         case QMetaType::Void:
            return 0;

         default:
            ;
      }

   } else {

      switch (type) {
         case QMetaType::VoidStar:
         case QMetaType::QObjectStar:
         case QMetaType::QWidgetStar:
            return new void *;

         case QMetaType::Long:
            return new long;

         case QMetaType::Int:
            return new int;

         case QMetaType::Short:
            return new short;

         case QMetaType::Char:
            return new char;

         case QMetaType::ULong:
            return new ulong;

         case QMetaType::UInt:
            return new uint;

         case QMetaType::LongLong:
            return new qint64;

         case QMetaType::ULongLong:
            return new quint64;

         case QMetaType::UShort:
            return new ushort;

         case QMetaType::UChar:
            return new uchar;

         case QMetaType::Bool:
            return new bool;

         case QMetaType::Float:
            return new float;

         case QMetaType::Double:
            return new double;

         case QMetaType::QChar:
            return new NS(QChar);

         case QMetaType::QChar32:
            return new NS(QChar32);

         case QMetaType::QVariantMap:
            return new NS(QVariantMap);

         case QMetaType::QVariantMultiMap:
            return new NS(QVariantMultiMap);

         case QMetaType::QVariantHash:
            return new NS(QVariantHash);

         case QMetaType::QVariantMultiHash:
            return new NS(QVariantMultiHash);

         case QMetaType::QVariantList:
            return new NS(QVariantList);

         case QMetaType::QVariant:
            return new NS(QVariant);

         case QMetaType::QByteArray:
            return new NS(QByteArray);

         case QMetaType::QString:
            return new NS(QString);

         case QMetaType::QString8:
            return new NS(QString8);

         case QMetaType::QString16:
            return new NS(QString16);

         case QMetaType::QStringList:
            return new NS(QStringList);

         case QMetaType::QBitArray:
            return new NS(QBitArray);

         case QMetaType::QDate:
            return new NS(QDate);

         case QMetaType::QTime:
            return new NS(QTime);
         case QMetaType::QDateTime:
            return new NS(QDateTime);
         case QMetaType::QUrl:
            return new NS(QUrl);
         case QMetaType::QLocale:
            return new NS(QLocale);
         case QMetaType::QRect:
            return new NS(QRect);
         case QMetaType::QRectF:
            return new NS(QRectF);
         case QMetaType::QSize:
            return new NS(QSize);
         case QMetaType::QSizeF:
            return new NS(QSizeF);
         case QMetaType::QLine:
            return new NS(QLine);
         case QMetaType::QLineF:
            return new NS(QLineF);
         case QMetaType::QPoint:
            return new NS(QPoint);
         case QMetaType::QPointF:
            return new NS(QPointF);

#ifndef QT_NO_REGEXP
         case QMetaType::QRegExp:
            return new NS(QRegExp);
#endif

         case QMetaType::QEasingCurve:
            return new NS(QEasingCurve);

         case QMetaType::QUuid:
            return new NS(QUuid);

         case QMetaType::QModelIndex:
            return new NS(QModelIndex);

         case QMetaType::Void:
            return 0;

         default:
            ;
      }
   }

   Constructor constr = 0;

   const QMap<Type, const QMetaTypeGuiHelper *> &temp = QMetaType::dataTypes_Gui();

   if (temp.contains(Type(type)))  {
      // assigns constr to a function pointer
      constr = temp.value(Type(type))->constr;

   } else {
      const QVector<QCustomTypeInfo> *const ct = customTypes();
      QReadLocker locker(customTypesLock());

      if (type < User || ! ct || ct->count() <= type - User)  {
         return 0;
      }

      if (ct->at(type - User).typeName.isEmpty()) {
         return 0;
      }
      constr = ct->at(type - User).constr;
   }

   return constr(copy);
}

void QMetaType::destroy(int type, void *data)
{
   if (! data) {
      return;
   }

   switch (type) {
      case QMetaType::VoidStar:
      case QMetaType::QObjectStar:
      case QMetaType::QWidgetStar:
         delete static_cast<void **>(data);
         break;

      case QMetaType::Long:
         delete static_cast<long *>(data);
         break;

      case QMetaType::Int:
         delete static_cast<int *>(data);
         break;

      case QMetaType::Short:
         delete static_cast<short *>(data);
         break;

      case QMetaType::Char:
         delete static_cast<char *>(data);
         break;

      case QMetaType::ULong:
         delete static_cast<ulong *>(data);
         break;

      case QMetaType::LongLong:
         delete static_cast<qint64 *>(data);
         break;

      case QMetaType::ULongLong:
         delete static_cast<quint64 *>(data);
         break;

      case QMetaType::UInt:
         delete static_cast<uint *>(data);
         break;

      case QMetaType::UShort:
         delete static_cast<ushort *>(data);
         break;

      case QMetaType::UChar:
         delete static_cast<uchar *>(data);
         break;

      case QMetaType::Bool:
         delete static_cast<bool *>(data);
         break;

      case QMetaType::Float:
         delete static_cast<float *>(data);
         break;

      case QMetaType::Double:
         delete static_cast<double *>(data);
         break;

      case QMetaType::QChar:
         delete static_cast< NS(QChar) * >(data);
         break;

      case QMetaType::QChar32:
         delete static_cast< NS(QChar32) * >(data);
         break;

      case QMetaType::QVariantMap:
         delete static_cast< NS(QVariantMap) * >(data);
         break;

      case QMetaType::QVariantMultiMap:
         delete static_cast< NS(QVariantMultiMap) * >(data);
         break;

      case QMetaType::QVariantHash:
         delete static_cast< NS(QVariantHash) * >(data);
         break;

      case QMetaType::QVariantMultiHash:
         delete static_cast< NS(QVariantMultiHash) * >(data);
         break;

      case QMetaType::QVariantList:
         delete static_cast< NS(QVariantList) * >(data);
         break;

      case QMetaType::QVariant:
         delete static_cast< NS(QVariant) * >(data);
         break;
      case QMetaType::QByteArray:
         delete static_cast< NS(QByteArray) * >(data);
         break;

      case QMetaType::QString:
         delete static_cast< NS(QString) * >(data);
         break;

      case QMetaType::QString8:
         delete static_cast<NS(QString8) *>(data);
         break;

      case QMetaType::QString16:
         delete static_cast< NS(QString16) * >(data);
         break;

      case QMetaType::QStringList:
         delete static_cast< NS(QStringList) * >(data);
         break;

      case QMetaType::QBitArray:
         delete static_cast< NS(QBitArray) * >(data);
         break;

      case QMetaType::QDate:
         delete static_cast< NS(QDate) * >(data);
         break;

      case QMetaType::QTime:
         delete static_cast< NS(QTime) * >(data);
         break;

      case QMetaType::QDateTime:
         delete static_cast< NS(QDateTime) * >(data);
         break;

      case QMetaType::QUrl:
         delete static_cast< NS(QUrl) * >(data);
         break;

      case QMetaType::QLocale:
         delete static_cast< NS(QLocale) * >(data);
         break;

      case QMetaType::QRect:
         delete static_cast< NS(QRect) * >(data);
         break;

      case QMetaType::QRectF:
         delete static_cast< NS(QRectF) * >(data);
         break;

      case QMetaType::QSize:
         delete static_cast< NS(QSize) * >(data);
         break;

      case QMetaType::QSizeF:
         delete static_cast< NS(QSizeF) * >(data);
         break;

      case QMetaType::QLine:
         delete static_cast< NS(QLine) * >(data);
         break;

      case QMetaType::QLineF:
         delete static_cast< NS(QLineF) * >(data);
         break;

      case QMetaType::QPoint:
         delete static_cast< NS(QPoint) * >(data);
         break;

      case QMetaType::QPointF:
         delete static_cast< NS(QPointF) * >(data);
         break;

#ifndef QT_NO_REGEXP
      case QMetaType::QRegExp:
         delete static_cast< NS(QRegExp) * >(data);
         break;
#endif

      case QMetaType::QEasingCurve:
         delete static_cast< NS(QEasingCurve) * >(data);
         break;

      case QMetaType::QUuid:
         delete static_cast< NS(QUuid) * >(data);
         break;

      case QMetaType::QModelIndex:
         delete static_cast< NS(QModelIndex) * >(data);
         break;

      case QMetaType::Void:
         break;

      default: {
         const QVector<QCustomTypeInfo> *const ct = customTypes();
         Destructor destr = 0;

         const QMap<Type, const QMetaTypeGuiHelper *> &temp = QMetaType::dataTypes_Gui();

         if (temp.contains(Type(type)))  {
            // assigns destr to a function pointer
            destr = temp.value(Type(type))->destr;

         } else {
            QReadLocker locker(customTypesLock());

            if (type < User || !ct || ct->count() <= type - User) {
               break;
            }

            if (ct->at(type - User).typeName.isEmpty()) {
               break;
            }

            destr = ct->at(type - User).destr;
         }

         destr(data);
         break;
      }
   }
}

bool QMetaType::isRegistered(int type)
{
   if (type >= 0 && type < User) {
      // predefined type
      return true;
   }

   QReadLocker locker(customTypesLock());
   const QVector<QCustomTypeInfo> *const ct = customTypes();
   return ((type >= User) && (ct && ct->count() > type - User) && !ct->at(type - User).typeName.isEmpty());
}

int QMetaType::type(const char *typeName)
{
   int length = qstrlen(typeName);
   if (!length)  {
      return 0;
   }

   int type = qMetaTypeStaticType(typeName, length);

   if (! type) {
      QReadLocker locker(customTypesLock());
      type = qMetaTypeCustomType_unlocked(typeName, length);

      if (! type) {
         const NS(QByteArray) normalizedTypeName = QMetaObject::normalizedType(typeName);
         type = qMetaTypeStaticType(normalizedTypeName.constData(), normalizedTypeName.size());

         if (! type) {
            type = qMetaTypeCustomType_unlocked(normalizedTypeName.constData(), normalizedTypeName.size());
         }
      }

   }
   return type;
}

const char *QMetaType::typeName(int typeId)
{
   if (typeId >= User) {
      const QVector<QCustomTypeInfo> *const ct = customTypes();
      QReadLocker locker(customTypesLock());

      return ct && ct->count() > typeId - User && !ct->at(typeId - User).typeName.isEmpty()
             ? ct->at(typeId - User).typeName.constData() : static_cast<const char *>(0);

   }  else {
      // look up the typeId in the table of static types

      for (int k = 0; types[k].type != QMetaType::UnknownType; ++k )  {

         if (typeId == types[k].type) {
            return types[k].typeName;
         }
      }
   }

   return 0;
}

// **
#ifndef QT_NO_DATASTREAM

bool QMetaType::save(QDataStream &stream, int type, const void *data)
{
   if (! data || !isRegistered(type)) {
      return false;
   }

   switch (type) {
      case QMetaType::Void:
      case QMetaType::VoidStar:
      case QMetaType::QObjectStar:
      case QMetaType::QWidgetStar:
         return false;

      case QMetaType::QJsonValue:
      case QMetaType::QJsonObject:
      case QMetaType::QJsonArray:
      case QMetaType::QJsonDocument:
      case QMetaType::QModelIndex:
         return false;

      case QMetaType::SChar:
         stream << *static_cast<const signed char *>(data);
         break;

      case QMetaType::QUuid:
         stream << *static_cast<const NS(QUuid) *>(data);
         break;

      case QMetaType::Long:
         stream << qint64(*static_cast<const long *>(data));
         break;

      case QMetaType::Int:
         stream << *static_cast<const int *>(data);
         break;

      case QMetaType::Short:
         stream << *static_cast<const short *>(data);
         break;

      case QMetaType::Char:
         // force a char to be signed
         stream << *static_cast<const signed char *>(data);
         break;

      case QMetaType::ULong:
         stream << quint64(*static_cast<const ulong *>(data));
         break;

      case QMetaType::UInt:
         stream << *static_cast<const uint *>(data);
         break;

      case QMetaType::LongLong:
         stream << *static_cast<const qint64 *>(data);
         break;

      case QMetaType::ULongLong:
         stream << *static_cast<const quint64 *>(data);
         break;

      case QMetaType::UShort:
         stream << *static_cast<const ushort *>(data);
         break;

      case QMetaType::UChar:
         stream << *static_cast<const uchar *>(data);
         break;

      case QMetaType::Bool:
         stream << qint8(*static_cast<const bool *>(data));
         break;

      case QMetaType::Float:
         stream << *static_cast<const float *>(data);
         break;

      case QMetaType::Double:
         stream << *static_cast<const double *>(data);
         break;

      case QMetaType::QChar:
         stream << *static_cast<const NS(QChar) *>(data);
         break;

      case QMetaType::QChar32:
         stream << *static_cast<const NS(QChar32) *>(data);
         break;

      case QMetaType::QVariantMap:
         stream << *static_cast<const NS(QVariantMap) *>(data);
         break;

      case QMetaType::QVariantMultiMap:
         stream << *static_cast<const NS(QVariantMultiMap) *>(data);
         break;

      case QMetaType::QVariantHash:
         stream << *static_cast<const NS(QVariantHash) *>(data);
         break;

      case QMetaType::QVariantMultiHash:
         stream << *static_cast<const NS(QVariantMultiHash) *>(data);
         break;

      case QMetaType::QVariantList:
         stream << *static_cast<const NS(QVariantList) *>(data);
         break;

      case QMetaType::QVariant:
         stream << *static_cast<const NS(QVariant) *>(data);
         break;

      case QMetaType::QByteArray:
         stream << *static_cast<const NS(QByteArray) *>(data);
         break;

      case QMetaType::QString:
         stream << *static_cast<const NS(QString) *>(data);
         break;

      case QMetaType::QString8:
         stream << *static_cast<const NS(QString8) *>(data);
         break;

      case QMetaType::QString16:
         stream << *static_cast<const NS(QString16) *>(data);
         break;

      case QMetaType::QStringList:
         stream << *static_cast<const NS(QStringList) *>(data);
         break;

      case QMetaType::QBitArray:
         stream << *static_cast<const NS(QBitArray) *>(data);
         break;

      case QMetaType::QDate:
         stream << *static_cast<const NS(QDate) *>(data);
         break;

      case QMetaType::QTime:
         stream << *static_cast<const NS(QTime) *>(data);
         break;

      case QMetaType::QDateTime:
         stream << *static_cast<const NS(QDateTime) *>(data);
         break;

      case QMetaType::QUrl:
         stream << *static_cast<const NS(QUrl) *>(data);
         break;

      case QMetaType::QLocale:
         stream << *static_cast<const NS(QLocale) *>(data);
         break;

      case QMetaType::QRect:
         stream << *static_cast<const NS(QRect) *>(data);
         break;

      case QMetaType::QRectF:
         stream << *static_cast<const NS(QRectF) *>(data);
         break;

      case QMetaType::QSize:
         stream << *static_cast<const NS(QSize) *>(data);
         break;

      case QMetaType::QSizeF:
         stream << *static_cast<const NS(QSizeF) *>(data);
         break;

      case QMetaType::QLine:
         stream << *static_cast<const NS(QLine) *>(data);
         break;

      case QMetaType::QLineF:
         stream << *static_cast<const NS(QLineF) *>(data);
         break;

      case QMetaType::QPoint:
         stream << *static_cast<const NS(QPoint) *>(data);
         break;

      case QMetaType::QPointF:
         stream << *static_cast<const NS(QPointF) *>(data);
         break;

#ifndef QT_NO_REGEXP
      case QMetaType::QRegExp:
         stream << *static_cast<const NS(QRegExp) *>(data);
         break;
#endif

      case QMetaType::QEasingCurve:
         stream << *static_cast<const NS(QEasingCurve) *>(data);
         break;

      case QMetaType::QFont:
      case QMetaType::QPixmap:
      case QMetaType::QBrush:
      case QMetaType::QColor:
      case QMetaType::QPalette:
      case QMetaType::QIcon:
      case QMetaType::QImage:
      case QMetaType::QPolygon:
      case QMetaType::QPolygonF:
      case QMetaType::QRegion:
      case QMetaType::QBitmap:
      case QMetaType::QCursor:
      case QMetaType::QSizePolicy:
      case QMetaType::QKeySequence:
      case QMetaType::QPen:
      case QMetaType::QTextLength:
      case QMetaType::QTextFormat:
      case QMetaType::QMatrix:
      case QMetaType::QTransform:
      case QMetaType::QMatrix4x4:
      case QMetaType::QVector2D:
      case QMetaType::QVector3D:
      case QMetaType::QVector4D:
      case QMetaType::QQuaternion: {
         const QMap<Type, const QMetaTypeGuiHelper *> &temp = QMetaType::dataTypes_Gui();

         if (temp.contains(Type(type)))  {
            temp.value(Type(type))->saveOp(stream, data);
         }

         break;
      }

      default: {
         const QVector<QCustomTypeInfo> *const ct = customTypes();
         if (! ct) {
            return false;
         }

         SaveOperator saveOp = 0;
         {
            QReadLocker locker(customTypesLock());
            saveOp = ct->at(type - User).saveOp;
         }

         if (! saveOp) {
            return false;
         }

         saveOp(stream, data);
         break;
      }
   }

   return true;
}

bool QMetaType::load(QDataStream &stream, int type, void *data)
{
   if (! data || !isRegistered(type))  {
      return false;
   }

   switch (type) {
      case QMetaType::Void:
      case QMetaType::VoidStar:
      case QMetaType::QObjectStar:
      case QMetaType::QWidgetStar:
         return false;

      case QMetaType::Long: {
         qint64 l;
         stream >> l;
         *static_cast<long *>(data) = long(l);
         break;
      }

      case QMetaType::Int:
         stream >> *static_cast<int *>(data);
         break;

      case QMetaType::Short:
         stream >> *static_cast<short *>(data);
         break;

      case QMetaType::Char:
         // force a char to be signed
         stream >> *static_cast<signed char *>(data);
         break;

      case QMetaType::ULong: {
         quint64 ul;
         stream >> ul;
         *static_cast<ulong *>(data) = ulong(ul);
         break;
      }

      case QMetaType::UInt:
         stream >> *static_cast<uint *>(data);
         break;

      case QMetaType::LongLong:
         stream >> *static_cast<qint64 *>(data);
         break;

      case QMetaType::ULongLong:
         stream >> *static_cast<quint64 *>(data);
         break;

      case QMetaType::UShort:
         stream >> *static_cast<ushort *>(data);
         break;

      case QMetaType::UChar:
         stream >> *static_cast<uchar *>(data);
         break;

      case QMetaType::Bool: {
         qint8 b;
         stream >> b;
         *static_cast<bool *>(data) = b;
         break;
      }
      case QMetaType::Float:
         stream >> *static_cast<float *>(data);
         break;

      case QMetaType::Double:
         stream >> *static_cast<double *>(data);
         break;

      case QMetaType::QChar:
         stream >> *static_cast< NS(QChar) *>(data);
         break;

      case QMetaType::QChar32:
         stream >> *static_cast< NS(QChar32) *>(data);
         break;

      case QMetaType::QVariantMap:
         stream >> *static_cast< NS(QVariantMap) *>(data);
         break;

      case QMetaType::QVariantMultiMap:
         stream >> *static_cast< NS(QVariantMultiMap) *>(data);
         break;

      case QMetaType::QVariantHash:
         stream >> *static_cast< NS(QVariantHash) *>(data);
         break;

      case QMetaType::QVariantMultiHash:
         stream >> *static_cast< NS(QVariantMultiHash) *>(data);
         break;

      case QMetaType::QVariantList:
         stream >> *static_cast< NS(QVariantList) *>(data);
         break;
      case QMetaType::QVariant:
         stream >> *static_cast< NS(QVariant) *>(data);
         break;
      case QMetaType::QByteArray:
         stream >> *static_cast< NS(QByteArray) *>(data);
         break;

      case QMetaType::QString:
         stream >> *static_cast< NS(QString) *>(data);
         break;

      case QMetaType::QString8:
         stream >> *static_cast< NS(QString8) *>(data);
         break;

      case QMetaType::QString16:
         stream >> *static_cast< NS(QString16) *>(data);
         break;

      case QMetaType::QStringList:
         stream >> *static_cast< NS(QStringList) *>(data);
         break;
      case QMetaType::QBitArray:
         stream >> *static_cast< NS(QBitArray) *>(data);
         break;
      case QMetaType::QDate:
         stream >> *static_cast< NS(QDate) *>(data);
         break;
      case QMetaType::QTime:
         stream >> *static_cast< NS(QTime) *>(data);
         break;
      case QMetaType::QDateTime:
         stream >> *static_cast< NS(QDateTime) *>(data);
         break;
      case QMetaType::QUrl:
         stream >> *static_cast< NS(QUrl) *>(data);
         break;
      case QMetaType::QLocale:
         stream >> *static_cast< NS(QLocale) *>(data);
         break;
      case QMetaType::QRect:
         stream >> *static_cast< NS(QRect) *>(data);
         break;
      case QMetaType::QRectF:
         stream >> *static_cast< NS(QRectF) *>(data);
         break;
      case QMetaType::QSize:
         stream >> *static_cast< NS(QSize) *>(data);
         break;
      case QMetaType::QSizeF:
         stream >> *static_cast< NS(QSizeF) *>(data);
         break;
      case QMetaType::QLine:
         stream >> *static_cast< NS(QLine) *>(data);
         break;
      case QMetaType::QLineF:
         stream >> *static_cast< NS(QLineF) *>(data);
         break;
      case QMetaType::QPoint:
         stream >> *static_cast< NS(QPoint) *>(data);
         break;
      case QMetaType::QPointF:
         stream >> *static_cast< NS(QPointF) *>(data);
         break;

#ifndef QT_NO_REGEXP
      case QMetaType::QRegExp:
         stream >> *static_cast< NS(QRegExp) *>(data);
         break;
#endif

      case QMetaType::QEasingCurve:
         stream >> *static_cast< NS(QEasingCurve) *>(data);
         break;

      case QMetaType::QFont:
      case QMetaType::QPixmap:
      case QMetaType::QBrush:
      case QMetaType::QColor:
      case QMetaType::QPalette:
      case QMetaType::QIcon:
      case QMetaType::QImage:
      case QMetaType::QPolygon:
      case QMetaType::QPolygonF:
      case QMetaType::QRegion:
      case QMetaType::QBitmap:
      case QMetaType::QCursor:
      case QMetaType::QSizePolicy:
      case QMetaType::QKeySequence:
      case QMetaType::QPen:
      case QMetaType::QTextLength:
      case QMetaType::QTextFormat:
      case QMetaType::QMatrix:
      case QMetaType::QTransform:
      case QMetaType::QMatrix4x4:
      case QMetaType::QVector2D:
      case QMetaType::QVector3D:
      case QMetaType::QVector4D:
      case QMetaType::QQuaternion: {
         const QMap<Type, const QMetaTypeGuiHelper *> &temp = QMetaType::dataTypes_Gui();

         if (temp.contains(Type(type)))  {
            temp.value(Type(type))->loadOp(stream, data);
         }

         break;
      }

      default: {
         const QVector<QCustomTypeInfo> *const ct = customTypes();
         if (!ct) {
            return false;
         }

         LoadOperator loadOp = 0;
         {
            QReadLocker locker(customTypesLock());
            loadOp = ct->at(type - User).loadOp;
         }

         if (!loadOp) {
            return false;
         }
         loadOp(stream, data);
         break;
      }

   }

   return true;
}

#endif

QT_END_NAMESPACE
