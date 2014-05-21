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

#ifndef QAXTYPES_H
#define QAXTYPES_H

#if !defined(_WINDOWS_) && !defined(_WINDOWS_H) && !defined(__WINDOWS__)
#error Must include windows.h first!
#endif

#include <QtGui/qcolor.h>
#include <QtGui/qfont.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qvariant.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(ActiveQt)

#ifndef QT_NO_WIN_ACTIVEQT

extern GUID IID_IAxServerBase;
struct IAxServerBase : public IUnknown
{
    virtual IUnknown *clientSite() const = 0;
    virtual void emitPropertyChanged(const char*) = 0;
    virtual bool emitRequestPropertyChange(const char*) = 0;
    virtual QObject *qObject() const = 0;
    virtual void reportError(int code, const QString &src, const QString &desc, const QString &context) = 0;
};

#define HIMETRIC_PER_INCH   2540
#define MAP_PIX_TO_LOGHIM(x,ppli)   ((HIMETRIC_PER_INCH*(x) + ((ppli)>>1)) / (ppli))
#define MAP_LOGHIM_TO_PIX(x,ppli)   (((ppli)*(x) + HIMETRIC_PER_INCH/2) / HIMETRIC_PER_INCH)
#define QAX_NUM_PARAMS 8

static inline BSTR QStringToBSTR(const QString &str)
{
    return SysAllocStringLen((OLECHAR*)str.unicode(), str.length());
}

static inline uint QColorToOLEColor(const QColor &col)
{
    return qRgba(col.blue(), col.green(), col.red(), 0x00);
}

extern QColor OLEColorToQColor(uint col);

extern bool QVariantToVARIANT(const QVariant &var, VARIANT &arg, const QByteArray &typeName = 0, bool out = false);
extern QVariant VARIANTToQVariant(const VARIANT &arg, const QByteArray &typeName, uint type = 0);
extern bool QVariantToVoidStar(const QVariant &var, void *data, const QByteArray &typeName, uint type = 0);
extern void clearVARIANT(VARIANT *var);

#define QAX_INPROC_SERVER  (0x51540001)
#define QAX_OUTPROC_SERVER (0x51540002)

QT_END_NAMESPACE
#endif // QT_NO_WIN_ACTIVEQT

QT_END_HEADER

#endif // QAXTYPES_H
