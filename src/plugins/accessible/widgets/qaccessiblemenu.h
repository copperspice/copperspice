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

#ifndef QACCESSIBLEMENU_H
#define QACCESSIBLEMENU_H

#include <QtGui/qaccessiblewidget.h>
#include <QtGui/qaccessible2.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_ACCESSIBILITY

#ifndef QT_NO_MENU
class QMenu;
class QMenuBar;
class QAction;

class QAccessibleMenu : public QAccessibleWidgetEx
{
public:
    explicit QAccessibleMenu(QWidget *w);

    int childCount() const;
    int childAt(int x, int y) const;

    QRect rect(int child) const;
    QString text(Text t, int child) const;
    Role role(int child) const;
    State state(int child) const;
    int navigate(RelationFlag relation, int entry, QAccessibleInterface **target) const;
    int indexOfChild( const QAccessibleInterface *child ) const;

    QString actionText(int action, QAccessible::Text text, int child) const;
    bool doAction(int action, int child, const QVariantList &params);

protected:
    QMenu *menu() const;
};

#ifndef QT_NO_MENUBAR
class QAccessibleMenuBar : public QAccessibleWidgetEx
{
public:
    explicit QAccessibleMenuBar(QWidget *w);

    int childCount() const;
    int childAt(int x, int y) const;

    QRect rect(int child) const;
    QString text(Text t, int child) const;
    Role role(int child) const;
    State state(int child) const;
    int navigate(RelationFlag relation, int entry, QAccessibleInterface **target) const;
    int indexOfChild( const QAccessibleInterface *child ) const;

    QString actionText(int action, QAccessible::Text text, int child) const;
    bool doAction(int action, int child, const QVariantList &params);

protected:
    QMenuBar *menuBar() const;
};
#endif // QT_NO_MENUBAR



class QAccessibleMenuItem : public QAccessibleActionInterface, public QAccessibleInterfaceEx
{
    Q_ACCESSIBLE_OBJECT
public:
    explicit QAccessibleMenuItem(QWidget *owner, QAction *w);

    virtual ~QAccessibleMenuItem();
    virtual QString actionText ( int action, Text t, int child ) const;
    virtual int childAt ( int x, int y ) const;
    virtual int childCount () const;
    virtual bool doAction ( int action, int child, const QVariantList & params = QVariantList() );
    virtual int indexOfChild ( const QAccessibleInterface * child ) const;
    virtual bool isValid () const;
    virtual int navigate ( RelationFlag relation, int entry, QAccessibleInterface ** target ) const;
    virtual QObject * object () const;
    virtual QRect rect ( int child ) const;
    virtual Relation relationTo ( int child, const QAccessibleInterface * other, int otherChild ) const;
    virtual Role role ( int child ) const;
    virtual void setText ( Text t, int child, const QString & text );
    virtual State state ( int child ) const;
    virtual QString text ( Text t, int child ) const;
    virtual int userActionCount ( int child ) const;

    // action interface
    virtual int actionCount();
    virtual void doAction(int actionIndex);
    virtual QString description(int);
    virtual QString name(int);
    virtual QString localizedName(int);
    virtual QStringList keyBindings(int);

    virtual QVariant invokeMethodEx(Method, int, const QVariantList &);

    QWidget *owner() const;
protected:
    QAction *action() const;
private:
    QAction *m_action;
    QWidget *m_owner; // can hold either QMenu or the QMenuBar that contains the action
};

#endif // QT_NO_MENU

QT_END_NAMESPACE
#endif // QT_NO_ACCESSIBILITY
#endif // QACCESSIBLEMENU_H
