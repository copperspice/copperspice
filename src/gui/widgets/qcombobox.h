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

#ifndef QCOMBOBOX_H
#define QCOMBOBOX_H

#include <qwidget.h>
#include <qabstractitemdelegate.h>
#include <qabstractitemmodel.h>
#include <qvariant.h>

#ifndef QT_NO_COMBOBOX

class QAbstractItemView;
class QLineEdit;
class QComboBoxPrivate;
class QCompleter;

class Q_GUI_EXPORT QComboBox : public QWidget
{
   GUI_CS_OBJECT(QComboBox)

   GUI_CS_ENUM(InsertPolicy)
   GUI_CS_ENUM(SizeAdjustPolicy)

   GUI_CS_PROPERTY_READ(editable, isEditable)
   GUI_CS_PROPERTY_WRITE(editable, setEditable)

   GUI_CS_PROPERTY_READ(count, count)

   GUI_CS_PROPERTY_READ(currentText, currentText)
   GUI_CS_PROPERTY_WRITE(currentText, setCurrentText)
   GUI_CS_PROPERTY_NOTIFY(currentText, currentTextChanged)
   GUI_CS_PROPERTY_USER(currentText, true)

   GUI_CS_PROPERTY_READ(currentIndex, currentIndex)
   GUI_CS_PROPERTY_WRITE(currentIndex, setCurrentIndex)
   GUI_CS_PROPERTY_NOTIFY(currentIndex, cs_currentIndexChanged)

   GUI_CS_PROPERTY_READ(maxVisibleItems, maxVisibleItems)
   GUI_CS_PROPERTY_WRITE(maxVisibleItems, setMaxVisibleItems)

   GUI_CS_PROPERTY_READ(maxCount, maxCount)
   GUI_CS_PROPERTY_WRITE(maxCount, setMaxCount)

   GUI_CS_PROPERTY_READ(insertPolicy, insertPolicy)
   GUI_CS_PROPERTY_WRITE(insertPolicy, setInsertPolicy)

   GUI_CS_PROPERTY_READ(sizeAdjustPolicy, sizeAdjustPolicy)
   GUI_CS_PROPERTY_WRITE(sizeAdjustPolicy, setSizeAdjustPolicy)

   GUI_CS_PROPERTY_READ(minimumContentsLength, minimumContentsLength)
   GUI_CS_PROPERTY_WRITE(minimumContentsLength, setMinimumContentsLength)

   GUI_CS_PROPERTY_READ(iconSize, iconSize)
   GUI_CS_PROPERTY_WRITE(iconSize, setIconSize)

#ifndef QT_NO_COMPLETER
   GUI_CS_PROPERTY_READ(autoCompletion, autoCompletion)
   GUI_CS_PROPERTY_WRITE(autoCompletion, setAutoCompletion)
   GUI_CS_PROPERTY_DESIGNABLE(autoCompletion, false)
   GUI_CS_PROPERTY_READ(autoCompletionCaseSensitivity, autoCompletionCaseSensitivity)
   GUI_CS_PROPERTY_WRITE(autoCompletionCaseSensitivity, setAutoCompletionCaseSensitivity)
   GUI_CS_PROPERTY_DESIGNABLE(autoCompletionCaseSensitivity, false)
#endif

   GUI_CS_PROPERTY_READ(duplicatesEnabled, duplicatesEnabled)
   GUI_CS_PROPERTY_WRITE(duplicatesEnabled, setDuplicatesEnabled)
   GUI_CS_PROPERTY_READ(frame, hasFrame)
   GUI_CS_PROPERTY_WRITE(frame, setFrame)
   GUI_CS_PROPERTY_READ(modelColumn, modelColumn)
   GUI_CS_PROPERTY_WRITE(modelColumn, setModelColumn)

 public:
   GUI_CS_REGISTER_ENUM(
      enum InsertPolicy {
         NoInsert,
         InsertAtTop,
         InsertAtCurrent,
         InsertAtBottom,
         InsertAfterCurrent,
         InsertBeforeCurrent,
         InsertAlphabetically
      };
   )

   GUI_CS_REGISTER_ENUM(
      enum SizeAdjustPolicy {
         AdjustToContents,
         AdjustToContentsOnFirstShow,
         AdjustToMinimumContentsLengthWithIcon
      };
   )

   explicit QComboBox(QWidget *parent = nullptr);

   QComboBox(const QComboBox &) = delete;
   QComboBox &operator=(const QComboBox &) = delete;

   ~QComboBox();

   int maxVisibleItems() const;
   void setMaxVisibleItems(int maxItems);

