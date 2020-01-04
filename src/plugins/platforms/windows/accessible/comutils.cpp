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

#include <qt_windows.h>

#include <ocidl.h>
#include <olectl.h>

#include <comutils.h>
#include <qdatetime.h>
#include <qpixmap.h>
#include <qfont.h>

#include <qvariant.h>
#include <qbytearray.h>
#include <qcolor.h>

static DATE QDateTimeToDATE(const QDateTime &dt)
{
    if (!dt.isValid() || dt.isNull())
        return 949998;  // Special value for no date (01/01/4501)

    SYSTEMTIME stime;
    memset(&stime, 0, sizeof(stime));
    QDate date = dt.date();
    QTime time = dt.time();
    if (date.isValid() && !date.isNull()) {
        stime.wDay = WORD(date.day());
        stime.wMonth = WORD(date.month());
        stime.wYear = WORD(date.year());
    }
    if (time.isValid() && !time.isNull()) {
        stime.wMilliseconds = WORD(time.msec());
        stime.wSecond = WORD(time.second());
        stime.wMinute = WORD(time.minute());
        stime.wHour = WORD(time.hour());
    }

    double vtime;
    SystemTimeToVariantTime(&stime, &vtime);

    return vtime;
}

inline uint QColorToOLEColor(const QColor &col)
{
    return qRgba(col.blue(), col.green(), col.red(), 0x00);
}

bool QVariant2VARIANT(const QVariant &var, VARIANT &arg, const QByteArray &typeName, bool out)
{
    QVariant qvar = var;

    // "type" is the expected type, so coerce if necessary

    QVariant::Type proptype = typeName.isEmpty() ? QVariant::Invalid : QVariant::nameToType(typeName);
    if (proptype == QVariant::UserType && !typeName.isEmpty()) {
        if (typeName == "short" || typeName == "char")
            proptype = QVariant::Int;
        else if (typeName == "float")
            proptype = QVariant::Double;
    }

    if (proptype != QVariant::Invalid && proptype != QVariant::UserType && proptype != qvar.type()) {
        if (qvar.canConvert(QVariant::Type(proptype)))
            qvar.convert(QVariant::Type(proptype));
        else
            qvar = QVariant(proptype);
    }

    if (out && arg.vt == (VT_VARIANT|VT_BYREF) && arg.pvarVal) {
        return QVariant2VARIANT(var, *arg.pvarVal, typeName, false);
    }

    if (out && proptype == QVariant::UserType && typeName == "QVariant") {
        VARIANT *pVariant = new VARIANT;
        QVariant2VARIANT(var, *pVariant, QByteArray(), false);
        arg.vt = VT_VARIANT|VT_BYREF;
        arg.pvarVal = pVariant;
        return true;
    }

    switch ((int)qvar.type()) {
    case QVariant::String:
        if (out && arg.vt == (VT_BSTR|VT_BYREF)) {
            if (*arg.pbstrVal)
                SysFreeString(*arg.pbstrVal);
            *arg.pbstrVal = QStringToBSTR(qvar.toString());
            arg.vt = VT_BSTR|VT_BYREF;
        } else {
            arg.vt = VT_BSTR;
            arg.bstrVal = QStringToBSTR(qvar.toString());
            if (out) {
                arg.pbstrVal = new BSTR(arg.bstrVal);
                arg.vt |= VT_BYREF;
            }
        }
        break;

    case QVariant::Int:
        if (out && arg.vt == (VT_I4|VT_BYREF)) {
            *arg.plVal = qvar.toInt();
        } else {
            arg.vt = VT_I4;
            arg.lVal = qvar.toInt();
            if (out) {
                if (typeName == "short") {
                    arg.vt = VT_I2;
                    arg.piVal = new short(arg.lVal);
                } else if (typeName == "char") {
                    arg.vt = VT_I1;
                    arg.pcVal= new char(arg.lVal);
                } else {
                    arg.plVal = new long(arg.lVal);
                }
                arg.vt |= VT_BYREF;
            }
        }
        break;

    case QVariant::UInt:
        if (out && (arg.vt == (VT_UINT|VT_BYREF) || arg.vt == (VT_I4|VT_BYREF))) {
            *arg.puintVal = qvar.toUInt();
        } else {
            arg.vt = VT_UINT;
            arg.uintVal = qvar.toUInt();
            if (out) {
                arg.puintVal = new uint(arg.uintVal);
                arg.vt |= VT_BYREF;
            }
        }
        break;

    case QVariant::LongLong:
        if (out && arg.vt == (VT_CY|VT_BYREF)) {
            arg.pcyVal->int64 = qvar.toLongLong();

#if defined(_MSC_VER)
        } else if (out && arg.vt == (VT_I8|VT_BYREF)) {
            *arg.pllVal = qvar.toLongLong();
        } else {
            arg.vt = VT_I8;
            arg.llVal = qvar.toLongLong();
            if (out) {
                arg.pllVal = new LONGLONG(arg.llVal);
                arg.vt |= VT_BYREF;
            }
        }
#else
        } else {
            arg.vt = VT_CY;
            arg.cyVal.int64 = qvar.toLongLong();
            if (out) {
                arg.pcyVal = new CY(arg.cyVal);
                arg.vt |= VT_BYREF;
            }
        }
#endif
        break;

    case QVariant::ULongLong:
        if (out && arg.vt == (VT_CY|VT_BYREF)) {
            arg.pcyVal->int64 = qvar.toULongLong();

#if defined(_MSC_VER)
        } else if (out && arg.vt == (VT_UI8|VT_BYREF)) {
            *arg.pullVal = qvar.toULongLong();
        } else {
            arg.vt = VT_UI8;
            arg.ullVal = qvar.toULongLong();
            if (out) {
                arg.pullVal = new ULONGLONG(arg.ullVal);
                arg.vt |= VT_BYREF;
            }
        }
#else
        } else {
            arg.vt = VT_CY;
            arg.cyVal.int64 = qvar.toULongLong();
            if (out) {
                arg.pcyVal = new CY(arg.cyVal);
                arg.vt |= VT_BYREF;
            }
        }

