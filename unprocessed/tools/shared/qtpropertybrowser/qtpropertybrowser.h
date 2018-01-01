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

#ifndef QTPROPERTYBROWSER_H
#define QTPROPERTYBROWSER_H

#include <QtGui/QWidget>
#include <QtCore/QSet>

QT_BEGIN_NAMESPACE

class QtAbstractPropertyManager;
class QtPropertyPrivate;

class QtProperty
{
public:
    virtual ~QtProperty();

    QList<QtProperty *> subProperties() const;

    QtAbstractPropertyManager *propertyManager() const;

    QString toolTip() const;
    QString statusTip() const;
    QString whatsThis() const;
    QString propertyName() const;
    bool isEnabled() const;
    bool isModified() const;

    bool hasValue() const;
    QIcon valueIcon() const;
    QString valueText() const;

    void setToolTip(const QString &text);
    void setStatusTip(const QString &text);
    void setWhatsThis(const QString &text);
    void setPropertyName(const QString &text);
    void setEnabled(bool enable);
    void setModified(bool modified);

    void addSubProperty(QtProperty *property);
    void insertSubProperty(QtProperty *property, QtProperty *afterProperty);
    void removeSubProperty(QtProperty *property);
protected:
    explicit QtProperty(QtAbstractPropertyManager *manager);
    void propertyChanged();
private:
    friend class QtAbstractPropertyManager;
    QScopedPointer<QtPropertyPrivate> d_ptr;
};

class QtAbstractPropertyManagerPrivate;

class QtAbstractPropertyManager : public QObject
{
    Q_OBJECT
public:

    explicit QtAbstractPropertyManager(QObject *parent = 0);
    ~QtAbstractPropertyManager();

    QSet<QtProperty *> properties() const;
    void clear() const;

    QtProperty *addProperty(const QString &name = QString());
Q_SIGNALS:

    void propertyInserted(QtProperty *property,
                QtProperty *parent, QtProperty *after);
    void propertyChanged(QtProperty *property);
    void propertyRemoved(QtProperty *property, QtProperty *parent);
    void propertyDestroyed(QtProperty *property);
protected:
    virtual bool hasValue(const QtProperty *property) const;
    virtual QIcon valueIcon(const QtProperty *property) const;
    virtual QString valueText(const QtProperty *property) const;
    virtual void initializeProperty(QtProperty *property) = 0;
    virtual void uninitializeProperty(QtProperty *property);
    virtual QtProperty *createProperty();
private:
    friend class QtProperty;
    QScopedPointer<QtAbstractPropertyManagerPrivate> d_ptr;
    Q_DECLARE_PRIVATE(QtAbstractPropertyManager)
    Q_DISABLE_COPY(QtAbstractPropertyManager)
};

class QtAbstractEditorFactoryBase : public QObject
{
    Q_OBJECT
public:
    virtual QWidget *createEditor(QtProperty *property, QWidget *parent) = 0;
protected:
    explicit QtAbstractEditorFactoryBase(QObject *parent = 0)
        : QObject(parent) {}

    virtual void breakConnection(QtAbstractPropertyManager *manager) = 0;
protected Q_SLOTS:
    virtual void managerDestroyed(QObject *manager) = 0;

    friend class QtAbstractPropertyBrowser;
};

template <class PropertyManager>
class QtAbstractEditorFactory : public QtAbstractEditorFactoryBase
{
public:
    explicit QtAbstractEditorFactory(QObject *parent) : QtAbstractEditorFactoryBase(parent) {}
    QWidget *createEditor(QtProperty *property, QWidget *parent)
    {
        QSetIterator<PropertyManager *> it(m_managers);
        while (it.hasNext()) {
            PropertyManager *manager = it.next();
            if (manager == property->propertyManager()) {
                return createEditor(manager, property, parent);
            }
        }
        return 0;
    }
    void addPropertyManager(PropertyManager *manager)
    {
        if (m_managers.contains(manager))
            return;
        m_managers.insert(manager);
        connectPropertyManager(manager);
        connect(manager, SIGNAL(destroyed(QObject *)),
                    this, SLOT(managerDestroyed(QObject *)));
    }
    void removePropertyManager(PropertyManager *manager)
    {
        if (!m_managers.contains(manager))
            return;
        disconnect(manager, SIGNAL(destroyed(QObject *)),
                    this, SLOT(managerDestroyed(QObject *)));
        disconnectPropertyManager(manager);
        m_managers.remove(manager);
    }
    QSet<PropertyManager *> propertyManagers() const
    {
        return m_managers;
    }
    PropertyManager *propertyManager(QtProperty *property) const
    {
        QtAbstractPropertyManager *manager = property->propertyManager();
        QSetIterator<PropertyManager *> itManager(m_managers);
        while (itManager.hasNext()) {
            PropertyManager *m = itManager.next();
            if (m == manager) {
                return m;
            }
        }
        return 0;
    }
protected:
    virtual void connectPropertyManager(PropertyManager *manager) = 0;
    virtual QWidget *createEditor(PropertyManager *manager, QtProperty *property,
                QWidget *parent) = 0;
    virtual void disconnectPropertyManager(PropertyManager *manager) = 0;
    void managerDestroyed(QObject *manager)
    {
        QSetIterator<PropertyManager *> it(m_managers);
        while (it.hasNext()) {
            PropertyManager *m = it.next();
            if (m == manager) {
                m_managers.remove(m);
                return;
            }
        }
    }
private:
    void breakConnection(QtAbstractPropertyManager *manager)
    {
        QSetIterator<PropertyManager *> it(m_managers);
        while (it.hasNext()) {
            PropertyManager *m = it.next();
            if (m == manager) {
                removePropertyManager(m);
                return;
            }
        }
    }
private:
    QSet<PropertyManager *> m_managers;
    friend class QtAbstractPropertyEditor;
};

