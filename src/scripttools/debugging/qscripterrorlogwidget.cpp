/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
* Copyright (c) 2015 The Qt Company Ltd.
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
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
* https://www.gnu.org/licenses/
*
***********************************************************************/

#include "qscripterrorlogwidget_p.h"
#include "qscripterrorlogwidgetinterface_p_p.h"

#include <QtCore/qdatetime.h>
#include <QtGui/qboxlayout.h>
#include <QtGui/qtextedit.h>
#include <QtGui/qscrollbar.h>
#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

namespace {

class QScriptErrorLogWidgetOutputEdit : public QTextEdit
{
 public:
   QScriptErrorLogWidgetOutputEdit(QWidget *parent = nullptr)
      : QTextEdit(parent) {
      setReadOnly(true);
      //        setFocusPolicy(Qt::NoFocus);
      document()->setMaximumBlockCount(255);
   }

   void scrollToBottom() {
      QScrollBar *bar = verticalScrollBar();
      bar->setValue(bar->maximum());
   }
};

} // namespace

class QScriptErrorLogWidgetPrivate
   : public QScriptErrorLogWidgetInterfacePrivate
{
   Q_DECLARE_PUBLIC(QScriptErrorLogWidget)
 public:
   QScriptErrorLogWidgetPrivate();
   ~QScriptErrorLogWidgetPrivate();

   QScriptErrorLogWidgetOutputEdit *outputEdit;
};

QScriptErrorLogWidgetPrivate::QScriptErrorLogWidgetPrivate()
{
}

QScriptErrorLogWidgetPrivate::~QScriptErrorLogWidgetPrivate()
{
}

QScriptErrorLogWidget::QScriptErrorLogWidget(QWidget *parent)
   : QScriptErrorLogWidgetInterface(*new QScriptErrorLogWidgetPrivate, parent, 0)
{
   Q_D(QScriptErrorLogWidget);
   d->outputEdit = new QScriptErrorLogWidgetOutputEdit();
   QVBoxLayout *vbox = new QVBoxLayout(this);
   vbox->setMargin(0);
   vbox->setSpacing(0);
   vbox->addWidget(d->outputEdit);

   //    QString sheet = QString::fromLatin1("font-size: 14px; font-family: \"Monospace\";");
   //    setStyleSheet(sheet);
}

QScriptErrorLogWidget::~QScriptErrorLogWidget()
{
}

void QScriptErrorLogWidget::message(
   QtMsgType type, const QString &text, const QString &fileName,
   int lineNumber, int columnNumber, const QVariant &/*data*/)
{
   // ### we need the error message rather than Error.toString()
   Q_UNUSED(type);
   Q_UNUSED(fileName);
   Q_UNUSED(lineNumber);
   Q_UNUSED(columnNumber);
   Q_D(QScriptErrorLogWidget);
   QString html;
   html.append(QString::fromLatin1("<b>%0</b> %1<br>")
               .arg(QDateTime::currentDateTime().toString()).arg(Qt::escape(text)));
   d->outputEdit->insertHtml(html);
   d->outputEdit->scrollToBottom();
}

void QScriptErrorLogWidget::clear()
{
   Q_D(QScriptErrorLogWidget);
   d->outputEdit->clear();
}

QT_END_NAMESPACE
