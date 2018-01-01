/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
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

#include <QAxObject>
#include <QFile>
#include <QMetaObject>
#include <QMetaEnum>
#include <QTextStream>
#include <QSettings>
#include <QStringList>
#include <QUuid>
#include <QWidget>
#include <QFileInfo>
#include <qt_windows.h>
#include <ocidl.h>

QT_BEGIN_NAMESPACE

static ITypeInfo *currentTypeInfo = 0;

enum ObjectCategory
{
    DefaultObject    = 0x00,
    SubObject        = 0x001,
    ActiveX          = 0x002,
    NoMetaObject     = 0x004,
    NoImplementation = 0x008,
    NoDeclaration    = 0x010,
    NoInlines        = 0x020,
    OnlyInlines      = 0x040,
    DoNothing        = 0x080,
    Licensed         = 0x100,
    TypeLibID        = 0x101
};

// this comes from moc/qmetaobject.cpp
enum ProperyFlags  {
    Invalid = 0x00000000,
    Readable = 0x00000001,
    Writable = 0x00000002,
    Resetable = 0x00000004,
    EnumOrFlag = 0x00000008,
    StdCppSet = 0x00000100,
    Override = 0x00000200,
    Designable = 0x00001000,
    ResolveDesignable = 0x00002000,
    Scriptable = 0x00004000,
    ResolveScriptable = 0x00008000,
    Stored = 0x00010000,
    ResolveStored = 0x00020000,
    Editable = 0x00040000,
    ResolveEditable = 0x00080000
};

enum MemberFlags {
    AccessPrivate = 0x00,
    AccessProtected = 0x01,
    AccessPublic = 0x02,
    MemberMethod = 0x00,
    MemberSignal = 0x04,
    MemberSlot = 0x08,
    MemberCompatibility = 0x10,
    MemberCloned = 0x20,
    MemberScriptable = 0x40,
};

extern QMetaObject *qax_readEnumInfo(ITypeLib *typeLib, const QMetaObject *parentObject);
extern QMetaObject *qax_readClassInfo(ITypeLib *typeLib, ITypeInfo *typeInfo, const QMetaObject *parentObject);
extern QMetaObject *qax_readInterfaceInfo(ITypeLib *typeLib, ITypeInfo *typeInfo, const QMetaObject *parentObject);
extern QList<QByteArray> qax_qualified_usertypes;
extern QString qax_docuFromName(ITypeInfo *typeInfo, const QString &name);
extern bool qax_dispatchEqualsIDispatch;

QByteArray nameSpace;
QMap<QByteArray, QByteArray> namespaceForType;

void writeEnums(QTextStream &out, const QMetaObject *mo)
{
    // enums
    for (int ienum = mo->enumeratorOffset(); ienum < mo->enumeratorCount(); ++ienum) {
        QMetaEnum metaEnum = mo->enumerator(ienum);
        out << "    enum " << metaEnum.name() << " {" << endl;
        for (int k = 0; k < metaEnum.keyCount(); ++k) {
            QByteArray key(metaEnum.key(k));
            out << "        " << key.leftJustified(24) << "= " << metaEnum.value(k);
            if (k < metaEnum.keyCount() - 1)
                out << ",";
            out << endl;
        }
        out << "    };" << endl;
        out << endl;
    }
}

void writeHeader(QTextStream &out, const QByteArray &nameSpace, const QString &outFileName)
{
    out << "#ifndef QAX_DUMPCPP_" << outFileName.toUpper() << "_H" << endl;
    out << "#define QAX_DUMPCPP_" << outFileName.toUpper() << "_H" << endl;
    out << endl;
    out << "// Define this symbol to __declspec(dllexport) or __declspec(dllimport)" << endl;
    out << "#ifndef " << nameSpace.toUpper() << "_EXPORT" << endl;
    out << "#define " << nameSpace.toUpper() << "_EXPORT" << endl;
    out << "#endif" << endl;
    out << endl;
    out << "#include <qaxobject.h>" << endl;
    out << "#include <qaxwidget.h>" << endl;
    out << "#include <qdatetime.h>" << endl;
    out << "#include <qpixmap.h>" << endl;
    out << endl;
    out << "struct IDispatch;" << endl;
    out << endl;
}

void generateNameSpace(QTextStream &out, const QMetaObject *mo, const QByteArray &nameSpace)
{
    out << "namespace " << nameSpace << " {" << endl;
    out << endl;
    writeEnums(out, mo);

    // don't close on purpose
}

static QByteArray joinParameterNames(const QList<QByteArray> &parameterNames)
{
    QByteArray slotParameters;
    for (int p = 0; p < parameterNames.count(); ++p) {
        slotParameters += parameterNames.at(p);
        if (p < parameterNames.count() - 1)
            slotParameters += ',';
    }

    return slotParameters;
}

QByteArray constRefify(const QByteArray &type)
{
    QByteArray ctype(type);
    if (type == "QString" || type == "QPixmap" 
        || type == "QVariant" || type == "QDateTime"
        || type == "QColor" || type == "QFont"
        || type == "QByteArray" || type == "QValueList<QVariant>"
        || type == "QStringList")
        ctype = "const " + ctype + "&";

    return ctype;
}

