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

#ifndef PALETTEEDITOR_H
#define PALETTEEDITOR_H

#include "ui_paletteeditor.h"
#include <QtGui/QItemDelegate>

QT_BEGIN_NAMESPACE

class QListView;
class QLabel;
class QtColorButton;
class QDesignerFormEditorInterface;

namespace qdesigner_internal {

class PaletteEditor: public QDialog
{
    Q_OBJECT
public:
    virtual ~PaletteEditor();

    static QPalette getPalette(QDesignerFormEditorInterface *core,
                QWidget* parent, const QPalette &init = QPalette(),
                const QPalette &parentPal = QPalette(), int *result = 0);

    QPalette palette() const;
    void setPalette(const QPalette &palette);
    void setPalette(const QPalette &palette, const QPalette &parentPalette);

private slots:

    void on_buildButton_colorChanged(const QColor &);
    void on_activeRadio_clicked();
    void on_inactiveRadio_clicked();
    void on_disabledRadio_clicked();
    void on_computeRadio_clicked();
    void on_detailsRadio_clicked();

    void paletteChanged(const QPalette &palette);

protected:

private:
    PaletteEditor(QDesignerFormEditorInterface *core, QWidget *parent);
    void buildPalette();

    void updatePreviewPalette();
    void updateStyledButton();

    QPalette::ColorGroup currentColorGroup() const
        { return m_currentColorGroup; }

    Ui::PaletteEditor ui;
    QPalette m_editPalette;
    QPalette m_parentPalette;
    QPalette::ColorGroup m_currentColorGroup;
    class PaletteModel *m_paletteModel;
    bool m_modelUpdated;
    bool m_paletteUpdated;
    bool m_compute;
    QDesignerFormEditorInterface *m_core;
};


class PaletteModel : public QAbstractTableModel
{
    Q_OBJECT
    Q_PROPERTY(QPalette::ColorRole colorRole READ colorRole)
public:
    explicit PaletteModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                int role = Qt::DisplayRole) const;

    QPalette getPalette() const;
    void setPalette(const QPalette &palette, const QPalette &parentPalette);

    QPalette::ColorRole colorRole() const { return QPalette::NoRole; }
    void setCompute(bool on) { m_compute = on; }
signals:
    void paletteChanged(const QPalette &palette);
private:

    QPalette::ColorGroup columnToGroup(int index) const;
    int groupToColumn(QPalette::ColorGroup group) const;

    QPalette m_palette;
    QPalette m_parentPalette;
    QMap<QPalette::ColorRole, QString> m_roleNames;
    bool m_compute;
};

class BrushEditor : public QWidget
{
    Q_OBJECT
public:
    explicit BrushEditor(QDesignerFormEditorInterface *core, QWidget *parent = 0);

    void setBrush(const QBrush &brush);
    QBrush brush() const;
    bool changed() const;
signals:
    void changed(QWidget *widget);
private slots:
    void brushChanged();
private:
    QtColorButton *m_button;
    bool m_changed;
    QDesignerFormEditorInterface *m_core;
};

class RoleEditor : public QWidget
{
    Q_OBJECT
public:
    explicit RoleEditor(QWidget *parent = 0);

    void setLabel(const QString &label);
    void setEdited(bool on);
    bool edited() const;
signals:
    void changed(QWidget *widget);
private slots:
    void emitResetProperty();
private:
    QLabel *m_label;
    bool    m_edited;
};

class ColorDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    explicit ColorDelegate(QDesignerFormEditorInterface *core, QObject *parent = 0);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                const QModelIndex &index) const;

    void setEditorData(QWidget *ed, const QModelIndex &index) const;
    void setModelData(QWidget *ed, QAbstractItemModel *model,
                const QModelIndex &index) const;

    void updateEditorGeometry(QWidget *ed,
                const QStyleOptionViewItem &option, const QModelIndex &index) const;

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &opt,
                       const QModelIndex &index) const;
    virtual QSize sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const;
private:
    QDesignerFormEditorInterface *m_core;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // PALETTEEDITOR_H
