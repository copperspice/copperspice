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

#include "signalslot_utils_p.h"

#include <qdesigner_membersheet_p.h>
#include <widgetdatabase_p.h>
#include <metadatabase_p.h>

#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerMetaDataBaseInterface>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerLanguageExtension>

#include <QtCore/QPair>

QT_BEGIN_NAMESPACE

typedef QPair<QString, QString> ClassNameSignaturePair;

// Find all member functions that match a predicate on the signature string
// using the member sheet and the fake methods stored in the widget
// database and the meta data base.
// Assign a pair of <classname,  signature> to OutputIterator.

template <class SignaturePredicate, class OutputIterator>
static void memberList(QDesignerFormEditorInterface *core,
                       QObject *object,
                       qdesigner_internal::MemberType member_type,
                       bool showAll,
                       SignaturePredicate predicate,
                       OutputIterator it)
{
    if (!object)
        return;
    // 1) member sheet
    const QDesignerMemberSheetExtension *members = qt_extension<QDesignerMemberSheetExtension*>(core->extensionManager(), object);
    Q_ASSERT(members != 0);
    const int count = members->count();
    for (int i = 0; i < count; ++i) {
        if (!members->isVisible(i))
            continue;

        if (member_type == qdesigner_internal::SignalMember && !members->isSignal(i))
            continue;

        if (member_type == qdesigner_internal::SlotMember && !members->isSlot(i))
            continue;

        if (!showAll && members->inheritedFromWidget(i))
            continue;

        const QString signature = members->signature(i);
        if (predicate(signature)) {
            *it = ClassNameSignaturePair(members->declaredInClass(i), signature);
            ++it;
        }
    }
    // 2) fake slots from widget DB
    const qdesigner_internal::WidgetDataBase *wdb = qobject_cast<qdesigner_internal::WidgetDataBase *>(core->widgetDataBase());
    if (!wdb)
        return;
    const int idx = wdb->indexOfObject(object);
    Q_ASSERT(idx != -1);
    // get the promoted class name
    const qdesigner_internal::WidgetDataBaseItem *wdbItem = static_cast<qdesigner_internal::WidgetDataBaseItem *>(wdb->item(idx));
    const QString className = wdbItem->name();

    const QStringList wdbFakeMethods = member_type == qdesigner_internal::SlotMember ? wdbItem->fakeSlots() : wdbItem->fakeSignals();
    if (!wdbFakeMethods.empty())
        foreach (const QString &fakeMethod, wdbFakeMethods)
            if (predicate(fakeMethod)) {
                *it = ClassNameSignaturePair(className, fakeMethod);
                ++it;
            }
    // 3) fake slots from meta DB
    qdesigner_internal::MetaDataBase *metaDataBase = qobject_cast<qdesigner_internal::MetaDataBase *>(core->metaDataBase());
    if (!metaDataBase)
        return;

    if (const qdesigner_internal::MetaDataBaseItem *mdbItem = metaDataBase->metaDataBaseItem(object)) {
        const QStringList mdbFakeMethods =  member_type == qdesigner_internal::SlotMember ? mdbItem->fakeSlots() : mdbItem->fakeSignals();
        if (!mdbFakeMethods.empty())
            foreach (const QString &fakeMethod, mdbFakeMethods)
                if (predicate(fakeMethod)) {
                    *it = ClassNameSignaturePair(className, fakeMethod);
                    ++it;
                }
    }
}

namespace {
    // Predicate that matches the exact signature string
    class EqualsPredicate {
    public:
        EqualsPredicate(const QString &pattern) : m_pattern(pattern) {}
        bool operator()(const QString &s) const { return s == m_pattern; }
    private:
        const QString  m_pattern;
    };
    // Predicate for a QString member signature that matches signals up with slots and vice versa
    class SignalMatchesSlotPredicate {
    public:
        SignalMatchesSlotPredicate(QDesignerFormEditorInterface *core, const QString &peer, qdesigner_internal::MemberType memberType);
        bool operator()(const QString &s) const;

    private:
        bool signalMatchesSlot(const QString &signal, const QString &slot) const;

        const QString  m_peer;
        qdesigner_internal::MemberType m_memberType;
        const QDesignerLanguageExtension *m_lang;
    };

    SignalMatchesSlotPredicate::SignalMatchesSlotPredicate(QDesignerFormEditorInterface *core, const QString &peer, qdesigner_internal::MemberType memberType) :
        m_peer(peer),
        m_memberType(memberType),
        m_lang(qt_extension<QDesignerLanguageExtension*>(core->extensionManager(), core))
    {
    }

    bool SignalMatchesSlotPredicate::operator()(const QString &s) const
    {
        return m_memberType == qdesigner_internal::SlotMember ? signalMatchesSlot(m_peer, s) :  signalMatchesSlot(s, m_peer);
    }

    bool SignalMatchesSlotPredicate::signalMatchesSlot(const QString &signal, const QString &slot) const
    {
        if (m_lang)
            return m_lang->signalMatchesSlot(signal, slot);

        return QDesignerMemberSheet::signalMatchesSlot(signal, slot);
    }

    // Output iterator for a pair of pair of <classname,  signature>
    // that builds the reverse class list for reverseClassesMemberFunctions()
    // (for the combos of the ToolWindow)
    class ReverseClassesMemberIterator {
    public:
        ReverseClassesMemberIterator(qdesigner_internal::ClassesMemberFunctions *result);

        ReverseClassesMemberIterator &operator*()     { return *this; }
        ReverseClassesMemberIterator &operator++(int) { return *this; }
        ReverseClassesMemberIterator &operator++()    { return *this; }
        void operator=(const ClassNameSignaturePair &classNameSignature);

