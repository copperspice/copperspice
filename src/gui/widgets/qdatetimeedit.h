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

#ifndef QDATETIMEEDIT_H
#define QDATETIMEEDIT_H

#include <QtCore/qdatetime.h>
#include <QtCore/qvariant.h>
#include <QtGui/qabstractspinbox.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_DATETIMEEDIT

class QDateTimeEditPrivate;
class QStyleOptionSpinBox;
class QCalendarWidget;

class Q_GUI_EXPORT QDateTimeEdit : public QAbstractSpinBox
{
   CS_OBJECT(QDateTimeEdit)

   GUI_CS_ENUM(Section)
   GUI_CS_FLAG(Section, Sections)

   GUI_CS_PROPERTY_READ(dateTime, dateTime)
   GUI_CS_PROPERTY_WRITE(dateTime, setDateTime)
   GUI_CS_PROPERTY_NOTIFY(dateTime, dateTimeChanged)
   GUI_CS_PROPERTY_USER(dateTime, true)
   GUI_CS_PROPERTY_READ(date, date)
   GUI_CS_PROPERTY_WRITE(date, setDate)
   GUI_CS_PROPERTY_NOTIFY(date, dateChanged)
   GUI_CS_PROPERTY_READ(time, time)
   GUI_CS_PROPERTY_WRITE(time, setTime)
   GUI_CS_PROPERTY_NOTIFY(time, timeChanged)
   GUI_CS_PROPERTY_READ(maximumDateTime, maximumDateTime)
   GUI_CS_PROPERTY_WRITE(maximumDateTime, setMaximumDateTime)
   GUI_CS_PROPERTY_RESET(maximumDateTime, clearMaximumDateTime)
   GUI_CS_PROPERTY_READ(minimumDateTime, minimumDateTime)
   GUI_CS_PROPERTY_WRITE(minimumDateTime, setMinimumDateTime)
   GUI_CS_PROPERTY_RESET(minimumDateTime, clearMinimumDateTime)
   GUI_CS_PROPERTY_READ(maximumDate, maximumDate)
   GUI_CS_PROPERTY_WRITE(maximumDate, setMaximumDate)
   GUI_CS_PROPERTY_RESET(maximumDate, clearMaximumDate)
   GUI_CS_PROPERTY_READ(minimumDate, minimumDate)
   GUI_CS_PROPERTY_WRITE(minimumDate, setMinimumDate)
   GUI_CS_PROPERTY_RESET(minimumDate, clearMinimumDate)
   GUI_CS_PROPERTY_READ(maximumTime, maximumTime)
   GUI_CS_PROPERTY_WRITE(maximumTime, setMaximumTime)
   GUI_CS_PROPERTY_RESET(maximumTime, clearMaximumTime)
   GUI_CS_PROPERTY_READ(minimumTime, minimumTime)
   GUI_CS_PROPERTY_WRITE(minimumTime, setMinimumTime)
   GUI_CS_PROPERTY_RESET(minimumTime, clearMinimumTime)
   GUI_CS_PROPERTY_READ(currentSection, currentSection)
   GUI_CS_PROPERTY_WRITE(currentSection, setCurrentSection)
   GUI_CS_PROPERTY_READ(displayedSections, displayedSections)
   GUI_CS_PROPERTY_READ(displayFormat, displayFormat)
   GUI_CS_PROPERTY_WRITE(displayFormat, setDisplayFormat)
   GUI_CS_PROPERTY_READ(calendarPopup, calendarPopup)
   GUI_CS_PROPERTY_WRITE(calendarPopup, setCalendarPopup)
   GUI_CS_PROPERTY_READ(currentSectionIndex, currentSectionIndex)
   GUI_CS_PROPERTY_WRITE(currentSectionIndex, setCurrentSectionIndex)
   GUI_CS_PROPERTY_READ(sectionCount, sectionCount)
   GUI_CS_PROPERTY_READ(timeSpec, timeSpec)
   GUI_CS_PROPERTY_WRITE(timeSpec, setTimeSpec)

 public:
   enum Section {
      NoSection = 0x0000,
      AmPmSection = 0x0001,
      MSecSection = 0x0002,
      SecondSection = 0x0004,
      MinuteSection = 0x0008,
      HourSection   = 0x0010,
      DaySection    = 0x0100,
      MonthSection  = 0x0200,
      YearSection   = 0x0400,
      TimeSections_Mask = AmPmSection | MSecSection | SecondSection | MinuteSection | HourSection,
      DateSections_Mask = DaySection | MonthSection | YearSection
   };

   using Sections = QFlags<Section>;

