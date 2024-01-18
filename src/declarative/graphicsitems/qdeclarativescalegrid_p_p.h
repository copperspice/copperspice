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

#ifndef QDECLARATIVESCALEGRID_P_P_H
#define QDECLARATIVESCALEGRID_P_P_H

#include <qdeclarative.h>
#include <QtCore/QString>
#include <QtCore/QObject>
#include <qdeclarativeborderimage_p.h>
#include <qdeclarativepixmapcache_p.h>
#include <qdeclarativeglobal_p.h>

QT_BEGIN_NAMESPACE

class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeScaleGrid : public QObject
{
   DECL_CS_OBJECT(QDeclarativeScaleGrid)
   CS_ENUM(TileRule)

   DECL_CS_PROPERTY_READ(left, left)
   DECL_CS_PROPERTY_WRITE(left, setLeft)
   DECL_CS_PROPERTY_NOTIFY(left, borderChanged)
   DECL_CS_PROPERTY_READ(top, top)
   DECL_CS_PROPERTY_WRITE(top, setTop)
   DECL_CS_PROPERTY_NOTIFY(top, borderChanged)
   DECL_CS_PROPERTY_READ(right, right)
   DECL_CS_PROPERTY_WRITE(right, setRight)
   DECL_CS_PROPERTY_NOTIFY(right, borderChanged)
   DECL_CS_PROPERTY_READ(bottom, bottom)
   DECL_CS_PROPERTY_WRITE(bottom, setBottom)
   DECL_CS_PROPERTY_NOTIFY(bottom, borderChanged)

 public:
   QDeclarativeScaleGrid(QObject *parent = nullptr);
   ~QDeclarativeScaleGrid();

   bool isNull() const;

   int left() const {
      return _left;
   }
   void setLeft(int);

   int top() const {
      return _top;
   }
   void setTop(int);

   int right() const {
      return _right;
   }
   void setRight(int);

   int  bottom() const {
      return _bottom;
   }
   void setBottom(int);

 public:
   DECL_CS_SIGNAL_1(Public, void borderChanged())
   DECL_CS_SIGNAL_2(borderChanged)

 private:
   int _left;
   int _top;
   int _right;
   int _bottom;
};

class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeGridScaledImage
{
 public:
   QDeclarativeGridScaledImage();
   QDeclarativeGridScaledImage(const QDeclarativeGridScaledImage &);
   QDeclarativeGridScaledImage(QIODevice *);
   QDeclarativeGridScaledImage &operator=(const QDeclarativeGridScaledImage &);
   bool isValid() const;
   int gridLeft() const;
   int gridRight() const;
   int gridTop() const;
   int gridBottom() const;
   QDeclarativeBorderImage::TileMode horizontalTileRule() const {
      return _h;
   }
   QDeclarativeBorderImage::TileMode verticalTileRule() const {
      return _v;
   }

   QString pixmapUrl() const;

 private:
   static QDeclarativeBorderImage::TileMode stringToRule(const QString &);

   int _l;
   int _r;
   int _t;
   int _b;
   QDeclarativeBorderImage::TileMode _h;
   QDeclarativeBorderImage::TileMode _v;
   QString _pix;
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeScaleGrid)

#endif // QDECLARATIVESCALEGRID_H
