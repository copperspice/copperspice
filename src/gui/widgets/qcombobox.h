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

#ifndef QCOMBOBOX_H
#define QCOMBOBOX_H

#include <QtGui/qwidget.h>
#include <QtGui/qabstractitemdelegate.h>
#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qvariant.h>

QT_BEGIN_NAMESPACE

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
   explicit QComboBox(QWidget *parent = nullptr);
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

   void setFrame(bool);
   bool hasFrame() const;

   inline int findText(const QString &text,
                       Qt::MatchFlags flags = static_cast<Qt::MatchFlags>(Qt::MatchExactly | Qt::MatchCaseSensitive)) const {
      return findData(text, Qt::DisplayRole, flags);
   }
   int findData(const QVariant &data, int role = Qt::UserRole,
                Qt::MatchFlags flags = static_cast<Qt::MatchFlags>(Qt::MatchExactly | Qt::MatchCaseSensitive)) const;

   enum InsertPolicy {
      NoInsert,
      InsertAtTop,
      InsertAtCurrent,
      InsertAtBottom,
      InsertAfterCurrent,
      InsertBeforeCurrent,
      InsertAlphabetically
   };

   InsertPolicy insertPolicy() const;
   void setInsertPolicy(InsertPolicy policy);

   enum SizeAdjustPolicy {
      AdjustToContents,
      AdjustToContentsOnFirstShow,
      AdjustToMinimumContentsLength, // ### Qt5/remove
      AdjustToMinimumContentsLengthWithIcon
   };

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
   void setValidator(const QValidator *v);
   const QValidator *validator() const;
#endif

#ifndef QT_NO_COMPLETER
   void setCompleter(QCompleter *c);
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

   QString itemText(int index) const;
   QIcon itemIcon(int index) const;
   QVariant itemData(int index, int role = Qt::UserRole) const;

   inline void addItem(const QString &text, const QVariant &userData = QVariant());
   inline void addItem(const QIcon &icon, const QString &text, const QVariant &userData = QVariant());

   inline void addItems(const QStringList &texts) {
      insertItems(count(), texts);
   }

   inline void insertItem(int index, const QString &text, const QVariant &userData = QVariant());

   void insertItem(int index, const QIcon &icon, const QString &text,
                   const QVariant &userData = QVariant());

   void insertItems(int index, const QStringList &texts);
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
 
   GUI_CS_SLOT_1(Public, void clear())
   GUI_CS_SLOT_2(clear)
   GUI_CS_SLOT_1(Public, void clearEditText())
   GUI_CS_SLOT_2(clearEditText)
   GUI_CS_SLOT_1(Public, void setEditText(const QString &text))
   GUI_CS_SLOT_2(setEditText)
   GUI_CS_SLOT_1(Public, void setCurrentIndex(int index))
   GUI_CS_SLOT_2(setCurrentIndex)

   GUI_CS_SIGNAL_1(Public, void editTextChanged(const QString &un_named_arg1))
   GUI_CS_SIGNAL_2(editTextChanged, un_named_arg1)

   GUI_CS_SIGNAL_1(Public, void activated(int index))
   GUI_CS_SIGNAL_OVERLOAD(activated, (int), index)

   GUI_CS_SIGNAL_1(Public, void activated(const QString &un_named_arg1))
   GUI_CS_SIGNAL_OVERLOAD(activated, (const QString &), un_named_arg1)

   GUI_CS_SIGNAL_1(Public, void highlighted(int index))
   GUI_CS_SIGNAL_OVERLOAD(highlighted, (int), index)

   GUI_CS_SIGNAL_1(Public, void highlighted(const QString &un_named_arg1))
   GUI_CS_SIGNAL_OVERLOAD(highlighted, (const QString &), un_named_arg1)

   GUI_CS_SIGNAL_1(Public, void currentIndexChanged(int index))
   GUI_CS_SIGNAL_OVERLOAD(currentIndexChanged, (int), index)

   GUI_CS_SIGNAL_1(Public, void currentIndexChanged(const QString &un_named_arg1))
   GUI_CS_SIGNAL_OVERLOAD(currentIndexChanged, (const QString &), un_named_arg1)

   GUI_CS_SIGNAL_1(Public, void cs_currentIndexChanged(int index))
   GUI_CS_SIGNAL_2(cs_currentIndexChanged, index)

 protected:
   void focusInEvent(QFocusEvent *e) override;
   void focusOutEvent(QFocusEvent *e) override;
   void changeEvent(QEvent *e) override;
   void resizeEvent(QResizeEvent *e) override;
   void paintEvent(QPaintEvent *e) override;
   void showEvent(QShowEvent *e) override;
   void hideEvent(QHideEvent *e) override;
   void mousePressEvent(QMouseEvent *e) override;
   void mouseReleaseEvent(QMouseEvent *e) override;
   void keyPressEvent(QKeyEvent *e) override;
   void keyReleaseEvent(QKeyEvent *e) override;

#ifndef QT_NO_WHEELEVENT
   void wheelEvent(QWheelEvent *e) override;
#endif

   void contextMenuEvent(QContextMenuEvent *e) override;
   void inputMethodEvent(QInputMethodEvent *) override;
   QVariant inputMethodQuery(Qt::InputMethodQuery) const override;
   void initStyleOption(QStyleOptionComboBox *option) const;
 
   QComboBox(QComboBoxPrivate &, QWidget *);

 private:
   Q_DECLARE_PRIVATE(QComboBox)
   Q_DISABLE_COPY(QComboBox)

   GUI_CS_SLOT_1(Private, void _q_itemSelected(const QModelIndex &item))
   GUI_CS_SLOT_2(_q_itemSelected)

   GUI_CS_SLOT_1(Private, void _q_emitHighlighted(const QModelIndex &un_named_arg1))
   GUI_CS_SLOT_2(_q_emitHighlighted)

   GUI_CS_SLOT_1(Private, void _q_emitCurrentIndexChanged(const QModelIndex &index))
   GUI_CS_SLOT_2(_q_emitCurrentIndexChanged)

   GUI_CS_SLOT_1(Private, void _q_editingFinished())
   GUI_CS_SLOT_2(_q_editingFinished)

   GUI_CS_SLOT_1(Private, void _q_returnPressed())
   GUI_CS_SLOT_2(_q_returnPressed)

   GUI_CS_SLOT_1(Private, void _q_resetButton())
   GUI_CS_SLOT_2(_q_resetButton)

   GUI_CS_SLOT_1(Private, void _q_dataChanged(const QModelIndex &un_named_arg1, const QModelIndex &un_named_arg2))
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

#ifdef QT_KEYPAD_NAVIGATION
   GUI_CS_SLOT_1(Private, void _q_completerActivated())
   GUI_CS_SLOT_2(_q_completerActivated)
#endif

};

inline void QComboBox::addItem(const QString &atext, const QVariant &auserData)
{
   insertItem(count(), atext, auserData);
}

inline void QComboBox::addItem(const QIcon &aicon, const QString &atext, const QVariant &auserData)
{
   insertItem(count(), aicon, atext, auserData);
}

inline void QComboBox::insertItem(int aindex, const QString &atext, const QVariant &auserData)
{
   insertItem(aindex, QIcon(), atext, auserData);
}

#endif // QT_NO_COMBOBOX

QT_END_NAMESPACE

#endif // QCOMBOBOX_H
