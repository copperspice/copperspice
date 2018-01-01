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

#ifndef FINDWIDGET_H
#define FINDWIDGET_H

#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class QCheckBox;
class QLabel;
class QLineEdit;
class QToolButton;

class FindWidget : public QWidget
{
    Q_OBJECT
public:
    FindWidget(QWidget *parent = 0);
    ~FindWidget();

    void show();
    void showAndClear();

    QString text() const;
    bool caseSensitive() const;

    void setPalette(bool found);
    void setTextWrappedVisible(bool visible);

signals:
    void findNext();
    void findPrevious();
    void escapePressed();
    void find(const QString &text, bool forward, bool incremental);

protected:
    void hideEvent(QHideEvent* event);
    void showEvent(QShowEvent * event);

private slots:
    void updateButtons();
    void textChanged(const QString &text);

private:
    bool eventFilter(QObject *object, QEvent *e);
    QToolButton* setupToolButton(const QString &text, const QString &icon);

private:
    QPalette appPalette;

    QLineEdit *editFind;
    QCheckBox *checkCase;
    QLabel *labelWrapped;
    QToolButton *toolNext;
    QToolButton *toolClose;
    QToolButton *toolPrevious;
};

QT_END_NAMESPACE

#endif  // FINDWIDGET_H
