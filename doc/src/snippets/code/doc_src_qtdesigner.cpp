/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the documentation of the Qt Toolkit.
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

//! [0]
#include <QtDesigner>
//! [0]


//! [2]
QDesignerMemberSheetExtension *memberSheet  = 0;
QExtensionManager manager = formEditor->extensionManager();

memberSheet = qt_extension<QDesignerMemberSheetExtension*>(manager, widget);
int index = memberSheet->indexOf(setEchoMode);
memberSheet->setVisible(index, false);

delete memberSheet;
//! [2]


//! [3]
class MyMemberSheetExtension : public QObject,
        public QDesignerMemberSheetExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerMemberSheetExtension)

public:
    ...
}
//! [3]


//! [4]
QObject *ANewExtensionFactory::createExtension(QObject *object,
        const QString &iid, QObject *parent) const
{
    if (iid != Q_TYPEID(QDesignerMemberSheetExtension))
        return 0;

    if (MyCustomWidget *widget = qobject_cast<MyCustomWidget*>
           (object))
        return new MyMemberSheetExtension(widget, parent);

    return 0;
}
//! [4]


//! [5]
QObject *AGeneralExtensionFactory::createExtension(QObject *object,
        const QString &iid, QObject *parent) const
{
    MyCustomWidget *widget = qobject_cast<MyCustomWidget*>(object);

    if (widget && (iid == Q_TYPEID(QDesignerTaskMenuExtension))) {
        return new MyTaskMenuExtension(widget, parent);

    } else if (widget && (iid == Q_TYPEID(QDesignerMemberSheetExtension))) {
        return new MyMemberSheetExtension(widget, parent);

    } else {
        return 0;
    }
}
//! [5]


//! [6]
class MyContainerExtension : public QObject,
       public QDesignerContainerExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerContainerExtension)

public:
    MyContainerExtension(MyCustomWidget *widget,
                         QObject *parent = 0);
    int count() const;
    QWidget *widget(int index) const;
    int currentIndex() const;
    void setCurrentIndex(int index);
    void addWidget(QWidget *widget);
    void insertWidget(int index, QWidget *widget);
    void remove(int index);

private:
    MyCustomWidget *myWidget;
};
//! [6]


//! [7]
QObject *ANewExtensionFactory::createExtension(QObject *object,
        const QString &iid, QObject *parent) const
{
    if (iid != Q_TYPEID(QDesignerContainerExtension))
        return 0;

    if (MyCustomWidget *widget = qobject_cast<MyCustomWidget*>
           (object))
        return new MyContainerExtension(widget, parent);

    return 0;
}
//! [7]


//! [8]
QObject *AGeneralExtensionFactory::createExtension(QObject *object,
        const QString &iid, QObject *parent) const
{
    MyCustomWidget *widget = qobject_cast<MyCustomWidget*>(object);

    if (widget && (iid == Q_TYPEID(QDesignerTaskMenuExtension))) {
        return new MyTaskMenuExtension(widget, parent);

    } else if (widget && (iid == Q_TYPEID(QDesignerContainerExtension))) {
        return new MyContainerExtension(widget, parent);

    } else {
        return 0;
    }
}
//! [8]


//! [9]
class MyTaskMenuExtension : public QObject,
        public QDesignerTaskMenuExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerTaskMenuExtension)

public:
    MyTaskMenuExtension(MyCustomWidget *widget, QObject *parent);

    QAction *preferredEditAction() const;
    QList<QAction *> taskActions() const;

private slots:
    void mySlot();

private:
    MyCustomWidget *widget;
    QAction *myAction;
};
//! [9]


//! [10]
QObject *ANewExtensionFactory::createExtension(QObject *object,
        const QString &iid, QObject *parent) const
{
    if (iid != Q_TYPEID(QDesignerTaskMenuExtension))
        return 0;

    if (MyCustomWidget *widget = qobject_cast<MyCustomWidget*>(object))
        return new MyTaskMenuExtension(widget, parent);

    return 0;
}
//! [10]


//! [11]
QObject *AGeneralExtensionFactory::createExtension(QObject *object,
        const QString &iid, QObject *parent) const
{
    MyCustomWidget *widget = qobject_cast<MyCustomWidget*>(object);

    if (widget && (iid == Q_TYPEID(QDesignerContainerExtension))) {
        return new MyContainerExtension(widget, parent);

    } else if (widget && (iid == Q_TYPEID(QDesignerTaskMenuExtension))) {
        return new MyTaskMenuExtension(widget, parent);

    } else {
        return 0;
    }
}
//! [11]


//! [12]
#include customwidgetoneinterface.h
#include customwidgettwointerface.h
#include customwidgetthreeinterface.h

#include <QtDesigner/QtDesigner>
#include <QtCore/qplugin.h>

class MyCustomWidgets: public QObject, public QDesignerCustomWidgetCollectionInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetCollectionInterface)

public:
    MyCustomWidgets(QObject *parent = 0);

    virtual QList<QDesignerCustomWidgetInterface*> customWidgets() const;

private:
    QList<QDesignerCustomWidgetInterface*> widgets;
};
//! [12]


//! [13]
MyCustomWidgets::MyCustomWidgets(QObject *parent)
        : QObject(parent)
{
    widgets.append(new CustomWidgetOneInterface(this));
    widgets.append(new CustomWidgetTwoInterface(this));
    widgets.append(new CustomWidgetThreeInterface(this));
}

QList<QDesignerCustomWidgetInterface*> MyCustomWidgets::customWidgets() const
{
    return widgets;
}

Q_EXPORT_PLUGIN2(customwidgetsplugin, MyCustomWidgets)
//! [13]


//! [14]
Q_EXPORT_PLUGIN2(customwidgetplugin, MyCustomWidget)
//! [14]


//! [15]
QDesignerPropertySheetExtension *propertySheet  = 0;
QExtensionManager manager = formEditor->extensionManager();

propertySheet = qt_extension<QDesignerPropertySheetExtension*>(manager, widget);
int index = propertySheet->indexOf(QLatin1String("margin"));

propertySheet->setProperty(index, 10);
propertySheet->setChanged(index, true);

delete propertySheet;
//! [15]


//! [16]
class MyPropertySheetExtension : public QObject,
        public QDesignerPropertySheetExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerPropertySheetExtension)

public:
    ...
}
//! [16]


//! [17]
QObject *ANewExtensionFactory::createExtension(QObject *object,
        const QString &iid, QObject *parent) const
{
    if (iid != Q_TYPEID(QDesignerPropertySheetExtension))
        return 0;

    if (MyCustomWidget *widget = qobject_cast<MyCustomWidget*>
           (object))
        return new MyPropertySheetExtension(widget, parent);

    return 0;
}
//! [17]


//! [18]
QObject *AGeneralExtensionFactory::createExtension(QObject *object,
        const QString &iid, QObject *parent) const
{
    MyCustomWidget *widget = qobject_cast<MyCustomWidget*>(object);

    if (widget && (iid == Q_TYPEID(QDesignerTaskMenuExtension))) {
        return new MyTaskMenuExtension(widget, parent);

    } else if (widget && (iid == Q_TYPEID(QDesignerPropertySheetExtension))) {
        return new MyPropertySheetExtension(widget, parent);

    } else {
        return 0;
    }
}
//! [18]