   int count() const;
   void setMaxCount(int max);
   int maxCount() const;

#ifndef QT_NO_COMPLETER
   bool autoCompletion() const;
   void setAutoCompletion(bool enable);

   Qt::CaseSensitivity autoCompletionCaseSensitivity() const;
   void setAutoCompletionCaseSensitivity(Qt::CaseSensitivity sensitivity);
#endif

   bool duplicatesEnabled() const;
   void setDuplicatesEnabled(bool enable);

   void setFrame(bool enable);
   bool hasFrame() const;

   int findText(const QString &text,
      Qt::MatchFlags flags = static_cast<Qt::MatchFlags>(Qt::MatchExactly | Qt::MatchCaseSensitive)) const {
      return findData(text, Qt::DisplayRole, flags);
   }

   int findData(const QVariant &data, int role = Qt::UserRole,
      Qt::MatchFlags flags = static_cast<Qt::MatchFlags>(Qt::MatchExactly | Qt::MatchCaseSensitive)) const;

   InsertPolicy insertPolicy() const;
   void setInsertPolicy(InsertPolicy policy);

   SizeAdjustPolicy sizeAdjustPolicy() const;
   void setSizeAdjustPolicy(SizeAdjustPolicy policy);
   int minimumContentsLength() const;
   void setMinimumContentsLength(int characters);
   QSize iconSize() const;
   void setIconSize(const QSize &size);

   bool isEditable() const;
   void setEditable(bool editable);
   void setLineEdit(QLineEdit *edit);
   QLineEdit *lineEdit() const;

#ifndef QT_NO_VALIDATOR
   void setValidator(const QValidator *validator);
   const QValidator *validator() const;
#endif

#ifndef QT_NO_COMPLETER
   void setCompleter(QCompleter *completer);
   QCompleter *completer() const;
#endif

   QAbstractItemDelegate *itemDelegate() const;
   void setItemDelegate(QAbstractItemDelegate *delegate);

   QAbstractItemModel *model() const;
   void setModel(QAbstractItemModel *model);

   QModelIndex rootModelIndex() const;
   void setRootModelIndex(const QModelIndex &index);

   int modelColumn() const;
   void setModelColumn(int visibleColumn);

   int currentIndex() const;

   QString currentText() const;
   QVariant currentData(int role = Qt::UserRole) const;

   QString itemText(int index) const;
   QIcon itemIcon(int index) const;
   QVariant itemData(int index, int role = Qt::UserRole) const;

   inline void addItem(const QString &text, const QVariant &userData = QVariant());
   inline void addItem(const QIcon &icon, const QString &text, const QVariant &userData = QVariant());

   void addItems(const QStringList &texts) {
      insertItems(count(), texts);
   }

   inline void insertItem(int index, const QString &text, const QVariant &userData = QVariant());

   void insertItem(int index, const QIcon &icon, const QString &text,
      const QVariant &userData = QVariant());

   void insertItems(int index, const QStringList &list);
   void insertSeparator(int index);

   void removeItem(int index);

   void setItemText(int index, const QString &text);
   void setItemIcon(int index, const QIcon &icon);
   void setItemData(int index, const QVariant &value, int role = Qt::UserRole);

   QAbstractItemView *view() const;
   void setView(QAbstractItemView *itemView);

   QSize sizeHint() const override;
   QSize minimumSizeHint() const override;

   virtual void showPopup();
   virtual void hidePopup();

   bool event(QEvent *event) override;
   QVariant inputMethodQuery(Qt::InputMethodQuery query) const override;

   GUI_CS_SLOT_1(Public, void clear())
   GUI_CS_SLOT_2(clear)

   GUI_CS_SLOT_1(Public, void clearEditText())
   GUI_CS_SLOT_2(clearEditText)

   GUI_CS_SLOT_1(Public, void setEditText(const QString &text))
   GUI_CS_SLOT_2(setEditText)

   GUI_CS_SLOT_1(Public, void setCurrentIndex(int index))
   GUI_CS_SLOT_2(setCurrentIndex)

   GUI_CS_SLOT_1(Public, void setCurrentText(const QString &text))
   GUI_CS_SLOT_2(setCurrentText)

   GUI_CS_SIGNAL_1(Public, void editTextChanged(const QString &text))
   GUI_CS_SIGNAL_2(editTextChanged, text)

   GUI_CS_SIGNAL_1(Public, void activated(int index))
   GUI_CS_SIGNAL_OVERLOAD(activated, (int), index)

   GUI_CS_SIGNAL_1(Public, void activated(const QString &text))
   GUI_CS_SIGNAL_OVERLOAD(activated, (const QString &), text)

