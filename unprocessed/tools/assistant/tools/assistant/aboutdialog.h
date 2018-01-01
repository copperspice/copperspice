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

#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QtGui/QTextBrowser>
#include <QtGui/QDialog>

QT_BEGIN_NAMESPACE

class QLabel;
class QPushButton;
class QGridLayout;

class AboutLabel : public QTextBrowser
{
    Q_OBJECT

public:
    AboutLabel(QWidget *parent = 0);
    void setText(const QString &text, const QByteArray &resources);
    QSize minimumSizeHint() const;
    
private:
    QVariant loadResource(int type, const QUrl &name);
    void setSource(const QUrl &url);

    QMap<QString, QByteArray> m_resourceMap;
};

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    AboutDialog(QWidget *parent = 0);
    void setText(const QString &text, const QByteArray &resources);
    void setPixmap(const QPixmap &pixmap);
    QString documentTitle() const;

private:
    void updateSize();

    QLabel *m_pixmapLabel;
    AboutLabel *m_aboutLabel;
    QPushButton *m_closeButton;
    QGridLayout *m_layout;
};

QT_END_NAMESPACE

#endif
