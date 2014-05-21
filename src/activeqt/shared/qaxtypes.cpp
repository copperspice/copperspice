/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the ActiveQt framework of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

#include <ocidl.h>
#include <olectl.h>

#include "qaxtypes.h"

#ifndef QT_NO_WIN_ACTIVEQT

#include <qcolormap.h>
#include <qcursor.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qobject.h>

#ifdef QAX_SERVER
#   include <qaxfactory.h>
#   include <qsystemlibrary_p.h>
#else
#   include <quuid.h>
#   include <qaxobject.h>
#endif

QT_BEGIN_NAMESPACE

#ifdef QAX_SERVER
extern ITypeLib *qAxTypeLibrary;

CLSID CLSID_QRect = { 0x34030f30, 0xe359, 0x4fe6, {0xab, 0x82, 0x39, 0x76, 0x6f, 0x5d, 0x91, 0xee } };
CLSID CLSID_QSize = { 0xcb5f84b3, 0x29e5, 0x491d, {0xba, 0x18, 0x54, 0x72, 0x48, 0x8e, 0xef, 0xba } };
CLSID CLSID_QPoint = { 0x3be838a3, 0x3fac, 0xbfc4, {0x4c, 0x6c, 0x37, 0xc4, 0x4d, 0x03, 0x02, 0x52 } };

GUID IID_IAxServerBase = { 0xbd2ec165, 0xdfc9, 0x4319, { 0x8b, 0x9b, 0x60, 0xa5, 0x74, 0x78, 0xe9, 0xe3} };
#else
extern void *qax_createObjectWrapper(int metaType, IUnknown *iface);
#endif

static IFontDisp *QFontToIFont(const QFont &font)
{

    FONTDESC fdesc;
    memset(&fdesc, 0, sizeof(fdesc));
    fdesc.cbSizeofstruct = sizeof(FONTDESC);
    fdesc.cySize.Lo = font.pointSize() * 10000;
    fdesc.fItalic = font.italic();
    fdesc.fStrikethrough = font.strikeOut();
    fdesc.fUnderline = font.underline();
    fdesc.lpstrName = QStringToBSTR(font.family());
    fdesc.sWeight = font.weight() * 10;
    
    IFontDisp *f;
    HRESULT res = OleCreateFontIndirect(&fdesc, IID_IFontDisp, (void**)&f);
    if (res != S_OK) {
        if (f) f->Release();
        f = 0;
#if defined(QT_CHECK_STATE)
        qWarning("QFontToIFont: Failed to create IFont");
#endif
    }
    return f;
}

static QFont IFontToQFont(IFont *f)
{
    BSTR name;
    BOOL bold;
    SHORT charset;
    BOOL italic;
    CY size;
    BOOL strike;
    BOOL underline;
    SHORT weight;
    f->get_Name(&name);
    f->get_Bold(&bold);
    f->get_Charset(&charset);
    f->get_Italic(&italic);
    f->get_Size(&size);
    f->get_Strikethrough(&strike);
    f->get_Underline(&underline);
    f->get_Weight(&weight);
    QFont font(QString::fromWCharArray(name), size.Lo/9750, weight / 97, italic);
    font.setBold(bold);
    font.setStrikeOut(strike);
    font.setUnderline(underline);
    SysFreeString(name);
    
    return font;
}

static IPictureDisp *QPixmapToIPicture(const QPixmap &pixmap)
{
    IPictureDisp *pic = 0;
    
    PICTDESC desc;
    desc.cbSizeofstruct = sizeof(PICTDESC);
    desc.picType = PICTYPE_BITMAP;
    
    desc.bmp.hbitmap = 0;
    desc.bmp.hpal = QColormap::hPal();
    
    if (!pixmap.isNull()) {
        desc.bmp.hbitmap = pixmap.toWinHBITMAP();
        Q_ASSERT(desc.bmp.hbitmap);
    }

    HRESULT res = OleCreatePictureIndirect(&desc, IID_IPictureDisp, true, (void**)&pic);
    if (res != S_OK) {
        if (pic) pic->Release();
        pic = 0;
#if defined(QT_CHECK_STATE)
        qWarning("QPixmapToIPicture: Failed to create IPicture");
#endif
    }
    return pic;
}

static QPixmap IPictureToQPixmap(IPicture *ipic)
{
    SHORT type;
    ipic->get_Type(&type);
    if (type != PICTYPE_BITMAP)
        return QPixmap();

    HBITMAP hbm = 0;
    ipic->get_Handle((OLE_HANDLE*)&hbm);
    if (!hbm)
        return QPixmap();

    return QPixmap::fromWinHBITMAP(hbm);
}

static QDateTime DATEToQDateTime(DATE ole)
{
    SYSTEMTIME stime;
    if (ole >= 949998 || VariantTimeToSystemTime(ole, &stime) == false)
        return QDateTime();
    
    QDate date(stime.wYear, stime.wMonth, stime.wDay);
    QTime time(stime.wHour, stime.wMinute, stime.wSecond, stime.wMilliseconds);
    return QDateTime(date, time);
}