   explicit QDateTimeEdit(QWidget *parent = 0);
   explicit QDateTimeEdit(const QDateTime &dt, QWidget *parent = 0);
   explicit QDateTimeEdit(const QDate &d, QWidget *parent = 0);
   explicit QDateTimeEdit(const QTime &t, QWidget *parent = 0);

   QDateTime dateTime() const;
   QDate date() const;
   QTime time() const;

   QDateTime minimumDateTime() const;
   void clearMinimumDateTime();
   void setMinimumDateTime(const QDateTime &dt);

   QDateTime maximumDateTime() const;
   void clearMaximumDateTime();
   void setMaximumDateTime(const QDateTime &dt);

   void setDateTimeRange(const QDateTime &min, const QDateTime &max);

   QDate minimumDate() const;
   void setMinimumDate(const QDate &min);
   void clearMinimumDate();

   QDate maximumDate() const;
   void setMaximumDate(const QDate &max);
   void clearMaximumDate();

   void setDateRange(const QDate &min, const QDate &max);

   QTime minimumTime() const;
   void setMinimumTime(const QTime &min);
   void clearMinimumTime();

   QTime maximumTime() const;
   void setMaximumTime(const QTime &max);
   void clearMaximumTime();

   void setTimeRange(const QTime &min, const QTime &max);

   Sections displayedSections() const;
   Section currentSection() const;
   Section sectionAt(int index) const;
   void setCurrentSection(Section section);

   int currentSectionIndex() const;
   void setCurrentSectionIndex(int index);

   QCalendarWidget *calendarWidget() const;
   void setCalendarWidget(QCalendarWidget *calendarWidget);

   int sectionCount() const;

   void setSelectedSection(Section section);

   QString sectionText(Section section) const;

   QString displayFormat() const;
   void setDisplayFormat(const QString &format);

   bool calendarPopup() const;
   void setCalendarPopup(bool enable);

   Qt::TimeSpec timeSpec() const;
   void setTimeSpec(Qt::TimeSpec spec);

   QSize sizeHint() const;

   virtual void clear();
   virtual void stepBy(int steps);

   bool event(QEvent *event);

   GUI_CS_SIGNAL_1(Public, void dateTimeChanged(const QDateTime &date))
   GUI_CS_SIGNAL_2(dateTimeChanged, date)
   GUI_CS_SIGNAL_1(Public, void timeChanged(const QTime &date))
   GUI_CS_SIGNAL_2(timeChanged, date)
   GUI_CS_SIGNAL_1(Public, void dateChanged(const QDate &date))
   GUI_CS_SIGNAL_2(dateChanged, date)

   GUI_CS_SLOT_1(Public, void setDateTime(const QDateTime &dateTime))
   GUI_CS_SLOT_2(setDateTime)
   GUI_CS_SLOT_1(Public, void setDate(const QDate &date))
   GUI_CS_SLOT_2(setDate)
   GUI_CS_SLOT_1(Public, void setTime(const QTime &time))
   GUI_CS_SLOT_2(setTime)

 protected:
   virtual void keyPressEvent(QKeyEvent *event);

#ifndef QT_NO_WHEELEVENT
   virtual void wheelEvent(QWheelEvent *event);
#endif

   virtual void focusInEvent(QFocusEvent *event);
   virtual bool focusNextPrevChild(bool next);
   virtual QValidator::State validate(QString &input, int &pos) const;
   virtual void fixup(QString &input) const;

   virtual QDateTime dateTimeFromText(const QString &text) const;
   virtual QString textFromDateTime(const QDateTime &dt) const;
   virtual StepEnabled stepEnabled() const;
   virtual void mousePressEvent(QMouseEvent *event);
   virtual void paintEvent(QPaintEvent *event);
   void initStyleOption(QStyleOptionSpinBox *option) const;

   QDateTimeEdit(const QVariant &val, QVariant::Type parserType, QWidget *parent = 0);

 private:
   Q_DECLARE_PRIVATE(QDateTimeEdit)
   Q_DISABLE_COPY(QDateTimeEdit)

   GUI_CS_SLOT_1(Private, void _q_resetButton())
   GUI_CS_SLOT_2(_q_resetButton)
};

class Q_GUI_EXPORT QTimeEdit : public QDateTimeEdit
{
   CS_OBJECT(QTimeEdit)

 public:
   QTimeEdit(QWidget *parent = 0);
   QTimeEdit(const QTime &time, QWidget *parent = 0);
};

class Q_GUI_EXPORT QDateEdit : public QDateTimeEdit
{
   CS_OBJECT(QDateEdit)

 public:
   QDateEdit(QWidget *parent = 0);
   QDateEdit(const QDate &date, QWidget *parent = 0);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QDateTimeEdit::Sections)

#endif // QT_NO_DATETIMEEDIT

QT_END_NAMESPACE

#endif // QDATETIMEEDIT_H
