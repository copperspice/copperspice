/***********************************************************************
*
* Copyright (c) 2012-2016 Barbara Geller
* Copyright (c) 2012-2016 Ansel Sermersheim
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

#ifndef QDATETIMEEDIT_P_H
#define QDATETIMEEDIT_P_H

#include <QtGui/qcombobox.h>
#include <QtGui/qcalendarwidget.h>
#include <QtGui/qspinbox.h>
#include <QtGui/qtoolbutton.h>
#include <QtGui/qmenu.h>
#include <QtGui/qlabel.h>
#include <QtGui/qdatetimeedit.h>
#include <qabstractspinbox_p.h>
#include <qdatetime_p.h>
#include <qdebug.h>

#ifndef QT_NO_DATETIMEEDIT

QT_BEGIN_NAMESPACE

class QCalendarPopup;

class QDateTimeEditPrivate : public QAbstractSpinBoxPrivate, public QDateTimeParser
{
   Q_DECLARE_PUBLIC(QDateTimeEdit)

 public:
   QDateTimeEditPrivate();

   void init(const QVariant &var);
   void readLocaleSettings();

   void emitSignals(EmitPolicy ep, const QVariant &old);
   QString textFromValue(const QVariant &f) const;
   QVariant valueFromText(const QString &f) const;
   virtual void _q_editorCursorPositionChanged(int oldpos, int newpos);
   virtual void interpret(EmitPolicy ep);
   virtual void clearCache() const;

   QDateTime validateAndInterpret(QString &input, int &, QValidator::State &state,
                                  bool fixup = false) const;
   void clearSection(int index);
   virtual QString displayText() const {
      return edit->text();   // this is from QDateTimeParser
   }

   int absoluteIndex(QDateTimeEdit::Section s, int index) const;
   int absoluteIndex(const SectionNode &s) const;
   void updateEdit();
   QDateTime stepBy(int index, int steps, bool test = false) const;
   int sectionAt(int pos) const;
   int closestSection(int index, bool forward) const;
   int nextPrevSection(int index, bool forward) const;
   void setSelected(int index, bool forward = false);

   void updateCache(const QVariant &val, const QString &str) const;

   void updateTimeSpec();
   virtual QDateTime getMinimum() const {
      return minimum.toDateTime();
   }

   virtual QDateTime getMaximum() const {
      return maximum.toDateTime();
   }
 
   virtual QLocale locale() const {
      return q_func()->locale();
   }

   QString valueToText(const QVariant &var) const {
      return textFromValue(var);
   }
  
   QString getAmPmText(AmPm ap, Case cs) const;
   int cursorPosition() const {
      return edit ? edit->cursorPosition() : -1;
   }

   virtual QStyle::SubControl newHoverControl(const QPoint &pos);
   virtual void updateEditFieldGeometry();
   virtual QVariant getZeroVariant() const;
   virtual void setRange(const QVariant &min, const QVariant &max);

   void _q_resetButton();
   void updateArrow(QStyle::StateFlag state);
   bool calendarPopupEnabled() const;
   void syncCalendarWidget();

   bool isSeparatorKey(const QKeyEvent *k) const;

   static QDateTimeEdit::Sections convertSections(QDateTimeParser::Sections s);
   static QDateTimeEdit::Section convertToPublic(QDateTimeParser::Section s);

   void initCalendarPopup(QCalendarWidget *cw = 0);
   void positionCalendarPopup();

   QDateTimeEdit::Sections sections;
   mutable bool cacheGuard;

   QString defaultDateFormat, defaultTimeFormat, defaultDateTimeFormat, unreversedFormat;
   mutable QVariant conflictGuard;
   bool hasHadFocus, formatExplicitlySet, calendarPopup;
   QStyle::StateFlag arrowState;
   QCalendarPopup *monthCalendar;

#ifdef QT_KEYPAD_NAVIGATION
   bool focusOnButton;
#endif
};


class QCalendarPopup : public QWidget
{
   GUI_CS_OBJECT(QCalendarPopup)

 public:
   QCalendarPopup(QWidget *parent = 0, QCalendarWidget *cw = 0);
   QDate selectedDate() {
      return verifyCalendarInstance()->selectedDate();
   }

   void setDate(const QDate &date);
   void setDateRange(const QDate &min, const QDate &max);
   void setFirstDayOfWeek(Qt::DayOfWeek dow) {
      verifyCalendarInstance()->setFirstDayOfWeek(dow);
   }

   QCalendarWidget *calendarWidget() const {
      return const_cast<QCalendarPopup *>(this)->verifyCalendarInstance();
   }

   void setCalendarWidget(QCalendarWidget *cw);
 
   GUI_CS_SIGNAL_1(Public, void activated(const QDate &date))
   GUI_CS_SIGNAL_2(activated, date)
   GUI_CS_SIGNAL_1(Public, void newDateSelected(const QDate &newDate))
   GUI_CS_SIGNAL_2(newDateSelected, newDate)
   GUI_CS_SIGNAL_1(Public, void hidingCalendar(const QDate &oldDate))
   GUI_CS_SIGNAL_2(hidingCalendar, oldDate)
   GUI_CS_SIGNAL_1(Public, void resetButton())
   GUI_CS_SIGNAL_2(resetButton)

 protected:
   void hideEvent(QHideEvent *) override;
   void mousePressEvent(QMouseEvent *e) override;
   void mouseReleaseEvent(QMouseEvent *) override;
   bool event(QEvent *e) override;

 private :
   GUI_CS_SLOT_1(Private, void dateSelected(const QDate &date))
   GUI_CS_SLOT_2(dateSelected)
   GUI_CS_SLOT_1(Private, void dateSelectionChanged())
   GUI_CS_SLOT_2(dateSelectionChanged)
 
   QCalendarWidget *verifyCalendarInstance();

   QWeakPointer<QCalendarWidget> calendar;
   QDate oldDate;
   bool dateChanged;
};

QT_END_NAMESPACE

#endif // QT_NO_DATETIMEEDIT

#endif // QDATETIMEEDIT_P_H
