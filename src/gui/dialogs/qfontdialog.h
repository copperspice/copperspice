/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QFONTDIALOG_H
#define QFONTDIALOG_H

#include <QtGui/qwindowdefs.h>
#include <QtGui/qdialog.h>
#include <QtGui/qfont.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_FONTDIALOG

class QFontDialogPrivate;

class Q_GUI_EXPORT QFontDialog : public QDialog
{
    CS_OBJECT(QFontDialog)
    Q_DECLARE_PRIVATE(QFontDialog)

    GUI_CS_ENUM(FontDialogOption)
    GUI_CS_PROPERTY_READ(currentFont, currentFont)
    GUI_CS_PROPERTY_WRITE(currentFont, setCurrentFont)
    GUI_CS_PROPERTY_NOTIFY(currentFont, currentFontChanged)
    GUI_CS_PROPERTY_READ(options, options)
    GUI_CS_PROPERTY_WRITE(options, setOptions)

public:
    enum FontDialogOption {
        NoButtons           = 0x00000001,
        DontUseNativeDialog = 0x00000002
    };

    using FontDialogOptions = QFlags<FontDialogOption>;

    explicit QFontDialog(QWidget *parent = 0);
    explicit QFontDialog(const QFont &initial, QWidget *parent = 0);
    ~QFontDialog();

    void setCurrentFont(const QFont &font);
    QFont currentFont() const;

    QFont selectedFont() const;

    void setOption(FontDialogOption option, bool on = true);
    bool testOption(FontDialogOption option) const;
    void setOptions(FontDialogOptions options);
    FontDialogOptions options() const;

    using QDialog::open;

    void open(QObject *receiver, const char *member);

    void setVisible(bool visible);

    // ### Qt5/merge overloads
    static QFont getFont(bool *ok, const QFont &initial, QWidget *parent, const QString &title,
                         FontDialogOptions options);
    static QFont getFont(bool *ok, const QFont &initial, QWidget *parent, const QString &title);
    static QFont getFont(bool *ok, const QFont &initial, QWidget *parent = 0);
    static QFont getFont(bool *ok, QWidget *parent = 0);

    GUI_CS_SIGNAL_1(Public, void currentFontChanged(const QFont & font))
    GUI_CS_SIGNAL_2(currentFontChanged,font) 
    GUI_CS_SIGNAL_1(Public, void fontSelected(const QFont & font))
    GUI_CS_SIGNAL_2(fontSelected,font) 

protected:
    void changeEvent(QEvent *event);
    void done(int result);

private:
    // ### Qt5/make protected
    bool eventFilter(QObject *object, QEvent *event);

    Q_DISABLE_COPY(QFontDialog)

    GUI_CS_SLOT_1(Private, void _q_sizeChanged(const QString & un_named_arg1))
    GUI_CS_SLOT_2(_q_sizeChanged)

    GUI_CS_SLOT_1(Private, void _q_familyHighlighted(int un_named_arg1))
    GUI_CS_SLOT_2(_q_familyHighlighted)

    GUI_CS_SLOT_1(Private, void _q_writingSystemHighlighted(int un_named_arg1))
    GUI_CS_SLOT_2(_q_writingSystemHighlighted)

    GUI_CS_SLOT_1(Private, void _q_styleHighlighted(int un_named_arg1))
    GUI_CS_SLOT_2(_q_styleHighlighted)

    GUI_CS_SLOT_1(Private, void _q_sizeHighlighted(int un_named_arg1))
    GUI_CS_SLOT_2(_q_sizeHighlighted)

    GUI_CS_SLOT_1(Private, void _q_updateSample())
    GUI_CS_SLOT_2(_q_updateSample)


#if defined(Q_WS_MAC)
    GUI_CS_SLOT_1(Private, void _q_macRunNativeAppModalPanel())
    GUI_CS_SLOT_2(_q_macRunNativeAppModalPanel)
#endif

};

Q_DECLARE_OPERATORS_FOR_FLAGS(QFontDialog::FontDialogOptions)

#endif // QT_NO_FONTDIALOG

QT_END_NAMESPACE

#endif // QFONTDIALOG_H