static DATE QDateTimeToDATE(const QDateTime &dt)
{
    if (!dt.isValid() || dt.isNull())
        return 949998;
    
    SYSTEMTIME stime;
    memset(&stime, 0, sizeof(stime));
    QDate date = dt.date();
    QTime time = dt.time();
    if (date.isValid() && !date.isNull()) {
        stime.wDay = date.day();
        stime.wMonth = date.month();
        stime.wYear = date.year();
    }
    if (time.isValid() && !time.isNull()) {
        stime.wMilliseconds = time.msec();
        stime.wSecond = time.second();
        stime.wMinute = time.minute();
        stime.wHour = time.hour();
    }
    
    double vtime;
    SystemTimeToVariantTime(&stime, &vtime);
    
    return vtime;
}

QColor OLEColorToQColor(uint col)
{
    COLORREF cref;
    OleTranslateColor(col, QColormap::hPal(), &cref);
    return QColor(GetRValue(cref),GetGValue(cref),GetBValue(cref));
}

/*
    Converts \a var to \a arg, and tries to coerce \a arg to \a type.

    Used by

    QAxServerBase:
        - QAxServerBase::qt_metacall
        - IDispatch::Invoke(PROPERTYGET, METHOD)
        - IPersistPropertyBag::Save

    QAxBase:
        - IDispatch::Invoke (QAxEventSink)
        - QAxBase::internalProperty(WriteProperty)
        - QAxBase::internalInvoke()
        - QAxBase::dynamicCallHelper()
        - IPropertyBag::Read (QtPropertyBag)

    Also called recoursively for lists.
*/
bool QVariantToVARIANT(const QVariant &var, VARIANT &arg, const QByteArray &typeName, bool out)
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
        if (qvar.canConvert(proptype))
            qvar.convert(proptype);
        else
            qvar = QVariant(proptype);
    }

    if (out && arg.vt == (VT_VARIANT|VT_BYREF) && arg.pvarVal) {
        return QVariantToVARIANT(var, *arg.pvarVal, typeName, false);
    }

    if (out && proptype == QVariant::UserType && typeName == "QVariant") {
        VARIANT *pVariant = new VARIANT;
        QVariantToVARIANT(var, *pVariant, QByteArray(), false);
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

        } else {
            arg.vt = VT_CY;
            arg.cyVal.int64 = qvar.toLongLong();
            if (out) {
                arg.pcyVal = new CY(arg.cyVal);
                arg.vt |= VT_BYREF;
            }
        }
        break;
        
    case QVariant::ULongLong:
        if (out && arg.vt == (VT_CY|VT_BYREF)) {
            arg.pcyVal->int64 = qvar.toULongLong();
        } else {
            arg.vt = VT_CY;
            arg.cyVal.int64 = qvar.toULongLong();
            if (out) {
                arg.pcyVal = new CY(arg.cyVal);
                arg.vt |= VT_BYREF;
            }
        }
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
    case QVariant::Font:
        if (out && arg.vt == (VT_DISPATCH|VT_BYREF)) {
            if (*arg.ppdispVal)
                (*arg.ppdispVal)->Release();
            *arg.ppdispVal = QFontToIFont(qvariant_cast<QFont>(qvar));
        } else {
            arg.vt = VT_DISPATCH;
            arg.pdispVal = QFontToIFont(qvariant_cast<QFont>(qvar));
            if (out) {
                arg.ppdispVal = new IDispatch*(arg.pdispVal);
                arg.vt |= VT_BYREF;
            }
        }
        break;
        
    case QVariant::Pixmap:
        if (out && arg.vt == (VT_DISPATCH|VT_BYREF)) {
            if (*arg.ppdispVal)
                (*arg.ppdispVal)->Release();
            *arg.ppdispVal = QPixmapToIPicture(qvariant_cast<QPixmap>(qvar));
        } else {
            arg.vt = VT_DISPATCH;
            arg.pdispVal = QPixmapToIPicture(qvariant_cast<QPixmap>(qvar));
            if (out) {
                arg.ppdispVal = new IDispatch*(arg.pdispVal);
                arg.vt |= VT_BYREF;
            }
        }
        break;

    case QVariant::Cursor:
        {
#ifndef QT_NO_CURSOR
            int shape = qvariant_cast<QCursor>(qvar).shape();
            if (out && (arg.vt & VT_BYREF)) {
                switch(arg.vt & ~VT_BYREF) {
                case VT_I4:
                    *arg.plVal = shape;
                    break;
                case VT_I2:
                    *arg.piVal = shape;
                    break;
                case VT_UI4:
                    *arg.pulVal = shape;
                    break;
                case VT_UI2:
                    *arg.puiVal = shape;
                    break;
                case VT_INT:
                    *arg.pintVal = shape;
                    break;
                case VT_UINT:
                    *arg.puintVal = shape;
                    break;
                }
            } else {
                arg.vt = VT_I4;
                arg.lVal = shape;
                if (out) {
                    arg.plVal = new long(arg.lVal);
                    arg.vt |= VT_BYREF;
                }
            }
#endif
        }
        break;
        
    case QVariant::List:
        {
            const QList<QVariant> list = qvar.toList();
            const int count = list.count();
            VARTYPE vt = VT_VARIANT;
            QVariant::Type listType = QVariant::LastType; // == QVariant
            if (!typeName.isEmpty() && typeName.startsWith("QList<")) {
                const QByteArray listTypeName = typeName.mid(6, typeName.length() - 7); // QList<int> -> int
                listType = QVariant::nameToType(listTypeName);
            }

            VARIANT variant;
            void *pElement = &variant;
            switch(listType) {
            case QVariant::Int:
                vt = VT_I4;
                pElement = &variant.lVal;
                break;
            case QVariant::Double:
                vt = VT_R8;
                pElement = &variant.dblVal;
                break;
            case QVariant::DateTime:
                vt = VT_DATE;
                pElement = &variant.date;
                break;
            case QVariant::Bool:
                vt = VT_BOOL;
                pElement = &variant.boolVal;
                break;
            case QVariant::LongLong:
                vt = VT_CY;
                pElement = &variant.cyVal;
                break;

            default:
                break;
            }
            SAFEARRAY *array = 0;
            bool is2D = false;
            // If the first element in the array is a list the whole list is 
            // treated as a 2D array. The column count is taken from the 1st element.
            if (count) {
                QVariantList col = list.at(0).toList();
                int maxColumns = col.count();
                if (maxColumns) {
                    is2D = true;
                    SAFEARRAYBOUND rgsabound[2] = { {0} };
                    rgsabound[0].cElements = count;
                    rgsabound[1].cElements = maxColumns;
                    array = SafeArrayCreate(VT_VARIANT, 2, rgsabound);
                    LONG rgIndices[2];
                    for (LONG i = 0; i < count; ++i) {
                        rgIndices[0] = i;
                        QVariantList columns = list.at(i).toList();
                        int columnCount = qMin(maxColumns, columns.count());
                        for (LONG j = 0;  j < columnCount; ++j) {
                            QVariant elem = columns.at(j);
                            VariantInit(&variant);
                            QVariantToVARIANT(elem, variant, elem.typeName());
                            rgIndices[1] = j;
                            SafeArrayPutElement(array, rgIndices, pElement);
                            clearVARIANT(&variant);
                        }
                    }

                }
            }
            if (!is2D) {
                array = SafeArrayCreateVector(vt, 0, count);
                for (LONG index = 0; index < count; ++index) {
                    QVariant elem = list.at(index);
                    if (listType != QVariant::LastType)
                        elem.convert(listType);
                    VariantInit(&variant);
                    QVariantToVARIANT(elem, variant, elem.typeName());
                    SafeArrayPutElement(array, &index, pElement);
                    clearVARIANT(&variant);
                }
            }
            if (out && arg.vt == (VT_ARRAY|vt|VT_BYREF)) {
                if (*arg.pparray)
                    SafeArrayDestroy(*arg.pparray);
                *arg.pparray = array;
            } else {
                arg.vt = VT_ARRAY|vt;
                arg.parray = array;
                if (out) {
                    arg.pparray = new SAFEARRAY*(arg.parray);
                    arg.vt |= VT_BYREF;
                }
            }
        }
        break;
        
    case QVariant::StringList:
        {
            const QStringList list = qvar.toStringList();
            const int count = list.count();
            SAFEARRAY *array = SafeArrayCreateVector(VT_BSTR, 0, count);
            for (LONG index = 0; index < count; ++index) {
                QString elem = list.at(index);
                BSTR bstr = QStringToBSTR(elem);
                SafeArrayPutElement(array, &index, bstr);
                SysFreeString(bstr);
            }
            
            if (out && arg.vt == (VT_ARRAY|VT_BSTR|VT_BYREF)) {
                if (*arg.pparray)
                    SafeArrayDestroy(*arg.pparray);
                *arg.pparray = array;
            } else {
                arg.vt = VT_ARRAY|VT_BSTR;
                arg.parray = array;
                if (out) {
                    arg.pparray = new SAFEARRAY*(arg.parray);
                    arg.vt |= VT_BYREF;
                }
            }
        }
        break;
        
    case QVariant::ByteArray:
        {
            const QByteArray bytes = qvar.toByteArray();
            const uint count = bytes.count();
            SAFEARRAY *array =	SafeArrayCreateVector(VT_UI1, 0, count);
            if (count) {
                const char *data = bytes.constData();
                char *dest;
                SafeArrayAccessData(array, (void **)&dest);
                memcpy(dest, data, count);
                SafeArrayUnaccessData(array);
            }
            
            if (out && arg.vt == (VT_ARRAY|VT_UI1|VT_BYREF)) {
                if (*arg.pparray)
                    SafeArrayDestroy(*arg.pparray);
                *arg.pparray = array;
            } else {
                arg.vt = VT_ARRAY|VT_UI1;
                arg.parray = array;
                if (out) {
                    arg.pparray = new SAFEARRAY*(arg.parray);
                    arg.vt |= VT_BYREF;
                }
            }
        }
        break;
        
