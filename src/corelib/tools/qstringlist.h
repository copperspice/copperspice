/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QSTRINGLIST_H
#define QSTRINGLIST_H

#include <QtCore/qalgorithms.h>
#include <QtCore/qdatastream.h>
#include <QtCore/qlist.h>
#include <QtCore/qregexp.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringmatcher.h>

QT_BEGIN_NAMESPACE

class QRegExp;

typedef QListIterator<QString> QStringListIterator;
typedef QMutableListIterator<QString> QMutableStringListIterator;

class QStringList : public QList<QString>
{
public:
    inline QStringList() { }
    inline explicit QStringList(const QString &i) { append(i); }
    inline QStringList(const QStringList &l) : QList<QString>(l) { }
    inline QStringList(const QList<QString> &l) : QList<QString>(l) { }
#ifdef Q_COMPILER_INITIALIZER_LISTS
    inline QStringList(std::initializer_list<QString> args) : QList<QString>(args) { }
#endif

    inline void sort();
    inline int removeDuplicates();

    inline QString join(const QString &sep) const;

    inline QStringList filter(const QString &str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    inline QBool contains(const QString &str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;

    inline QStringList &replaceInStrings(const QString &before, const QString &after, Qt::CaseSensitivity cs = Qt::CaseSensitive);

    inline QStringList operator+(const QStringList &other) const
    { QStringList n = *this; n += other; return n; }
    inline QStringList &operator<<(const QString &str)
    { append(str); return *this; }
    inline QStringList &operator<<(const QStringList &l)
    { *this += l; return *this; }

#ifndef QT_NO_REGEXP
    inline QStringList filter(const QRegExp &rx) const;
    inline QStringList &replaceInStrings(const QRegExp &rx, const QString &after);
    inline int indexOf(const QRegExp &rx, int from = 0) const;
    inline int lastIndexOf(const QRegExp &rx, int from = -1) const;
    inline int indexOf(QRegExp &rx, int from = 0) const;
    inline int lastIndexOf(QRegExp &rx, int from = -1) const;
#endif

    using QList<QString>::indexOf;
    using QList<QString>::lastIndexOf;
};

Q_DECLARE_TYPEINFO(QStringList, Q_MOVABLE_TYPE);

namespace QtPrivate {
    void Q_CORE_EXPORT QStringList_sort(QStringList *that);
    int Q_CORE_EXPORT QStringList_removeDuplicates(QStringList *that);
    QString Q_CORE_EXPORT QStringList_join(const QStringList *that, const QString &sep);
    QStringList Q_CORE_EXPORT QStringList_filter(const QStringList *that, const QString &str,
                                               Qt::CaseSensitivity cs);

    QBool Q_CORE_EXPORT QStringList_contains(const QStringList *that, const QString &str, Qt::CaseSensitivity cs);
    void Q_CORE_EXPORT QStringList_replaceInStrings(QStringList *that, const QString &before, const QString &after,
                                      Qt::CaseSensitivity cs);

#ifndef QT_NO_REGEXP
    void Q_CORE_EXPORT QStringList_replaceInStrings(QStringList *that, const QRegExp &rx, const QString &after);
    QStringList Q_CORE_EXPORT QStringList_filter(const QStringList *that, const QRegExp &re);
    int Q_CORE_EXPORT QStringList_indexOf(const QStringList *that, const QRegExp &rx, int from);
    int Q_CORE_EXPORT QStringList_lastIndexOf(const QStringList *that, const QRegExp &rx, int from);
    int Q_CORE_EXPORT QStringList_indexOf(const QStringList *that, QRegExp &rx, int from);
    int Q_CORE_EXPORT QStringList_lastIndexOf(const QStringList *that, QRegExp &rx, int from);
#endif
}

inline void QStringList::sort()
{
    QtPrivate::QStringList_sort(this);
}

inline int QStringList::removeDuplicates()
{
    return QtPrivate::QStringList_removeDuplicates(this);
}

inline QString QStringList::join(const QString &sep) const
{
    return QtPrivate::QStringList_join(this, sep);
}

inline QStringList QStringList::filter(const QString &str, Qt::CaseSensitivity cs) const
{
    return QtPrivate::QStringList_filter(this, str, cs);
}

inline QBool QStringList::contains(const QString &str, Qt::CaseSensitivity cs) const
{
    return QtPrivate::QStringList_contains(this, str, cs);
}

inline QStringList &QStringList::replaceInStrings(const QString &before, const QString &after, Qt::CaseSensitivity cs)
{
    QtPrivate::QStringList_replaceInStrings(this, before, after, cs);
    return *this;
}

#ifndef QT_NO_REGEXP
inline QStringList &QStringList::replaceInStrings(const QRegExp &rx, const QString &after)
{
    QtPrivate::QStringList_replaceInStrings(this, rx, after);
    return *this;
}

inline QStringList QStringList::filter(const QRegExp &rx) const
{
    return QtPrivate::QStringList_filter(this, rx);
}

inline int QStringList::indexOf(const QRegExp &rx, int from) const
{
    return QtPrivate::QStringList_indexOf(this, rx, from);
}

inline int QStringList::lastIndexOf(const QRegExp &rx, int from) const
{
    return QtPrivate::QStringList_lastIndexOf(this, rx, from);
}

inline int QStringList::indexOf(QRegExp &rx, int from) const
{
    return QtPrivate::QStringList_indexOf(this, rx, from);
}

inline int QStringList::lastIndexOf(QRegExp &rx, int from) const
{
    return QtPrivate::QStringList_lastIndexOf(this, rx, from);
}
#endif

#ifndef QT_NO_DATASTREAM
inline QDataStream &operator>>(QDataStream &in, QStringList &list)
{
    return operator>>(in, static_cast<QList<QString> &>(list));
}
inline QDataStream &operator<<(QDataStream &out, const QStringList &list)
{
    return operator<<(out, static_cast<const QList<QString> &>(list));
}
#endif // QT_NO_DATASTREAM

QT_END_NAMESPACE

#endif // QSTRINGLIST_H