void generateClassDecl(QTextStream &out, const QString &controlID, const QMetaObject *mo, const QByteArray &className, const QByteArray &nameSpace, ObjectCategory category)
{
    QList<QByteArray> functions;

    QByteArray indent;
    if (!(category & OnlyInlines))
        indent = "    ";

    if (!(category & OnlyInlines)) {
        // constructors
        out << "class " << nameSpace.toUpper() << "_EXPORT " << className << " : public ";
        if (category & ActiveX)
            out << "QAxWidget";
        else
            out << "QAxObject";
        out << endl;

        out << "{" << endl;
        out << "public:" << endl;
        out << "    " << className << "(";
        if (category & Licensed)
            out << "const QString &licenseKey = QString(), ";
        if (category & ActiveX)
            out << "QWidget *parent = 0, Qt::WindowFlags f";
        else if (category & SubObject)
            out << "IDispatch *subobject = 0, QAxObject *parent";
        else
            out << "QObject *parent";
        out << " = 0)" << endl;
        out << "    : ";
        if (category & ActiveX)
            out << "QAxWidget(parent, f";
        else if (category & SubObject)
            out << "QAxObject((IUnknown*)subobject, parent";
        else
            out << "QAxObject(parent";
        out << ")" << endl;
        out << "    {" << endl;
        if (category & SubObject)
            out << "        internalRelease();" << endl;
        else if (category & Licensed) {
            out << "        if (licenseKey.isEmpty())" << endl;
            out << "            setControl(\"" << controlID << "\");" << endl;
            out << "        else" << endl;
            out << "            setControl(\"" << controlID << ":\" + licenseKey);" << endl;
        } else {
            out << "        setControl(\"" << controlID << "\");" << endl;
        }
        out << "    }" << endl;
        out << endl;

        for (int ci = mo->classInfoOffset(); ci < mo->classInfoCount(); ++ci) {
            QMetaClassInfo info = mo->classInfo(ci);
            QByteArray iface_name = info.name();
            if (iface_name.startsWith("Event "))
                continue;

            QByteArray iface_class = info.value();

            out << "    " << className << "(" << iface_class << " *iface)" << endl;

            if (category & ActiveX)
                out << "    : QAxWidget()" << endl;
            else
                out << "    : QAxObject()" << endl;
            out << "    {" << endl;
            out << "        initializeFrom(iface);" << endl;
            out << "        delete iface;" << endl;
            out << "    }" << endl;
            out << endl;
        }
    }

    functions << className;

    // enums
    if (nameSpace.isEmpty() && !(category & OnlyInlines)) {
        for (int ienum = mo->enumeratorOffset(); ienum < mo->enumeratorCount(); ++ienum) {
            QMetaEnum metaEnum = mo->enumerator(ienum);
            out << "    enum " << metaEnum.name() << " {" << endl;
            for (int k = 0; k < metaEnum.keyCount(); ++k) {
                QByteArray key(metaEnum.key(k));
                out << "        " << key.leftJustified(24) << "= " << metaEnum.value(k);
                if (k < metaEnum.keyCount() - 1)
                    out << ",";
                out << endl;
            }
            out << "    };" << endl;
            out << endl;
        }
    }
    // QAxBase public virtual functions.
    QList<QByteArray> axBase_vfuncs;
    axBase_vfuncs.append("metaObject");
    axBase_vfuncs.append("qObject");
    axBase_vfuncs.append("className");
    axBase_vfuncs.append("propertyWritable");
    axBase_vfuncs.append("setPropertyWritable");

    // properties
    for (int iprop = mo->propertyOffset(); iprop < mo->propertyCount(); ++iprop) {
        QMetaProperty property = mo->property(iprop);
        if (!property.isReadable())
            continue;
        
        QByteArray propertyName(property.name());
        if (propertyName == "control" || propertyName == className)
            continue;

        if (!(category & OnlyInlines)) {
            out << indent << "/*" << endl << indent << "Property " << propertyName << endl;
            QString documentation = qax_docuFromName(currentTypeInfo, QString::fromLatin1(propertyName.constData()));
            if (!documentation.isEmpty()) {
                out << endl;
                out << indent << documentation << endl;
            }
            out << indent << "*/" << endl;
        }

        // Check whether the new function conflicts with any of QAxBase public virtual functions.
        // If so, prepend the function name with '<classname>_'. Since all internal metaobject magic 
        // remains the same, we have to use the original name when used with QObject::connect or QMetaObject
        QByteArray propertyFunctionName(propertyName);
        if (axBase_vfuncs.contains(propertyFunctionName)) {
            propertyFunctionName = className + "_" + propertyName;
            qWarning("property conflits with QAXBase: %s changed to %s", propertyName.constData(), propertyFunctionName.constData());
        }

        QByteArray propertyType(property.typeName());
        QByteArray castType(propertyType);

        QByteArray simplePropType = propertyType;
        simplePropType.replace('*', "");

        out << indent << "inline ";
        bool foreignNamespace = true;
        if (!propertyType.contains("::") && 
            (qax_qualified_usertypes.contains(simplePropType) || qax_qualified_usertypes.contains("enum "+ simplePropType))
           ) {
            propertyType = nameSpace + "::" + propertyType;
            foreignNamespace = false;
        }

        out << propertyType << " ";

        if (category & OnlyInlines)
            out << className << "::";
        out << propertyFunctionName << "() const";

        if (!(category & NoInlines)) {
            out << endl << indent << "{" << endl;
            if (qax_qualified_usertypes.contains(simplePropType)) {
                out << indent << "    " << propertyType << " qax_pointer = 0;" << endl;
                out << indent << "    qRegisterMetaType(\"" << property.typeName() << "\", &qax_pointer);" << endl;
                if (foreignNamespace)
                    out << "#ifdef QAX_DUMPCPP_" << propertyType.left(propertyType.indexOf("::")).toUpper() << "_H" << endl;
                out << indent << "    qRegisterMetaType(\"" << simplePropType << "\", qax_pointer);" << endl;
                if (foreignNamespace)
                    out << "#endif" << endl;
            }
            out << indent << "    QVariant qax_result = property(\"" << propertyName << "\");" << endl;
            if (propertyType.length() && propertyType.at(propertyType.length()-1) == '*')
                out << indent << "    if (!qax_result.constData()) return 0;" << endl;
            out << indent << "    Q_ASSERT(qax_result.isValid());" << endl;
            if (qax_qualified_usertypes.contains(simplePropType)) {
                simplePropType = propertyType;
                simplePropType.replace('*', "");
                if (foreignNamespace)
                    out << "#ifdef QAX_DUMPCPP_" << propertyType.left(propertyType.indexOf("::")).toUpper() << "_H" << endl;
                out << indent << "    return *(" << propertyType << "*)qax_result.constData();" << endl;
                if (foreignNamespace) {
                    out << "#else" << endl;
                    out << indent << "    return 0; // foreign namespace not included" << endl;
                    out << "#endif" << endl;
                }

            } else {
                out << indent << "    return *(" << propertyType << "*)qax_result.constData();" << endl;
            }
            out << indent << "}" << endl;
        } else {
            out << "; //Returns the value of " << propertyName << endl;
        }

        functions << propertyName;
        
        if (property.isWritable()) {
            QByteArray setter(propertyName);
            if (isupper(setter.at(0))) {
                setter = "Set" + setter;
            } else {
                setter[0] = toupper(setter[0]);
                setter = "set" + setter;
            }
            
            out << indent << "inline " << "void ";
            if (category & OnlyInlines)
                out << className << "::";
            out << setter << "(" << constRefify(propertyType) << " value)";
            
            if (!(category & NoInlines)) {
                if (propertyType.endsWith('*')) {
                    out << "{" << endl;
                    out << "    int typeId = qRegisterMetaType(\"" << propertyType << "\", &value);" << endl;
                    out << "    setProperty(\"" << propertyName << "\", QVariant(typeId, &value));" << endl;
                    out << "}" << endl;
                } else {
                    out << "{ setProperty(\"" << propertyName << "\", QVariant(value)); }" << endl;
                }
            } else {
                out << "; //Sets the value of the " << propertyName << " property" << endl;
            }

            functions << setter;
        }

        out << endl;
    }

    // slots - but not property setters
    int defaultArguments = 0;
    for (int islot = mo->methodOffset(); islot < mo->methodCount(); ++islot) {
        const QMetaMethod slot(mo->method(islot));
        if (slot.methodType() != QMetaMethod::Slot)
            continue;

#if 0
        // makes not sense really to respect default arguments...
        if (slot.attributes() & Cloned) {
            ++defaultArguments;
            continue;
        }
#endif

        QByteArray slotSignature(slot.signature());
        QByteArray slotName = slotSignature.left(slotSignature.indexOf('('));
        if (functions.contains(slotName))
            continue;

        if (!(category & OnlyInlines)) {
            out << indent << "/*" << endl << indent << "Method " << slotName << endl;
            QString documentation = qax_docuFromName(currentTypeInfo, QString::fromLatin1(slotName.constData()));
            if (!documentation.isEmpty()) {
                out << endl;
                out << indent << documentation << endl;
            }
            out << indent << "*/" << endl;
        }

        QByteArray slotParameters(joinParameterNames(slot.parameterNames()));
        QByteArray slotTag(slot.tag());
        QByteArray slotType(slot.typeName());

        QByteArray simpleSlotType = slotType;
        simpleSlotType.replace('*', "");
        if (!slotType.contains("::") && qax_qualified_usertypes.contains(simpleSlotType))
            slotType = nameSpace + "::" + slotType;


        QByteArray slotNamedSignature;
        if (slotSignature.endsWith("()")) { // no parameters - no names
            slotNamedSignature = slotSignature;
        } else {
            slotNamedSignature = slotSignature.left(slotSignature.indexOf('(') + 1);
            QByteArray slotSignatureTruncated(slotSignature.mid(slotNamedSignature.length()));
            slotSignatureTruncated.truncate(slotSignatureTruncated.length() - 1);

            QList<QByteArray> signatureSplit = slotSignatureTruncated.split(',');
            QList<QByteArray> parameterSplit;
            if (slotParameters.isEmpty()) { // generate parameter names
                for (int i = 0; i < signatureSplit.count(); ++i)
                    parameterSplit << QByteArray("p") + QByteArray::number(i);
            } else {
                parameterSplit = slotParameters.split(',');
            }
            
            for (int i = 0; i < signatureSplit.count(); ++i) {
                QByteArray parameterType = signatureSplit.at(i);
                if (!parameterType.contains("::") && namespaceForType.contains(parameterType))
                    parameterType = namespaceForType.value(parameterType) + "::" + parameterType;

                slotNamedSignature += constRefify(parameterType);
                slotNamedSignature += " ";
                slotNamedSignature += parameterSplit.at(i);
                if (defaultArguments >= signatureSplit.count() - i) {
                    slotNamedSignature += " = ";
                    slotNamedSignature += parameterType + "()";
                }
                if (i + 1 < signatureSplit.count())
                    slotNamedSignature += ", ";
            }
            slotNamedSignature += ')';
        }

        out << indent << "inline ";

        if (!slotTag.isEmpty())
            out << slotTag << " ";
        if (slotType.isEmpty())
            out << "void ";
        else
            out << slotType << " ";
        if (category & OnlyInlines)
            out << className << "::";

        // Update function name in case of conflicts with QAxBase public virtual functions.
        int parnIdx = slotNamedSignature.indexOf('(');
        QByteArray slotOriginalName =  slotNamedSignature.left(parnIdx);
        if (axBase_vfuncs.contains(slotOriginalName)) {
            QByteArray newSignature = className + "_" + slotOriginalName;
            newSignature += slotNamedSignature.mid(parnIdx);
            qWarning("function name conflits with QAXBase %s changed to %s", slotNamedSignature.constData(), newSignature.constData());
            slotNamedSignature = newSignature;
        }

        out << slotNamedSignature;

        if (category & NoInlines) {
            out << ";" << endl;
        } else {
            out << endl;
            out << indent << "{" << endl;
            
            if (!slotType.isEmpty()) {
                out << indent << "    " << slotType << " qax_result";
                if (slotType.endsWith('*'))
                    out << " = 0";
                out << ";" << endl;
                if (qax_qualified_usertypes.contains(simpleSlotType)) {
                    out << indent << "    qRegisterMetaType(\"" << simpleSlotType << "*\", &qax_result);" << endl;
                    bool foreignNamespace = simpleSlotType.contains("::");
                    if (foreignNamespace)
                        out << "#ifdef QAX_DUMPCPP_" << simpleSlotType.left(simpleSlotType.indexOf(':')).toUpper() << "_H" << endl;
                    out << indent << "    qRegisterMetaType(\"" << simpleSlotType << "\", qax_result);" << endl;
                    if (foreignNamespace)
                        out << "#endif" << endl;
                }
            }
            out << indent << "    void *_a[] = {";
            if (!slotType.isEmpty())
                out << "(void*)&qax_result";
            else
                out << "0";
            if (!slotParameters.isEmpty()) {
                out << ", (void*)&";
                out << slotParameters.replace(",", ", (void*)&");
            }
            out << "};" << endl;

            out << indent << "    qt_metacall(QMetaObject::InvokeMetaMethod, " << islot << ", _a);" << endl;
            if (!slotType.isEmpty())
                out << indent << "    return qax_result;" << endl;
            out << indent << "}" << endl;
        }

        out << endl;
        defaultArguments = 0;
    }

    if (!(category & OnlyInlines)) {
        if (!(category & NoMetaObject)) {
            out << "// meta object functions" << endl;
            out << "    static const QMetaObject staticMetaObject;" << endl;
            out << "    virtual const QMetaObject *metaObject() const { return &staticMetaObject; }" << endl;
            out << "    virtual void *qt_metacast(const char *);" << endl;
        }

        out << "};" << endl;
    }
}

