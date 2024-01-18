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

#include "config.h"
#include "qt_runtime.h"

#include "BooleanObject.h"
#include "DateInstance.h"
#include "DateMath.h"
#include "DatePrototype.h"
#include "DumpRenderTreeSupportQt.h"
#include "FunctionPrototype.h"
#include "Interpreter.h"
#include "JSArray.h"
#include "JSByteArray.h"
#include "JSDocument.h"
#include "JSDOMBinding.h"
#include "JSDOMWindow.h"
#include <JSFunction.h>
#include "JSGlobalObject.h"
#include "JSHTMLElement.h"
#include "JSLock.h"
#include "JSObject.h"
#include "ObjectPrototype.h"
#include "PropertyNameArray.h"
#include "RegExpConstructor.h"
#include "RegExpObject.h"
#include "qdatetime.h"
#include "qdebug.h"
#include "qmetaobject.h"
#include "qobject.h"
#include "qstringlist.h"
#include "qt_instance.h"
#include "qt_pixmapruntime.h"
#include "qvarlengtharray.h"
#include "qwebelement.h"
#include <limits.h>
#include <runtime/Error.h>
#include <runtime_array.h>
#include <runtime_object.h>

#include <qstring16.h>
#include <qtimezone.h>

using namespace WebCore;

namespace JSC {
namespace Bindings {

// Debugging
//#define QTWK_RUNTIME_CONVERSION_DEBUG
//#define QTWK_RUNTIME_MATCH_DEBUG

class QWKNoDebug
{
public:
    inline QWKNoDebug(){}
    inline ~QWKNoDebug(){}

    template<typename T>
    inline QWKNoDebug &operator<<(const T &) { return *this; }
};

#ifdef QTWK_RUNTIME_CONVERSION_DEBUG
#define qConvDebug() qDebug()
#else
#define qConvDebug() QWKNoDebug()
#endif

#ifdef QTWK_RUNTIME_MATCH_DEBUG
#define qMatchDebug() qDebug()
#else
#define qMatchDebug() QWKNoDebug()
#endif

typedef enum {
    Variant = 0,
    Number,
    Boolean,
    String,
    Date,
    RegExp,
    Array,
    QObj,
    Object,
    Null,
    RTArray,
    JSByteArray
} JSRealType;

#if defined(QTWK_RUNTIME_CONVERSION_DEBUG) || defined(QTWK_RUNTIME_MATCH_DEBUG)
QDebug operator<<(QDebug dbg, const JSRealType &c)
{
     const char *map[] = { "Variant", "Number", "Boolean", "String", "Date",
         "RegExp", "Array", "RTObject", "Object", "Null", "RTArray"};

     dbg.nospace() << "JSType(" << ((int)c) << ", " <<  map[c] << ")";

     return dbg.space();
}
#endif

// this is here as a proxy, so we'd have a class to friend in QWebElement,
// as getting/setting a WebCore in QWebElement is private
class QtWebElementRuntime {
public:
    static QWebElement create(Element* element)
    {
        return QWebElement(element);
    }

    static Element* get(const QWebElement& element)
    {
        return element.m_element;
    }
};

// this is here as a proxy, so we'd have a class to friend in QDRTNode,
// as getting/setting a WebCore in QDRTNode is private.
// We only need to pass WebCore Nodes for layout tests.
class QtDRTNodeRuntime {
public:
    static QDRTNode create(Node* node)
    {
        return QDRTNode(node);
    }

