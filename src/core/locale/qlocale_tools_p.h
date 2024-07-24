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

#ifndef QLOCALE_TOOLS_P_H
#define QLOCALE_TOOLS_P_H

#include <qlocale_p.h>

#include <cmath>

QString qulltoa(quint64 l, int base, const QChar _zero);
QString qlltoa(qint64 l, int base, const QChar zero);

enum PrecisionMode {
   PMDecimalDigits =      0x01,
   PMSignificantDigits =  0x02,
   PMChopTrailingZeros =  0x03
};

QString &decimalForm(QChar zero, QChar decimal, QChar group, QString &digits, int decpt, uint precision,
      PrecisionMode pm, bool always_show_decpt, bool thousands_group);

QString &exponentForm(QChar zero, QChar decimal, QChar exponential, QChar group, QChar plus, QChar minus,
      QString &digits, int decpt, uint precision, PrecisionMode pm, bool always_show_decpt);

inline bool isZero(double d)
{
   return std::fpclassify(d) == FP_ZERO;
}

Q_CORE_EXPORT char *qdtoa(double d, int mode, int ndigits, int *decpt, int *sign, char **rve, char **digits_str);
Q_CORE_EXPORT double qstrtod(const char *s00, char const **se, bool *ok);

qint64  qstrtoll(const char *nptr, const char **endptr, int base, bool *ok);
quint64 qstrtoull(const char *nptr, const char **endptr, int base, bool *ok);

#endif
