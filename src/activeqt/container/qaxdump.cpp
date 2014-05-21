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

#include "qaxbase.h"

#ifndef QT_NO_WIN_ACTIVEQT

#include <qmetaobject.h>
#include <quuid.h>
#include <qt_windows.h>
#include <qtextstream.h>

#include <ctype.h>

#include "../shared/qaxtypes.h"

QT_BEGIN_NAMESPACE

QString qax_docuFromName(ITypeInfo *typeInfo, const QString &name)
{
    QString docu;
    if (!typeInfo)
        return docu;

    MEMBERID memId;
    BSTR names = QStringToBSTR(name);
    typeInfo->GetIDsOfNames((BSTR*)&names, 1, &memId);
    SysFreeString(names);
    if (memId != DISPID_UNKNOWN) {
        BSTR docStringBstr, helpFileBstr;
        ulong helpContext;
        HRESULT hres = typeInfo->GetDocumentation(memId, 0, &docStringBstr, &helpContext, &helpFileBstr);
        QString docString = QString::fromWCharArray(docStringBstr);
        QString helpFile = QString::fromWCharArray(helpFileBstr);
        SysFreeString(docStringBstr);
        SysFreeString(helpFileBstr);
        if (hres == S_OK) {
            if (!docString.isEmpty())
                docu += docString + QLatin1String("\n");
            if (!helpFile.isEmpty())
                docu += QString::fromLatin1("For more information, see help context %1 in %2.").arg((uint)helpContext).arg(helpFile);
        }
    }

    return docu;
}

static inline QString docuFromName(ITypeInfo *typeInfo, const QString &name)
{
    return QLatin1String("<p>") + qax_docuFromName(typeInfo, name) + QLatin1String("\n");
}

static QByteArray namedPrototype(const QList<QByteArray> &parameterTypes, const QList<QByteArray> &parameterNames, int numDefArgs = 0)
{
    QByteArray prototype("(");
    for (int p = 0; p < parameterTypes.count(); ++p) {
        QByteArray type(parameterTypes.at(p));
        prototype += type;

        if (p < parameterNames.count())
            prototype += ' ' + parameterNames.at(p);
         
        if (numDefArgs >= parameterTypes.count() - p)
            prototype += " = 0";
        if (p < parameterTypes.count() - 1)
            prototype += ", ";
    }
    prototype += ')';

    return prototype;
}

static QByteArray toType(const QByteArray &t)
{
    QByteArray type = t;
    int vartype = QVariant::nameToType(type);
    if (vartype == QVariant::Invalid)
        type = "int";
    
    if (type.at(0) == 'Q')
        type = type.mid(1);
    type[0] = toupper(type.at(0));
    if (type == "VariantList")
        type = "List";
    else if (type == "Map<QVariant,QVariant>")
        type = "Map";
    else if (type == "Uint")
        type = "UInt";
    
    return "to" + type + "()";
}

