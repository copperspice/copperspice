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

#ifndef QMETATYPE_H
#define QMETATYPE_H

#include <qglobal.h>
#include <qatomic.h>
#include <qcontainerfwd.h>

#ifdef Bool
#error qmetatype.h must be included before any header file that defines Bool
#endif

QT_BEGIN_NAMESPACE

class QDataStream;
struct QMetaTypeGuiHelper;

class Q_CORE_EXPORT QMetaType
{

 public:
   enum Type {
      UnknownType = 0,

      Bool = 1, Int = 2, UInt = 3, LongLong = 4, ULongLong = 5, Double = 6,

      QChar = 7, QString = 8, QStringList = 9, QByteArray = 10, QBitArray = 11,
      QChar32 = 12, QString8 = 13, QString16 = 14,
      QVariantList = 15, QVariantHash = 16, QVariantMultiHash = 17, QVariantMap = 18, QVariantMultiMap = 19,

      QDate = 20, QTime = 21, QDateTime = 22, QUrl = 23,
      QLocale = 24, QRect = 25, QRectF = 26, QSize = 27, QSizeF = 28,
      QLine = 29, QLineF = 30, QPoint = 31, QPointF = 32, QRegExp = 33,
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

      // must match both files:  qmetatype.h & qglobal.h
#if defined(QT_COORD_TYPE)
      QReal = 0,
#elif defined(QT_NO_FPU) || defined(QT_ARCH_ARM)
      QReal = Float,
#else
      QReal = Double,
#endif

      User = 256
   };

   typedef void (*Destructor)(void *);
   typedef void *(*Constructor)(const void *);

#ifndef QT_NO_DATASTREAM
   typedef void (*SaveOperator)(QDataStream &, const void *);
   typedef void (*LoadOperator)(QDataStream &, void *);
   static void registerStreamOperators(const char *typeName, SaveOperator saveOp, LoadOperator loadOp);
   static void registerStreamOperators(int type, SaveOperator saveOp, LoadOperator loadOp);
#endif

   static int registerType(const char *typeName, Destructor destructor, Constructor constructor);
   static int registerTypedef(const char *typeName, int aliasId);
   static int type(const char *typeName);
   static const char *typeName(int type);
   static bool isRegistered(int type);
   static void *construct(int type, const void *copy = 0);
   static void destroy(int type, void *data);

#ifndef QT_NO_DATASTREAM
   static bool save(QDataStream &stream, int type, const void *data);
   static bool load(QDataStream &stream, int type, void *data);
#endif

 private:
   // global array containing the Gui data types
   static const QMap<Type, const QMetaTypeGuiHelper *> &dataTypes_Gui();

};

struct QMetaTypeGuiHelper {
   QMetaType::Type typeId;
   QMetaType::Constructor constr;
   QMetaType::Destructor destr;

#ifndef QT_NO_DATASTREAM
   QMetaType::SaveOperator saveOp;
   QMetaType::LoadOperator loadOp;
#endif

};

template <typename T>
void qMetaTypeDeleteHelper(T *v)
{
   delete v;
}

template <typename T>
void *qMetaTypeConstructHelper(const T *v)
{
   if (! v) {
      return new T();
   }

   return new T(*static_cast<const T *>(v));
}


#ifndef QT_NO_DATASTREAM
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
#endif

template <typename T>
struct QMetaTypeId {
   enum { Defined = 0 };
};

template <typename T>
struct QMetaTypeId2 {
   enum { Defined = QMetaTypeId<T>::Defined };
   static inline int qt_metatype_id() {
      return QMetaTypeId<T>::qt_metatype_id();
   }
};

namespace QtPrivate {

template <typename T, bool Defined = QMetaTypeId2<T>::Defined>
struct QMetaTypeIdHelper {
   static inline int qt_metatype_id() {
      return QMetaTypeId2<T>::qt_metatype_id();
   }
};

template <typename T>
struct QMetaTypeIdHelper<T, false> {
   static inline int qt_metatype_id() {
      return -1;
   }
};

}

template <typename T>
int qRegisterMetaType(const char *typeName, T *dummy = nullptr)
{
   if (! dummy) {
      int typedefOf = QtPrivate::QMetaTypeIdHelper<T>::qt_metatype_id();
      return QMetaType::registerTypedef(typeName, typedefOf);
   }

   typedef void *(*ConstructPtr)(const T *);
   ConstructPtr cptr = qMetaTypeConstructHelper<T>;

   typedef void(*DeletePtr)(T *);
   DeletePtr dptr = qMetaTypeDeleteHelper<T>;

   return QMetaType::registerType(typeName, reinterpret_cast<QMetaType::Destructor>(dptr),
                                  reinterpret_cast<QMetaType::Constructor>(cptr));
}

template <typename T>
inline int qMetaTypeId(T * = nullptr)
{
   return QMetaTypeId2<T>::qt_metatype_id();
}