#ifdef QAX_SERVER
    case QVariant::Rect:
    case QVariant::Size:
    case QVariant::Point:
        {
            typedef HRESULT(WINAPI* PGetRecordInfoFromTypeInfo)(ITypeInfo *, IRecordInfo **);
            static PGetRecordInfoFromTypeInfo pGetRecordInfoFromTypeInfo = 0;
            static bool resolved = false;
            if (!resolved) {
                QSystemLibrary oleaut32(QLatin1String("oleaut32"));
                pGetRecordInfoFromTypeInfo = (PGetRecordInfoFromTypeInfo)oleaut32.resolve("GetRecordInfoFromTypeInfo");
                resolved = true;
            }
            if (!pGetRecordInfoFromTypeInfo)
                break;
            
            ITypeInfo *typeInfo = 0;
            IRecordInfo *recordInfo = 0;
            CLSID clsid = qvar.type() == QVariant::Rect ? CLSID_QRect
                :qvar.type() == QVariant::Size ? CLSID_QSize
                :CLSID_QPoint;
            qAxTypeLibrary->GetTypeInfoOfGuid(clsid, &typeInfo);
            if (!typeInfo)
                break;
            pGetRecordInfoFromTypeInfo(typeInfo, &recordInfo);
            typeInfo->Release();
            if (!recordInfo)
                break;
            
            void *record = 0;
            switch (qvar.type()) {
            case QVariant::Rect:
                {
                    QRect qrect(qvar.toRect());
                    recordInfo->RecordCreateCopy(&qrect, &record);
                }
                break;
            case QVariant::Size:
                {
                    QSize qsize(qvar.toSize());
                    recordInfo->RecordCreateCopy(&qsize, &record);
                }
                break;
            case QVariant::Point:
                {
                    QPoint qpoint(qvar.toPoint());
                    recordInfo->RecordCreateCopy(&qpoint, &record);
                }
                break;
            }
            
            arg.vt = VT_RECORD;
            arg.pRecInfo = recordInfo,
            arg.pvRecord = record;
            if (out) {
                qWarning("QVariantToVARIANT: out-parameter not supported for records");
                return false;
           }
        }
        break;
