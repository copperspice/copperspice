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

#include <QtGui/qplaintextedit.h>
#include <QtGui/qtextedit.h>
#include <QtGui/qabstractkineticscroller.h>
#include <QtGui/qscrollarea.h>
#include <QtDebug>

#ifndef TEXTEDITAUTORESIZER_H
#define TEXTEDITAUTORESIZER_H

class TextEditAutoResizer : public QObject
{
    Q_OBJECT
public:
    TextEditAutoResizer(QWidget *parent)
        : QObject(parent), plainTextEdit(qobject_cast<QPlainTextEdit *>(parent)),
          textEdit(qobject_cast<QTextEdit *>(parent)), edit(qobject_cast<QFrame *>(parent))
    {
        // parent must either inherit QPlainTextEdit or  QTextEdit!
        Q_ASSERT(plainTextEdit || textEdit);

        connect(parent, SIGNAL(textChanged()), this, SLOT(textEditChanged()));
        connect(parent, SIGNAL(cursorPositionChanged()), this, SLOT(textEditChanged()));

        textEditChanged();
    }

private Q_SLOTS:
    inline void textEditChanged();

private:
    QPlainTextEdit *plainTextEdit;
    QTextEdit *textEdit;
    QFrame *edit;
};

void TextEditAutoResizer::textEditChanged()
{
    QTextDocument *doc = textEdit ? textEdit->document() : plainTextEdit->document();
    QRect cursor = textEdit ? textEdit->cursorRect() : plainTextEdit->cursorRect();

    QSize s = doc->size().toSize();
    if (plainTextEdit)
        s.setHeight((s.height() + 2) * edit->fontMetrics().lineSpacing());

    const QRect fr = edit->frameRect();
    const QRect cr = edit->contentsRect();

    edit->setMinimumHeight(qMax(70, s.height() + (fr.height() - cr.height() - 1)));

    // make sure the cursor is visible in case we have a QAbstractScrollArea parent
    QPoint pos = edit->pos();
    QWidget *pw = edit->parentWidget();
    while (pw) {
        if (qobject_cast<QScrollArea *>(pw))
            break;
        pw = pw->parentWidget();
    }

    if (pw) {
        QScrollArea *area = static_cast<QScrollArea *>(pw);
        QPoint scrollto = area->widget()->mapFrom(edit, cursor.center());
        QPoint margin(10 + cursor.width(), 2 * cursor.height());

        if (QAbstractKineticScroller *scroller = area->property("kineticScroller").value<QAbstractKineticScroller *>()) {
            scroller->ensureVisible(scrollto, margin.x(), margin.y());
        } else {
            area->ensureVisible(scrollto.x(), scrollto.y(), margin.x(), margin.y());
        }
    }
}

#endif