    static Node* get(const QDRTNode& node)
    {
        return node.m_node;
    }
};

static JSRealType valueRealType(ExecState* exec, JSValue val)
{
    if (val.isNumber())
        return Number;

    else if (val.isString())
        return String;

    else if (val.isBoolean())
        return Boolean;

    else if (val.isNull())
        return Null;

    else if (isJSByteArray(&exec->globalData(), val))
        return JSByteArray;

    else if (val.isObject()) {
        JSObject *object = val.toObject(exec);

        if (object->inherits(&RuntimeArray::s_info))  // RuntimeArray 'inherits' from Array, but not in C++
            return RTArray;

        else if (object->inherits(&JSArray::s_info))
            return Array;

        else if (object->inherits(&DateInstance::s_info))
            return Date;

        else if (object->inherits(&RegExpObject::s_info))
            return RegExp;

        else if (object->inherits(&RuntimeObject::s_info))
            return QObj;

        return Object;
    }

    return String; // I don't know.
}

QVariant convertValueToQVariant(ExecState*, JSValue, QVariant::Type, int*, HashSet<JSObject*>*, int);

static QVariantMap convertValueToQVariantMap(ExecState* exec, JSObject* object, HashSet<JSObject*>* visitedObjects, int recursionLimit)
{
    Q_ASSERT(!exec->hadException());

    PropertyNameArray properties(exec);
    object->getPropertyNames(exec, properties);
    PropertyNameArray::const_iterator it = properties.begin();

    QVariantMap result;
    int objdist = 0;

    while (it != properties.end()) {
        if (object->propertyIsEnumerable(exec, *it)) {
            JSValue val = object->get(exec, *it);
            if (exec->hadException())
                exec->clearException();
            else {
                QVariant v = convertValueToQVariant(exec, val, QVariant::Void, &objdist, visitedObjects, recursionLimit);
                if (objdist >= 0) {
                    UString ustring = (*it).ustring();
                    QString id = QString((const QChar*)ustring.impl()->characters(), ustring.length());
                    result.insert(id, v);
                }
            }
        }
        ++it;
    }
    return result;
}

QVariant convertValueToQVariant(ExecState* exec, JSValue value, QVariant::Type hint, int *distance,
                  HashSet<JSObject*>* visitedObjects, int recursionLimit)
{
    --recursionLimit;

    if (!value || !recursionLimit)
        return QVariant();

    JSObject* object = 0;
    if (value.isObject()) {
        object = value.toObject(exec);
        if (visitedObjects->contains(object))
            return QVariant();

        visitedObjects->add(object);
    }

    // check magic pointer values before dereferencing value
    if (value == jsNaN() || (value == jsUndefined() && hint != QVariant::String && hint != QVariant::Variant)) {

        if (distance) {
            *distance = -1;
        }

        return QVariant();
    }

    JSLock lock(SilenceAssertionsOnly);
    JSRealType type = valueRealType(exec, value);

    if (hint == QVariant::Void) {
        switch(type) {
            case Number:
                hint = QVariant::Double;
                break;

            case Boolean:
                hint = QVariant::Bool;
                break;

            case String:
            default:
                hint = QVariant::String;
                break;

            case Date:
                hint = QVariant::DateTime;
                break;

            case RegExp:
                hint = QVariant::RegularExpression;
                break;

            case Object:
                if (object->inherits(&NumberObject::s_info))
                    hint = QVariant::Double;

                else if (object->inherits(&BooleanObject::s_info))
                    hint = QVariant::Bool;

                else
                    hint = QVariant::Map;

                break;

            case QObj:
                hint = QVariant::ObjectStar;
                break;

            case JSByteArray:
                hint = QVariant::ByteArray;
                break;

            case Array:
            case RTArray:
                hint = QVariant::List;
                break;
        }
    }

    qConvDebug() << "convertValueToQVariant: jstype is " << type << ", hint is" << hint;

    if (value == jsNull()
        && hint != QVariant::ObjectStar
        && hint != QVariant::VoidStar
        && hint != QVariant::String
        && hint != QVariant::Variant) {
        if (distance) {
            *distance = -1;
        }

        return QVariant();
    }

    QVariant ret;
    int dist = -1;

    switch (hint) {
        case QVariant::Bool:
            if (type == Object && object->inherits(&BooleanObject::s_info))
                ret = QVariant(asBooleanObject(value)->internalValue().toBoolean(exec));
            else
                ret = QVariant(value.toBoolean(exec));
            if (type == Boolean)
                dist = 0;
            else
                dist = 10;
            break;

        case QVariant::Int:
        case QVariant::UInt:
        case QVariant::Long:
        case QVariant::ULong:
        case QVariant::LongLong:
        case QVariant::ULongLong:
        case QVariant::Short:
        case QVariant::UShort:
        case QVariant::Float:
        case QVariant::Double:
            ret = QVariant(value.toNumber(exec));
            ret.convert(hint);

            if (type == Number) {
                switch (hint) {
                case QVariant::Double:
                    dist = 0;
                    break;

                case QVariant::Float:
                    dist = 1;
                    break;

                case QVariant::LongLong:
                case QVariant::ULongLong:
                    dist = 2;
                    break;

                case QVariant::Long:
                case QVariant::ULong:
                    dist = 3;
                    break;
                case QVariant::Int:
                case QVariant::UInt:
                    dist = 4;
                    break;
                case QVariant::Short:
                case QVariant::UShort:
                    dist = 5;
                    break;
                    break;
                default:
                    dist = 10;
                    break;
                }
            } else {
                dist = 10;
            }
            break;

        case QVariant::QChar:
            if (type == Number || type == Boolean) {
                ret = QVariant(QChar((ushort)value.toNumber(exec)));
                if (type == Boolean)
                    dist = 3;
                else
                    dist = 6;
            } else {
                UString str = value.toString(exec);
                ret = QVariant(QChar(str.length() ? *(const ushort*)str.impl()->characters() : 0));
                if (type == String)
                    dist = 3;
                else
                    dist = 10;
            }
            break;

        case QVariant::String: {
            if (value.isUndefinedOrNull()) {
                if (distance)
                    *distance = 1;
                return QString();
            } else {
                UString ustring = value.toString(exec);
                ret = QVariant(QString((const QChar*)ustring.impl()->characters(), ustring.length()));
                if (type == String)
                    dist = 0;
                else
                    dist = 10;
            }
            break;
        }

        case QVariant::Map:
            if (type == Object || type == Array || type == RTArray) {
                ret = QVariant(convertValueToQVariantMap(exec, object, visitedObjects, recursionLimit));
                // Those types can still have perfect matches, e.g. 'bool' if value is a Boolean Object.
                dist = 1;
            }
            break;

        case QVariant::List:
            if (type == RTArray) {
                RuntimeArray* rtarray = static_cast<RuntimeArray*>(object);

                QVariantList result;
                int len = rtarray->getLength();
                int objdist = 0;
                qConvDebug() << "converting a " << len << " length Array";
                for (int i = 0; i < len; ++i) {
                    JSValue val = rtarray->getConcreteArray()->valueAt(exec, i);
                    result.append(convertValueToQVariant(exec, val, QVariant::Void, &objdist, visitedObjects, recursionLimit));
                    if (objdist == -1) {
                        qConvDebug() << "Failed converting element at index " << i;
                        break; // Failed converting a list entry, so fail the array
                    }
                }
                if (objdist != -1) {
                    dist = 5;
                    ret = QVariant(result);
                }

            } else if (type == Array) {
                JSArray* array = static_cast<JSArray*>(object);

                QVariantList result;
                int len = array->length();
                int objdist = 0;
                qConvDebug() << "converting a " << len << " length Array";
                for (int i = 0; i < len; ++i) {
                    JSValue val = array->get(exec, i);
                    result.append(convertValueToQVariant(exec, val, QVariant::Void, &objdist, visitedObjects, recursionLimit));
                    if (objdist == -1) {
                        qConvDebug() << "Failed converting element at index " << i;
                        break; // Failed converting a list entry, so fail the array
                    }
                }
                if (objdist != -1) {
                    dist = 5;
                    ret = QVariant(result);
                }

            } else {
                // Make a single length array
                int objdist;
                qConvDebug() << "making a single length variantlist";
                QVariant var = convertValueToQVariant(exec, value, QVariant::Void, &objdist, visitedObjects, recursionLimit);
                if (objdist != -1) {
                    QVariantList result;
                    result << var;
                    ret = QVariant(result);
                    dist = 10;
                } else {
                    qConvDebug() << "failed making single length varlist";
                }
            }
            break;

        case QVariant::StringList: {
            if (type == RTArray) {
                RuntimeArray* rtarray = static_cast<RuntimeArray*>(object);

                QStringList result;
                int len = rtarray->getLength();
                for (int i = 0; i < len; ++i) {
                    JSValue val = rtarray->getConcreteArray()->valueAt(exec, i);
                    UString ustring = val.toString(exec);
                    QString qstring = QString((const QChar*)ustring.impl()->characters(), ustring.length());

                    result.append(qstring);
                }
                dist = 5;
                ret = QVariant(result);

            } else if (type == Array) {
                JSArray* array = static_cast<JSArray*>(object);

                QStringList result;
                int len = array->length();
                for (int i = 0; i < len; ++i) {
                    JSValue val = array->get(exec, i);
                    UString ustring = val.toString(exec);
                    QString qstring = QString((const QChar*)ustring.impl()->characters(), ustring.length());

                    result.append(qstring);
                }
                dist = 5;
                ret = QVariant(result);

            } else {
                // Make a single length array
                UString ustring = value.toString(exec);
                QString qstring = QString((const QChar*)ustring.impl()->characters(), ustring.length());
                QStringList result;
                result.append(qstring);
                ret = QVariant(result);
                dist = 10;
            }
            break;
        }

        case QVariant::ByteArray: {
            if (type == JSByteArray) {
                WTF::ByteArray* arr = asByteArray(value)->storage();
                ret = QVariant(QByteArray(reinterpret_cast<const char*>(arr->data()), arr->length()));
                dist = 0;
            } else {
                UString ustring = value.toString(exec);
                ret = QVariant(QString((const QChar*)ustring.impl()->characters(), ustring.length()).toLatin1());
                if (type == String)
                    dist = 5;
                else
                    dist = 10;
            }
            break;
        }

        case QVariant::DateTime:
        case QVariant::Date:
        case QVariant::Time:
            if (type == Date) {
                DateInstance* date = static_cast<DateInstance*>(object);
                GregorianDateTime gdt;
                msToGregorianDateTime(exec, date->internalNumber(), true, gdt);

                if (hint == QVariant::DateTime) {
                    ret = QDateTime(QDate(gdt.year + 1900, gdt.month + 1, gdt.monthDay), QTime(gdt.hour, gdt.minute, gdt.second), QTimeZone::utc());
                    dist = 0;

                } else if (hint == QVariant::Date) {
                    ret = QDate(gdt.year + 1900, gdt.month + 1, gdt.monthDay);
                    dist = 1;
                } else {
                    ret = QTime(gdt.hour + 1900, gdt.minute, gdt.second);
                    dist = 2;
                }

            } else if (type == Number) {
                double b = value.toNumber(exec);
                GregorianDateTime gdt;
                msToGregorianDateTime(exec, b, true, gdt);

                if (hint == QVariant::DateTime) {
                    ret = QDateTime(QDate(gdt.year + 1900, gdt.month + 1, gdt.monthDay), QTime(gdt.hour, gdt.minute, gdt.second), QTimeZone::utc());
                    dist = 6;
                } else if (hint == QVariant::Date) {
                    ret = QDate(gdt.year + 1900, gdt.month + 1, gdt.monthDay);
                    dist = 8;
                } else {
                    ret = QTime(gdt.hour, gdt.minute, gdt.second);
                    dist = 10;
                }

            } else if (type == String) {
                UString ustring = value.toString(exec);
                QString qstring = QString((const QChar*)ustring.impl()->characters(), ustring.length());

                if (hint == QVariant::DateTime) {
                    QDateTime dt = QDateTime::fromString(qstring, Qt::ISODate);

                    if (!dt.isValid())
                        dt = QDateTime::fromString(qstring, Qt::TextDate);
                    if (!dt.isValid())
                        dt = QDateTime::fromString(qstring, Qt::SystemLocaleDate);
                    if (!dt.isValid())
                        dt = QDateTime::fromString(qstring, Qt::LocaleDate);
                    if (dt.isValid()) {
                        ret = dt;
                        dist = 2;
                    }

                } else if (hint == QVariant::Date) {
                    QDate dt = QDate::fromString(qstring, Qt::ISODate);

                    if (!dt.isValid())
                        dt = QDate::fromString(qstring, Qt::TextDate);
                    if (!dt.isValid())
                        dt = QDate::fromString(qstring, Qt::SystemLocaleDate);
                    if (!dt.isValid())
                        dt = QDate::fromString(qstring, Qt::LocaleDate);
                    if (dt.isValid()) {
                        ret = dt;
                        dist = 3;
                    }
                } else {
                    QTime dt = QTime::fromString(qstring, Qt::ISODate);

                    if (!dt.isValid())
                        dt = QTime::fromString(qstring, Qt::TextDate);
                    if (!dt.isValid())
                        dt = QTime::fromString(qstring, Qt::SystemLocaleDate);
                    if (!dt.isValid())
                        dt = QTime::fromString(qstring, Qt::LocaleDate);
                    if (dt.isValid()) {
                        ret = dt;
                        dist = 3;
                    }
                }
            }
            break;

        case QVariant::RegularExpression:
            if (type == RegExp) {

                // Attempt to convert.. a bit risky
                UString ustring = value.toString(exec);
                QString qstring = QString((const QChar*)ustring.impl()->characters(), ustring.length());

                // this is of the form '/xxxxxx/i'
                int firstSlash = qstring.indexOf('/');
                int lastSlash  = qstring.lastIndexOf('/');

                if (firstSlash >= 0 && lastSlash > firstSlash) {
                    QPatternOptionFlags flags = QPatternOption::NoPatternOption;

                    if (qstring.mid(lastSlash + 1).contains('i')) {
                       flags |= QPatternOption::CaseInsensitiveOption;
                    }

                    QRegularExpression realReg(qstring.mid(firstSlash + 1, lastSlash - firstSlash - 1), flags);

                    ret = QVariant::fromValue(realReg);
                    dist = 0;

                } else {
                    qConvDebug() << "could not parse a JS regexp";
                }

            } else if (type == String) {
                UString ustring = value.toString(exec);
                QString qstring = QString((const QChar*)ustring.impl()->characters(), ustring.length());

                QRegularExpression re(qstring);

                if (re.isValid()) {
                    ret  = QVariant::fromValue(re);
                    dist = 10;
                }
            }

            break;

        case QVariant::ObjectStar:
            if (type == QObj) {
                QtInstance* qtinst = QtInstance::getInstance(object);

                if (qtinst) {
                    if (qtinst->getObject()) {
                        qConvDebug() << "found instance, with object:" << (void*) qtinst->getObject();
                        ret = QVariant::fromValue(qtinst->getObject());
                        qConvDebug() << ret;
                        dist = 0;

                    } else {
                        qConvDebug() << "can not convert deleted qobject";
                    }

                } else {
                    qConvDebug() << "was not a qtinstance";
                }

            } else if (type == Null) {
                QObject* nullobj = 0;
                ret = QVariant::fromValue(nullobj);
                dist = 0;
            } else {
                qConvDebug() << "previous type was not an object:" << type;
            }
            break;

        case QVariant::VoidStar:
            if (type == QObj) {
                QtInstance* qtinst = QtInstance::getInstance(object);
                if (qtinst) {
                    if (qtinst->getObject()) {
                        qConvDebug() << "found instance, with object:" << (void*) qtinst->getObject();
                        ret = QVariant::fromValue((void *)qtinst->getObject());
                        qConvDebug() << ret;
                        dist = 0;
                    } else {
                        qConvDebug() << "can't convert deleted qobject";
                    }
                } else {
                    qConvDebug() << "wasn't a qtinstance";
                }
            } else if (type == Null) {
                ret = QVariant::fromValue((void*)0);
                dist = 0;
            } else if (type == Number) {
                // I don't think that converting a double to a pointer is a wise
                // move.  Except maybe 0.
                qConvDebug() << "got number for void * - not converting, seems unsafe:" << value.toNumber(exec);
            } else {
                qConvDebug() << "void* - unhandled type" << type;
            }
            break;

        default:
            // Non const type ids
            if (hint == QVariant::typeToTypeId<QObjectList>()) {

                if (type == RTArray) {
                    RuntimeArray* rtarray = static_cast<RuntimeArray*>(object);

                    QObjectList result;
                    int len = rtarray->getLength();
                    for (int i = 0; i < len; ++i) {
                        JSValue val = rtarray->getConcreteArray()->valueAt(exec, i);
                        int itemdist = -1;
                        QVariant item = convertValueToQVariant(exec, val, QVariant::ObjectStar, &itemdist, visitedObjects, recursionLimit);
                        if (itemdist >= 0)
                            result.append(item.value<QObject*>());
                        else
                            break;
                    }
                    // If we didn't fail conversion
                    if (result.count() == len) {
                        dist = 5;
                        ret = QVariant::fromValue(result);
                    }

                } else if (type == Array) {
                    JSObject* object = value.toObject(exec);
                    JSArray* array = static_cast<JSArray *>(object);
                    QObjectList result;
                    int len = array->length();
                    for (int i = 0; i < len; ++i) {
                        JSValue val = array->get(exec, i);
                        int itemdist = -1;
                        QVariant item = convertValueToQVariant(exec, val, QVariant::ObjectStar, &itemdist, visitedObjects, recursionLimit);
                        if (itemdist >= 0)
                            result.append(item.value<QObject*>());
                        else
                            break;
                    }
                    // If we didn't fail conversion
                    if (result.count() == len) {
                        dist = 5;
                        ret = QVariant::fromValue(result);
                    }
                } else {
                    // Make a single length array
                    QObjectList result;
                    int itemdist = -1;
                    QVariant item = convertValueToQVariant(exec, value, QVariant::ObjectStar, &itemdist, visitedObjects, recursionLimit);
                    if (itemdist >= 0) {
                        result.append(item.value<QObject*>());
                        dist = 10;
                        ret = QVariant::fromValue(result);
                    }
                }
                break;

            } else if (hint == QVariant::typeToTypeId<QList<int>>()) {

                if (type == RTArray) {
                    RuntimeArray* rtarray = static_cast<RuntimeArray*>(object);

                    QList<int> result;
                    int len = rtarray->getLength();

                    for (int i = 0; i < len; ++i) {
                        JSValue val = rtarray->getConcreteArray()->valueAt(exec, i);
                        int itemdist = -1;
                        QVariant item = convertValueToQVariant(exec, val, QVariant::Int, &itemdist, visitedObjects, recursionLimit);
                        if (itemdist >= 0)
                            result.append(item.value<int>());
                        else
                            break;
                    }
                    // If we didn't fail conversion
                    if (result.count() == len) {
                        dist = 5;
                        ret = QVariant::fromValue(result);
                    }
                } else if (type == Array) {
                    JSArray* array = static_cast<JSArray *>(object);

                    QList<int> result;
                    int len = array->length();
                    for (int i = 0; i < len; ++i) {
                        JSValue val = array->get(exec, i);
                        int itemdist = -1;
                        QVariant item = convertValueToQVariant(exec, val, QVariant::Int, &itemdist, visitedObjects, recursionLimit);
                        if (itemdist >= 0)
                            result.append(item.value<int>());
                        else
                            break;
                    }
                    // If we didn't fail conversion
                    if (result.count() == len) {
                        dist = 5;
                        ret = QVariant::fromValue(result);
                    }
                } else {
                    // Make a single length array
                    QList<int> result;
                    int itemdist = -1;
                    QVariant item = convertValueToQVariant(exec, value, QVariant::Int, &itemdist, visitedObjects, recursionLimit);
                    if (itemdist >= 0) {
                        result.append(item.value<int>());
                        dist = 10;
                        ret = QVariant::fromValue(result);
                    }
                }
                break;

            } else if (QtPixmapInstance::canHandle(hint)) {
                ret = QtPixmapInstance::variantFromObject(object, hint);

            } else if (hint == QVariant::typeToTypeId<QWebElement>()) {
                if (object && object->inherits(&JSElement::s_info)) {
                    ret = QVariant::fromValue<QWebElement>(QtWebElementRuntime::create((static_cast<JSElement*>(object))->impl()));
                    dist = 0;
                    // Allow other objects to reach this one. This won't cause our algorithm to
                    // loop since when we find an Element we do not recurse.
                    visitedObjects->remove(object);
                    break;
                }

                if (object && object->inherits(&JSDocument::s_info)) {
                    // To support LayoutTestControllerQt::nodesFromRect(), used in DRT, we do an implicit
                    // conversion from 'document' to the QWebElement representing the 'document.documentElement'.
                    // We can't simply use a QVariantMap in nodesFromRect() because it currently times out
                    // when serializing DOMMimeType and DOMPlugin, even if we limit the recursion.
                    ret = QVariant::fromValue<QWebElement>(
                           QtWebElementRuntime::create((static_cast<JSDocument*>(object))->impl()->documentElement()));
                } else
                    ret = QVariant::fromValue<QWebElement>(QWebElement());

            } else if (hint == QVariant::typeToTypeId<QDRTNode>()) {
                if (object && object->inherits(&JSNode::s_info))
                    ret = QVariant::fromValue<QDRTNode>(QtDRTNodeRuntime::create((static_cast<JSNode*>(object))->impl()));

            } else if (hint == QVariant::Variant) {
                if (value.isUndefinedOrNull()) {
                    if (distance)
                        *distance = 1;
                    return QVariant();

                } else {
                    if (type == Object) {
                        // Since we haven't really visited this object yet, we remove it
                        visitedObjects->remove(object);
                    }

                    // And then recurse with the autodetect flag
                    ret = convertValueToQVariant(exec, value, QVariant::Void, distance, visitedObjects, recursionLimit);
                    dist = 10;
                }
                break;
            }

            dist = 10;
            break;
    }

    if (!ret.isValid())
        dist = -1;
    if (distance)
        *distance = dist;

    return ret;
}

QVariant convertValueToQVariant(ExecState* exec, JSValue value, QVariant::Type hint, int *distance)
{
    const int recursionLimit = 200;
    HashSet<JSObject*> visitedObjects;
    return convertValueToQVariant(exec, value, hint, distance, &visitedObjects, recursionLimit);
}

JSValue convertQVariantToValue(ExecState* exec, PassRefPtr<RootObject> root, const QVariant& variant)
{
    // Variants with QObject * can be isNull but not a null pointer
    // An empty QString variant is also null
    QVariant::Type type = variant.type();

    qConvDebug() << "convertQVariantToValue: Variant:" << type << ", is not valid";
    if (! variant.isValid() &&
        type != QVariant::ObjectStar &&
        type != QVariant::VoidStar &&
        type != QVariant::WidgetStar &&
        type != QVariant::String) {
        return jsNull();
    }

    JSLock lock(SilenceAssertionsOnly);

    if (type == QVariant::Bool) {
        return jsBoolean(variant.toBool());
    }

    if (type == QVariant::Int ||
        type == QVariant::UInt ||
        type == QVariant::Long ||
        type == QVariant::ULong ||
        type == QVariant::LongLong ||
        type == QVariant::ULongLong ||
        type == QVariant::Short ||
        type == QVariant::UShort ||
        type == QVariant::Float ||
        type == QVariant::Double) {
        return jsNumber(variant.toDouble());
    }

    if (type == QVariant::RegularExpression) {
        QRegularExpression re = variant.value<QRegularExpression>();

        if (re.isValid()) {
            QString16 tmp = re.pattern().toUtf16();
            UString pattern((UChar*)tmp.constData(), tmp.size_storage());

            RegExpFlags flags = NoFlags;

            if (re.patternOptions() & QPatternOption::CaseInsensitiveOption) {
               flags = FlagIgnoreCase;
            }

            RefPtr<JSC::RegExp> regExp = JSC::RegExp::create(&exec->globalData(), pattern, flags);

            if (regExp->isValid()) {
                return new (exec) RegExpObject(exec->lexicalGlobalObject(), exec->lexicalGlobalObject()->regExpStructure(), regExp.release());
            }

            return jsNull();
        }
    }

    if (type == QVariant::DateTime || type == QVariant::Date || type == QVariant::Time) {

        QDate date = QDate::currentDate();
        QTime time(0,0,0); // midnight

        if (type == QVariant::Date)
            date = variant.value<QDate>();

        else if (type == QVariant::Time)
            time = variant.value<QTime>();

        else {
            QDateTime dt = variant.value<QDateTime>().toLocalTime();
            date = dt.date();
            time = dt.time();
        }

        // Dates specified this way are in local time (we convert DateTimes above)
        GregorianDateTime dt;
        dt.year = date.year() - 1900;
        dt.month = date.month() - 1;
        dt.monthDay = date.day();
        dt.hour = time.hour();
        dt.minute = time.minute();
        dt.second = time.second();
        dt.isDST = -1;
        double ms = gregorianDateTimeToMS(exec, dt, time.msec(), /*inputIsUTC*/ false);

        return new (exec) DateInstance(exec, exec->lexicalGlobalObject()->dateStructure(), trunc(ms));
    }

    if (type == QVariant::ByteArray) {
        QByteArray qtByteArray = variant.value<QByteArray>();
        WTF::RefPtr<WTF::ByteArray> wtfByteArray = WTF::ByteArray::create(qtByteArray.length());
        memcpy(wtfByteArray->data(), qtByteArray.constData(), qtByteArray.length());
        return new (exec) JSC::JSByteArray(exec, JSC::JSByteArray::createStructure(exec->globalData(), jsNull()), wtfByteArray.get());
    }

    if (type == QVariant::ObjectStar || type == QVariant::WidgetStar) {
        QObject* obj = variant.value<QObject*>();

        if (!obj)
            return jsNull();

        return QtInstance::getQtInstance(obj, root, QScriptEngine::QtOwnership)->createRuntimeObject(exec);
    }

    if (QtPixmapInstance::canHandle(variant.type())) {
        return QtPixmapInstance::createPixmapRuntimeObject(exec, root, variant);
    }

    if (type == QVariant::typeToTypeId<QWebElement>()) {
        if (! root->globalObject()->inherits(&JSDOMWindow::s_info))
            return jsUndefined();

        Document* document = (static_cast<JSDOMWindow*>(root->globalObject()))->impl()->document();
        if (!document)
            return jsUndefined();

        return toJS(exec, toJSDOMGlobalObject(document, exec), QtWebElementRuntime::get(variant.value<QWebElement>()));
    }

    if (type == QVariant::typeToTypeId<QDRTNode>()) {
        if (!root->globalObject()->inherits(&JSDOMWindow::s_info))
            return jsUndefined();

        Document* document = (static_cast<JSDOMWindow*>(root->globalObject()))->impl()->document();
        if (!document)
            return jsUndefined();

        return toJS(exec, toJSDOMGlobalObject(document, exec), QtDRTNodeRuntime::get(variant.value<QDRTNode>()));
    }

    if (type == QVariant::Map) {
        // create a new object, and stuff properties into it
        JSObject* ret = constructEmptyObject(exec);
        QVariantMap map = variant.value<QVariantMap>();
        QVariantMap::const_iterator i = map.constBegin();

        while (i != map.constEnd()) {
            QString s = i.key();
            JSValue val = convertQVariantToValue(exec, root.get(), i.value());
            if (val) {
                PutPropertySlot slot;
                ret->put(exec, Identifier(exec, reinterpret_cast_ptr<const UChar *>(s.constData()), s.length()), val, slot);
                // ### error case?
            }
            ++i;
        }

        return ret;
    }

    // List types
    if (type == QVariant::List) {
        QVariantList vl = variant.toList();
        qConvDebug() << "got a " << vl.count() << " length list:" << vl;
        return new (exec) RuntimeArray(exec, new QtArray<QVariant>(vl, QVariant::Void, root));

    } else if (type == QVariant::StringList) {
        QStringList sl = variant.value<QStringList>();
        return new (exec) RuntimeArray(exec, new QtArray<QString>(sl, QVariant::String, root));

    } else if (type == QVariant::typeToTypeId<QObjectList>()) {
        QObjectList ol= variant.value<QObjectList>();
        return new (exec) RuntimeArray(exec, new QtArray<QObject*>(ol, QVariant::ObjectStar, root));

    } else if (type == QVariant::typeToTypeId<QList<int> >()) {
        QList<int> il= variant.value<QList<int> >();
        return new (exec) RuntimeArray(exec, new QtArray<int>(il, QVariant::Int, root));
    }

    if (type == QVariant::Variant) {
        QVariant real = variant.value<QVariant>();
        qConvDebug() << "real variant is:" << real;

        return convertQVariantToValue(exec, root, real);
    }

    qConvDebug() << "fallback path for" << variant << variant.userType();

    QString16 tmp = variant.toString().toUtf16();
    UString ustring((UChar*)tmp.constData(), tmp.size_storage());

    return jsString(exec, ustring);
}

// ===============

#define QW_D(Class) Class##Data* d = d_func()
#define QW_DS(Class,Instance) Class##Data* d = Instance->d_func()

const ClassInfo QtRuntimeMethod::s_info = { "QtRuntimeMethod", &InternalFunction::s_info, 0, 0 };

QtRuntimeMethod::QtRuntimeMethod(QtRuntimeMethodData* dd, ExecState* exec, const Identifier& ident, PassRefPtr<QtInstance> inst)
    : InternalFunction(&exec->globalData(), exec->lexicalGlobalObject(), deprecatedGetDOMStructure<QtRuntimeMethod>(exec), ident)
    , d_ptr(dd)
{
    QW_D(QtRuntimeMethod);
    d->m_instance = inst;
}

QtRuntimeMethod::~QtRuntimeMethod()
{
    QW_D(QtRuntimeMethod);
    d->m_instance->removeCachedMethod(this);
    delete d_ptr;
}

// ===============

QtRuntimeMethodData::~QtRuntimeMethodData()
{
}

QtRuntimeMetaMethodData::~QtRuntimeMetaMethodData()
{

}

QtRuntimeConnectionMethodData::~QtRuntimeConnectionMethodData()
{

}

// ===============

// Type conversion metadata (from QtScript originally)
class QtMethodMatchType
{
public:
    enum Kind {
        Invalid,
        Variant,
        MetaType,
        Unresolved,
        MetaEnum
    };

