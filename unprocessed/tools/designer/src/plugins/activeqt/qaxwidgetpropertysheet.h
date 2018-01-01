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

#ifndef QACTIVEXPROPERTYSHEET_H
#define QACTIVEXPROPERTYSHEET_H

#include <QtDesigner/private/qdesigner_propertysheet_p.h>

QT_BEGIN_NAMESPACE

class QDesignerAxWidget;
class QDesignerFormWindowInterface;

/* The propertysheet has a method to delete itself and repopulate
 * if the "control" property changes. Pre 4.5, the control property
 * might not be the first one, so, the properties are stored and
 * re-applied. If the "control" is the first one, it should be
 * sufficient to reapply the changed flags, however, care must be taken when
 * resetting the control.
 * Resetting a control: The current behaviour is that the modified Active X properties are added again
 * as Fake-Properties, which is a nice side-effect as not cause a loss. */

class QAxWidgetPropertySheet: public QDesignerPropertySheet
{
    Q_OBJECT
    Q_INTERFACES(QDesignerPropertySheetExtension)
public:
    explicit QAxWidgetPropertySheet(QDesignerAxWidget *object, QObject *parent = 0);

    virtual bool isEnabled(int index) const;
    virtual QVariant property(int index) const;
    virtual void setProperty(int index, const QVariant &value);
    virtual bool reset(int index);
    int indexOf(const QString &name) const;
    bool dynamicPropertiesAllowed() const;

    static const char *controlPropertyName;

public slots:
    void updatePropertySheet();

private:
    QDesignerAxWidget *axWidget() const;

    const QString m_controlProperty;
    const QString m_propertyGroup;
    int m_controlIndex;
    struct SavedProperties {
        typedef QMap<QString, QVariant> NamePropertyMap;
        NamePropertyMap changedProperties;
        QWidget *widget;
        QString clsid;
    } m_currentProperties;

    static void reloadPropertySheet(const struct SavedProperties &properties, QDesignerFormWindowInterface *formWin);
};

typedef QDesignerPropertySheetFactory<QDesignerAxWidget, QAxWidgetPropertySheet> ActiveXPropertySheetFactory;

QT_END_NAMESPACE

#endif
