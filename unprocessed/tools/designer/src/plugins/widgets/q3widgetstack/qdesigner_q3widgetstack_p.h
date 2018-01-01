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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef QDESIGNER_Q3WIDGETSTACK_P_H
#define QDESIGNER_Q3WIDGETSTACK_P_H

#include <Qt3Support/Q3WidgetStack>

QT_BEGIN_NAMESPACE

class QDesignerFormWindowInterface;
class QDesignerContainerExtension;
class QToolButton;
class QChildEvent;
class QResizeEvent;
class QShowEvent;
class QEvent;

class QDesignerQ3WidgetStack : public Q3WidgetStack
{
    Q_OBJECT
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex STORED false DESIGNABLE true)
    Q_PROPERTY(QString currentPageName READ currentPageName WRITE setCurrentPageName STORED false DESIGNABLE true)
public:
    QDesignerQ3WidgetStack(QWidget *parent = 0);
    int currentIndex();
    QString currentPageName();

public slots:
    void updateButtons();
    void setCurrentIndex(int index);
    void setCurrentPageName(const QString &pageName);

private slots:
    void prevPage();
    void nextPage();
    void slotCurrentChanged(int index);

signals:
    void currentChanged(int index);

protected:
    virtual void childEvent(QChildEvent *e);
    virtual void resizeEvent(QResizeEvent *e);
    virtual void showEvent(QShowEvent *e);
    virtual bool event(QEvent *e);

private:
    void gotoPage(int page);
    QDesignerFormWindowInterface *formWindow();
    QDesignerContainerExtension *container();
    int count();
    QWidget *widget(int index);
    QToolButton *m_prev, *m_next;
};

QT_END_NAMESPACE

#endif // !QDESIGNER_Q3WIDGETSTACK_P_H
