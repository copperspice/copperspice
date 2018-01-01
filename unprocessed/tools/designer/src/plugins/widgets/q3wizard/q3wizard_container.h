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

#ifndef Q3WIZARD_CONTAINER_H
#define Q3WIZARD_CONTAINER_H

#include <QtDesigner/QDesignerContainerExtension>
#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QDesignerExtraInfoExtension>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/private/qdesigner_propertysheet_p.h>

#include <QtCore/QPointer>
#include <Qt3Support/Q3Wizard>

QT_BEGIN_NAMESPACE

class Q3Wizard;

class Q3WizardHelper : public QObject
{
    Q_OBJECT
public:
    explicit Q3WizardHelper(Q3Wizard *wizard);
private slots:
    void slotCurrentChanged();
private:
    Q3Wizard *m_wizard;
};

class Q3WizardExtraInfo: public QObject, public QDesignerExtraInfoExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerExtraInfoExtension)
public:
    explicit Q3WizardExtraInfo(Q3Wizard *wizard, QDesignerFormEditorInterface *core, QObject *parent);

    virtual QWidget *widget() const;
    virtual Q3Wizard *wizard() const;
    virtual QDesignerFormEditorInterface *core() const;

    virtual bool saveUiExtraInfo(DomUI *ui);
    virtual bool loadUiExtraInfo(DomUI *ui);

    virtual bool saveWidgetExtraInfo(DomWidget *ui_widget);
    virtual bool loadWidgetExtraInfo(DomWidget *ui_widget);

private:
    QPointer<Q3Wizard> m_wizard;
    QPointer<QDesignerFormEditorInterface> m_core;
};

class Q3WizardExtraInfoFactory: public QExtensionFactory
{
    Q_OBJECT
public:
    Q3WizardExtraInfoFactory(QDesignerFormEditorInterface *core, QExtensionManager *parent = 0);

protected:
    virtual QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;

private:
    QDesignerFormEditorInterface *m_core;
};

class Q3WizardContainer: public QObject, public QDesignerContainerExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerContainerExtension)
public:
    explicit Q3WizardContainer(Q3Wizard *wizard, QObject *parent = 0);

    virtual int count() const;
    virtual QWidget *widget(int index) const;
    virtual int currentIndex() const;
    virtual void setCurrentIndex(int index);
    virtual void addWidget(QWidget *widget);
    virtual void insertWidget(int index, QWidget *widget);
    virtual void remove(int index);

private:
    Q3Wizard *m_wizard;
};

class Q3WizardContainerFactory: public QExtensionFactory
{
    Q_OBJECT
public:
    explicit Q3WizardContainerFactory(QExtensionManager *parent = 0);

protected:
    virtual QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;
};

class Q3WizardPropertySheet : public QDesignerPropertySheet {
public:
    explicit Q3WizardPropertySheet(Q3Wizard *object, QObject *parent = 0);

    virtual void setProperty(int index, const QVariant &value);
    virtual QVariant property(int index) const;
    virtual bool reset(int index);

private:
    Q3Wizard *m_wizard;
};

typedef QDesignerPropertySheetFactory<Q3Wizard, Q3WizardPropertySheet> Q3WizardPropertySheetFactory;

QT_END_NAMESPACE

#endif // Q3WIZARD_CONTAINER_H