class QtAbstractPropertyBrowser;
class QtBrowserItemPrivate;

class QtBrowserItem
{
public:
    QtProperty *property() const;
    QtBrowserItem *parent() const;
    QList<QtBrowserItem *> children() const;
    QtAbstractPropertyBrowser *browser() const;
private:
    explicit QtBrowserItem(QtAbstractPropertyBrowser *browser, QtProperty *property, QtBrowserItem *parent);
    ~QtBrowserItem();
    QScopedPointer<QtBrowserItemPrivate> d_ptr;
    friend class QtAbstractPropertyBrowserPrivate;
};

class QtAbstractPropertyBrowserPrivate;

class QtAbstractPropertyBrowser : public QWidget
{
    Q_OBJECT
public:

    explicit QtAbstractPropertyBrowser(QWidget *parent = 0);
    ~QtAbstractPropertyBrowser();

    QList<QtProperty *> properties() const;
    QList<QtBrowserItem *> items(QtProperty *property) const;
    QtBrowserItem *topLevelItem(QtProperty *property) const;
    QList<QtBrowserItem *> topLevelItems() const;
    void clear();

    template <class PropertyManager>
    void setFactoryForManager(PropertyManager *manager,
                    QtAbstractEditorFactory<PropertyManager> *factory) {
        QtAbstractPropertyManager *abstractManager = manager;
        QtAbstractEditorFactoryBase *abstractFactory = factory;

        if (addFactory(abstractManager, abstractFactory))
            factory->addPropertyManager(manager);
    }

    void unsetFactoryForManager(QtAbstractPropertyManager *manager);

    QtBrowserItem *currentItem() const;
    void setCurrentItem(QtBrowserItem *);

Q_SIGNALS:
    void currentItemChanged(QtBrowserItem *);

public Q_SLOTS:

    QtBrowserItem *addProperty(QtProperty *property);
    QtBrowserItem *insertProperty(QtProperty *property, QtProperty *afterProperty);
    void removeProperty(QtProperty *property);

protected:

    virtual void itemInserted(QtBrowserItem *item, QtBrowserItem *afterItem) = 0;
    virtual void itemRemoved(QtBrowserItem *item) = 0;
    // can be tooltip, statustip, whatsthis, name, icon, text.
    virtual void itemChanged(QtBrowserItem *item) = 0;

    virtual QWidget *createEditor(QtProperty *property, QWidget *parent);
private:

    bool addFactory(QtAbstractPropertyManager *abstractManager,
                QtAbstractEditorFactoryBase *abstractFactory);

    QScopedPointer<QtAbstractPropertyBrowserPrivate> d_ptr;
    Q_DECLARE_PRIVATE(QtAbstractPropertyBrowser)
    Q_DISABLE_COPY(QtAbstractPropertyBrowser)
    Q_PRIVATE_SLOT(d_func(), void slotPropertyInserted(QtProperty *,
                            QtProperty *, QtProperty *))
    Q_PRIVATE_SLOT(d_func(), void slotPropertyRemoved(QtProperty *,
                            QtProperty *))
    Q_PRIVATE_SLOT(d_func(), void slotPropertyDestroyed(QtProperty *))
    Q_PRIVATE_SLOT(d_func(), void slotPropertyDataChanged(QtProperty *))

};

QT_END_NAMESPACE

#endif // QTPROPERTYBROWSER_H
