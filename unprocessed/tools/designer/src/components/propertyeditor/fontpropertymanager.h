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

#ifndef FONTPROPERTYMANAGER_H
#define FONTPROPERTYMANAGER_H

#include <QtCore/QMap>
#include <QtCore/QStringList>
#include <QtGui/QFont>

QT_BEGIN_NAMESPACE

class QtProperty;
class QtVariantPropertyManager;

class QString;
class QVariant;

namespace qdesigner_internal {

/* FontPropertyManager: A mixin for DesignerPropertyManager that manages font
 * properties. Adds an antialiasing subproperty and reset flags/mask handling
 * for the other subproperties. It also modifies the font family
 * enumeration names, which it reads from an XML mapping file that
 * contains annotations indicating the platform the font is available on. */

class FontPropertyManager {
    Q_DISABLE_COPY(FontPropertyManager)

public:
    FontPropertyManager();

    typedef QMap<QtProperty *, bool> ResetMap;
    typedef QMap<QString, QString> NameMap;

    // Call before QtVariantPropertyManager::initializeProperty.
    void preInitializeProperty(QtProperty *property, int type, ResetMap &resetMap);
    // Call after QtVariantPropertyManager::initializeProperty. This will trigger
    // a recursion for the sub properties
    void postInitializeProperty(QtVariantPropertyManager *vm, QtProperty *property, int type, int enumTypeId);

    bool uninitializeProperty(QtProperty *property);

    // Call from  QtPropertyManager's propertyDestroyed signal
    void slotPropertyDestroyed(QtProperty *property);

    bool resetFontSubProperty(QtVariantPropertyManager *vm, QtProperty *subProperty);

    // Call from slotValueChanged().
    enum ValueChangedResult { NoMatch, Unchanged, Changed };
    ValueChangedResult valueChanged(QtVariantPropertyManager *vm, QtProperty *property, const QVariant &value);

    // Call from setValue() before calling setValue() on  QtVariantPropertyManager.
    void setValue(QtVariantPropertyManager *vm, QtProperty *property, const QVariant &value);

    static bool readFamilyMapping(NameMap *rc, QString *errorMessage);

private:
    typedef QMap<QtProperty *, QtProperty *> PropertyToPropertyMap;
    typedef QList<QtProperty *> PropertyList;
    typedef QMap<QtProperty *, PropertyList>  PropertyToSubPropertiesMap;

    void removeAntialiasingProperty(QtProperty *);
    void updateModifiedState(QtProperty *property, const QVariant &value);
    static int antialiasingToIndex(QFont::StyleStrategy antialias);
    static QFont::StyleStrategy indexToAntialiasing(int idx);
    static unsigned fontFlag(int idx);

    PropertyToPropertyMap m_propertyToAntialiasing;
    PropertyToPropertyMap m_antialiasingToProperty;

    PropertyToSubPropertiesMap m_propertyToFontSubProperties;
    QMap<QtProperty *, int> m_fontSubPropertyToFlag;
    PropertyToPropertyMap m_fontSubPropertyToProperty;
    QtProperty *m_createdFontProperty;
    QStringList m_aliasingEnumNames;
    // Font families with Designer annotations
    QStringList m_designerFamilyNames;
    NameMap m_familyMappings;
};

}

QT_END_NAMESPACE

#endif // FONTPROPERTYMANAGER_H