    QtMethodMatchType()
        : m_kind(Invalid) { }

    Kind kind() const
    { return m_kind; }

    QVariant::Type typeId() const;

    bool isValid() const
    { return (m_kind != Invalid); }

    bool isVariant() const
    { return (m_kind == Variant); }

    bool isMetaType() const
    { return (m_kind == MetaType); }

    bool isUnresolved() const
    { return (m_kind == Unresolved); }

    bool isMetaEnum() const
    { return (m_kind == MetaEnum); }

    QString name() const;

    int enumeratorIndex() const
    { Q_ASSERT(isMetaEnum()); return m_typeId; }

    static QtMethodMatchType variant()
    { return QtMethodMatchType(Variant); }

    static QtMethodMatchType metaType(int typeId, const QString &name)
    { return QtMethodMatchType(MetaType, typeId, name); }

    static QtMethodMatchType metaEnum(int enumIndex, const QString &name)
    { return QtMethodMatchType(MetaEnum, enumIndex, name); }

    static QtMethodMatchType unresolved(const QString &name)
    { return QtMethodMatchType(Unresolved, 0, name); }

private:
    QtMethodMatchType(Kind kind, int typeId = 0, const QString &name = QString())
        : m_kind(kind), m_typeId(typeId), m_name(name) { }