#define addString(string, stringData) \
    out << stringDataLength << ", "; \
    stringData += string; \
    stringDataLength += qstrlen(string); \
    stringData += "\\0"; \
    lineLength += qstrlen(string) + 1; \
    if (lineLength > 200) { stringData += "\"\n    \""; lineLength = 0; } \
    ++stringDataLength;

void generateClassImpl(QTextStream &out, const QMetaObject *mo, const QByteArray &className, const QByteArray &nameSpace, ObjectCategory category)
{
    QByteArray qualifiedClassName;
    if (!nameSpace.isEmpty())
        qualifiedClassName = nameSpace + "::";
    qualifiedClassName += className;

    QByteArray stringData(qualifiedClassName);
    int stringDataLength = stringData.length();
    stringData += "\\0\"\n";
    ++stringDataLength;
    int lineLength = 0;

    int classInfoCount = mo->classInfoCount() - mo->classInfoOffset();
    int enumCount = mo->enumeratorCount() - mo->enumeratorOffset();
    int methodCount = mo->methodCount() - mo->methodOffset();
    int propertyCount = mo->propertyCount() - mo->propertyOffset();
    int enumStart = 10;

    out << "static const uint qt_meta_data_" << qualifiedClassName.replace(':', '_') << "[] = {" << endl;
    out << endl;
    out << " // content:" << endl;
    out << "       1,       // revision" << endl;
    out << "       0,       // classname" << endl;
    out << "       " << classInfoCount << ",    " << (classInfoCount ? enumStart : 0) << ", // classinfo" << endl;
    enumStart += classInfoCount * 2;
    out << "       " << methodCount << ",    " << (methodCount ? enumStart : 0) << ", // methods" << endl;
    enumStart += methodCount * 5;
    out << "       " << propertyCount << ",    " << (propertyCount ? enumStart : 0) << ", // properties" << endl;
    enumStart += propertyCount * 3;
    out << "       " << enumCount << ",    " << (enumCount ? enumStart : 0)
        << ", // enums/sets" << endl;
    out << endl;

    if (classInfoCount) {
        out << " // classinfo: key, value" << endl;
        stringData += "    \"";
        for (int i = 0; i < classInfoCount; ++i) {
            QMetaClassInfo classInfo = mo->classInfo(i + mo->classInfoOffset());
            out << "       ";
            addString(classInfo.name(), stringData);
            addString(classInfo.value(), stringData);
            out << endl;
        }
        stringData += "\"\n";
        out << endl;
    }
    if (methodCount) {
        out << " // signals: signature, parameters, type, tag, flags" << endl;
        stringData += "    \"";
        for (int i = 0; i < methodCount; ++i) {
            const QMetaMethod signal(mo->method(i + mo->methodOffset()));
            if (signal.methodType() != QMetaMethod::Signal)
                continue;
            out << "       ";
            addString(signal.signature(), stringData);
            addString(joinParameterNames(signal.parameterNames()), stringData);
            addString(signal.typeName(), stringData);
            addString(signal.tag(), stringData);
            out << (AccessProtected | signal.attributes() | MemberSignal) << "," << endl;
        }
        stringData += "\"\n";
        out << endl;

        out << " // slots: signature, parameters, type, tag, flags" << endl;
        stringData += "    \"";
        for (int i = 0; i < methodCount; ++i) {
            const QMetaMethod slot(mo->method(i + mo->methodOffset()));
            if (slot.methodType() != QMetaMethod::Slot)
                continue;
            out << "       ";
            addString(slot.signature(), stringData);
            addString(joinParameterNames(slot.parameterNames()), stringData);
            addString(slot.typeName(), stringData);
            addString(slot.tag(), stringData);
            out << (0x01 | slot.attributes() | MemberSlot) << "," << endl;
        }
        stringData += "\"\n";
        out << endl;
    }
    if (propertyCount) {
        out << " // properties: name, type, flags" << endl;
        stringData += "    \"";
        for (int i = 0; i < propertyCount; ++i) {
            QMetaProperty property = mo->property(i + mo->propertyOffset());
            out << "       ";
            addString(property.name(), stringData);
            addString(property.typeName(), stringData);

            uint flags = 0;
            uint vartype = property.type();
            if (vartype != QVariant::Invalid && vartype != QVariant::UserType)
                flags = vartype << 24;
            else if (QByteArray(property.typeName()) == "QVariant")
                flags |= 0xff << 24;

            if (property.isReadable())
                flags |= Readable;
            if (property.isWritable())
                flags |= Writable;
            if (property.isEnumType())
                flags |= EnumOrFlag;
            if (property.isDesignable())
                flags |= Designable;
            if (property.isScriptable())
                flags |= Scriptable;
            if (property.isStored())
                flags |= Stored;
            if (property.isEditable())
                flags |= Editable;

            out << "0x" << QString::number(flags, 16).rightJustified(8, '0') << ", \t\t // " << property.typeName() << " " << property.name();
            out << endl;
        }
        stringData += "\"\n";
        out << endl;
    }

    QByteArray enumStringData;
    if (enumCount) {
        out << " // enums: name, flags, count, data" << endl;
        enumStringData += "    \"";
        enumStart += enumCount * 4;
        for (int i = 0; i < enumCount; ++i) {
            QMetaEnum enumerator = mo->enumerator(i + mo->enumeratorOffset());
            out << "       ";
            addString(enumerator.name(), enumStringData);
            out << (enumerator.isFlag() ? "0x1" : "0x0") << ", " << enumerator.keyCount() << ", " << enumStart << ", " << endl;
            enumStart += enumerator.keyCount() * 2;
        }
        enumStringData += "\"\n";
        out << endl;

        out << " // enum data: key, value" << endl;
        for (int i = 0; i < enumCount; ++i) {
            enumStringData += "    \"";
            QMetaEnum enumerator = mo->enumerator(i + mo->enumeratorOffset());
            for (int j = 0; j < enumerator.keyCount(); ++j) {
                out << "       ";
                addString(enumerator.key(j), enumStringData);
                if (nameSpace.isEmpty())
                    out << className << "::";
                else
                    out << nameSpace << "::";
                out << enumerator.key(j) << "," << endl;
            }
            enumStringData += "\"\n";
        }
        out << endl;
    }
    out << "        0        // eod" << endl;
    out << "};" << endl;
    out << endl;

    QByteArray stringGenerator;

    if (!nameSpace.isEmpty()) {
        static bool firstStringData = true;
        if (firstStringData) { // print enums only once
            firstStringData = false;
            if (!enumStringData.isEmpty()) {
                // Maximum string length supported is 64K
                int maxStringLength = 65535;
                if (enumStringData.size() < maxStringLength)  {
                    out << "static const char qt_meta_enumstringdata_" << nameSpace << "[] = {" << endl;
                    out << enumStringData << endl;
                    out << "};" << endl;
                    out << endl;
                } else {
                    // split the string into fragments of 64k
                    int fragments = (enumStringData.size() / maxStringLength);
                    fragments += (enumStringData.size() % maxStringLength) ? 1 : 0;
                    int i, index;
                    // define the fragments (qt_meta_enumstringdata_<nameSpace>fragment#)
                    for (i = 0 , index = 0; i < fragments; i++, index += maxStringLength) {
                        out << "static const char qt_meta_enumstringdata_" << nameSpace << "fragment"<< QString::number(i) << "[] = {" << endl;
                        QByteArray fragment = enumStringData.mid(index, maxStringLength);
                        if (!(fragment[0] == ' ' || fragment[0] == '\n' || fragment[0] == '\"'))
                            out << "\"";
                        out << fragment;
                        int endIx  = fragment.size() - 1;
                        if (!(fragment[endIx] == ' ' || fragment[endIx] == '\n' || fragment[endIx] == '\"' || fragment[endIx] == '\0'))
                            out << "\"" << endl;
                        else 
                            out << endl;
                        out << "};" << endl;
                    }
                    // original array definition, size will be the combined size of the arrays defined above
                    out << "static char qt_meta_enumstringdata_" << nameSpace << "[" << endl;
                    for (i = 0; i < fragments; i++, index += maxStringLength) {
                         out << "        ";
                        if (i)
                            out << "+ ";
                        out << "sizeof(qt_meta_enumstringdata_" << nameSpace << "fragment"<< QString::number(i) <<")" << endl;
                    }
                    out << "] = {0};" << endl << endl;
                    // this class will initializes the original array in constructor
                    out << "class qt_meta_enumstringdata_" << nameSpace << "_init " << endl <<"{" <<endl;
                    out << "public:"<<endl;
                    out << "    qt_meta_enumstringdata_" << nameSpace << "_init() " << endl <<"    {" <<endl;
                    out << "        int index = 0;" << endl;
                    for (i = 0; i < fragments; i++, index += maxStringLength) {
                        out << "        memcpy(qt_meta_enumstringdata_" << nameSpace << " + index, " <<"qt_meta_enumstringdata_" << nameSpace << "fragment"<< QString::number(i);
                        out << ", sizeof(qt_meta_enumstringdata_" << nameSpace << "fragment"<< QString::number(i) <<") - 1);" << endl;
                        out << "        index += sizeof(qt_meta_enumstringdata_" << nameSpace << "fragment"<< QString::number(i) <<") - 1;" << endl;
                    }
                    out << "    }" << endl << "};" << endl;
                    // a global variable of the class
                    out << "static qt_meta_enumstringdata_" << nameSpace << "_init qt_meta_enumstringdata_"  << nameSpace << "_init_instance;" << endl << endl;
                }
            }
        }
        stringGenerator = "qt_meta_stringdata_" + qualifiedClassName.replace(':','_') + "()";
        out << "static const char *" << stringGenerator << " {" << endl;
        QList<QByteArray> splitStrings;

        // workaround for compilers that can't handle string literals longer than 64k
        int splitCount = 0;
        do {
            int lastNewline = stringData.lastIndexOf('\n', 64000);
            QByteArray splitString = stringData.left(lastNewline);

            splitStrings << splitString;
            out << "    static const char stringdata" << splitCount << "[] = {" << endl;
            out << "    \"" << splitString << endl;
            out << "    };" << endl;
            stringData = stringData.mid(lastNewline + 1);
            if (stringData.startsWith("    \""))
                stringData = stringData.mid(5);
            ++splitCount;
        } while (!stringData.isEmpty());

        out << "    static char data[";
        for (int i = 0; i < splitCount; ++i) {
            out << "sizeof(stringdata" << i << ") + ";
        }
        if (!enumStringData.isEmpty()) {
            out << "sizeof(qt_meta_enumstringdata_" << nameSpace << ")";
        } else {
            out << "0";
        }
        out << "];" << endl;
        out << "    if (!data[0]) {" << endl;
        out << "        int index = 0;" << endl;

        int dataIndex = 0;
        for (int i = 0; i < splitCount; ++i) {
            out << "        memcpy(data + index";
            out << ", stringdata" << i << ", sizeof(stringdata" << i << ") - 1);" << endl;
            out << "        index += sizeof(stringdata" << i << ") - 1;" << endl;
            dataIndex += splitStrings.at(i).length();
        }
        if (!enumStringData.isEmpty()) {
            out << "        memcpy(data + index, qt_meta_enumstringdata_" << nameSpace << ", sizeof(qt_meta_enumstringdata_" << nameSpace << "));" << endl;
        }
        out << "    }" << endl;
        out << endl;
        out << "    return data;" << endl;
        out << "};" << endl;
        out << endl;
    } else {
        stringData += enumStringData;
        stringGenerator = "qt_meta_stringdata_" + qualifiedClassName.replace(':','_');
        out << "static const char qt_meta_stringdata_" << stringGenerator << "[] = {" << endl;
        out << "    \"" << stringData << endl;
        out << "};" << endl;
        out << endl;
    }

    out << "const QMetaObject " << className << "::staticMetaObject = {" << endl;
    if (category & ActiveX)
        out << "{ &QWidget::staticMetaObject," << endl;
    else
        out << "{ &QObject::staticMetaObject," << endl;
    out << stringGenerator << "," << endl;
    out << "qt_meta_data_" << qualifiedClassName.replace(':','_') << " }" << endl;
    out << "};" << endl;
    out << endl;

    out << "void *" << className << "::qt_metacast(const char *_clname)" << endl;
    out << "{" << endl;
    out << "    if (!_clname) return 0;" << endl;
    out << "    if (!strcmp(_clname, " << stringGenerator << "))" << endl;
    out << "        return static_cast<void*>(const_cast<" << className << "*>(this));" << endl;
    if (category & ActiveX)
        out << "    return QAxWidget::qt_metacast(_clname);" << endl;
    else
        out << "    return QAxObject::qt_metacast(_clname);" << endl;
    out << "}" << endl;
}

