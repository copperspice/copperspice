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

#ifndef QDESIGNER_TEMPLATEOPTIONS_H
#define QDESIGNER_TEMPLATEOPTIONS_H

#include <QtDesigner/private/abstractoptionspage_p.h>

#include <QtCore/QPointer>
#include <QtCore/QStringList>

#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class QDesignerFormEditorInterface;

namespace qdesigner_internal {

namespace Ui {
    class TemplateOptionsWidget;
}

/* Present the user with a list of form template paths to save
 * form templates. */
class TemplateOptionsWidget : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(TemplateOptionsWidget)
public:
    explicit TemplateOptionsWidget(QDesignerFormEditorInterface *core,
                                              QWidget *parent = 0);
    ~TemplateOptionsWidget();


    QStringList templatePaths() const;
    void setTemplatePaths(const QStringList &l);

    static QString chooseTemplatePath(QDesignerFormEditorInterface *core, QWidget *parent);

private slots:
    void addTemplatePath();
    void removeTemplatePath();
    void templatePathSelectionChanged();

private:
    QDesignerFormEditorInterface *m_core;
    Ui::TemplateOptionsWidget *m_ui;
};

class TemplateOptionsPage : public QDesignerOptionsPageInterface
{
     Q_DISABLE_COPY(TemplateOptionsPage)
public:
    explicit TemplateOptionsPage(QDesignerFormEditorInterface *core);

    virtual QString name() const;
    virtual QWidget *createPage(QWidget *parent);
    virtual void apply();
    virtual void finish();

private:
    QDesignerFormEditorInterface *m_core;
    QStringList m_initialTemplatePaths;
    QPointer<TemplateOptionsWidget> m_widget;
};

}

QT_END_NAMESPACE

#endif // QDESIGNER_TEMPLATEOPTIONS_H