#endif

        break;

    case QVariant::Bool:
        if (out && arg.vt == (VT_BOOL|VT_BYREF)) {
            *arg.pboolVal = qvar.toBool() ? VARIANT_TRUE : VARIANT_FALSE;
        } else {
            arg.vt = VT_BOOL;
            arg.boolVal = qvar.toBool() ? VARIANT_TRUE : VARIANT_FALSE;
            if (out) {
                arg.pboolVal = new short(arg.boolVal);
                arg.vt |= VT_BYREF;
            }
        }
        break;
    case QVariant::Double:
        if (out && arg.vt == (VT_R8|VT_BYREF)) {
            *arg.pdblVal = qvar.toDouble();
        } else {
            arg.vt = VT_R8;
            arg.dblVal = qvar.toDouble();
            if (out) {
                if (typeName == "float") {
                    arg.vt = VT_R4;
                    arg.pfltVal = new float(arg.dblVal);
                } else {
                    arg.pdblVal = new double(arg.dblVal);
                }
                arg.vt |= VT_BYREF;
            }
        }
        break;
    case QVariant::Color:
        if (out && arg.vt == (VT_COLOR|VT_BYREF)) {

            *arg.plVal = QColorToOLEColor(qvariant_cast<QColor>(qvar));
        } else {
            arg.vt = VT_COLOR;
            arg.lVal = QColorToOLEColor(qvariant_cast<QColor>(qvar));
            if (out) {
                arg.plVal = new long(arg.lVal);
                arg.vt |= VT_BYREF;
            }
        }
        break;

    case QVariant::Date:
    case QVariant::Time:
    case QVariant::DateTime:
        if (out && arg.vt == (VT_DATE|VT_BYREF)) {
            *arg.pdate = QDateTimeToDATE(qvar.toDateTime());
        } else {
            arg.vt = VT_DATE;
            arg.date = QDateTimeToDATE(qvar.toDateTime());
            if (out) {
                arg.pdate = new DATE(arg.date);
                arg.vt |= VT_BYREF;
            }
        }
        break;

    case QVariant::Invalid: // default-parameters not set
        if (out && arg.vt == (VT_ERROR|VT_BYREF)) {
            *arg.plVal = DISP_E_PARAMNOTFOUND;
        } else {
            arg.vt = VT_ERROR;
            arg.lVal = DISP_E_PARAMNOTFOUND;
            if (out) {
                arg.plVal = new long(arg.lVal);
                arg.vt |= VT_BYREF;
            }
        }
        break;

    default:
        return false;
    }

    Q_ASSERT(!out || (arg.vt & VT_BYREF));
    return true;
}


