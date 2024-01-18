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

#ifndef QCALENDARWIDGET_H
#define QCALENDARWIDGET_H

#include <qwidget.h>
#include <qdatetime.h>

#ifndef QT_NO_CALENDARWIDGET

class QDate;
class QTextCharFormat;
class QCalendarWidgetPrivate;

class Q_GUI_EXPORT QCalendarWidget : public QWidget
{
   GUI_CS_OBJECT(QCalendarWidget)

   GUI_CS_ENUM(HorizontalHeaderFormat)
   GUI_CS_ENUM(VerticalHeaderFormat)
   GUI_CS_ENUM(SelectionMode)

   GUI_CS_PROPERTY_READ(selectedDate, selectedDate)
   GUI_CS_PROPERTY_WRITE(selectedDate, setSelectedDate)

   GUI_CS_PROPERTY_READ(minimumDate, minimumDate)
   GUI_CS_PROPERTY_WRITE(minimumDate, setMinimumDate)

   GUI_CS_PROPERTY_READ(maximumDate, maximumDate)
   GUI_CS_PROPERTY_WRITE(maximumDate, setMaximumDate)

   GUI_CS_PROPERTY_READ(firstDayOfWeek, firstDayOfWeek)
   GUI_CS_PROPERTY_WRITE(firstDayOfWeek, setFirstDayOfWeek)

   GUI_CS_PROPERTY_READ(gridVisible, isGridVisible)
   GUI_CS_PROPERTY_WRITE(gridVisible, setGridVisible)

   GUI_CS_PROPERTY_READ(selectionMode, selectionMode)
   GUI_CS_PROPERTY_WRITE(selectionMode, setSelectionMode)

   GUI_CS_PROPERTY_READ(horizontalHeaderFormat, horizontalHeaderFormat)
   GUI_CS_PROPERTY_WRITE(horizontalHeaderFormat, setHorizontalHeaderFormat)

   GUI_CS_PROPERTY_READ(verticalHeaderFormat, verticalHeaderFormat)
   GUI_CS_PROPERTY_WRITE(verticalHeaderFormat, setVerticalHeaderFormat)

   GUI_CS_PROPERTY_READ(navigationBarVisible, isNavigationBarVisible)
   GUI_CS_PROPERTY_WRITE(navigationBarVisible, setNavigationBarVisible)

   GUI_CS_PROPERTY_READ(dateEditEnabled, isDateEditEnabled)
   GUI_CS_PROPERTY_WRITE(dateEditEnabled, setDateEditEnabled)

   GUI_CS_PROPERTY_READ(dateEditAcceptDelay, dateEditAcceptDelay)
   GUI_CS_PROPERTY_WRITE(dateEditAcceptDelay, setDateEditAcceptDelay)

 public:
   GUI_CS_REGISTER_ENUM(
      enum HorizontalHeaderFormat {
         NoHorizontalHeader,
         SingleLetterDayNames,
         ShortDayNames,
         LongDayNames
      };
   )

   GUI_CS_REGISTER_ENUM(
      enum VerticalHeaderFormat {
         NoVerticalHeader,
         ISOWeekNumbers
      };
   )

   GUI_CS_REGISTER_ENUM(
      enum SelectionMode {
         NoSelection,
         SingleSelection
      };
   )

   explicit QCalendarWidget(QWidget *parent = nullptr);

   QCalendarWidget(const QCalendarWidget &) = delete;
   QCalendarWidget &operator=(const QCalendarWidget &) = delete;

   ~QCalendarWidget();

   QSize sizeHint() const override;
   QSize minimumSizeHint() const override;

   QDate selectedDate() const;

   int yearShown() const;
   int monthShown() const;

   QDate minimumDate() const;
   void setMinimumDate(const QDate &date);

   QDate maximumDate() const;
   void setMaximumDate(const QDate &date);

   Qt::DayOfWeek firstDayOfWeek() const;
   void setFirstDayOfWeek(Qt::DayOfWeek dayOfWeek);

   bool isNavigationBarVisible() const;

   bool isGridVisible() const;

   SelectionMode selectionMode() const;
   void setSelectionMode(SelectionMode mode);

   HorizontalHeaderFormat horizontalHeaderFormat() const;
   void setHorizontalHeaderFormat(HorizontalHeaderFormat format);

   VerticalHeaderFormat verticalHeaderFormat() const;
   void setVerticalHeaderFormat(VerticalHeaderFormat format);

   QTextCharFormat headerTextFormat() const;
   void setHeaderTextFormat(const QTextCharFormat &format);

