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

#ifndef QDEBUG_P_H
#define QDEBUG_P_H

#include <qdebug.h>

#include <qflags.h>
#include <qmetaobject.h>

namespace QtDebugUtils {

// inline helpers for formatting basic classes

template <class Point>
static inline void formatQPoint(QDebug &debug, const Point &point)
{
   debug << point.x() << ',' << point.y();
}

template <class Size>
static inline void formatQSize(QDebug &debug, const Size &size)
{
   debug << size.width() << ", " << size.height();
}

template <class Rect>
static inline void formatQRect(QDebug &debug, const Rect &rect)
{
   debug << rect.x() << ',' << rect.y() << ' ' << rect.width() << 'x' << rect.height();
}

template <class Margins>
static inline void formatQMargins(QDebug &debug, const Margins &margins)
{
   debug << margins.left() << ", " << margins.top() << ", " << margins.right()
         << ", " << margins.bottom();
}

template <class QEnum>
static inline void formatQEnum(QDebug &debug, QEnum value)
{
   /* emerald - enum meta object lookup

   const QMetaObject *metaObject = qt_getEnumMetaObject(value);
   const QMetaEnum me = metaObject->enumerator(metaObject->indexOfEnumerator(qt_getEnumName(value)));

    if (const char *key = me.valueToKey(value))
        debug << key;
    else
        debug << int(value);
   */

   debug << int(value);
}

template <class QEnum>
static inline void formatNonNullQEnum(QDebug &debug, const char *prefix, QEnum value)
{
   if (value) {
      debug << prefix;
      formatQEnum(debug, value);
   }
}

template <class Enum>
static inline void formatQFlags(QDebug &debug, const QFlags<Enum> &value)
{
   /* emerald - enum meta object lookup

    const QMetaObject *metaObject = qt_getEnumMetaObject(Enum());
    const QMetaEnum me = metaObject->enumerator(metaObject->indexOfEnumerator(qt_getEnumName(Enum())));
    const QDebugStateSaver saver(debug);
    debug.noquote();
    debug << me.valueToKeys(value);

    */

   const QDebugStateSaver saver(debug);
   debug.noquote();
   debug << int(value);
}

template <class Enum>
static inline void formatNonNullQFlags(QDebug &debug, const char *prefix, const QFlags<Enum> &value)
{
   if (value) {
      debug << prefix;
      formatQFlags(debug, value);
   }
}

} // namespace QtDebugUtils

#endif