    Kind m_kind;
    int m_typeId;
    QString m_name;
};

QVariant::Type QtMethodMatchType::typeId() const
{
   if (isVariant()) {
      return QVariant::Variant;
   }

   QVariant::Type retval;

   if (isMetaEnum()) {
      retval = QVariant::Int;
   } else {
      retval = static_cast<QVariant::Type>(m_typeId);
   }

   return retval;
}

QString QtMethodMatchType::name() const
{
    if (! m_name.isEmpty())
        return m_name;

    else if (m_kind == Variant)
        return QString("QVariant");

    return QString();
}

struct QtMethodMatchData
{
    int matchDistance;
    int index;
    QVector<QtMethodMatchType> types;
    QVarLengthArray<QVariant, 10> args;

    QtMethodMatchData(int dist, int idx, QVector<QtMethodMatchType> typs, const QVarLengthArray<QVariant, 10> &as)
        : matchDistance(dist), index(idx), types(typs), args(as) { }

    QtMethodMatchData()
        : index(-1) { }

    bool isValid() const
    { return (index != -1); }

    int firstUnresolvedIndex() const
    {
        for (int i=0; i < types.count(); i++) {
            if (types.at(i).isUnresolved())
                return i;
        }
        return -1;
    }
};

static int indexOfMetaEnum(const QMetaObject *meta, const QString &str)
{
    QString scope;
    QString name;

    int scopeIdx = str.indexOf("::");

    if (scopeIdx != -1) {
        scope = str.left(scopeIdx);
        name  = str.mid(scopeIdx + 2);

    } else {
        name = str;
    }

    for (int i = meta->enumeratorCount() - 1; i >= 0; --i) {
        QMetaEnum m = meta->enumerator(i);

        if ((m.name() == name))
            return i;
    }

    return -1;
}

// Helper function for resolving methods
// Largely based on code in QtScript for compatibility reasons
static int findMethodIndex(ExecState* exec, const QMetaObject *meta, const QString &signature, bool allowPrivate,
                  QVarLengthArray<QVariant, 10> &vars, void **ptrToVoids, JSObject **pError)
{
    QList<int> matchingIndices;

    bool overloads = ! signature.contains('(');
    int count = meta->methodCount();

    for (int i = count - 1; i >= 0; --i) {
        const QMetaMethod m = meta->method(i);

        // Don't choose private methods
        if (m.access() == QMetaMethod::Private && !allowPrivate)
            continue;

        // try and find all matching named methods
        if (m.methodSignature() == signature)
            matchingIndices.append(i);

        else if (overloads) {
            QString rawsignature = m.methodSignature();
            rawsignature.truncate(rawsignature.indexOf('('));

            if (rawsignature == signature)
                matchingIndices.append(i);
        }
    }

    int chosenIndex = -1;
    *pError = 0;

    QVector<QtMethodMatchType> chosenTypes;

    QVarLengthArray<QVariant, 10> args;
    QVector<QtMethodMatchData> candidates;
    QVector<QtMethodMatchData> unresolved;

    QVector<int> tooFewArgs;
    QVector<int> conversionFailed;

    for(int index : matchingIndices) {
        QMetaMethod method = meta->method(index);

        QVector<QtMethodMatchType> types;
        bool unresolvedTypes = false;

        // resolve return type
        QString returnTypeName = method.typeName();
        uint rtype = QVariant::nameToType(returnTypeName);

        if ((rtype == 0) && ! returnTypeName.isEmpty()) {
            if (returnTypeName == "QVariant") {
                types.append(QtMethodMatchType::variant());

            } else if (returnTypeName.endsWith('*')) {
                types.append(QtMethodMatchType::metaType(QVariant::VoidStar, returnTypeName));

            } else {
                int enumIndex = indexOfMetaEnum(meta, returnTypeName);

                if (enumIndex != -1)
                    types.append(QtMethodMatchType::metaEnum(enumIndex, returnTypeName));
                else {
                    unresolvedTypes = true;
                    types.append(QtMethodMatchType::unresolved(returnTypeName));
                }
            }

        } else {
            if (returnTypeName == "QVariant")
                types.append(QtMethodMatchType::variant());
            else
                types.append(QtMethodMatchType::metaType(rtype, returnTypeName));
        }

        // resolve argument types
        QList<QString> parameterTypeNames = method.parameterTypes();

        for (int i = 0; i < parameterTypeNames.count(); ++i) {
            QString argTypeName = parameterTypeNames.at(i);
            uint atype = QVariant::nameToType(argTypeName);

            if (atype == 0) {
                if (argTypeName == "QVariant") {
                    types.append(QtMethodMatchType::variant());

                } else {
                    int enumIndex = indexOfMetaEnum(meta, argTypeName);

                    if (enumIndex != -1)
                        types.append(QtMethodMatchType::metaEnum(enumIndex, argTypeName));
                    else {
                        unresolvedTypes = true;
                        types.append(QtMethodMatchType::unresolved(argTypeName));
                    }
                }

            } else {
                if (argTypeName == "QVariant")
                    types.append(QtMethodMatchType::variant());
                else
                    types.append(QtMethodMatchType::metaType(atype, argTypeName));
            }
        }

        // If the native method requires more arguments than what was passed from JavaScript
        if (exec->argumentCount() + 1 < static_cast<unsigned>(types.count())) {
            qMatchDebug() << "Match:too few args for" << method.methodSignature().constData();
            tooFewArgs.append(index);
            continue;
        }

        if (unresolvedTypes) {
            qMatchDebug() << "Match:unresolved arg types for" << method.methodSignature().constData();

            // remember it so we can give an error message later, if necessary
            unresolved.append(QtMethodMatchData(/*matchDistance=*/INT_MAX, index, types, QVarLengthArray<QVariant, 10>()));
            continue;
        }

        // Now convert arguments
        if (args.count() != types.count())
            args.resize(types.count());

        QtMethodMatchType retType = types[0];
        args[0] = QVariant(retType.typeId(), (void *)0); // the return value

        bool converted = true;
        int matchDistance = 0;

        for (unsigned i = 0; converted && i + 1 < static_cast<unsigned>(types.count()); ++i) {
            JSValue arg = i < exec->argumentCount() ? exec->argument(i) : jsUndefined();

            int argdistance = -1;
            QVariant v = convertValueToQVariant(exec, arg, types.at(i+1).typeId(), &argdistance);

            if (argdistance >= 0) {
                matchDistance += argdistance;
                args[i+1] = v;
            } else {
                converted = false;
            }
        }

        qMatchDebug() << "Match: " << method.methodSignature().constData() << (converted ? "converted":"failed to convert")
                  << "distance " << matchDistance;

        if (converted) {
            if ((exec->argumentCount() + 1 == static_cast<unsigned>(types.count()))
                && (matchDistance == 0)) {
                // perfect match, use this one
                chosenIndex = index;
                break;

            } else {
                QtMethodMatchData currentMatch(matchDistance, index, types, args);

                if (candidates.isEmpty()) {
                    candidates.append(currentMatch);

                } else {
                    QtMethodMatchData bestMatchSoFar = candidates.at(0);

                    if ((args.count() > bestMatchSoFar.args.count()) || ((args.count() == bestMatchSoFar.args.count())
                            && (matchDistance <= bestMatchSoFar.matchDistance))) {

                        candidates.prepend(currentMatch);
                    } else {
                        candidates.append(currentMatch);
                    }
                }
            }
        } else {
            conversionFailed.append(index);
        }

        if (!overloads)
            break;
    }

    if (chosenIndex == -1 && candidates.count() == 0) {
        // No valid functions at all - format an error message
        if (! conversionFailed.isEmpty()) {

            QString message = QString("Incompatible type of argument(s) in call to %0(), candidates were\n").formatArg(signature);

            for (int i = 0; i < conversionFailed.size(); ++i) {
                if (i > 0) {
                    message += "\n";
                }

                QMetaMethod mtd = meta->method(conversionFailed.at(i));
                message += QString("    %0").formatArg(mtd.methodSignature());
            }

            *pError = throwError(exec, createTypeError(exec, message.toLatin1().constData()));

        } else if (!unresolved.isEmpty()) {
            QtMethodMatchData argsInstance = unresolved.first();
            int unresolvedIndex = argsInstance.firstUnresolvedIndex();

            Q_ASSERT(unresolvedIndex != -1);

            QtMethodMatchType unresolvedType = argsInstance.types.at(unresolvedIndex);

            QString message = QString("Can not call %0(): unknown type `%1'").formatArg(signature).formatArg(unresolvedType.name());

            *pError = throwError(exec, createTypeError(exec, message.toLatin1().constData()));

        } else {
            QString message = QString("Too few arguments in call to %0(), candidates are\n").formatArg(signature);

            for (int i = 0; i < tooFewArgs.size(); ++i) {
                if (i > 0)
                    message += "\n";

                QMetaMethod mtd = meta->method(tooFewArgs.at(i));
                message += QString("    %0").formatArg(mtd.methodSignature());
            }

            *pError = throwError(exec, createSyntaxError(exec, message.toLatin1().constData()));
        }
    }

    if (chosenIndex == -1 && candidates.count() > 0) {
        QtMethodMatchData bestMatch = candidates.at(0);

        if ((candidates.size() > 1) && (bestMatch.args.count() == candidates.at(1).args.count())
            && (bestMatch.matchDistance == candidates.at(1).matchDistance)) {

            // ambiguous call
            QString message = QString("ambiguous call of overloaded function %0(), candidates were\n").formatArg(signature);

            for (int i = 0; i < candidates.size(); ++i) {
                // Only candidate for overload if argument count and match distance is same as best match

                if (candidates.at(i).args.count() == bestMatch.args.count()
                    || candidates.at(i).matchDistance == bestMatch.matchDistance) {

                    if (i > 0)
                        message += QLatin1String("\n");

                    QMetaMethod mtd = meta->method(candidates.at(i).index);
                    message += QString("    %0").formatArg(mtd.methodSignature());
                }
            }

            *pError = throwError(exec, createTypeError(exec, message.toLatin1().constData()));

        } else {
            chosenIndex = bestMatch.index;
            args = bestMatch.args;
        }
    }

    if (chosenIndex != -1) {
        /* Copy the stuff over */
        int i;
        vars.resize(args.count());

        for (i = 0; i < args.count(); i++) {
            vars[i] = args[i];
            // emerald (webkit, ok) ptrToVoids[i] = vars[i].data();
        }
    }

    return chosenIndex;
}

// Signals are not fuzzy matched as much as methods
static int findSignalIndex(const QMetaObject* meta, int initialIndex, QString signature)
{
    int index = initialIndex;
    QMetaMethod method = meta->method(index);
    bool overloads = !signature.contains('(');

    if (overloads && (method.attributes() & QMetaMethod::Cloned)) {
        // find the most general method
        do {
            method = meta->method(--index);
        } while (method.attributes() & QMetaMethod::Cloned);
    }
    return index;
}

QtRuntimeMetaMethod::QtRuntimeMetaMethod(ExecState* exec, const Identifier& ident, PassRefPtr<QtInstance> inst,
                  int index, const QString &signature, bool allowPrivate)
    : QtRuntimeMethod (new QtRuntimeMetaMethodData(), exec, ident, inst)
{
    QW_D(QtRuntimeMetaMethod);

    d->m_signature = signature;
    d->m_index = index;
    d->m_allowPrivate = allowPrivate;
}

void QtRuntimeMetaMethod::visitChildren(SlotVisitor& visitor)
{
    QtRuntimeMethod::visitChildren(visitor);
    QW_D(QtRuntimeMetaMethod);

    if (d->m_connect)
        visitor.append(&d->m_connect);
    if (d->m_disconnect)
        visitor.append(&d->m_disconnect);
}

EncodedJSValue QtRuntimeMetaMethod::call(ExecState* exec)
{
   // emerald (webkit, hold)
   qWarning("(CopperSpice) QtRuntimeMetaMethod::call, Not implemented");


/*
    QtRuntimeMetaMethodData* d = static_cast<QtRuntimeMetaMethod *>(exec->callee())->d_func();

    // limted to 10 args
    if (exec->argumentCount() > 10)
        return JSValue::encode(jsUndefined());

    // pick a method that matches
    JSLock lock(SilenceAssertionsOnly);

    QObject *obj = d->m_instance->getObject();

    if (obj) {
        QVarLengthArray<QVariant, 10> vargs;
        void *qargs[11];

        int methodIndex;
        JSObject* errorObj = 0;
        if ((methodIndex = findMethodIndex(exec, obj->metaObject(), d->m_signature, d->m_allowPrivate, vargs, (void **)qargs, &errorObj)) != -1) {
            if (QMetaObject::metacall(obj, QMetaObject::InvokeMetaMethod, methodIndex, qargs) >= 0)
                return JSValue::encode(jsUndefined());

            if (vargs[0].isValid())
                return JSValue::encode(convertQVariantToValue(exec, d->m_instance->rootObject(), vargs[0]));
        }

        if (errorObj) {
            return JSValue::encode(errorObj);
        }

    } else {
        return throwVMError(exec, createError(exec, "can not call function of deleted QObject"));
    }

*/

    // void functions return undefined
    return JSValue::encode(jsUndefined());
}

CallType QtRuntimeMetaMethod::getCallData(CallData& callData)
{
    callData.native.function = call;
    return CallTypeHost;
}

bool QtRuntimeMetaMethod::getOwnPropertySlot(ExecState* exec, const Identifier& propertyName, PropertySlot& slot)
{
    if (propertyName == "connect") {
        slot.setCustom(this, connectGetter);
        return true;
    } else if (propertyName == "disconnect") {
        slot.setCustom(this, disconnectGetter);
        return true;
    } else if (propertyName == exec->propertyNames().length) {
        slot.setCustom(this, lengthGetter);
        return true;
    }

    return QtRuntimeMethod::getOwnPropertySlot(exec, propertyName, slot);
}

bool QtRuntimeMetaMethod::getOwnPropertyDescriptor(ExecState* exec, const Identifier& propertyName, PropertyDescriptor& descriptor)
{
    if (propertyName == "connect") {
        PropertySlot slot;
        slot.setCustom(this, connectGetter);
        descriptor.setDescriptor(slot.getValue(exec, propertyName), DontDelete | ReadOnly | DontEnum);
        return true;
    }

    if (propertyName == "disconnect") {
        PropertySlot slot;
        slot.setCustom(this, disconnectGetter);
        descriptor.setDescriptor(slot.getValue(exec, propertyName), DontDelete | ReadOnly | DontEnum);
        return true;
    }

    if (propertyName == exec->propertyNames().length) {
        PropertySlot slot;
        slot.setCustom(this, lengthGetter);
        descriptor.setDescriptor(slot.getValue(exec, propertyName), DontDelete | ReadOnly | DontEnum);
        return true;
    }

    return QtRuntimeMethod::getOwnPropertyDescriptor(exec, propertyName, descriptor);
}

void QtRuntimeMetaMethod::getOwnPropertyNames(ExecState* exec, PropertyNameArray& propertyNames, EnumerationMode mode)
{
    if (mode == IncludeDontEnumProperties) {
        propertyNames.add(Identifier(exec, "connect"));
        propertyNames.add(Identifier(exec, "disconnect"));
        propertyNames.add(exec->propertyNames().length);
    }

    QtRuntimeMethod::getOwnPropertyNames(exec, propertyNames, mode);
}

JSValue QtRuntimeMetaMethod::lengthGetter(ExecState*, JSValue, const Identifier&)
{
    // QtScript always returns 0
    return jsNumber(0);
}

JSValue QtRuntimeMetaMethod::connectGetter(ExecState* exec, JSValue slotBase, const Identifier& ident)
{
    QtRuntimeMetaMethod* thisObj = static_cast<QtRuntimeMetaMethod*>(asObject(slotBase));
    QW_DS(QtRuntimeMetaMethod, thisObj);

    if (!d->m_connect) {
        d->m_connect.set(exec->globalData(), thisObj, new (exec) QtRuntimeConnectionMethod(exec, ident, true,
                  d->m_instance, d->m_index, d->m_signature));
   }

    return d->m_connect.get();
}

JSValue QtRuntimeMetaMethod::disconnectGetter(ExecState* exec, JSValue slotBase, const Identifier& ident)
{
    QtRuntimeMetaMethod* thisObj = static_cast<QtRuntimeMetaMethod*>(asObject(slotBase));
    QW_DS(QtRuntimeMetaMethod, thisObj);

    if (! d->m_disconnect)  {
        d->m_disconnect.set(exec->globalData(), thisObj, new (exec) QtRuntimeConnectionMethod(exec, ident, false,
                  d->m_instance, d->m_index, d->m_signature));
    }

    return d->m_disconnect.get();
}

// ===============

QMultiMap<QObject*, QtConnectionObject*> QtRuntimeConnectionMethod::connections;

QtRuntimeConnectionMethod::QtRuntimeConnectionMethod(ExecState* exec, const Identifier& ident, bool isConnect,
                  PassRefPtr<QtInstance> inst, int index, const QString &signature)
    : QtRuntimeMethod (new QtRuntimeConnectionMethodData(), exec, ident, inst)
{
    QW_D(QtRuntimeConnectionMethod);

    d->m_signature = signature;
    d->m_index = index;
    d->m_isConnect = isConnect;
}

EncodedJSValue QtRuntimeConnectionMethod::call(ExecState *exec)
{
    QtRuntimeConnectionMethodData* d = static_cast<QtRuntimeConnectionMethod *>(exec->callee())->d_func();

    JSLock lock(SilenceAssertionsOnly);

    QObject* sender = d->m_instance->getObject();

    if (sender) {

        JSObject* thisObject = exec->lexicalGlobalObject();
        JSObject* funcObject = 0;

        // QtScript checks signalness first, arguments second
        int signalIndex = -1;

        // Make sure the initial index is a signal
        QMetaMethod m = sender->metaObject()->method(d->m_index);
        if (m.methodType() == QMetaMethod::Signal)
            signalIndex = findSignalIndex(sender->metaObject(), d->m_index, d->m_signature);

        if (signalIndex != -1) {
            if (exec->argumentCount() == 1) {
                funcObject = exec->argument(0).toObject(exec);
                CallData callData;
                if (funcObject->getCallData(callData) == CallTypeNone) {
                    if (d->m_isConnect)
                        return throwVMError(exec, createTypeError(exec, "QtMetaMethod.connect: target is not a function"));
                    else
                        return throwVMError(exec, createTypeError(exec, "QtMetaMethod.disconnect: target is not a function"));
                }
            } else if (exec->argumentCount() >= 2) {
                if (exec->argument(0).isObject()) {
                    thisObject = exec->argument(0).toObject(exec);

                    // Get the actual function to call
                    JSObject *asObj = exec->argument(1).toObject(exec);
                    CallData callData;
                    if (asObj->getCallData(callData) != CallTypeNone) {
                        // Function version
                        funcObject = asObj;
                    } else {
                        // Convert it to a string
                        UString funcName = exec->argument(1).toString(exec);
                        Identifier funcIdent(exec, funcName);

                        // ### DropAllLocks
                        // This is resolved at this point in QtScript
                        JSValue val = thisObject->get(exec, funcIdent);
                        JSObject* asFuncObj = val.toObject(exec);

                        if (asFuncObj->getCallData(callData) != CallTypeNone) {
                            funcObject = asFuncObj;
                        } else {
                            if (d->m_isConnect)
                                return throwVMError(exec, createTypeError(exec, "QtMetaMethod.connect: target is not a function"));
                            else
                                return throwVMError(exec, createTypeError(exec, "QtMetaMethod.disconnect: target is not a function"));
                        }
                    }
                } else {
                    if (d->m_isConnect)
                        return throwVMError(exec, createTypeError(exec, "QtMetaMethod.connect: thisObject is not an object"));
                    else
                        return throwVMError(exec, createTypeError(exec, "QtMetaMethod.disconnect: thisObject is not an object"));
                }
            } else {
                if (d->m_isConnect)
                    return throwVMError(exec, createError(exec, "QtMetaMethod.connect: no arguments given"));
                else
                    return throwVMError(exec, createError(exec, "QtMetaMethod.disconnect: no arguments given"));
            }

            if (d->m_isConnect) {
                // to connect, we need:
                //  target object [from ctor]
                //  target signal index etc. [from ctor]
                //  receiver function [from arguments]
                //  receiver this object [from arguments]

                QtConnectionObject *conn = new QtConnectionObject(exec->globalData(), d->m_instance, signalIndex, thisObject, funcObject);


/*  CopperSpice Test 03/21/2014
                bool ok = QMetaObject::connect(sender, signalIndex, conn, conn->metaObject()->methodOffset());
*/
                // CopperSpice Test 03/21/2014
                bool ok = false;

                if (!ok) {
                    delete conn;
                    QString msg = QString("QtMetaMethod.connect: failed to connect to %1::%2()")
                            .formatArg(sender->metaObject()->className()).formatArg(d->m_signature);

                    return throwVMError(exec, createError(exec, msg.toLatin1().constData()));
                }
                else {
                    // Store connection
                    connections.insert(sender, conn);
                }

            } else {
                // Now to find our previous connection object. Hmm.
                QList<QtConnectionObject*> conns = connections.values(sender);
                bool ret = false;

                for (QtConnectionObject* conn : conns) {
                    // Is this the right connection?
                    if (conn->match(sender, signalIndex, thisObject, funcObject)) {


/*  CopperSpice Test 03/21/2014
                        // yes, disconnect it
                        QMetaObject::disconnect(sender, signalIndex, conn, conn->metaObject()->methodOffset());
*/
                        delete conn; // this will also remove it from the map
                        ret = true;
                        break;
                    }
                }

                if (! ret) {
                    QString msg = QString("QtMetaMethod.disconnect: failed to disconnect from %1::%2()")
                            .formatArg(sender->metaObject()->className()).formatArg(d->m_signature);

                    return throwVMError(exec, createError(exec, msg.toLatin1().constData()));
                }
            }

        } else {
            QString msg = QString("QtMetaMethod.%1: %2::%3() is not a signal")
                    .formatArg(d->m_isConnect ? QString("connect") : QString("disconnect"))
                    .formatArg(sender->metaObject()->className())
                    .formatArg(d->m_signature);

            return throwVMError(exec, createTypeError(exec, msg.toLatin1().constData()));
        }

    } else {
        return throwVMError(exec, createError(exec, "Can not call function of deleted QObject"));
    }

    return JSValue::encode(jsUndefined());
}

CallType QtRuntimeConnectionMethod::getCallData(CallData& callData)
{
    callData.native.function = call;
    return CallTypeHost;
}

bool QtRuntimeConnectionMethod::getOwnPropertySlot(ExecState* exec, const Identifier& propertyName, PropertySlot& slot)
{
    if (propertyName == exec->propertyNames().length) {
        slot.setCustom(this, lengthGetter);
        return true;
    }

    return QtRuntimeMethod::getOwnPropertySlot(exec, propertyName, slot);
}

bool QtRuntimeConnectionMethod::getOwnPropertyDescriptor(ExecState* exec, const Identifier& propertyName, PropertyDescriptor& descriptor)
{
    if (propertyName == exec->propertyNames().length) {
        PropertySlot slot;
        slot.setCustom(this, lengthGetter);
        descriptor.setDescriptor(slot.getValue(exec, propertyName), DontDelete | ReadOnly | DontEnum);
        return true;
    }

    return QtRuntimeMethod::getOwnPropertyDescriptor(exec, propertyName, descriptor);
}

void QtRuntimeConnectionMethod::getOwnPropertyNames(ExecState* exec, PropertyNameArray& propertyNames, EnumerationMode mode)
{
    if (mode == IncludeDontEnumProperties)
        propertyNames.add(exec->propertyNames().length);

    QtRuntimeMethod::getOwnPropertyNames(exec, propertyNames, mode);
}

JSValue QtRuntimeConnectionMethod::lengthGetter(ExecState*, JSValue, const Identifier&)
{
    // we have one formal argument, and one optional
    return jsNumber(1);
}

// ===============

QtConnectionObject::QtConnectionObject(JSGlobalData& globalData, PassRefPtr<QtInstance> instance, int signalIndex, JSObject* thisObject, JSObject* funcObject)
    : m_instance(instance)
    , m_signalIndex(signalIndex)
    , m_originalObject(m_instance->getObject())
    , m_thisObject(globalData, thisObject)
    , m_funcObject(globalData, funcObject)
{
    setParent(m_originalObject);
    ASSERT(JSLock::currentThreadIsHoldingLock()); // so our ProtectedPtrs are safe
}

QtConnectionObject::~QtConnectionObject()
{
    // Remove us from the map of active connections
    QtRuntimeConnectionMethod::connections.remove(m_originalObject, this);
}

void QtConnectionObject::execute(void **argv)
{
    // emerald (webkit, hold)
    qDebug("CopperSpice CsWebKit: execute method not implemented");

/*
    QObject *obj = m_instance->getObject();

    if (obj) {
        const QMetaObject* meta = obj->metaObject();
        const QMetaMethod method = meta->method(m_signalIndex);

        QList<QByteArray> parameterTypes = method.parameterTypes();

        int argc = parameterTypes.count();

        JSLock lock(SilenceAssertionsOnly);

        // ### Should the Interpreter/ExecState come from somewhere else?
        RefPtr<RootObject> ro = m_instance->rootObject();

        if (ro) {
            JSGlobalObject* globalobj = ro->globalObject();

            if (globalobj) {
                ExecState* exec = globalobj->globalExec();

                if (exec) {
                    // Build the argument list (up to the formal argument length of the slot)
                    MarkedArgumentBuffer l;

                    // ### DropAllLocks?
                    int funcArgC = m_funcObject->get(exec, exec->propertyNames().length).toInt32(exec);
                    int argTotal = qMax(funcArgC, argc);

                    for(int i=0; i < argTotal; i++) {
                        if (i < argc) {
                            int argType = QVariant::type(parameterTypes.at(i));
                            l.append(convertQVariantToValue(exec, ro, QVariant(argType, argv[i+1])));
                        } else {
                            l.append(jsUndefined());
                        }
                    }

                    // Stuff in the __qt_sender property, if we can
                    ScopeChainNode* oldsc = 0;
                    JSFunction* fimp = 0;

                    if (m_funcObject->inherits(&JSFunction::s_info)) {
                        fimp = static_cast<JSFunction*>(m_funcObject.get());

                        JSObject* qt_sender = QtInstance::getQtInstance(sender(), ro, QScriptEngine::QtOwnership)->createRuntimeObject(exec);
                        JSObject* wrapper = constructEmptyObject(exec, createEmptyObjectStructure(exec->globalData(), jsNull()));
                        PutPropertySlot slot;
                        wrapper->put(exec, Identifier(exec, "__qt_sender__"), qt_sender, slot);
                        oldsc = fimp->scope();
                        fimp->setScope(exec->globalData(), oldsc->push(wrapper));
                    }

                    CallData callData;
                    CallType callType = m_funcObject->getCallData(callData);
                    call(exec, m_funcObject.get(), callType, callData, m_thisObject.get(), l);

                    if (fimp)
                        fimp->setScope(exec->globalData(), oldsc);
                }
            }
        }

    } else {
        // unsure, deleted object emitted a signal
        qWarning() << "sender deleted, can not deliver signal";
    }

*/

}

bool QtConnectionObject::match(QObject* sender, int signalIndex, JSObject* thisObject, JSObject *funcObject)
{
    if (m_originalObject == sender && m_signalIndex == signalIndex
        && thisObject == (JSObject*)m_thisObject.get() && funcObject == (JSObject*)m_funcObject.get())
        return true;
    return false;
}

template <typename T> QtArray<T>::QtArray(QList<T> list, QVariant::Type type, PassRefPtr<RootObject> rootObject)
    : Array(rootObject)
    , m_list(list)
    , m_type(type)
{
    m_length = m_list.count();
}

template <typename T> QtArray<T>::~QtArray ()
{
}

template <typename T> RootObject* QtArray<T>::rootObject() const
{
    return m_rootObject && m_rootObject->isValid() ? m_rootObject.get() : 0;
}

template <typename T> void QtArray<T>::setValueAt(ExecState* exec, unsigned index, JSValue aValue) const
{
    // QtScript sets the value, but doesn't forward it to the original source
    // (e.g. if you do 'object.intList[5] = 6', the object is not updated, but the
    // copy of the list is).
    int dist = -1;
    QVariant val = convertValueToQVariant(exec, aValue, m_type, &dist);

    if (dist >= 0) {
        m_list[index] = val.value<T>();
    }
}


template <typename T> JSValue QtArray<T>::valueAt(ExecState *exec, unsigned int index) const
{
    if (index < m_length) {
        T val = m_list.at(index);
        return convertQVariantToValue(exec, rootObject(), QVariant::fromValue(val));
    }

    return jsUndefined();
}

// ===============

} }
