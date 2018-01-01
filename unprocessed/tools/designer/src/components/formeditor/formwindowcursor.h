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

#ifndef FORMWINDOWCURSOR_H
#define FORMWINDOWCURSOR_H

#include "formeditor_global.h"
#include "formwindow.h"
#include <QtDesigner/QDesignerFormWindowCursorInterface>

#include <QtCore/QObject>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

class QT_FORMEDITOR_EXPORT FormWindowCursor: public QObject, public QDesignerFormWindowCursorInterface
{
    Q_OBJECT
public:
    explicit FormWindowCursor(FormWindow *fw, QObject *parent = 0);
    virtual ~FormWindowCursor();

    virtual QDesignerFormWindowInterface *formWindow() const;

    virtual bool movePosition(MoveOperation op, MoveMode mode);

    virtual int position() const;
    virtual void setPosition(int pos, MoveMode mode);

    virtual QWidget *current() const;

    virtual int widgetCount() const;
    virtual QWidget *widget(int index) const;

    virtual bool hasSelection() const;
    virtual int selectedWidgetCount() const;
    virtual QWidget *selectedWidget(int index) const;

    virtual void setProperty(const QString &name, const QVariant &value);
    virtual void setWidgetProperty(QWidget *widget, const QString &name, const QVariant &value);
    virtual void resetWidgetProperty(QWidget *widget, const QString &name);

public slots:
    void update();

private:
    FormWindow *m_formWindow;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // FORMWINDOWCURSOR_H
