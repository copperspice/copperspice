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

#ifndef QDATETIMEEDIT_P_H
#define QDATETIMEEDIT_P_H

#include <qcombobox.h>
#include <qcalendarwidget.h>
#include <qspinbox.h>
#include <qtoolbutton.h>
#include <qmenu.h>
#include <qlabel.h>
#include <qdatetimeedit.h>
#include <qabstractspinbox_p.h>
#include <qdebug.h>

#include <qdatetimeparser_p.h>

#ifndef QT_NO_DATETIMEEDIT

class QCalendarPopup;

class QDateTimeEditPrivate : public QAbstractSpinBoxPrivate, public QDateTimeParser
{
   Q_DECLARE_PUBLIC(QDateTimeEdit)

 public:
   QDateTimeEditPrivate();
   ~QDateTimeEditPrivate();

   void init(const QVariant &var);
   void readLocaleSettings();

   void emitSignals(EmitPolicy ep, const QVariant &old) override;
   QString textFromValue(const QVariant &f) const override;
   QVariant valueFromText(const QString &f) const override;
   void _q_editorCursorPositionChanged(int oldpos, int newpos) override;
   void interpret(EmitPolicy ep) override;
   void clearCache() const override;

   QStyle::SubControl newHoverControl(const QPoint &pos) override;
   void updateEditFieldGeometry() override;
   QVariant getZeroVariant() const override;
   void setRange(const QVariant &min, const QVariant &max) override;
   void updateEdit() override;

   QDateTime validateAndInterpret(QString &input, int &, QValidator::State &state, bool fixup = false) const;
   void clearSection(int index);

   QString displayText() const override {
      return edit->text();
   }

   QDateTime getMinimum() const override {
      return minimum.toDateTime();
   }

   QDateTime getMaximum() const override {
      return maximum.toDateTime();
   }

   QLocale locale() const override {
      return q_func()->locale();
   }

   QString getAmPmText(AmPm ap, Case cs) const override;

   int cursorPosition() const override {
      return edit ? edit->cursorPosition() : -1;
   }

   int absoluteIndex(QDateTimeEdit::Section s, int index) const;
   int absoluteIndex(const SectionNode &s) const;

   QDateTime stepBy(int index, int steps, bool test = false) const;
   int sectionAt(int pos) const;
   int closestSection(int index, bool forward) const;
   int nextPrevSection(int index, bool forward) const;
   void setSelected(int index, bool forward = false);

   void updateCache(const QVariant &val, const QString &str) const;
   void updateTimeZone();

   QString valueToText(const QVariant &var) const {
      return textFromValue(var);
   }

   void _q_resetButton();
   void updateArrow(QStyle::StateFlag state);
   bool calendarPopupEnabled() const;
   void syncCalendarWidget();

   bool isSeparatorKey(const QKeyEvent *k) const;

   static QDateTimeEdit::Sections convertSections(QDateTimeParser::Sections s);
   static QDateTimeEdit::Section convertToPublic(QDateTimeParser::Section s);

   void initCalendarPopup(QCalendarWidget *cw = nullptr);
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
   explicit  QCalendarPopup(QWidget *parent = nullptr, QCalendarWidget *cw = nullptr);

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

 private:
   GUI_CS_SLOT_1(Private, void dateSelected(const QDate &date))
   GUI_CS_SLOT_2(dateSelected)

   GUI_CS_SLOT_1(Private, void dateSelectionChanged())
   GUI_CS_SLOT_2(dateSelectionChanged)

   QCalendarWidget *verifyCalendarInstance();

   QPointer<QCalendarWidget> calendar;
   QDate oldDate;
   bool dateChanged;
};

#endif // QT_NO_DATETIMEEDIT

#endif // QDATETIMEEDIT_P_H
