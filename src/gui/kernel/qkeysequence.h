/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef QKEYSEQUENCE_H
#define QKEYSEQUENCE_H

#include <qnamespace.h>
#include <qstring.h>
#include <qobject.h>

#ifndef QT_NO_SHORTCUT

class QKeySequence;
Q_GUI_EXPORT QDataStream &operator<<(QDataStream &in, const QKeySequence &ks);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &out, QKeySequence &ks);

class QVariant;
class QKeySequencePrivate;

class Q_GUI_EXPORT QKeySequence
{
   GUI_CS_GADGET(QKeySequence)

   GUI_CS_ENUM(StandardKey)

 public:
   enum StandardKey {
      UnknownKey,
      HelpContents,
      WhatsThis,
      Open,
      Close,
      Save,
      New,
      Delete,
      Cut,
      Copy,
      Paste,
      Undo,
      Redo,
      Back,
      Forward,
      Refresh,
      ZoomIn,
      ZoomOut,
      Print,
      AddTab,
      NextChild,
      PreviousChild,
      Find,
      FindNext,
      FindPrevious,
      Replace,
      SelectAll,
      Bold,
      Italic,
      Underline,
      MoveToNextChar,
      MoveToPreviousChar,
      MoveToNextWord,
      MoveToPreviousWord,
      MoveToNextLine,
      MoveToPreviousLine,
      MoveToNextPage,
      MoveToPreviousPage,
      MoveToStartOfLine,
      MoveToEndOfLine,
      MoveToStartOfBlock,
      MoveToEndOfBlock,
      MoveToStartOfDocument,
      MoveToEndOfDocument,
      SelectNextChar,
      SelectPreviousChar,
      SelectNextWord,
      SelectPreviousWord,
      SelectNextLine,
      SelectPreviousLine,
      SelectNextPage,
      SelectPreviousPage,
      SelectStartOfLine,
      SelectEndOfLine,
      SelectStartOfBlock,
      SelectEndOfBlock,
      SelectStartOfDocument,
      SelectEndOfDocument,
      DeleteStartOfWord,
      DeleteEndOfWord,
      DeleteEndOfLine,
      InsertParagraphSeparator,
      InsertLineSeparator,
      SaveAs,
      Preferences,
      Quit
   };

   enum SequenceFormat {
      NativeText,
      PortableText
   };

   QKeySequence();
   QKeySequence(const QString &key);
   QKeySequence(const QString &key, SequenceFormat format);
   QKeySequence(int k1, int k2 = 0, int k3 = 0, int k4 = 0);
   QKeySequence(const QKeySequence &ks);
   QKeySequence(StandardKey key);
   ~QKeySequence();

   int count() const;
   bool isEmpty() const;

   enum SequenceMatch {
      NoMatch,
      PartialMatch,
      ExactMatch
   };

   QString toString(SequenceFormat format = PortableText) const;
   static QKeySequence fromString(const QString &str, SequenceFormat format = PortableText);

   static QList<QKeySequence> listFromString(const QString &str, SequenceFormat format = PortableText);
   static QString listToString(const QList<QKeySequence> &list, SequenceFormat format = PortableText);

   SequenceMatch matches(const QKeySequence &seq) const;
   static QKeySequence mnemonic(const QString &text);
   static QList<QKeySequence> keyBindings(StandardKey key);

   operator QVariant() const;

   int operator[](uint i) const;
   QKeySequence &operator=(const QKeySequence &other);

   inline QKeySequence &operator=(QKeySequence && other) {
      qSwap(d, other.d);
      return *this;
   }

   inline void swap(QKeySequence &other) {
      qSwap(d, other.d);
   }

   bool operator==(const QKeySequence &other) const;
   inline bool operator!= (const QKeySequence &other) const {
      return !(*this == other);
   }

   bool operator< (const QKeySequence &ks) const;
   inline bool operator> (const QKeySequence &other) const {
      return other < *this;
   }

   inline bool operator<= (const QKeySequence &other) const {
      return !(other < *this);
   }

   inline bool operator>= (const QKeySequence &other) const {
      return !(*this < other);
   }

   bool isDetached() const;

 private:
   static int decodeString(const QString &ks);
   static QString encodeString(int key);
   int assign(const QString &str);
   int assign(const QString &str, SequenceFormat format);
   void setKey(int key, int index);

   QKeySequencePrivate *d;

   friend Q_GUI_EXPORT QDataStream &operator<<(QDataStream &in, const QKeySequence &ks);
   friend Q_GUI_EXPORT QDataStream &operator>>(QDataStream &in, QKeySequence &ks);
   friend class QShortcutMap;
   friend class QShortcut;

 public:
   typedef QKeySequencePrivate *DataPtr;
   inline DataPtr &data_ptr() {
      return d;
   }

};
Q_DECLARE_TYPEINFO(QKeySequence, Q_MOVABLE_TYPE);
Q_DECLARE_SHARED(QKeySequence)

Q_GUI_EXPORT QDebug operator<<(QDebug, const QKeySequence &);


#else

class Q_GUI_EXPORT QKeySequence
{
 public:
   QKeySequence() {}
   QKeySequence(int) {}
};

#endif // QT_NO_SHORTCUT

#endif // QKEYSEQUENCE_H