#endif // QAX_SERVER
    case QVariant::UserType:
        {
            QByteArray subType = qvar.typeName();
#ifdef QAX_SERVER
            if (subType.endsWith('*'))
                subType.truncate(subType.length() - 1);
#endif
            if (!qstrcmp(qvar.typeName(), "IDispatch*")) {
                arg.vt = VT_DISPATCH;
                arg.pdispVal = *(IDispatch**)qvar.data();
                if (arg.pdispVal)
                    arg.pdispVal->AddRef();
                if (out) {
                    qWarning("QVariantToVARIANT: out-parameter not supported for IDispatch");
                    return false;
                }
            } else if (!qstrcmp(qvar.typeName(), "IDispatch**")) {
                arg.vt = VT_DISPATCH;
                arg.ppdispVal = *(IDispatch***)qvar.data();
                if (out)
                    arg.vt |= VT_BYREF;
            } else if (!qstrcmp(qvar.typeName(), "IUnknown*")) {
                arg.vt = VT_UNKNOWN;
                arg.punkVal = *(IUnknown**)qvar.data();
                if (arg.punkVal)
                    arg.punkVal->AddRef();
                if (out) {
                    qWarning("QVariantToVARIANT: out-parameter not supported for IUnknown");
                    return false;
                }
#ifdef QAX_SERVER
            } else if (qAxFactory()->metaObject(QString::fromLatin1(subType.constData()))) {
                arg.vt = VT_DISPATCH;
                void *user = *(void**)qvar.constData();
//                qVariantGet(qvar, user, qvar.typeName());
                if (!user) {
                    arg.pdispVal = 0;
                } else {
                    qAxFactory()->createObjectWrapper(static_cast<QObject*>(user), &arg.pdispVal);
                }
                if (out) {
                    qWarning("QVariantToVARIANT: out-parameter not supported for subtype");
                    return false;
                }
#else
            } else if (QMetaType::type(subType)) {
                QAxObject *object = *(QAxObject**)qvar.constData();
//                qVariantGet(qvar, object, subType);
                arg.vt = VT_DISPATCH;
                object->queryInterface(IID_IDispatch, (void**)&arg.pdispVal);
                if (out) {
                    qWarning("QVariantToVARIANT: out-parameter not supported for subtype");
                    return false;
                }
#endif
            } else {
                return false;
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

/*!
    Copies the data in \a var into \a data.

    Used by

    QAxServerBase:
        - QAxServerBase::qt_metacall (update out parameters/return value)

    QAxBase:
        - internalProperty(ReadProperty)
        - internalInvoke(update out parameters/return value)

*/
bool QVariantToVoidStar(const QVariant &var, void *data, const QByteArray &typeName, uint type)
{
    if (!data)
        return true;

    if (type == QVariant::LastType || (type == 0 && typeName == "QVariant")) {
        *(QVariant*)data = var;
        return true;
    }

    switch (var.type()) {
    case QVariant::Invalid:
        break;
    case QVariant::String:
        *(QString*)data = var.toString();
        break;
    case QVariant::Int:
        *(int*)data = var.toInt();
        break;
    case QVariant::UInt:
        *(uint*)data = var.toUInt();
        break;
    case QVariant::Bool:
        *(bool*)data = var.toBool();
        break;
    case QVariant::Double:
        *(double*)data = var.toDouble();
        break;
    case QVariant::Color:
        *(QColor*)data = qvariant_cast<QColor>(var);
        break;
    case QVariant::Date:
        *(QDate*)data = var.toDate();
        break;
    case QVariant::Time:
        *(QTime*)data = var.toTime();
        break;
    case QVariant::DateTime:
        *(QDateTime*)data = var.toDateTime();
        break;
    case QVariant::Font:
        *(QFont*)data = qvariant_cast<QFont>(var);
        break;
    case QVariant::Pixmap:
        *(QPixmap*)data = qvariant_cast<QPixmap>(var);
        break;
#ifndef QT_NO_CURSOR
    case QVariant::Cursor:
        *(QCursor*)data = qvariant_cast<QCursor>(var);
        break;
#endif
    case QVariant::List:
        *(QList<QVariant>*)data = var.toList();
        break;
    case QVariant::StringList:
        *(QStringList*)data = var.toStringList();
        break;
    case QVariant::ByteArray:
        *(QByteArray*)data = var.toByteArray();
        break;
    case QVariant::LongLong:
        *(qint64*)data = var.toLongLong();
        break;
    case QVariant::ULongLong:
        *(quint64*)data = var.toULongLong();
        break;
    case QVariant::Rect:
        *(QRect*)data = var.toRect();
        break;
    case QVariant::Size:
        *(QSize*)data = var.toSize();
        break;
    case QVariant::Point:
        *(QPoint*)data = var.toPoint();
        break;
    case QVariant::UserType:
        *(void**)data = *(void**)var.constData();
//        qVariantGet(var, *(void**)data, typeName);
        break;
    default:
        qWarning("QVariantToVoidStar: Unhandled QVariant type");
        return false;
    }
    
    return true;
}
  
/*!
    Returns \a arg as a QVariant of type \a type.

    Used by

    QAxServerBase:
        - QAxServerBase::qt_metacall(update out parameters/return value)
        - IDispatch::Invoke(METHOD, PROPERTYPUT)
        - IPersistPropertyBag::Load

    QAxBase:
        - IDispatch::Invoke (QAxEventSink)
        - QAxBase::internalProperty(ReadProperty)
        - QAxBase::internalInvoke(update out parameters/return value)
        - QAxBase::dynamicCallHelper(update out parameters)
        - QAxBase::dynamicCall(return value)
        - IPropertyBag::Write (QtPropertyBag)
*/
QVariant VARIANTToQVariant(const VARIANT &arg, const QByteArray &typeName, uint type)
{
    QVariant var;
    switch(arg.vt) {
    case VT_BSTR:
        var = QString::fromWCharArray(arg.bstrVal);
        break;
    case VT_BSTR|VT_BYREF:
        var = QString::fromWCharArray(*arg.pbstrVal);
        break;
    case VT_BOOL:
        var = QVariant((bool)arg.boolVal);
        break;
    case VT_BOOL|VT_BYREF:
        var = QVariant((bool)*arg.pboolVal);
        break;
    case VT_I1:
        var = arg.cVal;
        if (typeName == "char")
            type = QVariant::Int;
        break;
    case VT_I1|VT_BYREF:
        var = *arg.pcVal;
        if (typeName == "char")
            type = QVariant::Int;
        break;
    case VT_I2:
        var = arg.iVal;
        if (typeName == "short")
            type = QVariant::Int;
        break;
    case VT_I2|VT_BYREF:
        var = *arg.piVal;
        if (typeName == "short")
            type = QVariant::Int;
        break;
    case VT_I4:
        if (type == QVariant::Color || (!type && typeName == "QColor"))
            var = QVariant::fromValue(OLEColorToQColor(arg.lVal));
#ifndef QT_NO_CURSOR
        else if (type == QVariant::Cursor || (!type && (typeName == "QCursor" || typeName == "QCursor*")))
            var = QVariant::fromValue(QCursor(static_cast<Qt::CursorShape>(arg.lVal)));
#endif
        else
            var = (int)arg.lVal;
        break;
    case VT_I4|VT_BYREF:
        if (type == QVariant::Color || (!type && typeName == "QColor"))
            var = QVariant::fromValue(OLEColorToQColor((int)*arg.plVal));
#ifndef QT_NO_CURSOR
        else if (type == QVariant::Cursor || (!type && (typeName == "QCursor" || typeName == "QCursor*")))
            var = QVariant::fromValue(QCursor(static_cast<Qt::CursorShape>(*arg.plVal)));
#endif
        else
            var = (int)*arg.plVal;
        break;
    case VT_INT:
        var = arg.intVal;
        break;
    case VT_INT|VT_BYREF:
        var = *arg.pintVal;
        break;
    case VT_UI1:
        var = arg.bVal;
        break;
    case VT_UI1|VT_BYREF:
        var = *arg.pbVal;
        break;
    case VT_UI2:
        var = arg.uiVal;
        break;
    case VT_UI2|VT_BYREF:
        var = *arg.puiVal;
        break;
    case VT_UI4:
        if (type == QVariant::Color || (!type && typeName == "QColor"))
            var = QVariant::fromValue(OLEColorToQColor(arg.ulVal));
#ifndef QT_NO_CURSOR
        else if (type == QVariant::Cursor || (!type && (typeName == "QCursor" || typeName == "QCursor*")))
            var = QVariant::fromValue(QCursor(static_cast<Qt::CursorShape>(arg.ulVal)));
#endif
        else
            var = (int)arg.ulVal;
        break;
    case VT_UI4|VT_BYREF:
        if (type == QVariant::Color || (!type && typeName == "QColor"))
            var = QVariant::fromValue(OLEColorToQColor((uint)*arg.pulVal));
#ifndef QT_NO_CURSOR
        else if (type == QVariant::Cursor || (!type && (typeName == "QCursor" || typeName == "QCursor*")))
            var = QVariant::fromValue(QCursor(static_cast<Qt::CursorShape>(*arg.pulVal)));
#endif
        else
            var = (int)*arg.pulVal;
        break;
    case VT_UINT:
        var = arg.uintVal;
        break;
    case VT_UINT|VT_BYREF:
        var = *arg.puintVal;
        break;
    case VT_CY:
        var = arg.cyVal.int64;
        break;
    case VT_CY|VT_BYREF:
        var = arg.pcyVal->int64;
        break;
    case VT_R4:
        var = arg.fltVal;
        break;
    case VT_R4|VT_BYREF:
        var = *arg.pfltVal;
        break;
    case VT_R8:
        var = arg.dblVal;
        break;
    case VT_R8|VT_BYREF:
        var = *arg.pdblVal;
        break;
    case VT_DATE:
        var = DATEToQDateTime(arg.date);
        if (type == QVariant::Date || (!type && (typeName == "QDate" || typeName == "QDate*"))) {
            var.convert(QVariant::Date);
        } else if (type == QVariant::Time || (!type && (typeName == "QTime" || typeName == "QTime*"))) {
            var.convert(QVariant::Time);
        }
        break;
    case VT_DATE|VT_BYREF:
        var = DATEToQDateTime(*arg.pdate);
        if (type == QVariant::Date || (!type && (typeName == "QDate" || typeName == "QDate*"))) {
            var.convert(QVariant::Date);
        } else if (type == QVariant::Time || (!type && (typeName == "QTime" || typeName == "QTime*"))) {
            var.convert(QVariant::Time);
        }
        break;
    case VT_VARIANT:
    case VT_VARIANT|VT_BYREF:
        if (arg.pvarVal)
            var = VARIANTToQVariant(*arg.pvarVal, typeName);
        break;
        
    case VT_DISPATCH:
    case VT_DISPATCH|VT_BYREF:
        {
            // pdispVal and ppdispVal are a union
            IDispatch *disp = 0;
            if (arg.vt & VT_BYREF)
                disp = *arg.ppdispVal;
            else
                disp = arg.pdispVal;
            if (type == QVariant::Font || (!type && (typeName == "QFont" || typeName == "QFont*"))) {
                IFont *ifont = 0;
                if (disp)
                    disp->QueryInterface(IID_IFont, (void**)&ifont);
                if (ifont) {
                    var = QVariant::fromValue(IFontToQFont(ifont));
                    ifont->Release();
                } else {
                    var = QVariant::fromValue(QFont());
                }
            } else if (type == QVariant::Pixmap || (!type && (typeName == "QPixmap" || typeName == "QPixmap*"))) {
                IPicture *ipic = 0;
                if (disp)
                    disp->QueryInterface(IID_IPicture, (void**)&ipic);
                if (ipic) {
                    var = QVariant::fromValue(IPictureToQPixmap(ipic));
                    ipic->Release();
                } else {
                    var = QVariant::fromValue(QPixmap());
                }
            } else {
#ifdef QAX_SERVER
                IAxServerBase *iface = 0;
                if (disp && typeName != "IDispatch*")
                    disp->QueryInterface(IID_IAxServerBase, (void**)&iface);
                if (iface) {
                    QObject *qObj = iface->qObject();
                    iface->Release();
                    var = QVariant(qRegisterMetaType<QObject*>(qObj ? QByteArray(qObj->metaObject()->className()) + '*' : typeName), &qObj);
                } else
#endif
                {
                    if (!typeName.isEmpty()) {
                        if (arg.vt & VT_BYREF) {
                            var = QVariant(qRegisterMetaType<IDispatch**>("IDispatch**"), &arg.ppdispVal);
                        } else {
#ifndef QAX_SERVER
                            if (typeName == "QVariant") {
                                QAxObject *object = new QAxObject(disp);
                                var = QVariant::fromValue<QAxObject*>(object);
                            } else if (typeName != "IDispatch*" && QMetaType::type(typeName)) {
                                QByteArray typeNameStr = QByteArray(typeName);
                                int pIndex = typeName.lastIndexOf('*');
                                if (pIndex != -1)
                                    typeNameStr = typeName.left(pIndex);
                                int metaType = QMetaType::type(typeNameStr);
                                Q_ASSERT(metaType != 0);
                                QAxObject *object = (QAxObject*)qax_createObjectWrapper(metaType, disp);
                                var = QVariant(QMetaType::type(typeName), &object);
                            } else
#endif
                                var = QVariant(qRegisterMetaType<IDispatch*>(typeName), &disp);
                        }
                    }
                }
            }
        }
        break;
    case VT_UNKNOWN:
    case VT_UNKNOWN|VT_BYREF:
        {
            IUnknown *unkn = 0;
            if (arg.vt & VT_BYREF)
                unkn = *arg.ppunkVal;
            else
                unkn = arg.punkVal;
            var.setValue(unkn);
        }
        break;
    case VT_ARRAY|VT_VARIANT:
    case VT_ARRAY|VT_VARIANT|VT_BYREF:
	{
	    SAFEARRAY *array = 0;
	    if ( arg.vt & VT_BYREF )
		array = *arg.pparray;
	    else
		array = arg.parray;

            UINT cDims = array ? SafeArrayGetDim(array) : 0;
            switch(cDims) {
            case 1:
                {
                    QVariantList list;

	            long lBound, uBound;
	            SafeArrayGetLBound( array, 1, &lBound );
	            SafeArrayGetUBound( array, 1, &uBound );

	            for ( long i = lBound; i <= uBound; ++i ) {
		        VARIANT var;
		        VariantInit( &var );
		        SafeArrayGetElement( array, &i, &var );

		        QVariant qvar = VARIANTToQVariant( var, 0 );
		        clearVARIANT( &var );
		        list << qvar;
	            }

                    var = list;
                }
                break;

            case 2:
                {
                    QVariantList listList; //  a list of lists
                    long dimIndices[2];

                    long xlBound, xuBound, ylBound, yuBound;
	            SafeArrayGetLBound(array, 1, &xlBound);
	            SafeArrayGetUBound(array, 1, &xuBound);
	            SafeArrayGetLBound(array, 2, &ylBound);
	            SafeArrayGetUBound(array, 2, &yuBound);

                    for (long x = xlBound; x <= xuBound; ++x) {
                        QVariantList list;

                        dimIndices[0] = x;
	                for (long y = ylBound; y <= yuBound; ++y) {
		            VARIANT var;
		            VariantInit(&var);
                            dimIndices[1] = y;
		            SafeArrayGetElement(array, dimIndices, &var);

		            QVariant qvar = VARIANTToQVariant(var, 0);
		            clearVARIANT(&var);
		            list << qvar;
                        }

                        listList << QVariant(list);
                    }
                    var = listList;
                }
                break;
            default:
                var = QVariantList();
                break;
            }
	}
        break;
        
    case VT_ARRAY|VT_BSTR:
    case VT_ARRAY|VT_BSTR|VT_BYREF:
        {
            SAFEARRAY *array = 0;
            if (arg.vt & VT_BYREF)
                array = *arg.pparray;
            else
                array = arg.parray;
            
            QStringList strings;
            if (!array || array->cDims != 1) {
                var = strings;
                break;
            }
            
            long lBound, uBound;
            SafeArrayGetLBound(array, 1, &lBound);
            SafeArrayGetUBound(array, 1, &uBound);
            
            for (long i = lBound; i <= uBound; ++i) {
                BSTR bstr;
                SafeArrayGetElement(array, &i, &bstr);
                strings << QString::fromWCharArray(bstr);
                SysFreeString(bstr);
            }
            
            var = strings;
        }
        break;
        
    case VT_ARRAY|VT_UI1:
    case VT_ARRAY|VT_UI1|VT_BYREF:
        {
            SAFEARRAY *array = 0;
            if (arg.vt & VT_BYREF)
                array = *arg.pparray;
            else
                array = arg.parray;
            
            QByteArray bytes;
            if (!array || array->cDims != 1) {
                var = bytes;
                break;
            }
            
            long lBound, uBound;
            SafeArrayGetLBound(array, 1, &lBound);
            SafeArrayGetUBound(array, 1, &uBound);
            
            if (uBound != -1) { // non-empty array
                bytes.resize(uBound - lBound + 1);
                char *data = bytes.data();
                char *src;
                SafeArrayAccessData(array, (void**)&src);
                memcpy(data, src, bytes.size());
                SafeArrayUnaccessData(array);
            }
            
            var = bytes;
        }
        break;
        
#if defined(QAX_SERVER)
    case VT_RECORD:
    case VT_RECORD|VT_BYREF:
        if (arg.pvRecord && arg.pRecInfo) {
            IRecordInfo *recordInfo = arg.pRecInfo;
            void *record = arg.pvRecord;
            GUID guid;
            recordInfo->GetGuid(&guid);
            
            if (guid == CLSID_QRect) {
                QRect qrect;
                recordInfo->RecordCopy(record, &qrect);
                var = qrect;
            } else if (guid == CLSID_QSize) {
                QSize qsize;
                recordInfo->RecordCopy(record, &qsize);
                var = qsize;
            } else if (guid == CLSID_QPoint) {
                QPoint qpoint;
                recordInfo->RecordCopy(record, &qpoint);
                var = qpoint;
            }
        }
        break;
#endif // QAX_SERVER

    default:
        // support for any SAFEARRAY(Type) where Type can be converted to a QVariant 
        // -> QVariantList
        if (arg.vt & VT_ARRAY) {
            SAFEARRAY *array = 0;
            if (arg.vt & VT_BYREF)
                array = *arg.pparray;
            else
                array = arg.parray;

            QVariantList list;
            if (!array || array->cDims != 1) {
                var = list;
                break;
            }

            // find out where to store the element
            VARTYPE vt;
            VARIANT variant;
            SafeArrayGetVartype(array, &vt);

            void *pElement = 0;
            switch(vt) {
            case VT_BSTR: Q_ASSERT(false); break; // already covered
            case VT_BOOL: pElement = &variant.boolVal; break;
            case VT_I1: pElement = &variant.cVal; break;
            case VT_I2: pElement = &variant.iVal; break;
            case VT_I4: pElement = &variant.lVal; break;
            case VT_INT: pElement = &variant.intVal; break;
            case VT_UI1: Q_ASSERT(false); break; // already covered
            case VT_UI2: pElement = &variant.uiVal; break;
            case VT_UI4: pElement = &variant.ulVal; break;
            case VT_UINT: pElement = &variant.uintVal; break;
            case VT_CY: pElement = &variant.cyVal; break;
            case VT_R4: pElement = &variant.fltVal; break;
            case VT_R8: pElement = &variant.dblVal; break;
            case VT_DATE: pElement = &variant.date; break;
            case VT_VARIANT: Q_ASSERT(false); break; // already covered
            default:
                break;
            }
            if (!pElement) {
                var = list;
                break;
            }

            long lBound, uBound;
            SafeArrayGetLBound( array, 1, &lBound );
            SafeArrayGetUBound( array, 1, &uBound );

	    for ( long i = lBound; i <= uBound; ++i ) {
                variant.vt = vt;
		SafeArrayGetElement(array, &i, pElement);
		QVariant qvar = VARIANTToQVariant(variant, 0);
		clearVARIANT(&variant);
		list << qvar;
	    }

            var = list;
        }

        break;
    }
    
    QVariant::Type proptype = (QVariant::Type)type;
    if (proptype == QVariant::Invalid && !typeName.isEmpty()) {
        if (typeName != "QVariant")
            proptype = QVariant::nameToType(typeName);
    }
    if (proptype != QVariant::LastType && proptype != QVariant::Invalid && var.type() != proptype) {
        if (var.canConvert(proptype)) {
            QVariant oldvar = var;
            if (oldvar.convert(proptype))
                var = oldvar;
        } else if (proptype == QVariant::StringList && var.type() == QVariant::List) {
            bool allStrings = true;
            QStringList strings;
            const QList<QVariant> list(var.toList());
            for (QList<QVariant>::ConstIterator it(list.begin()); it != list.end(); ++it) {
                QVariant variant = *it;
                if (variant.canConvert(QVariant::String))
                    strings << variant.toString();
                else
                    allStrings = false;
            }
            if (allStrings)
                var = strings;
        } else {
            var = QVariant();
        }
    }
    return var;
}

void clearVARIANT(VARIANT *var)
{
    if (var->vt & VT_BYREF) {
        switch(var->vt) {
        case VT_BSTR|VT_BYREF:
            SysFreeString(*var->pbstrVal);
            delete var->pbstrVal;
            break;
        case VT_BOOL|VT_BYREF:
            delete var->pboolVal;
            break;
        case VT_I1|VT_BYREF:
            delete var->pcVal;
            break;
        case VT_I2|VT_BYREF:
            delete var->piVal;
            break;
        case VT_I4|VT_BYREF:
            delete var->plVal;
            break;
        case VT_INT|VT_BYREF:
            delete var->pintVal;
            break;
        case VT_UI1|VT_BYREF:
            delete var->pbVal;
            break;
        case VT_UI2|VT_BYREF:
            delete var->puiVal;
            break;
        case VT_UI4|VT_BYREF:
            delete var->pulVal;
            break;
        case VT_UINT|VT_BYREF:
            delete var->puintVal;
            break;
        case VT_CY|VT_BYREF:
            delete var->pcyVal;
            break;
        case VT_R4|VT_BYREF:
            delete var->pfltVal;
            break;
        case VT_R8|VT_BYREF:
            delete var->pdblVal;
            break;
        case VT_DATE|VT_BYREF:
            delete var->pdate;
            break;
        case VT_DISPATCH|VT_BYREF:
            (*var->ppdispVal)->Release();
            delete var->ppdispVal;
            break;
        case VT_ARRAY|VT_VARIANT|VT_BYREF:
        case VT_ARRAY|VT_UI1|VT_BYREF:
        case VT_ARRAY|VT_BSTR|VT_BYREF:
            SafeArrayDestroy(*var->pparray);
            delete var->pparray;
            break;
        case VT_VARIANT|VT_BYREF:
            delete var->pvarVal;
            break;
        }
        VariantInit(var);
    } else {
        VariantClear(var);
    }
}

QT_END_NAMESPACE
#endif // QT_NO_WIN_ACTIVEQT
