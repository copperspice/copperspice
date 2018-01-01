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

   void emitSignals(EmitPolicy ep, const QVariant &old) override;
   QString textFromValue(const QVariant &f) const override;
   QVariant valueFromText(const QString &f) const override;
   void _q_editorCursorPositionChanged(int oldpos, int newpos) override;
   void interpret(EmitPolicy ep) override;
   void clearCache() const override;

   QDateTime validateAndInterpret(QString &input, int &, QValidator::State &state, bool fixup = false) const;
   void clearSection(int index);

   QString displayText() const override {
      return edit->text();   // this is from QDateTimeParser
   }

   int absoluteIndex(QDateTimeEdit::Section s, int index) const;
   int absoluteIndex(const SectionNode &s) const;
   void updateEdit() override;
   QDateTime stepBy(int index, int steps, bool test = false) const;
   int sectionAt(int pos) const;
   int closestSection(int index, bool forward) const;
   int nextPrevSection(int index, bool forward) const;
   void setSelected(int index, bool forward = false);

   void updateCache(const QVariant &val, const QString &str) const;

   void updateTimeSpec();
   QDateTime getMinimum() const override {
      return minimum.toDateTime();
   }

   QDateTime getMaximum() const override {
      return maximum.toDateTime();
   }
 
   QLocale locale() const override {
      return q_func()->locale();
   }

   QString valueToText(const QVariant &var) const {
      return textFromValue(var);
   }
  
   QString getAmPmText(AmPm ap, Case cs) const override;
   int cursorPosition() const override {
      return edit ? edit->cursorPosition() : -1;
   }

   QStyle::SubControl newHoverControl(const QPoint &pos) override;
   void updateEditFieldGeometry() override;
   QVariant getZeroVariant() const override;
   void setRange(const QVariant &min, const QVariant &max) override;

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
   QCalendarPopup(QWidget *parent = nullptr, QCalendarWidget *cw = 0);
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
