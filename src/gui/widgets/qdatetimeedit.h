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

#ifndef QDATETIMEEDIT_H
#define QDATETIMEEDIT_H

#include <qabstractspinbox.h>
#include <qdatetime.h>
#include <qtimezone.h>
#include <qvariant.h>

#ifndef QT_NO_DATETIMEEDIT

class QDateTimeEditPrivate;
class QStyleOptionSpinBox;
class QCalendarWidget;

class Q_GUI_EXPORT QDateTimeEdit : public QAbstractSpinBox
{
   GUI_CS_OBJECT(QDateTimeEdit)

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

   GUI_CS_PROPERTY_READ(timeZone, timeZone)
   GUI_CS_PROPERTY_WRITE(timeZone, setTimeZone)

 public:
   GUI_CS_REGISTER_ENUM(
      enum Section {
         NoSection     = 0x0000,
         AmPmSection   = 0x0001,
         MSecSection   = 0x0002,
         SecondSection = 0x0004,
         MinuteSection = 0x0008,
         HourSection   = 0x0010,
         DaySection    = 0x0100,
         MonthSection  = 0x0200,
         YearSection   = 0x0400,
         TimeSections_Mask = AmPmSection | MSecSection | SecondSection | MinuteSection | HourSection,
         DateSections_Mask = DaySection | MonthSection | YearSection
      };
   )

   using Sections = QFlags<Section>;

   explicit QDateTimeEdit(QWidget *parent = nullptr);
   explicit QDateTimeEdit(const QDateTime &datetime, QWidget *parent = nullptr);
   explicit QDateTimeEdit(const QDate &date, QWidget *parent = nullptr);
   explicit QDateTimeEdit(const QTime &time, QWidget *parent = nullptr);

   QDateTimeEdit(const QDateTimeEdit &) = delete;
   QDateTimeEdit &operator=(const QDateTimeEdit &) = delete;

   ~QDateTimeEdit();

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

   QTimeZone timeZone() const;
   void setTimeZone(QTimeZone tz);

   QSize sizeHint() const override;

   void clear() override;
   void stepBy(int steps) override;

   bool event(QEvent *event) override;

   GUI_CS_SIGNAL_1(Public, void dateTimeChanged(const QDateTime &datetime))
   GUI_CS_SIGNAL_2(dateTimeChanged, datetime)

   GUI_CS_SIGNAL_1(Public, void timeChanged(const QTime &time))
   GUI_CS_SIGNAL_2(timeChanged, time)

   GUI_CS_SIGNAL_1(Public, void dateChanged(const QDate &date))
   GUI_CS_SIGNAL_2(dateChanged, date)

   GUI_CS_SLOT_1(Public, void setDateTime(const QDateTime &datetime))
   GUI_CS_SLOT_2(setDateTime)

   GUI_CS_SLOT_1(Public, void setDate(const QDate &date))
   GUI_CS_SLOT_2(setDate)

   GUI_CS_SLOT_1(Public, void setTime(const QTime &time))
   GUI_CS_SLOT_2(setTime)

 protected:
   void keyPressEvent(QKeyEvent *event) override;

#ifndef QT_NO_WHEELEVENT
   void wheelEvent(QWheelEvent *event) override;
#endif

   void focusInEvent(QFocusEvent *event) override;
   bool focusNextPrevChild(bool next) override;
   QValidator::State validate(QString &input, int &pos) const override;
   void fixup(QString &input) const override;

   virtual QDateTime dateTimeFromText(const QString &text) const;
   virtual QString textFromDateTime(const QDateTime &datetime) const;

   StepEnabled stepEnabled() const override;
   void mousePressEvent(QMouseEvent *event) override;
   void paintEvent(QPaintEvent *event) override;
   void initStyleOption(QStyleOptionSpinBox *option) const;

   QDateTimeEdit(const QVariant &val, QVariant::Type parserType, QWidget *parent = nullptr);

 private:
   Q_DECLARE_PRIVATE(QDateTimeEdit)

   GUI_CS_SLOT_1(Private, void _q_resetButton())
   GUI_CS_SLOT_2(_q_resetButton)
};

class Q_GUI_EXPORT QTimeEdit : public QDateTimeEdit
{
   GUI_CS_OBJECT(QTimeEdit)

   GUI_CS_PROPERTY_READ(time, time)
   GUI_CS_PROPERTY_WRITE(time, setTime)
   GUI_CS_PROPERTY_NOTIFY(time, userTimeChanged)
   GUI_CS_PROPERTY_USER(time, true)

 public:
   explicit QTimeEdit(QWidget *parent = nullptr);
   explicit QTimeEdit(const QTime &time, QWidget *parent = nullptr);
   ~QTimeEdit();

   GUI_CS_SIGNAL_1(Public, void userTimeChanged(const QTime &time))
   GUI_CS_SIGNAL_2(userTimeChanged, time)
};

class Q_GUI_EXPORT QDateEdit : public QDateTimeEdit
{
   GUI_CS_OBJECT(QDateEdit)

   GUI_CS_PROPERTY_READ(date, date)
   GUI_CS_PROPERTY_WRITE(date, setDate)
   GUI_CS_PROPERTY_NOTIFY(date, userDateChanged)
   GUI_CS_PROPERTY_USER(date, true)

 public:
   explicit QDateEdit(QWidget *parent = nullptr);
   explicit QDateEdit(const QDate &date, QWidget *parent = nullptr);
   ~QDateEdit();

   GUI_CS_SIGNAL_1(Public, void userDateChanged(const QDate &date))
   GUI_CS_SIGNAL_2(userDateChanged, date)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QDateTimeEdit::Sections)

#endif // QT_NO_DATETIMEEDIT

#endif
