/***********************************************************************
*
* Copyright (c) 2012-2017 Barbara Geller
* Copyright (c) 2012-2017 Ansel Sermersheim
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

#ifndef QLOCALE_TOOLS_P_H
#define QLOCALE_TOOLS_P_H

#include <qlocale_p.h>
#include <qstring.h>

#if ! defined(QT_QLOCALE_NEEDS_VOLATILE)
#  if defined(Q_CC_GNU)
#    if  __GNUC__ == 4
#      define QT_QLOCALE_NEEDS_VOLATILE
#    elif defined(Q_OS_WIN)
#      define QT_QLOCALE_NEEDS_VOLATILE
#    endif
#  endif
#endif

#if defined(QT_QLOCALE_NEEDS_VOLATILE)
#   define NEEDS_VOLATILE volatile
#else
#   define NEEDS_VOLATILE
#endif

QString qulltoa(quint64 l, int base, const QChar _zero);
QString qlltoa(qint64 l, int base, const QChar zero);

enum PrecisionMode {
   PMDecimalDigits =       0x01,
   PMSignificantDigits =   0x02,
   PMChopTrailingZeros =   0x03
};

QString &decimalForm(QChar zero, QChar decimal, QChar group, QString &digits, int decpt, uint precision,
                     PrecisionMode pm, bool always_show_decpt, bool thousands_group);

QString &exponentForm(QChar zero, QChar decimal, QChar exponential, QChar group, QChar plus, QChar minus,
                      QString &digits, int decpt, uint precision, PrecisionMode pm, bool always_show_decpt);

inline bool isZero(double d)
{
   uchar *ch = (uchar *)&d;

#ifdef QT_ARMFPA
   return !(ch[3] & 0x7F || ch[2] || ch[1] || ch[0] || ch[7] || ch[6] || ch[5] || ch[4]);
#else
   if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
      return !(ch[0] & 0x7F || ch[1] || ch[2] || ch[3] || ch[4] || ch[5] || ch[6] || ch[7]);
   } else {
      return !(ch[7] & 0x7F || ch[6] || ch[5] || ch[4] || ch[3] || ch[2] || ch[1] || ch[0]);
   }
#endif
}

// Removes thousand-group separators in "C" locale.
bool removeGroupSeparators(QLocalePrivate::CharBuff *num);

Q_CORE_EXPORT char *qdtoa(double d, int mode, int ndigits, int *decpt, int *sign, char **rve, char **digits_str);
Q_CORE_EXPORT double qstrtod(const char *s00, char const **se, bool *ok);
qint64 qstrtoll(const char *nptr, const char **endptr, int base, bool *ok);
quint64 qstrtoull(const char *nptr, const char **endptr, int base, bool *ok);


#endif