   GUI_CS_SIGNAL_1(Public, void highlighted(int index))
   GUI_CS_SIGNAL_OVERLOAD(highlighted, (int), index)

   GUI_CS_SIGNAL_1(Public, void highlighted(const QString &text))
   GUI_CS_SIGNAL_OVERLOAD(highlighted, (const QString &), text)

   GUI_CS_SIGNAL_1(Public, void currentIndexChanged(int index))
   GUI_CS_SIGNAL_OVERLOAD(currentIndexChanged, (int), index)

   GUI_CS_SIGNAL_1(Public, void currentIndexChanged(const QString &text))
   GUI_CS_SIGNAL_OVERLOAD(currentIndexChanged, (const QString &), text)

   GUI_CS_SIGNAL_1(Public, void cs_currentIndexChanged(int index))
   GUI_CS_SIGNAL_2(cs_currentIndexChanged, index)

   GUI_CS_SIGNAL_1(Public, void currentTextChanged(const QString &str))
   GUI_CS_SIGNAL_2(currentTextChanged, str)

 protected:
   void focusInEvent(QFocusEvent *event) override;
   void focusOutEvent(QFocusEvent *event) override;
   void changeEvent(QEvent *event) override;
   void resizeEvent(QResizeEvent *event) override;
   void paintEvent(QPaintEvent *event) override;
   void showEvent(QShowEvent *event) override;
   void hideEvent(QHideEvent *event) override;
   void mousePressEvent(QMouseEvent *event) override;
   void mouseReleaseEvent(QMouseEvent *event) override;
   void keyPressEvent(QKeyEvent *event) override;
   void keyReleaseEvent(QKeyEvent *event) override;

#ifndef QT_NO_WHEELEVENT
   void wheelEvent(QWheelEvent *event) override;
#endif

   void contextMenuEvent(QContextMenuEvent *event) override;
   void inputMethodEvent(QInputMethodEvent *event) override;
   void initStyleOption(QStyleOptionComboBox *option) const;

   QComboBox(QComboBoxPrivate &, QWidget *widget);

 private:
   Q_DECLARE_PRIVATE(QComboBox)

   GUI_CS_SLOT_1(Private, void _q_itemSelected(const QModelIndex &item))
   GUI_CS_SLOT_2(_q_itemSelected)

   GUI_CS_SLOT_1(Private, void _q_emitHighlighted(const QModelIndex &index))
   GUI_CS_SLOT_2(_q_emitHighlighted)

   GUI_CS_SLOT_1(Private, void _q_emitCurrentIndexChanged(const QModelIndex &index))
   GUI_CS_SLOT_2(_q_emitCurrentIndexChanged)

   GUI_CS_SLOT_1(Private, void _q_editingFinished())
   GUI_CS_SLOT_2(_q_editingFinished)

   GUI_CS_SLOT_1(Private, void _q_returnPressed())
   GUI_CS_SLOT_2(_q_returnPressed)

   GUI_CS_SLOT_1(Private, void _q_resetButton())
   GUI_CS_SLOT_2(_q_resetButton)

   GUI_CS_SLOT_1(Private, void _q_dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight))
   GUI_CS_SLOT_2(_q_dataChanged)

   GUI_CS_SLOT_1(Private, void _q_updateIndexBeforeChange())
   GUI_CS_SLOT_2(_q_updateIndexBeforeChange)

   GUI_CS_SLOT_1(Private, void _q_rowsInserted(const QModelIndex &parent, int start, int end))
   GUI_CS_SLOT_2(_q_rowsInserted)

   GUI_CS_SLOT_1(Private, void _q_rowsRemoved(const QModelIndex &parent, int start, int end))
   GUI_CS_SLOT_2(_q_rowsRemoved)

   GUI_CS_SLOT_1(Private, void _q_modelDestroyed())
   GUI_CS_SLOT_2(_q_modelDestroyed)

   GUI_CS_SLOT_1(Private, void _q_modelReset())
   GUI_CS_SLOT_2(_q_modelReset)

#ifndef QT_NO_COMPLETER
   GUI_CS_SLOT_1(Private, void _q_completerActivated(const QModelIndex &index))
   GUI_CS_SLOT_2(_q_completerActivated)
#endif

};

inline void QComboBox::addItem(const QString &text, const QVariant &userData)
{
   insertItem(count(), text, userData);
}

inline void QComboBox::addItem(const QIcon &icon, const QString &text, const QVariant &userData)
{
   insertItem(count(), icon, text, userData);
}

inline void QComboBox::insertItem(int index, const QString &text, const QVariant &userData)
{
   insertItem(index, QIcon(), text, userData);
}

#endif // QT_NO_COMBOBOX

#endif