bool generateClass(QAxObject *object, const QByteArray &className, const QByteArray &nameSpace, const QByteArray &outname, ObjectCategory category)
{
    IOleControl *control = 0;
    object->queryInterface(IID_IOleControl, (void**)&control);
    if (control) {
        category = ActiveX;
        control->Release();
    }

    const QMetaObject *mo = object->metaObject();

    if (!nameSpace.isEmpty() && !(category & NoDeclaration)) {
        QFile outfile(QString::fromLatin1(nameSpace.toLower().constData()) + QLatin1String(".h"));
		if (!outfile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning("dumpcpp: Could not open output file '%s'", qPrintable(outfile.fileName()));
            return false;
        }
        QTextStream out(&outfile);

        out << "/****************************************************************************" << endl;
        out << "**" << endl;
        out << "** Namespace " << nameSpace << " generated by dumpcpp" << endl;
        out << "**" << endl;
        out << "****************************************************************************/" << endl;
        out << endl;

        writeHeader(out, nameSpace, outfile.fileName());
        generateNameSpace(out, mo, nameSpace);

        // close namespace file
        out << "};" << endl;
        out << endl;

        out << "#endif" << endl;
        out << endl;
    }

    if (!(category & NoDeclaration)) {
        QFile outfile(QString::fromLatin1(outname.constData()) + QLatin1String(".h"));
        if (!outfile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning("dumpcpp: Could not open output file '%s'", qPrintable(outfile.fileName()));
            return false;
        }
        QTextStream out(&outfile);

        out << "/****************************************************************************" << endl;
        out << "**" << endl;
        out << "** Class declaration generated by dumpcpp" << endl;
        out << "**" << endl;
        out << "****************************************************************************/" << endl;
        out << endl;

        out << "#include <qdatetime.h>" << endl;
        if (category & ActiveX)
            out << "#include <qaxwidget.h>" << endl;
        else
            out << "#include <qaxobject.h>" << endl;
        out << endl;

        out << "struct IDispatch;" << endl,
        out << endl;

        if (!nameSpace.isEmpty()) {
            out << "#include \"" << nameSpace.toLower() << ".h\"" << endl;
            out << endl;
            out << "namespace " << nameSpace << " {" << endl;
        }

        generateClassDecl(out, object->control(), mo, className, nameSpace, category);

        if (!nameSpace.isEmpty()) {
            out << endl;
            out << "};" << endl;
        }
    }

    if (!(category & (NoMetaObject|NoImplementation))) {
        QFile outfile(QString::fromLatin1(outname.constData()) + QLatin1String(".cpp"));
        if (!outfile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning("dumpcpp: Could not open output file '%s'", qPrintable(outfile.fileName()));
            return false;
        }
        QTextStream out(&outfile);

        out << "#include <qmetaobject.h>" << endl;
        out << "#include \"" << outname << ".h\"" << endl;
        out << endl;

        if (!nameSpace.isEmpty()) {
            out << "using namespace " << nameSpace << ";" << endl;
            out << endl;
        }

        generateClassImpl(out, mo, className, nameSpace, category);
    }

    return true;
}

