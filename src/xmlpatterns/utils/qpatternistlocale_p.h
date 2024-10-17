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

#ifndef QPatternistLocale_P_H
#define QPatternistLocale_P_H

#include <qcoreapplication.h>
#include <qstringfwd.h>
#include <qurl.h>

#include <qcardinality_p.h>
#include <qnamepool_p.h>
#include <qprimitives_p.h>

namespace QPatternist {
class QtXmlPatterns
{
 public:
   Q_DECLARE_TR_FUNCTIONS(QtXmlPatterns)

 private:
   inline QtXmlPatterns();

   QtXmlPatterns(const QtXmlPatterns &) = delete;
   QtXmlPatterns &operator=(const QtXmlPatterns &) = delete;
};

// don't make this function static, otherwise xlC 7 ca nnot find it
inline QString formatKeyword(const QString &keyword)
{
   return QLatin1String("<span class='XQuery-keyword'>")   +
          escape(keyword)                                  +
          QLatin1String("</span>");
}

static inline QString formatKeyword(QStringView keyword)
{
   return formatKeyword(QString(keyword));
}

static inline QString formatKeyword(const char *const keyword)
{
   return formatKeyword(QString::fromLatin1(keyword));
}

static inline QString formatKeyword(const QChar keyword)
{
   return formatKeyword(QString(keyword));
}

static inline QString formatElement(const QString &element)
{
   // for the moment we forward to formatKeyword, that will change later
   return formatKeyword(element);
}

static inline QString formatElement(const char *const element)
{
   return formatElement(QString::fromLatin1(element));
}

static inline QString formatAttribute(const QString &attribute)
{
   // for the moment we forward to formatKeyword, that will change later
   return formatKeyword(attribute);
}

static inline QString formatAttribute(const char *const attribute)
{
   return formatAttribute(QString::fromLatin1(attribute));
}

template<typename T>
inline QString formatType(const NamePool::Ptr &np, const T &type)
{
   Q_ASSERT(type);
   return QLatin1String("<span class='XQuery-type'>")  +
          escape(type->displayName(np))                +
          QLatin1String("</span>");
}

static inline QString formatType(const NamePool::Ptr &np, const QXmlName &name)
{
   return QLatin1String("<span class='XQuery-type'>")  +
          escape(np->displayName(name))                +
          QLatin1String("</span>");
}

static inline QString formatType(const Cardinality &type)
{
   return QLatin1String("<span class='XQuery-type'>")                      +
          escape(type.displayName(Cardinality::IncludeExplanation))        +
          QLatin1String("</span>");
}

static inline QString formatResourcePath(const QUrl &uri)
{
   const QString normalizedURI(escape(uri.toString(QUrl::RemovePassword)));

   return QLatin1String("<span class='XQuery-filepath'><a href='") +
          normalizedURI                                            +
          QLatin1String("'>")                                      +
          normalizedURI                                            +
          QLatin1String("</a></span>");
}

static inline QString formatURI(const QUrl &uri)
{
   return QLatin1String("<span class='XQuery-uri'>")       +
          escape(uri.toString(QUrl::RemovePassword))       +
          QLatin1String("</span>");
}

static inline QString formatURI(const QString &uri)
{
   const QUrl realURI(uri);
   return formatURI(realURI);
}

static inline QString formatData(const QString &data)
{
   return QLatin1String("<span class='XQuery-data'>")  +
          escape(data)                                 +
          QLatin1String("</span>");
}

static inline QString formatData(const xsInteger data)
{
   return formatData(QString::number(data));
}

static inline QString formatData(const char *const data)
{
   return formatData(QString::fromLatin1(data));
}

static inline QString formatExpression(const QString &expr)
{
   return "<span class='XQuery-expression'>" + escape(expr) + "</span>";
}

}

#endif