QString qax_generateDocumentation(QAxBase *that)
{
    that->metaObject();

    if (that->isNull())
	return QString();

    ITypeInfo *typeInfo = 0;
    IDispatch *dispatch = 0;
    that->queryInterface(IID_IDispatch, (void**)&dispatch);
    if (dispatch)
	dispatch->GetTypeInfo(0, LOCALE_SYSTEM_DEFAULT, &typeInfo);

    QString docu;
    QTextStream stream(&docu, QIODevice::WriteOnly);

    const QMetaObject *mo = that->metaObject();
    QString coClass  = QLatin1String(mo->classInfo(mo->indexOfClassInfo("CoClass")).value());

    stream << "<h1 align=center>" << coClass << " Reference</h1>" << endl;
    stream << "<p>The " << coClass << " COM object is a " << that->qObject()->metaObject()->className();
    stream << " with the CLSID " <<  that->control() << ".</p>" << endl;

    stream << "<h3>Interfaces</h3>" << endl;
    stream << "<ul>" << endl;
    const char *inter = 0;
    int interCount = 1;
    while ((inter = mo->classInfo(mo->indexOfClassInfo("Interface " + QByteArray::number(interCount))).value())) {
	stream << "<li>" << inter << endl;
	interCount++;
    }
    stream << "</ul>" << endl;

    stream << "<h3>Event Interfaces</h3>" << endl;
    stream << "<ul>" << endl;
    interCount = 1;  
    while ((inter = mo->classInfo(mo->indexOfClassInfo("Event Interface " + QByteArray::number(interCount))).value())) {
	stream << "<li>" << inter << endl;
	interCount++;
    }
    stream << "</ul>" << endl;

    QList<QString> methodDetails, propDetails;

    const int slotCount = mo->methodCount();
    if (slotCount) {
	stream << "<h2>Public Slots:</h2>" << endl;
	stream << "<ul>" << endl;

        int defArgCount = 0;
	for (int islot = mo->methodOffset(); islot < slotCount; ++islot) {
	    const QMetaMethod slot = mo->method(islot);
            if (slot.methodType() != QMetaMethod::Slot)
                continue;

            if (slot.attributes() & QMetaMethod::Cloned) {
                ++defArgCount;
                continue;
            }

	    QByteArray returntype(slot.typeName());
            if (returntype.isEmpty())
                returntype = "void";
            QByteArray prototype = namedPrototype(slot.parameterTypes(), slot.parameterNames(), defArgCount);
            QByteArray signature = slot.signature();
	    QByteArray name = signature.left(signature.indexOf('('));
	    stream << "<li>" << returntype << " <a href=\"#" << name << "\"><b>" << name << "</b></a>" << prototype << ";</li>" << endl;
            
            prototype = namedPrototype(slot.parameterTypes(), slot.parameterNames());
	    QString detail = QString::fromLatin1("<h3><a name=") + QString::fromLatin1(name.constData()) + QLatin1String("></a>") +
                             QLatin1String(returntype.constData()) + QLatin1Char(' ') +
                             QLatin1String(name.constData()) + QLatin1Char(' ') +
                             QString::fromLatin1(prototype.constData()) + QLatin1String("<tt> [slot]</tt></h3>\n");
            prototype = namedPrototype(slot.parameterTypes(), QList<QByteArray>());
	    detail += docuFromName(typeInfo, QString::fromLatin1(name.constData()));
	    detail += QLatin1String("<p>Connect a signal to this slot:<pre>\n");
	    detail += QString::fromLatin1("\tQObject::connect(sender, SIGNAL(someSignal") + QString::fromLatin1(prototype.constData()) +
                      QLatin1String("), object, SLOT(") + QString::fromLatin1(name.constData()) +
                      QString::fromLatin1(prototype.constData()) + QLatin1String("));");
	    detail += QLatin1String("</pre>\n");

            if (1) {
                detail += QLatin1String("<p>Or call the function directly:<pre>\n");

                bool hasParams = slot.parameterTypes().count() != 0;
                if (hasParams)
                    detail += QLatin1String("\tQVariantList params = ...\n");
                detail += QLatin1String("\t");
                QByteArray functionToCall = "dynamicCall";
                if (returntype == "IDispatch*" || returntype == "IUnknown*") {
                    functionToCall = "querySubObject";
                    returntype = "QAxObject *";
                }
                if (returntype != "void")
                    detail += QLatin1String(returntype.constData()) + QLatin1String(" result = ");
                detail += QLatin1String("object->") + QLatin1String(functionToCall.constData()) +
                          QLatin1String("(\"" + name + prototype + '\"');
                if (hasParams)
                    detail += QLatin1String(", params");
                detail += QLatin1Char(')');
                if (returntype != "void" && returntype != "QAxObject *" && returntype != "QVariant")
                    detail += QLatin1Char('.') + QLatin1String(toType(returntype));
	        detail += QLatin1String(";</pre>\n");
	    } else {
		detail += QLatin1String("<p>This function has parameters of unsupported types and cannot be called directly.");
	    }

	    methodDetails << detail;
            defArgCount = 0;
	}

	stream << "</ul>" << endl;
    }
    int signalCount = mo->methodCount();
    if (signalCount) {
        ITypeLib *typeLib = 0;
        if (typeInfo) {
            UINT index = 0;
            typeInfo->GetContainingTypeLib(&typeLib, &index);
            typeInfo->Release();
        }
        typeInfo = 0;

	stream << "<h2>Signals:</h2>" << endl;
	stream << "<ul>" << endl;

	for (int isignal = mo->methodOffset(); isignal < signalCount; ++isignal) {
	    const QMetaMethod signal(mo->method(isignal));
            if (signal.methodType() != QMetaMethod::Signal)
                continue;

            QByteArray prototype = namedPrototype(signal.parameterTypes(), signal.parameterNames());
	    QByteArray signature = signal.signature();
	    QByteArray name = signature.left(signature.indexOf('('));
	    stream << "<li>void <a href=\"#" << name << "\"><b>" << name << "</b></a>" << prototype << ";</li>" << endl;

            QString detail = QLatin1String("<h3><a name=") + QLatin1String(name.constData()) + QLatin1String("></a>void ") +
                             QLatin1String(name.constData()) + QLatin1Char(' ') +
                             QLatin1String(prototype.constData()) + QLatin1String("<tt> [signal]</tt></h3>\n");
            if (typeLib) {
                interCount = 0;
                do {
                    if (typeInfo)
                        typeInfo->Release();
                    typeInfo = 0;
                    typeLib->GetTypeInfo(++interCount, &typeInfo);
                    QString typeLibDocu = docuFromName(typeInfo, QString::fromLatin1(name.constData()));
                    if (!typeLibDocu.isEmpty()) {
                        detail += typeLibDocu;
                        break;
                    }
                } while (typeInfo);
            }
            prototype = namedPrototype(signal.parameterTypes(), QList<QByteArray>());
	    detail += QLatin1String("<p>Connect a slot to this signal:<pre>\n");
	    detail += QLatin1String("\tQObject::connect(object, SIGNAL(") + QString::fromLatin1(name.constData()) +
                      QString::fromLatin1(prototype.constData()) +
                      QLatin1String("), receiver, SLOT(someSlot") + QString::fromLatin1(prototype.constData()) + QLatin1String("));");
	    detail += QLatin1String("</pre>\n");

	    methodDetails << detail;
            if (typeInfo)
                typeInfo->Release();
            typeInfo = 0;
	}
	stream << "</ul>" << endl;

        if (typeLib)
            typeLib->Release();
    }

    const int propCount = mo->propertyCount();
    if (propCount) {
        if (dispatch)
	    dispatch->GetTypeInfo(0, LOCALE_SYSTEM_DEFAULT, &typeInfo);
	stream << "<h2>Properties:</h2>" << endl;
	stream << "<ul>" << endl;

	for (int iprop = 0; iprop < propCount; ++iprop) {
	    const QMetaProperty prop = mo->property(iprop);
	    QByteArray name(prop.name());
	    QByteArray type(prop.typeName());

	    stream << "<li>" << type << " <a href=\"#" << name << "\"><b>" << name << "</b></a>;</li>" << endl;
	    QString detail = QLatin1String("<h3><a name=") + QString::fromLatin1(name.constData()) + QLatin1String("></a>") +
                             QLatin1String(type.constData()) +
		             QLatin1Char(' ') + QLatin1String(name.constData()) + QLatin1String("</h3>\n");
	    detail += docuFromName(typeInfo, QString::fromLatin1(name));
	    QVariant::Type vartype = QVariant::nameToType(type);
	    if (!prop.isReadable())
		continue;

	    if (prop.isEnumType())
		vartype = QVariant::Int;

            if (vartype != QVariant::Invalid) {
		detail += QLatin1String("<p>Read this property's value using QObject::property:<pre>\n");
                if (prop.isEnumType())
		    detail += QLatin1String("\tint val = ");
                else
                    detail += QLatin1Char('\t') + QLatin1String(type.constData()) + QLatin1String(" val = ");
		detail += QLatin1String("object->property(\"") + QLatin1String(name.constData()) +
                          QLatin1String("\").") + QLatin1String(toType(type).constData()) + QLatin1String(";\n");
		detail += QLatin1String("</pre>\n");
	    } else if (type == "IDispatch*" || type == "IUnknown*") {
		detail += QLatin1String("<p>Get the subobject using querySubObject:<pre>\n");
		detail += QLatin1String("\tQAxObject *") + QLatin1String(name.constData()) +
                          QLatin1String(" = object->querySubObject(\"") + QLatin1String(name.constData()) + QLatin1String("\");\n");
		detail += QLatin1String("</pre>\n");
	    } else {
		detail += QLatin1String("<p>This property is of an unsupported type.\n");
	    }
	    if (prop.isWritable()) {
		detail += QLatin1String("Set this property' value using QObject::setProperty:<pre>\n");
                if (prop.isEnumType()) {
                    detail += QLatin1String("\tint newValue = ... // string representation of values also supported\n");
                } else {
		    detail += QLatin1String("\t") + QString::fromLatin1(type.constData()) + QLatin1String(" newValue = ...\n");
                }
		detail += QLatin1String("\tobject->setProperty(\"") + QString::fromLatin1(name) + QLatin1String("\", newValue);\n");
		detail += QLatin1String("</pre>\n");
		detail += QLatin1String("Or using the ");
		QByteArray setterSlot;
                if (isupper(name.at(0))) {
		    setterSlot = "Set" + name;
		} else {
		    QByteArray nameUp = name;
		    nameUp[0] = toupper(nameUp.at(0));
		    setterSlot = "set" + nameUp;
		}
		detail += QLatin1String("<a href=\"#") + QString::fromLatin1(setterSlot) + QLatin1String("\">") +
                          QString::fromLatin1(setterSlot.constData()) + QLatin1String("</a> slot.\n");
	    }
	    if (prop.isEnumType()) {
		detail += QLatin1String("<p>See also <a href=\"#") + QString::fromLatin1(type) +
                QLatin1String("\">") + QString::fromLatin1(type) + QLatin1String("</a>.\n");
	    }

	    propDetails << detail;
	}
	stream << "</ul>" << endl;
    }

    const int enumCount = mo->enumeratorCount();
    if (enumCount) {
	stream << "<hr><h2>Member Type Documentation</h2>" << endl;
	for (int i = 0; i < enumCount; ++i) {
	    const QMetaEnum enumdata = mo->enumerator(i);
	    stream << "<h3><a name=" << enumdata.name() << "></a>" << enumdata.name() << "</h3>" << endl;
	    stream << "<ul>" << endl;
	    for (int e = 0; e < enumdata.keyCount(); ++e) {
		stream << "<li>" << enumdata.key(e) << "\t=" << enumdata.value(e) << "</li>" << endl;
	    }
	    stream << "</ul>" << endl;
	}
    }
    if (methodDetails.count()) {
	stream << "<hr><h2>Member Function Documentation</h2>" << endl;
	for (int i = 0; i < methodDetails.count(); ++i)
	    stream << methodDetails.at(i) << endl;
    }
    if (propDetails.count()) {
	stream << "<hr><h2>Property Documentation</h2>" << endl;
	for (int i = 0; i < propDetails.count(); ++i)
	    stream << propDetails.at(i) << endl;
    }

    if (typeInfo)
        typeInfo->Release();
    if (dispatch)
        dispatch->Release();
    return docu;
}

QT_END_NAMESPACE
#endif // QT_NO_WIN_ACTIVEQT