bool generateTypeLibrary(const QByteArray &typeLib, const QByteArray &outname, ObjectCategory category)
{
    QString typeLibFile(QString::fromLatin1(typeLib.constData()));
    typeLibFile = typeLibFile.replace(QLatin1Char('/'), QLatin1Char('\\'));
    QString cppFile(QString::fromLatin1(outname.constData()));

    ITypeLib *typelib;
    LoadTypeLibEx(reinterpret_cast<const wchar_t *>(typeLibFile.utf16()), REGKIND_NONE, &typelib);
    if (!typelib) {
        qWarning("dumpcpp: loading '%s' as a type library failed", qPrintable(typeLibFile));
        return false;
    }

    QString libName;
    BSTR nameString;
    typelib->GetDocumentation(-1, &nameString, 0, 0, 0);
    libName = QString::fromWCharArray(nameString);
    SysFreeString(nameString);
    if (!nameSpace.isEmpty())
        libName = QString(nameSpace);

    QString libVersion(QLatin1String("1.0"));

    TLIBATTR *tlibattr = 0;
    typelib->GetLibAttr(&tlibattr);
    if (tlibattr) {
        libVersion = QString::fromLatin1("%1.%2").arg(tlibattr->wMajorVerNum).arg(tlibattr->wMinorVerNum);
        typelib->ReleaseTLibAttr(tlibattr);
    }

    if (cppFile.isEmpty())
        cppFile = libName.toLower();

    if (cppFile.isEmpty()) {
        qWarning("dumpcpp: no output filename provided, and cannot deduce output filename");
        return false;
    }

    QMetaObject *namespaceObject = qax_readEnumInfo(typelib, 0);

    QFile implFile(cppFile + QLatin1String(".cpp"));
    QTextStream implOut(&implFile);
    if (!(category & (NoMetaObject|NoImplementation))) {
        if (!implFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning("dumpcpp: Could not open output file '%s'", qPrintable(implFile.fileName()));
            return false;
        }

        implOut << "/****************************************************************************" << endl;
        implOut << "**" << endl;
        implOut << "** Metadata for " << libName << " generated by dumpcpp from type library" << endl;
        implOut << "** " << typeLibFile << endl;
        implOut << "**" << endl;
        implOut << "****************************************************************************/" << endl;
        implOut << endl;

        implOut << "#define QAX_DUMPCPP_" << libName.toUpper() << "_NOINLINES" << endl;

        implOut << "#include \"" << cppFile << ".h\"" << endl;
        implOut << endl;
        implOut << "using namespace " << libName << ";" << endl;
        implOut << endl;
    }

    QFile declFile(cppFile + QLatin1String(".h"));
    QTextStream declOut(&declFile);
    QByteArray classes;
    QTextStream classesOut(&classes, QIODevice::WriteOnly);
    QByteArray inlines;
    QTextStream inlinesOut(&inlines, QIODevice::WriteOnly);

    QMap<QByteArray, QList<QByteArray> > namespaces;

    if(!(category & NoDeclaration)) {
        if (!declFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning("dumpcpp: Could not open output file '%s'", qPrintable(declFile.fileName()));
            return false;
        }

        declOut << "/****************************************************************************" << endl;
        declOut << "**" << endl;
        declOut << "** Namespace " << libName << " generated by dumpcpp from type library" << endl;
        declOut << "** " << typeLibFile << endl;
        declOut << "**" << endl;
        declOut << "****************************************************************************/" << endl;
        declOut << endl;

        QFileInfo cppFileInfo(cppFile);
        writeHeader(declOut, libName.toLatin1(), cppFileInfo.fileName());

        UINT typeCount = typelib->GetTypeInfoCount();
        if (declFile.isOpen()) {
            declOut << endl;
            declOut << "// Referenced namespace" << endl;
            for (UINT index = 0; index < typeCount; ++index) {
                ITypeInfo *typeinfo = 0;
                typelib->GetTypeInfo(index, &typeinfo);
                if (!typeinfo)
                    continue;

                TYPEATTR *typeattr;
                typeinfo->GetTypeAttr(&typeattr);
                if (!typeattr) {
                    typeinfo->Release();
                    continue;
                }

                TYPEKIND typekind;
                typelib->GetTypeInfoType(index, &typekind);

                QMetaObject *metaObject = 0;

                // trigger meta object to collect references to other type libraries
                switch (typekind) {
                case TKIND_COCLASS:
                    if (category & ActiveX)
                        metaObject = qax_readClassInfo(typelib, typeinfo, &QWidget::staticMetaObject);
                    else
                        metaObject = qax_readClassInfo(typelib, typeinfo, &QObject::staticMetaObject);
                    break;
                case TKIND_DISPATCH:
                    if (category & ActiveX)
                        metaObject = qax_readInterfaceInfo(typelib, typeinfo, &QWidget::staticMetaObject);
                    else
                        metaObject = qax_readInterfaceInfo(typelib, typeinfo, &QObject::staticMetaObject);
                    break;
                case TKIND_RECORD:
                case TKIND_ENUM:
                case TKIND_INTERFACE: // only for forward declarations
                    {
                        QByteArray className;
                        BSTR bstr;
                        if (S_OK != typeinfo->GetDocumentation(-1, &bstr, 0, 0, 0))
                            break;
                        className = QString::fromWCharArray(bstr).toLatin1();
                        SysFreeString(bstr);
                        switch (typekind) {
                        case TKIND_RECORD:
                            className = "struct " + className;
                            break;
                        case TKIND_ENUM:
                            className = "enum " + className;
                            break;
                        default:
                            break;
                        }
                        namespaces[libName.toLatin1()].append(className);
                        if (!qax_qualified_usertypes.contains(className))
                            qax_qualified_usertypes << className;
                    }
                    break;
                default:
                    break;
                }

                delete metaObject;
                typeinfo->ReleaseTypeAttr(typeattr);
                typeinfo->Release();
            }

            for (int i = 0; i < qax_qualified_usertypes.count(); ++i) {
                QByteArray refType = qax_qualified_usertypes.at(i);
                QByteArray refTypeLib;
                if (refType.contains("::")) {
                    refTypeLib = refType;
                    refType = refType.mid(refType.lastIndexOf("::") + 2);
                    if (refTypeLib.contains(' ')) {
                        refType = refTypeLib.left(refTypeLib.indexOf(' ')) + ' ' + refType;
                    }
                    refTypeLib = refTypeLib.left(refTypeLib.indexOf("::"));
                    refTypeLib = refTypeLib.mid(refTypeLib.lastIndexOf(' ') + 1);
                    namespaces[refTypeLib].append(refType);
                } else {
                    namespaces[libName.toLatin1()].append(refType);
                }
            }

            QList<QByteArray> keys = namespaces.keys();
            for (int n = 0; n < keys.count(); ++n) {
                QByteArray nspace = keys.at(n);
                if (QString::fromLatin1(nspace.constData()) != libName) {
                    declOut << "namespace " << nspace << " {" << endl;
                    QList<QByteArray> classList = namespaces.value(nspace);
                    for (int c = 0; c < classList.count(); ++c) {
                        QByteArray className = classList.at(c);
                        if (className.contains(' ')) {
                            declOut << "    " << className << ";" << endl;
                            namespaceForType.insert(className.mid(className.indexOf(' ') + 1), nspace);
                        } else {
                            declOut << "    class " << className << ";" << endl;
                            namespaceForType.insert(className, nspace);
                            namespaceForType.insert(className + "*", nspace);
                        }
                    }
                    declOut << "}" << endl << endl;
                }
            }

            declOut << endl;
        }
        generateNameSpace(declOut, namespaceObject, libName.toLatin1());

        QList<QByteArray> classList = namespaces.value(libName.toLatin1());
        if (classList.count())
            declOut << "// forward declarations" << endl;
        for (int c = 0; c < classList.count(); ++c) {
            QByteArray className = classList.at(c);
            if (className.contains(' ')) {
                declOut << "    " << className << ";" << endl;
                namespaceForType.insert(className.mid(className.indexOf(' ') + 1), libName.toLatin1());
            } else {
                declOut << "    class " << className << ";" << endl;
                namespaceForType.insert(className, libName.toLatin1());
                namespaceForType.insert(className + "*", libName.toLatin1());
            }
        }

        declOut << endl;
    }

    QList<QByteArray> subtypes;

    UINT typeCount = typelib->GetTypeInfoCount();
    for (UINT index = 0; index < typeCount; ++index) {
        ITypeInfo *typeinfo = 0;
        typelib->GetTypeInfo(index, &typeinfo);
        if (!typeinfo)
            continue;

        TYPEATTR *typeattr;
        typeinfo->GetTypeAttr(&typeattr);
        if (!typeattr) {
            typeinfo->Release();
            continue;
        }

        TYPEKIND typekind;
        typelib->GetTypeInfoType(index, &typekind);

        uint object_category = category;
        if (!(typeattr->wTypeFlags & TYPEFLAG_FCANCREATE))
            object_category |= SubObject;
        else if (typeattr->wTypeFlags & TYPEFLAG_FCONTROL)
            object_category |= ActiveX;

        QMetaObject *metaObject = 0;
        QUuid guid(typeattr->guid);

        if (!(object_category & ActiveX)) {
            QSettings settings(QLatin1String("HKEY_LOCAL_MACHINE\\Software\\Classes\\CLSID\\") + guid.toString(), QSettings::NativeFormat);
            if (settings.childGroups().contains(QLatin1String("Control"))) {
                object_category |= ActiveX;
                object_category &= ~SubObject;
            }
        }

        switch (typekind) {
        case TKIND_COCLASS:
            if (object_category & ActiveX)
                metaObject = qax_readClassInfo(typelib, typeinfo, &QWidget::staticMetaObject);
            else
                metaObject = qax_readClassInfo(typelib, typeinfo, &QObject::staticMetaObject);
            break;
        case TKIND_DISPATCH:
            if (object_category & ActiveX)
                metaObject = qax_readInterfaceInfo(typelib, typeinfo, &QWidget::staticMetaObject);
            else
                metaObject = qax_readInterfaceInfo(typelib, typeinfo, &QObject::staticMetaObject);
            break;
        case TKIND_INTERFACE: // only stub
            {
                QByteArray className;
                BSTR bstr;
                if (S_OK != typeinfo->GetDocumentation(-1, &bstr, 0, 0, 0))
                    break;
                className = QString::fromWCharArray(bstr).toLatin1();
                SysFreeString(bstr);

                declOut << "// stub for vtable-only interface" << endl;
                declOut << "class " << className << " : public QAxObject {};" << endl << endl;
            }
            break;
        default:
            break;
        }

        if (metaObject) {
            currentTypeInfo = typeinfo;
            QByteArray className(metaObject->className());
            if (!(typeattr->wTypeFlags & TYPEFLAG_FDUAL) 
                && (metaObject->propertyCount() - metaObject->propertyOffset()) == 1 
                && className.contains("Events")) {
                declOut << "// skipping event interface " << className << endl << endl;
            } else {
                if (declFile.isOpen()) {
                    if (typeattr->wTypeFlags & TYPEFLAG_FLICENSED)
                        object_category |= Licensed;
                    if (typekind == TKIND_COCLASS) { // write those later...
                        generateClassDecl(classesOut, guid.toString(), metaObject, className, libName.toLatin1(), (ObjectCategory)(object_category|NoInlines));
                        classesOut << endl;
                    } else {
                        generateClassDecl(declOut, guid.toString(), metaObject, className, libName.toLatin1(), (ObjectCategory)(object_category|NoInlines));
                        declOut << endl;
                    }
                    subtypes << className;
                    generateClassDecl(inlinesOut, guid.toString(), metaObject, className, libName.toLatin1(), (ObjectCategory)(object_category|OnlyInlines));
                    inlinesOut << endl;
                }
                if (implFile.isOpen()) {
                    generateClassImpl(implOut, metaObject, className, libName.toLatin1(), (ObjectCategory)object_category);
                    implOut  << endl;
                }
            }
            currentTypeInfo = 0;
        }

        delete metaObject;

        typeinfo->ReleaseTypeAttr(typeattr);
        typeinfo->Release();
    }

    delete namespaceObject;

    classesOut.flush();
    inlinesOut.flush();

    if (declFile.isOpen()) {
        if (classes.size()) {
            declOut << "// Actual coclasses" << endl;
            declOut << classes;
        }
        if (inlines.size()) {
            declOut << "// member function implementation" << endl;
            declOut << "#ifndef QAX_DUMPCPP_" << libName.toUpper() << "_NOINLINES" << endl;
            declOut << inlines << endl;
            declOut << "#endif" << endl << endl;
        }
        // close namespace
        declOut << "}" << endl;
        declOut << endl;

        // partial template specialization for qMetaTypeConstructHelper
        for (int t = 0; t < subtypes.count(); ++t) {
            QByteArray subType(subtypes.at(t));
            declOut << "template<>" << endl;
            declOut << "inline void *qMetaTypeConstructHelper(const " << libName << "::" << subType << " *t)" << endl;
            declOut << "{ Q_ASSERT(!t); return new " << libName << "::" << subType << "; }" << endl;
            declOut << endl;
        }

        declOut << "#endif" << endl;
        declOut << endl;
    }

    typelib->Release();
    return true;
}