   QTextCharFormat weekdayTextFormat(Qt::DayOfWeek dayOfWeek) const;
   void setWeekdayTextFormat(Qt::DayOfWeek dayOfWeek, const QTextCharFormat &format);

   QMap<QDate, QTextCharFormat> dateTextFormat() const;
   QTextCharFormat dateTextFormat(const QDate &date) const;
   void setDateTextFormat(const QDate &date, const QTextCharFormat &format);

   bool isDateEditEnabled() const;
   void setDateEditEnabled(bool enable);

   int dateEditAcceptDelay() const;
   void setDateEditAcceptDelay(int delay);

   GUI_CS_SLOT_1(Public, void setSelectedDate(const QDate &date))
   GUI_CS_SLOT_2(setSelectedDate)

   GUI_CS_SLOT_1(Public, void setDateRange(const QDate &min, const QDate &max))
   GUI_CS_SLOT_2(setDateRange)

   GUI_CS_SLOT_1(Public, void setCurrentPage(int year, int month))
   GUI_CS_SLOT_2(setCurrentPage)

   GUI_CS_SLOT_1(Public, void setGridVisible(bool show))
   GUI_CS_SLOT_2(setGridVisible)

   GUI_CS_SLOT_1(Public, void setNavigationBarVisible(bool visible))
   GUI_CS_SLOT_2(setNavigationBarVisible)

   GUI_CS_SLOT_1(Public, void showNextMonth())
   GUI_CS_SLOT_2(showNextMonth)

   GUI_CS_SLOT_1(Public, void showPreviousMonth())
   GUI_CS_SLOT_2(showPreviousMonth)

   GUI_CS_SLOT_1(Public, void showNextYear())
   GUI_CS_SLOT_2(showNextYear)

   GUI_CS_SLOT_1(Public, void showPreviousYear())
   GUI_CS_SLOT_2(showPreviousYear)

   GUI_CS_SLOT_1(Public, void showSelectedDate())
   GUI_CS_SLOT_2(showSelectedDate)

   GUI_CS_SLOT_1(Public, void showToday())
   GUI_CS_SLOT_2(showToday)

   GUI_CS_SIGNAL_1(Public, void selectionChanged())
   GUI_CS_SIGNAL_2(selectionChanged)

   GUI_CS_SIGNAL_1(Public, void clicked(const QDate &date))
   GUI_CS_SIGNAL_2(clicked, date)

   GUI_CS_SIGNAL_1(Public, void activated(const QDate &date))
   GUI_CS_SIGNAL_2(activated, date)

   GUI_CS_SIGNAL_1(Public, void currentPageChanged(int year, int month))
   GUI_CS_SIGNAL_2(currentPageChanged, year, month)

 protected:
   bool event(QEvent *event) override;
   bool eventFilter(QObject *watched, QEvent *event) override;
   void mousePressEvent(QMouseEvent *event) override;
   void resizeEvent(QResizeEvent *event) override;
   void keyPressEvent(QKeyEvent *event) override;

   virtual void paintCell(QPainter *painter, const QRect &rect, const QDate &date) const;
   void updateCell(const QDate &date);
   void updateCells();

 private:
   Q_DECLARE_PRIVATE(QCalendarWidget)

   GUI_CS_SLOT_1(Private, void _q_slotShowDate(const QDate &date))
   GUI_CS_SLOT_2(_q_slotShowDate)

   GUI_CS_SLOT_1(Private, void _q_slotChangeDate(const QDate &date))
   GUI_CS_SLOT_OVERLOAD(_q_slotChangeDate, (const QDate &) )

   GUI_CS_SLOT_1(Private, void _q_slotChangeDate(const QDate &date, bool changeMonth))
   GUI_CS_SLOT_OVERLOAD(_q_slotChangeDate, (const QDate &, bool))

   GUI_CS_SLOT_1(Private, void _q_editingFinished())
   GUI_CS_SLOT_2(_q_editingFinished)

   GUI_CS_SLOT_1(Private, void _q_prevMonthClicked())
   GUI_CS_SLOT_2(_q_prevMonthClicked)

   GUI_CS_SLOT_1(Private, void _q_nextMonthClicked())
   GUI_CS_SLOT_2(_q_nextMonthClicked)

   GUI_CS_SLOT_1(Private, void _q_yearEditingFinished())
   GUI_CS_SLOT_2(_q_yearEditingFinished)

   GUI_CS_SLOT_1(Private, void _q_yearClicked())
   GUI_CS_SLOT_2(_q_yearClicked)

   GUI_CS_SLOT_1(Private, void _q_monthChanged(QAction *act))
   GUI_CS_SLOT_2(_q_monthChanged)
};

#endif // QT_NO_CALENDARWIDGET



#endif

