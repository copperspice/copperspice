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

#ifndef FILTERWIDGET_H
#define FILTERWIDGET_H

#include "shared_global_p.h"

#include <QtGui/QWidget>
#include <QtGui/QLineEdit>
#include <QtGui/QColor>
#include <QtGui/QToolButton>

QT_BEGIN_NAMESPACE

class QToolButton;

namespace qdesigner_internal {

/* This widget should never have initial focus
 * (ie, be the first widget of a dialog, else, the hint cannot be displayed.
 * For situations, where it is the only focusable control (widget box),
 * there is a special "refuseFocus()" mode, in which it clears the focus
 * policy and focusses explicitly on click (note that setting Qt::ClickFocus
 * is not sufficient for that as an ActivationFocus will occur). */

#define ICONBUTTON_SIZE 16

class QDESIGNER_SHARED_EXPORT HintLineEdit : public QLineEdit {
    Q_OBJECT
public:
    explicit HintLineEdit(QWidget *parent = 0);

    bool refuseFocus() const;
    void setRefuseFocus(bool v);

protected:
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void focusInEvent(QFocusEvent *e);

private:
    const Qt::FocusPolicy m_defaultFocusPolicy;
    bool m_refuseFocus;
};

// IconButton: This is a simple helper class that represents clickable icons

class IconButton: public QToolButton
{
    Q_OBJECT
    Q_PROPERTY(float fader READ fader WRITE setFader)
public:
    IconButton(QWidget *parent);
    void paintEvent(QPaintEvent *event);
    float fader() { return m_fader; }
    void setFader(float value) { m_fader = value; update(); }
    void animateShow(bool visible);

private:
    float m_fader;
};

// FilterWidget: For filtering item views, with reset button Uses HintLineEdit.

class  QDESIGNER_SHARED_EXPORT FilterWidget : public QWidget
{
    Q_OBJECT
public:
    enum LayoutMode {
        // For use in toolbars: Expand to the right
        LayoutAlignRight,
        // No special alignment
        LayoutAlignNone
    };

    explicit FilterWidget(QWidget *parent = 0, LayoutMode lm = LayoutAlignRight);

    QString text() const;
    void resizeEvent(QResizeEvent *);
    bool refuseFocus() const; // see HintLineEdit
    void setRefuseFocus(bool v);

signals:
    void filterChanged(const QString &);

public slots:
    void reset();

private slots:
    void checkButton(const QString &text);

private:
    HintLineEdit *m_editor;
    IconButton *m_button;
    int m_buttonwidth;
    QString m_oldText;
};
} // namespace qdesigner_internal

QT_END_NAMESPACE

#endif