QT_END_NAMESPACE

QT_USE_NAMESPACE

int main(int argc, char **argv)
{
    qax_dispatchEqualsIDispatch = false;

    CoInitialize(0);

    uint category = DefaultObject;

    enum State {
        Default = 0,
        Output,
        NameSpace,
        GetTypeLib
    } state;
    state = Default;
    
    QByteArray outname;
    QByteArray typeLib;
    
    for (int a = 1; a < argc; ++a) {
        QByteArray arg(argv[a]);
        const char first = arg[0];
        switch(state) {
        case Default:
            if (first == '-' || first == '/') {
                arg = arg.mid(1);
                arg.toLower();

                if (arg == "o") {
                    state = Output;
                } else if (arg == "n") {
                    state = NameSpace;
                } else if (arg == "v") {
                    qWarning("dumpcpp: Version 1.0");
                    return 0;
                } else if (arg == "nometaobject") {
                    category |= NoMetaObject;
                } else if (arg == "impl") {
                    category |= NoDeclaration;
                } else if (arg == "decl") {
                    category |= NoImplementation;
                } else if (arg == "donothing") {
                    category = DoNothing;
                    break;
                } else if (arg == "compat") {
                    qax_dispatchEqualsIDispatch = true;
                    break;
                } else if (arg == "getfile") {
                    state = GetTypeLib;
                    break;
                } else if (arg == "h") {
                    qWarning("dumpcpp Version1.0\n\n"
                        "Generate a C++ namespace from a type library.\n\n"
                        "Usage:\n"
                        "dumpcpp input [-[-n <namespace>] [-o <filename>]\n\n"
                        "   input:     A type library file, type library ID, ProgID or CLSID\n\n"
                        "Optional parameters:\n"
                        "   namespace: The name of the generated C++ namespace\n"
                        "   filename:  The file name (without extension) of the generated files\n"
                        "\n"
                        "Other parameters:\n"
                        "   -nometaobject Don't generate meta object information (no .cpp file)\n"
                        "   -impl Only generate the .cpp file\n"
                        "   -decl Only generate the .h file\n"
                        "   -compat Treat all coclass parameters as IDispatch\n"
                        "\n"
                        "Examples:\n"
                        "   dumpcpp Outlook.Application -o outlook\n"
                        "   dumpcpp {3B756301-0075-4E40-8BE8-5A81DE2426B7}\n"
                        "\n");
                    return 0;
                }
            } else {
                typeLib = arg;
            }
            break;

        case Output:
            outname = arg;
            state = Default;
            break;

        case NameSpace:
            nameSpace = arg;
            state = Default;
            break;

        case GetTypeLib:
            typeLib = arg;
            state = Default;
            category = TypeLibID;
            break;
        default:
            break;
        }
    }

    if (category == TypeLibID) {
        QSettings settings(QLatin1String("HKEY_LOCAL_MACHINE\\Software\\Classes\\TypeLib\\") +
                           QString::fromLatin1(typeLib.constData()), QSettings::NativeFormat);
        typeLib = QByteArray();
        QStringList codes = settings.childGroups();
        for (int c = 0; c < codes.count(); ++c) {
            typeLib = settings.value(QLatin1String("/") + codes.at(c) + QLatin1String("/0/win32/.")).toByteArray();
            if (QFile::exists(QString::fromLatin1(typeLib))) {
                break;
            }
        }

        if (!typeLib.isEmpty())
            fprintf(stdout, "\"%s\"\n", typeLib.data());
        return 0;
    }

    if (category == DoNothing)
        return 0;
    
    if (typeLib.isEmpty()) {
        qWarning("dumpcpp: No object class or type library name provided.\n"
            "         Use -h for help.");
        return -1;
    }

    // not a file - search registry
    if (!QFile::exists(QString::fromLatin1(typeLib.constData()))) {
        bool isObject = false;
        QSettings settings(QLatin1String("HKEY_LOCAL_MACHINE\\Software\\Classes"), QSettings::NativeFormat);

        // regular string and not a file - must be ProgID
        if (typeLib.at(0) != '{') {
            CLSID clsid;
            if (CLSIDFromProgID(reinterpret_cast<const wchar_t *>(QString(QLatin1String(typeLib)).utf16()), &clsid) != S_OK) {
                qWarning("dumpcpp: '%s' is not a type library and not a registered ProgID", typeLib.constData());
                return -2;
            }
            QUuid uuid(clsid);
            typeLib = uuid.toString().toLatin1();
            isObject = true;
        }

        // check if CLSID
        if (!isObject) {
            QVariant test = settings.value(QLatin1String("/CLSID/") +
                                           QString::fromLatin1(typeLib.constData()) + QLatin1String("/."));
            isObject = test.isValid();
        }

        // search typelib ID for CLSID
        if (isObject)
            typeLib = settings.value(QLatin1String("/CLSID/") +
                                     QString::fromLatin1(typeLib.constData()) + QLatin1String("/Typelib/.")).toByteArray();

        // interpret input as type library ID
        QString key = QLatin1String("/TypeLib/") + QLatin1String(typeLib);
        settings.beginGroup(key);
        QStringList versions = settings.childGroups();
        QStringList codes;
        if (versions.count()) {
            settings.beginGroup(QLatin1String("/") + versions.last());
            codes = settings.childGroups();
            key += QLatin1String("/") + versions.last();
            settings.endGroup();
        }
        settings.endGroup();

        for (int c = 0; c < codes.count(); ++c) {
            typeLib = settings.value(key + QLatin1String("/") + codes.at(c) + QLatin1String("/win32/.")).toByteArray();
            if (QFile::exists(QString::fromLatin1(typeLib.constData()))) {
                break;
            }
        }
    }

    if (!QFile::exists(QString::fromLatin1(typeLib.constData()))) {
        qWarning("dumpcpp: type library '%s' not found", typeLib.constData());
        return -2;
    }

    if (!generateTypeLibrary(typeLib, outname, (ObjectCategory)category)) {
        qWarning("dumpcpp: error processing type library '%s'", typeLib.constData());
        return -1;
    }

    return 0;
}
