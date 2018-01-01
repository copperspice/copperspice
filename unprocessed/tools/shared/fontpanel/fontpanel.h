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
// of the Qt tools.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef FONTPANEL_H
#define FONTPANEL_H

#include <QtGui/QGroupBox>
#include <QtGui/QFont>
#include <QtGui/QFontDatabase>

QT_BEGIN_NAMESPACE

class QComboBox;
class QFontComboBox;
class QTimer;
class QLineEdit;

class FontPanel: public QGroupBox
{
    Q_OBJECT
public:
    FontPanel(QWidget *parentWidget = 0);

    QFont selectedFont() const;
    void setSelectedFont(const QFont &);

    QFontDatabase::WritingSystem writingSystem() const;
    void setWritingSystem(QFontDatabase::WritingSystem ws);

private slots:
    void slotWritingSystemChanged(int);
    void slotFamilyChanged(const QFont &);
    void slotStyleChanged(int);
    void slotPointSizeChanged(int);
    void slotUpdatePreviewFont();

private:
    QString family() const;
    QString styleString() const;
    int pointSize() const;
    int closestPointSizeIndex(int ps) const;

    void updateWritingSystem(QFontDatabase::WritingSystem ws);
    void updateFamily(const QString &family);
    void updatePointSizes(const QString &family, const QString &style);
    void delayedPreviewFontUpdate();

    QFontDatabase m_fontDatabase;
    QLineEdit *m_previewLineEdit;
    QComboBox *m_writingSystemComboBox;
    QFontComboBox* m_familyComboBox;
    QComboBox *m_styleComboBox;
    QComboBox *m_pointSizeComboBox;
    QTimer *m_previewFontUpdateTimer;
};

QT_END_NAMESPACE

#endif // FONTPANEL_H
