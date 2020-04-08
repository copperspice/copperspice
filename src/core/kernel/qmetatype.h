/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#ifndef QMETATYPE_H
#define QMETATYPE_H

#include <qglobal.h>
#include <qatomic.h>
#include <qcontainerfwd.h>
#include <qstringfwd.h>

// can not include qstring.h since it includes qstringparser.h, which then includes qlocale.h (circular dependency)
#include <qstring8.h>

class QDataStream;
struct QMetaTypeGuiHelper;

class Q_CORE_EXPORT QMetaType
{

 public:
   enum Type {
      UnknownType = 0,

      Bool = 1, Int = 2, UInt = 3, LongLong = 4, ULongLong = 5, Double = 6,

      QByteArray = 7, QBitArray = 8, QChar = 9, QString = 10, QString16 = 11,
      QRegularExpression = 12,  QStringView = 13,

      QChar32  = QChar,
      QString8 = QString,

      QStringList  = 14,
      QVariantList = 15, QVariantHash = 16, QVariantMultiHash = 17, QVariantMap = 18, QVariantMultiMap = 19,

      QDate = 20, QTime = 21,  QDateTime = 22, QUrl = 23, QLocale = 24,
      QRect = 25, QRectF = 26, QSize = 27,  QSizeF = 28,
      QLine = 29, QLineF = 30, QPoint = 31, QPointF = 32, QMargins = 33,

      QEasingCurve = 34, QUuid = 35, QModelIndex = 36,

      // not part of QVariant::Type
      Void = 37, SChar = 38,
      QJsonValue = 39, QJsonObject = 40, QJsonArray = 41, QJsonDocument = 42,

      //
      QFont = 64, QPixmap = 65, QBrush = 66, QColor = 67, QPalette = 68,
      QIcon = 69, QImage = 70, QPolygon = 71, QRegion = 72, QBitmap = 73,
      QCursor = 74, QSizePolicy = 75, QKeySequence = 76, QPen = 77,
      QTextLength = 78, QTextFormat = 79, QMatrix = 80, QTransform = 81,
      QMatrix4x4 = 82, QVector2D = 83, QVector3D = 84, QVector4D = 85,
      QQuaternion = 86, QPolygonF = 87,

      // not part of QVariant::Type
      VoidStar = 128, Long = 129, Short = 130, Char = 131, ULong = 132,
      UShort = 133, UChar = 134, Float = 135, QObjectStar = 136,
      QWidgetStar = 137, QVariant = 138,

      QReal = Double,

      User = 256
   };

   typedef void (*Destructor)(void *);
   typedef void *(*Constructor)(const void *);

   using SaveOperator = void (*)(QDataStream &, const void *);
   using LoadOperator = void (*)(QDataStream &, void *);

#if ! defined (CS_DOXYPRESS)
// required so QString is used as a data type and not an enum value
#define QString ::QString
#endif

   static void registerStreamOperators(const QString &typeName, SaveOperator saveOp, LoadOperator loadOp);
   static void registerStreamOperators(int type, SaveOperator saveOp, LoadOperator loadOp);

   static int type(const QString &typeName);
   static const QString &typeName(int type);
   static bool isRegistered(int type);
   static void *construct(int type, const void *original = nullptr);
   static void destroy(int type, void *data);

#if ! defined (CS_DOXYPRESS)
#undef QString
#endif

   static bool save(QDataStream &stream, int type, const void *data);
   static bool load(QDataStream &stream, int type, void *data);

 private:
   // global array containing the Gui data types
   static const QMap<Type, const QMetaTypeGuiHelper *> &dataTypes_Gui();

};

struct QMetaTypeGuiHelper {
   QMetaType::Type typeId;
   QMetaType::SaveOperator saveOp;
   QMetaType::LoadOperator loadOp;

};

template <typename T>
void qMetaTypeSaveHelper(QDataStream &stream, const T *v)
{
   stream << *v;
}

template <typename T>
void qMetaTypeLoadHelper(QDataStream &stream, T *v)
{
   stream >> *v;
}

template <typename T>
struct QMetaTypeId2 {
   enum { Defined = 0 };
   static inline int qt_metatype_id() {
      return QMetaType::UnknownType;
   }
};

template <typename T>
inline int qMetaTypeId(T * = nullptr)
{
   return QMetaType::UnknownType;
}



template <typename T, bool = QMetaTypeId2<T>::Defined>
struct csTypeQuery {
   static int query() {
      return 0;
   }
};

template <typename T>
struct csTypeQuery<T, true> {
   static int query() {
      return QMetaTypeId2<T>::qt_metatype_id();
   }
};

template <typename T>
inline int qMetaTypeId_Query()
{
   return csTypeQuery<T>::query();
}

template <typename T>
void qRegisterMetaTypeStreamOperators(const QString &typeName, T * = nullptr )
{
   using SavePtr = void (*)(QDataStream &, const T *);
   using LoadPtr = void (*)(QDataStream &, T *);

   SavePtr sptr = qMetaTypeSaveHelper<T>;
   LoadPtr lptr = qMetaTypeLoadHelper<T>;

   QMetaType::registerStreamOperators(typeName, reinterpret_cast<QMetaType::SaveOperator>(sptr),
                  reinterpret_cast<QMetaType::LoadOperator>(lptr));
}

template <typename T>
inline int qRegisterMetaTypeStreamOperators()
{
   using SavePtr = void (*)(QDataStream &, const T *);
   using LoadPtr = void (*)(QDataStream &, T *);

   SavePtr sptr = qMetaTypeSaveHelper<T>;
   LoadPtr lptr = qMetaTypeLoadHelper<T>;

   int id = qMetaTypeId<T>();
   QMetaType::registerStreamOperators(id, reinterpret_cast<QMetaType::SaveOperator>(sptr),
                  reinterpret_cast<QMetaType::LoadOperator>(lptr));

   return id;
}




#endif