    private:
        qdesigner_internal::ClassesMemberFunctions *m_result;
        QString m_lastClassName;
        QStringList *m_memberList;
    };

    ReverseClassesMemberIterator::ReverseClassesMemberIterator(qdesigner_internal::ClassesMemberFunctions *result) :
       m_result(result),
       m_memberList(0)
    {
    }

    void ReverseClassesMemberIterator::operator=(const ClassNameSignaturePair &classNameSignature)
    {
        // prepend a new entry if class changes
        if (!m_memberList || classNameSignature.first != m_lastClassName) {
            m_lastClassName = classNameSignature.first;
            m_result->push_front(qdesigner_internal::ClassMemberFunctions(m_lastClassName));
            m_memberList = &(m_result->front().m_memberList);
        }
        m_memberList->push_back(classNameSignature.second);
    }

    // Output iterator for a pair of pair of <classname,  signature>
    // that adds the signatures to a string list
    class SignatureIterator {
    public:
        SignatureIterator(QMap<QString, QString> *result) : m_result(result) {}

        SignatureIterator &operator*()     { return *this; }
        SignatureIterator &operator++(int) { return *this; }
        SignatureIterator &operator++()    { return *this; }
        void operator=(const ClassNameSignaturePair &classNameSignature) {
            m_result->insert(classNameSignature.second, classNameSignature.first);
        }

    private:
        QMap<QString, QString> *m_result;
    };
}

static inline bool truePredicate(const QString &) { return true; }

namespace qdesigner_internal {

    ClassMemberFunctions::ClassMemberFunctions(const QString &class_name) :
        m_className(class_name)
    {
    }

    bool signalMatchesSlot(QDesignerFormEditorInterface *core, const QString &signal, const QString &slot)
    {
        const SignalMatchesSlotPredicate predicate(core, signal, qdesigner_internal::SlotMember);
        return predicate(slot);
    }

    // return classes and members in reverse class order to
    // populate of the combo of the ToolWindow
    ClassesMemberFunctions reverseClassesMemberFunctions(const QString &obj_name, MemberType member_type,
                                                         const QString &peer, QDesignerFormWindowInterface *form)
    {
        QObject *object = 0;
        if (obj_name == form->mainContainer()->objectName()) {
            object = form->mainContainer();
        } else {
            object = form->mainContainer()->findChild<QObject*>(obj_name);
        }
        if (!object)
            return ClassesMemberFunctions();
        QDesignerFormEditorInterface *core = form->core();

        ClassesMemberFunctions rc;
        memberList(form->core(), object, member_type, true, SignalMatchesSlotPredicate(core, peer,  member_type),
                   ReverseClassesMemberIterator(&rc));
        return rc;
    }

    QMap<QString, QString> getSignals(QDesignerFormEditorInterface *core, QObject *object, bool showAll)
    {
        QMap<QString, QString> rc;
        memberList(core, object, SignalMember, showAll, truePredicate, SignatureIterator(&rc));
        return rc;
    }

    bool isQt3Signal(QDesignerFormEditorInterface *core,
                     QObject *object, const QString &signalSignature)
    {
        if (const QDesignerMemberSheetExtension *members
                = qt_extension<QDesignerMemberSheetExtension*>(core->extensionManager(), object)) {
            const int count = members->count();
            for (int i = 0; i < count; ++i)
                if (members->isSignal(i) && members->signature(i) == signalSignature) {
                    const QDesignerMemberSheet *memberSheet
                            = qobject_cast<QDesignerMemberSheet*>(core->extensionManager()->extension(object,
                                                                                                      Q_TYPEID(QDesignerMemberSheetExtension)));
                    return (memberSheet && memberSheet->isQt3Signal(i));
                }
        }

        return false;
    }

    bool isQt3Slot(QDesignerFormEditorInterface *core,
                   QObject *object, const QString &slotSignature)
    {
        if (const QDesignerMemberSheetExtension *members
                = qt_extension<QDesignerMemberSheetExtension*>(core->extensionManager(), object)) {
            Q_ASSERT(members != 0);
            const int count = members->count();
            for (int i = 0; i < count; ++i)
                if (members->isSlot(i) && members->signature(i) == slotSignature) {
                    const QDesignerMemberSheet *memberSheet
                            = qobject_cast<QDesignerMemberSheet*>(core->extensionManager()->extension(object,
                                                                                                      Q_TYPEID(QDesignerMemberSheetExtension)));
                    return (memberSheet && memberSheet->isQt3Slot(i));
                }
        }
        return false;
    }

    QMap<QString, QString> getMatchingSlots(QDesignerFormEditorInterface *core, QObject *object, const QString &signalSignature, bool showAll)
    {
        QMap<QString, QString> rc;
        memberList(core, object, SlotMember, showAll, SignalMatchesSlotPredicate(core, signalSignature,  qdesigner_internal::SlotMember), SignatureIterator(&rc));
        return rc;
    }

    bool memberFunctionListContains(QDesignerFormEditorInterface *core, QObject *object, MemberType type, const QString &signature)
    {
        QMap<QString, QString> rc;
        memberList(core, object, type, true, EqualsPredicate(signature), SignatureIterator(&rc));
        return !rc.empty();
    }

    // ### deprecated
    QString realObjectName(QDesignerFormEditorInterface *core, QObject *object)
    {
        if (!object)
        return QString();

        const QDesignerMetaDataBaseInterface *mdb = core->metaDataBase();
        if (const QDesignerMetaDataBaseItemInterface *item = mdb->item(object))
            return item->name();

        return object->objectName();
    }
} // namespace qdesigner_internal

QT_END_NAMESPACE