template <typename T>
inline int qRegisterMetaType(T *v = nullptr)
{
   return qMetaTypeId(v);
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

#ifndef QT_NO_DATASTREAM

template <typename T>
void qRegisterMetaTypeStreamOperators(const char *typeName, T * = 0 )
{
   typedef void(*SavePtr)(QDataStream &, const T *);
   typedef void(*LoadPtr)(QDataStream &, T *);
   SavePtr sptr = qMetaTypeSaveHelper<T>;
   LoadPtr lptr = qMetaTypeLoadHelper<T>;

   qRegisterMetaType<T>(typeName);
   QMetaType::registerStreamOperators(typeName, reinterpret_cast<QMetaType::SaveOperator>(sptr),
                                      reinterpret_cast<QMetaType::LoadOperator>(lptr));
}

template <typename T>
inline int qRegisterMetaTypeStreamOperators()
{
   typedef void(*SavePtr)(QDataStream &, const T *);
   typedef void(*LoadPtr)(QDataStream &, T *);
   SavePtr sptr = qMetaTypeSaveHelper<T>;
   LoadPtr lptr = qMetaTypeLoadHelper<T>;

   int id = qMetaTypeId<T>();
   QMetaType::registerStreamOperators(id, reinterpret_cast<QMetaType::SaveOperator>(sptr), reinterpret_cast<QMetaType::LoadOperator>(lptr));

   return id;
}
#endif

#define Q_DECLARE_METATYPE(TYPE)                                      \
   QT_BEGIN_NAMESPACE                                                 \
   template <>                                                        \
   struct QMetaTypeId<TYPE>                                           \
   {                                                                  \
       enum { Defined = 1 };                                          \
       static int qt_metatype_id()                                    \
       {                                                              \
          static QAtomicInt metatype_id = QAtomicInt{ 0 };            \
          if (! metatype_id.load()) {                                 \
             metatype_id.storeRelease(qRegisterMetaType< TYPE >(#TYPE,reinterpret_cast< TYPE *>(quintptr(-1)))); \
          }                                                           \
          return metatype_id.loadAcquire();                           \
       }                                                              \
   };                                                                 \
   QT_END_NAMESPACE

#define Q_DECLARE_BUILTIN_METATYPE(TYPE, NAME)                        \
   QT_BEGIN_NAMESPACE                                                 \
   template<> struct QMetaTypeId2<TYPE>                               \
   {                                                                  \
      enum { Defined = 1, MetaType = QMetaType::NAME };               \
      static inline int qt_metatype_id() { return QMetaType::NAME; }  \
   };                                                                 \
   QT_END_NAMESPACE


class QChar;
class QString;
class QByteArray;
class QStringList;
class QBitArray;

class QChar32;
class QString8;
class QString16;

class QDate;
class QTime;
class QDateTime;
class QUrl;
class QLocale;
class QRect;
class QRectF;
class QSize;
class QSizeF;
class QLine;
class QLineF;
class QPoint;
class QPointF;

class QUuid;
class QModelIndex;
class QJsonValue;
class QJsonObject;
class QJsonArray;
class QJsonDocument;

#ifndef QT_NO_REGEXP
class QRegExp;
#endif

class QEasingCurve;
class QWidget;
class QObject;
class QFont;
class QPixmap;
class QBrush;
class QColor;
class QPalette;
class QIcon;
class QImage;
class QPolygon;
class QPolygonF;
class QRegion;
class QBitmap;
class QCursor;
class QSizePolicy;
class QKeySequence;
class QPen;
class QTextLength;
class QTextFormat;
class QMatrix;
class QTransform;
class QMatrix4x4;
class QVector2D;
class QVector3D;
class QVector4D;
class QQuaternion;
class QVariant;

QT_END_NAMESPACE

Q_DECLARE_BUILTIN_METATYPE(int, Int)
Q_DECLARE_BUILTIN_METATYPE(uint, UInt)
Q_DECLARE_BUILTIN_METATYPE(bool, Bool)
Q_DECLARE_BUILTIN_METATYPE(double, Double)
Q_DECLARE_BUILTIN_METATYPE(long, Long)
Q_DECLARE_BUILTIN_METATYPE(short, Short)
Q_DECLARE_BUILTIN_METATYPE(char, Char)
Q_DECLARE_BUILTIN_METATYPE(ulong, ULong)
Q_DECLARE_BUILTIN_METATYPE(ushort, UShort)
Q_DECLARE_BUILTIN_METATYPE(uchar, UChar)
Q_DECLARE_BUILTIN_METATYPE(float, Float)
Q_DECLARE_BUILTIN_METATYPE(qint64, LongLong)
Q_DECLARE_BUILTIN_METATYPE(quint64, ULongLong)

Q_DECLARE_BUILTIN_METATYPE(QChar, QChar)
Q_DECLARE_BUILTIN_METATYPE(QString, QString)
Q_DECLARE_BUILTIN_METATYPE(QByteArray, QByteArray)
Q_DECLARE_BUILTIN_METATYPE(QStringList, QStringList)
Q_DECLARE_BUILTIN_METATYPE(QBitArray, QBitArray)

Q_DECLARE_BUILTIN_METATYPE(QChar32, QChar32)
Q_DECLARE_BUILTIN_METATYPE(QString8, QString8)
Q_DECLARE_BUILTIN_METATYPE(QString16, QString16)

Q_DECLARE_BUILTIN_METATYPE(QObject *, QObjectStar)
Q_DECLARE_BUILTIN_METATYPE(QWidget *, QWidgetStar)
Q_DECLARE_BUILTIN_METATYPE(void *, VoidStar)

Q_DECLARE_BUILTIN_METATYPE(QDate, QDate)
Q_DECLARE_BUILTIN_METATYPE(QTime, QTime)
Q_DECLARE_BUILTIN_METATYPE(QDateTime, QDateTime)
Q_DECLARE_BUILTIN_METATYPE(QUrl, QUrl)
Q_DECLARE_BUILTIN_METATYPE(QLocale, QLocale)
Q_DECLARE_BUILTIN_METATYPE(QRect, QRect)
Q_DECLARE_BUILTIN_METATYPE(QRectF, QRectF)
Q_DECLARE_BUILTIN_METATYPE(QSize, QSize)
Q_DECLARE_BUILTIN_METATYPE(QSizeF, QSizeF)
Q_DECLARE_BUILTIN_METATYPE(QLine, QLine)
Q_DECLARE_BUILTIN_METATYPE(QLineF, QLineF)
Q_DECLARE_BUILTIN_METATYPE(QPoint, QPoint)
Q_DECLARE_BUILTIN_METATYPE(QPointF, QPointF)

Q_DECLARE_BUILTIN_METATYPE(signed char, SChar);
Q_DECLARE_BUILTIN_METATYPE(QUuid, QUuid);
Q_DECLARE_BUILTIN_METATYPE(QModelIndex, QModelIndex);

Q_DECLARE_BUILTIN_METATYPE(QJsonValue, QJsonValue);
Q_DECLARE_BUILTIN_METATYPE(QJsonObject, QJsonObject);
Q_DECLARE_BUILTIN_METATYPE(QJsonArray, QJsonArray);
Q_DECLARE_BUILTIN_METATYPE(QJsonDocument, QJsonDocument);

#ifndef QT_NO_REGEXP
Q_DECLARE_BUILTIN_METATYPE(QRegExp, QRegExp)
#endif

Q_DECLARE_BUILTIN_METATYPE(QEasingCurve, QEasingCurve)
Q_DECLARE_BUILTIN_METATYPE(QFont, QFont)
Q_DECLARE_BUILTIN_METATYPE(QPixmap, QPixmap)
Q_DECLARE_BUILTIN_METATYPE(QBrush, QBrush)
Q_DECLARE_BUILTIN_METATYPE(QColor, QColor)
Q_DECLARE_BUILTIN_METATYPE(QPalette, QPalette)
Q_DECLARE_BUILTIN_METATYPE(QIcon, QIcon)
Q_DECLARE_BUILTIN_METATYPE(QImage, QImage)
Q_DECLARE_BUILTIN_METATYPE(QPolygon, QPolygon)
Q_DECLARE_BUILTIN_METATYPE(QPolygonF, QPolygonF);
Q_DECLARE_BUILTIN_METATYPE(QRegion, QRegion)
Q_DECLARE_BUILTIN_METATYPE(QBitmap, QBitmap)
Q_DECLARE_BUILTIN_METATYPE(QCursor, QCursor)
Q_DECLARE_BUILTIN_METATYPE(QSizePolicy, QSizePolicy)
Q_DECLARE_BUILTIN_METATYPE(QKeySequence, QKeySequence)
Q_DECLARE_BUILTIN_METATYPE(QPen, QPen)
Q_DECLARE_BUILTIN_METATYPE(QTextLength, QTextLength)
Q_DECLARE_BUILTIN_METATYPE(QTextFormat, QTextFormat)
Q_DECLARE_BUILTIN_METATYPE(QMatrix, QMatrix)
Q_DECLARE_BUILTIN_METATYPE(QTransform, QTransform)
Q_DECLARE_BUILTIN_METATYPE(QMatrix4x4, QMatrix4x4)
Q_DECLARE_BUILTIN_METATYPE(QVector2D, QVector2D)
Q_DECLARE_BUILTIN_METATYPE(QVector3D, QVector3D)
Q_DECLARE_BUILTIN_METATYPE(QVector4D, QVector4D)
Q_DECLARE_BUILTIN_METATYPE(QQuaternion, QQuaternion)
Q_DECLARE_BUILTIN_METATYPE(QVariant, QVariant)

#endif
