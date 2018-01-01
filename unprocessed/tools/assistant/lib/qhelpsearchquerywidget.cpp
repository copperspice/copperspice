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

#include "qhelpsearchquerywidget.h"

#include <QtCore/QAbstractListModel>
#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QtGlobal>

#include <QtGui/QCompleter>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QFocusEvent>
#include <QtGui/QPushButton>
#include <QtGui/QToolButton>

QT_BEGIN_NAMESPACE

class QHelpSearchQueryWidgetPrivate : public QObject
{
    Q_OBJECT

private:
    struct QueryHistory {
        explicit QueryHistory() : curQuery(-1) {}
        QList<QList<QHelpSearchQuery> > queries;
        int curQuery;
    };

    class CompleterModel : public QAbstractListModel
    {
    public:
        explicit CompleterModel(QObject *parent)
          : QAbstractListModel(parent) {}

        int rowCount(const QModelIndex &parent = QModelIndex()) const
        {
            return parent.isValid() ? 0 : termList.size();
        }

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const
        {
            if (!index.isValid() || index.row() >= termList.count()||
                (role != Qt::EditRole && role != Qt::DisplayRole))
                return QVariant();
            return termList.at(index.row());
        }

        void addTerm(const QString &term)
        {
            if (!termList.contains(term)) {
                termList.append(term);
                reset();
            }
        }

    private:
        QStringList termList;
    };

    QHelpSearchQueryWidgetPrivate()
        : QObject()
        , simpleSearch(true)
        , searchCompleter(new CompleterModel(this), this)
    {
        searchButton = 0;
        advancedSearchWidget = 0;
        showHideAdvancedSearchButton = 0;
        defaultQuery = 0;
        exactQuery = 0;
        similarQuery = 0;
        withoutQuery = 0;
        allQuery = 0;
        atLeastQuery = 0;
    }

    ~QHelpSearchQueryWidgetPrivate()
    {
        // nothing todo
    }

    void retranslate()
    {
        simpleSearchLabel->setText(QHelpSearchQueryWidget::tr("Search for:"));
        prevQueryButton->setToolTip(QHelpSearchQueryWidget::tr("Previous search"));
        nextQueryButton->setToolTip(QHelpSearchQueryWidget::tr("Next search"));
        searchButton->setText(QHelpSearchQueryWidget::tr("Search"));
#ifdef QT_CLUCENE_SUPPORT
        advancedSearchLabel->setText(QHelpSearchQueryWidget::tr("Advanced search"));
        similarLabel->setText(QHelpSearchQueryWidget::tr("words <B>similar</B> to:"));
        withoutLabel->setText(QHelpSearchQueryWidget::tr("<B>without</B> the words:"));
        exactLabel->setText(QHelpSearchQueryWidget::tr("with <B>exact phrase</B>:"));
        allLabel->setText(QHelpSearchQueryWidget::tr("with <B>all</B> of the words:"));
        atLeastLabel->setText(QHelpSearchQueryWidget::tr("with <B>at least one</B> of the words:"));
#endif
    }

    QStringList buildTermList(const QString query)
    {
        bool s = false;
        QString phrase;
        QStringList wordList;
        QString searchTerm = query;

        for (int i = 0; i < searchTerm.length(); ++i) {
            if (searchTerm[i] == QLatin1Char('\"') && !s) {
                s = true;
                phrase = searchTerm[i];
                continue;
            }
            if (searchTerm[i] != QLatin1Char('\"') && s)
                phrase += searchTerm[i];
            if (searchTerm[i] == QLatin1Char('\"') && s) {
                s = false;
                phrase += searchTerm[i];
                wordList.append(phrase);
                searchTerm.remove(phrase);
            }
        }
        if (s)
            searchTerm.replace(phrase, phrase.mid(1));

        const QRegExp exp(QLatin1String("\\s+"));
        wordList += searchTerm.split(exp, QString::SkipEmptyParts);
        return wordList;
    }

    void saveQuery(const QList<QHelpSearchQuery> &query, QueryHistory &queryHist)
    {
        // We only add the query to the list if it is different from the last one.
        bool insert = false;
        if (queryHist.queries.empty())
            insert = true;
        else {
            const QList<QHelpSearchQuery> &lastQuery = queryHist.queries.last();
            if (lastQuery.size() != query.size()) {
                insert = true;
            } else {
                for (int i = 0; i < query.size(); ++i) {
                    if (query.at(i).fieldName != lastQuery.at(i).fieldName
                        || query.at(i).wordList != lastQuery.at(i).wordList) {
                        insert = true;
                        break;
                    }
                }
            }
        }
        if (insert) {
            queryHist.queries.append(query);
            foreach (const QHelpSearchQuery &queryPart, query) {
                static_cast<CompleterModel *>(searchCompleter.model())->
                        addTerm(queryPart.wordList.join(" "));
            }
        }
    }

    void nextOrPrevQuery(int maxOrMinIndex, int addend, QToolButton *thisButton,
        QToolButton *otherButton)
    {
        QueryHistory *queryHist;
        QList<QLineEdit *> lineEdits;
        if (simpleSearch) {
            queryHist = &simpleQueries;
            lineEdits << defaultQuery;
        } else {
            queryHist = &complexQueries;
            lineEdits << allQuery << atLeastQuery << similarQuery
                << withoutQuery << exactQuery;
        }
        foreach (QLineEdit *lineEdit, lineEdits)
            lineEdit->clear();

        // Otherwise, the respective button would be disabled.
        Q_ASSERT(queryHist->curQuery != maxOrMinIndex);

        queryHist->curQuery += addend;
        const QList<QHelpSearchQuery> &query =
            queryHist->queries.at(queryHist->curQuery);
        foreach (const QHelpSearchQuery &queryPart, query) {
            if (QLineEdit *lineEdit = lineEditFor(queryPart.fieldName))
                lineEdit->setText(queryPart.wordList.join(" "));
        }

        if (queryHist->curQuery == maxOrMinIndex)
            thisButton->setEnabled(false);
        otherButton->setEnabled(true);
    }

    QLineEdit* lineEditFor(const QHelpSearchQuery::FieldName &fieldName) const
    {
        switch (fieldName) {
            case QHelpSearchQuery::DEFAULT:
                return defaultQuery;
            case QHelpSearchQuery::ALL:
                return allQuery;
            case QHelpSearchQuery::ATLEAST:
                return atLeastQuery;
            case QHelpSearchQuery::FUZZY:
                return similarQuery;
            case QHelpSearchQuery::WITHOUT:
                return withoutQuery;
            case QHelpSearchQuery::PHRASE:
                return exactQuery;
            default:
                Q_ASSERT(0);
        }
        return 0;
    }

    void enableOrDisableToolButtons()
    {
        const QueryHistory &queryHist = simpleSearch ? simpleQueries
            : complexQueries;
        prevQueryButton->setEnabled(queryHist.curQuery > 0);
        nextQueryButton->setEnabled(queryHist.curQuery
            < queryHist.queries.size() - 1);
    }

private slots:
    void showHideAdvancedSearch()
    {
        if (simpleSearch) {
            advancedSearchWidget->show();
            showHideAdvancedSearchButton->setText((QLatin1String("-")));
        } else {
            advancedSearchWidget->hide();
            showHideAdvancedSearchButton->setText((QLatin1String("+")));
        }

        simpleSearch = !simpleSearch;
        defaultQuery->setEnabled(simpleSearch);
        enableOrDisableToolButtons();
    }

    void searchRequested()
    {
        QList<QHelpSearchQuery> queryList;
#if !defined(QT_CLUCENE_SUPPORT)
        queryList.append(QHelpSearchQuery(QHelpSearchQuery::DEFAULT,
            QStringList(defaultQuery->text())));

#else
        if (defaultQuery->isEnabled()) {
            queryList.append(QHelpSearchQuery(QHelpSearchQuery::DEFAULT,
                buildTermList(defaultQuery->text())));
        } else {
            const QRegExp exp(QLatin1String("\\s+"));
            QStringList lst = similarQuery->text().split(exp,
                QString::SkipEmptyParts);
            if (!lst.isEmpty()) {
                QStringList fuzzy;
                foreach (const QString &term, lst)
                    fuzzy += buildTermList(term);
                queryList.append(QHelpSearchQuery(QHelpSearchQuery::FUZZY,
                    fuzzy));
            }

            lst = withoutQuery->text().split(exp, QString::SkipEmptyParts);
            if (!lst.isEmpty()) {
                QStringList without;
                foreach (const QString &term, lst)
                    without.append(term);
                queryList.append(QHelpSearchQuery(QHelpSearchQuery::WITHOUT,
                    without));
            }

            if (!exactQuery->text().isEmpty()) {
                QString phrase = exactQuery->text().remove(QLatin1Char('\"'));
                phrase = phrase.simplified();
                queryList.append(QHelpSearchQuery(QHelpSearchQuery::PHRASE,
                    QStringList(phrase)));
            }

            lst = allQuery->text().split(exp, QString::SkipEmptyParts);
            if (!lst.isEmpty()) {
                QStringList all;
                foreach (const QString &term, lst)
                    all.append(term);
                queryList.append(QHelpSearchQuery(QHelpSearchQuery::ALL, all));
            }

            lst = atLeastQuery->text().split(exp, QString::SkipEmptyParts);
            if (!lst.isEmpty()) {
                QStringList atLeast;
                foreach (const QString &term, lst)
                    atLeast += buildTermList(term);
                queryList.append(QHelpSearchQuery(QHelpSearchQuery::ATLEAST,
                    atLeast));
            }
        }
#endif
        QueryHistory &queryHist = simpleSearch ? simpleQueries : complexQueries;
        saveQuery(queryList, queryHist);
        queryHist.curQuery = queryHist.queries.size() - 1;
        if (queryHist.curQuery > 0)
            prevQueryButton->setEnabled(true);
        nextQueryButton->setEnabled(false);
    }

    void nextQuery()
    {
        nextOrPrevQuery((simpleSearch ? simpleQueries
            : complexQueries).queries.size() - 1, 1, nextQueryButton,
                prevQueryButton);
    }

    void prevQuery()
    {
        nextOrPrevQuery(0, -1, prevQueryButton, nextQueryButton);
    }

private:
    friend class QHelpSearchQueryWidget;

    bool simpleSearch;
    QLabel *simpleSearchLabel;
    QLabel *advancedSearchLabel;
    QLabel *similarLabel;
    QLabel *withoutLabel;
    QLabel *exactLabel;
    QLabel *allLabel;
    QLabel *atLeastLabel;
    QPushButton *searchButton;
    QWidget* advancedSearchWidget;
    QToolButton *showHideAdvancedSearchButton;
    QLineEdit *defaultQuery;
    QLineEdit *exactQuery;
    QLineEdit *similarQuery;
    QLineEdit *withoutQuery;
    QLineEdit *allQuery;
    QLineEdit *atLeastQuery;
    QToolButton *nextQueryButton;
    QToolButton *prevQueryButton;
    QueryHistory simpleQueries;
    QueryHistory complexQueries;
    QCompleter searchCompleter;
};

#include "qhelpsearchquerywidget.moc"


/*!
    \class QHelpSearchQueryWidget
    \since 4.4
    \inmodule QtHelp
    \brief The QHelpSearchQueryWidget class provides a simple line edit or
    an advanced widget to enable the user to input a search term in a
    standardized input mask.
*/

/*!
    \fn void QHelpSearchQueryWidget::search()

    This signal is emitted when a the user has the search button invoked.
    After reciving the signal you can ask the QHelpSearchQueryWidget for the
    build list of QHelpSearchQuery's that you may pass to the QHelpSearchEngine's
    search() function.
*/

/*!
    Constructs a new search query widget with the given \a parent.
*/
QHelpSearchQueryWidget::QHelpSearchQueryWidget(QWidget *parent)
    : QWidget(parent)
{
    d = new QHelpSearchQueryWidgetPrivate();

    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->setMargin(0);

    QHBoxLayout* hBoxLayout = new QHBoxLayout();
    d->simpleSearchLabel = new QLabel(this);
    d->defaultQuery = new QLineEdit(this);
    d->defaultQuery->setCompleter(&d->searchCompleter);
    d->prevQueryButton = new QToolButton(this);
    d->prevQueryButton->setArrowType(Qt::LeftArrow);
    d->prevQueryButton->setEnabled(false);
    d->nextQueryButton = new QToolButton(this);
    d->nextQueryButton->setArrowType(Qt::RightArrow);
    d->nextQueryButton->setEnabled(false);
    d->searchButton = new QPushButton(this);
    hBoxLayout->addWidget(d->simpleSearchLabel);
    hBoxLayout->addWidget(d->defaultQuery);
    hBoxLayout->addWidget(d->prevQueryButton);
    hBoxLayout->addWidget(d->nextQueryButton);
    hBoxLayout->addWidget(d->searchButton);

    vLayout->addLayout(hBoxLayout);

    connect(d->prevQueryButton, SIGNAL(clicked()), d, SLOT(prevQuery()));
    connect(d->nextQueryButton, SIGNAL(clicked()), d, SLOT(nextQuery()));
    connect(d->searchButton, SIGNAL(clicked()), this, SIGNAL(search()));
    connect(d->defaultQuery, SIGNAL(returnPressed()), this, SIGNAL(search()));

#if defined(QT_CLUCENE_SUPPORT)
    hBoxLayout = new QHBoxLayout();
    d->showHideAdvancedSearchButton = new QToolButton(this);
    d->showHideAdvancedSearchButton->setText(QLatin1String("+"));
    d->showHideAdvancedSearchButton->setMinimumSize(25, 20);

    d->advancedSearchLabel = new QLabel(this);
    QSizePolicy sizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    sizePolicy.setHeightForWidth(d->advancedSearchLabel->sizePolicy().hasHeightForWidth());
    d->advancedSearchLabel->setSizePolicy(sizePolicy);

    QFrame* hLine = new QFrame(this);
    hLine->setFrameStyle(QFrame::HLine);
    hBoxLayout->addWidget(d->showHideAdvancedSearchButton);
    hBoxLayout->addWidget(d->advancedSearchLabel);
    hBoxLayout->addWidget(hLine);

    vLayout->addLayout(hBoxLayout);

    // setup advanced search layout
    d->advancedSearchWidget = new QWidget(this);
    QGridLayout *gLayout = new QGridLayout(d->advancedSearchWidget);
    gLayout->setMargin(0);

    d->similarLabel = new QLabel(this);
    gLayout->addWidget(d->similarLabel, 0, 0);
    d->similarQuery = new QLineEdit(this);
    d->similarQuery->setCompleter(&d->searchCompleter);
    gLayout->addWidget(d->similarQuery, 0, 1);

    d->withoutLabel = new QLabel(this);
    gLayout->addWidget(d->withoutLabel, 1, 0);
    d->withoutQuery = new QLineEdit(this);
    d->withoutQuery->setCompleter(&d->searchCompleter);
    gLayout->addWidget(d->withoutQuery, 1, 1);

    d->exactLabel = new QLabel(this);
    gLayout->addWidget(d->exactLabel, 2, 0);
    d->exactQuery = new QLineEdit(this);
    d->exactQuery->setCompleter(&d->searchCompleter);
    gLayout->addWidget(d->exactQuery, 2, 1);

    d->allLabel = new QLabel(this);
    gLayout->addWidget(d->allLabel, 3, 0);
    d->allQuery = new QLineEdit(this);
    d->allQuery->setCompleter(&d->searchCompleter);
    gLayout->addWidget(d->allQuery, 3, 1);

    d->atLeastLabel = new QLabel(this);
    gLayout->addWidget(d->atLeastLabel, 4, 0);
    d->atLeastQuery = new QLineEdit(this);
    d->atLeastQuery->setCompleter(&d->searchCompleter);
    gLayout->addWidget(d->atLeastQuery, 4, 1);

    vLayout->addWidget(d->advancedSearchWidget);
    d->advancedSearchWidget->hide();

    d->retranslate();

    connect(d->exactQuery, SIGNAL(returnPressed()), this, SIGNAL(search()));
    connect(d->similarQuery, SIGNAL(returnPressed()), this, SIGNAL(search()));
    connect(d->withoutQuery, SIGNAL(returnPressed()), this, SIGNAL(search()));
    connect(d->allQuery, SIGNAL(returnPressed()), this, SIGNAL(search()));
    connect(d->atLeastQuery, SIGNAL(returnPressed()), this, SIGNAL(search()));
    connect(d->showHideAdvancedSearchButton, SIGNAL(clicked()),
        d, SLOT(showHideAdvancedSearch()));
#endif
    connect(this, SIGNAL(search()), d, SLOT(searchRequested()));
}

/*!
    Destroys the search query widget.
*/
QHelpSearchQueryWidget::~QHelpSearchQueryWidget()
{
    delete d;
}

/*!
    \since 4.8

    Expands the search query widget so that the extended search fields are shown.
*/
void QHelpSearchQueryWidget::expandExtendedSearch()
{
    if (d->simpleSearch)
        d->showHideAdvancedSearch();
}

/*!
    \since 4.8

    Collapses the search query widget so that only the default search field is
    shown.
*/
void QHelpSearchQueryWidget::collapseExtendedSearch()
{
    if (!d->simpleSearch)
        d->showHideAdvancedSearch();
}

/*!
    Returns a list of queries to use in combination with the search engines
    search(QList<QHelpSearchQuery> &queryList) function.
*/
QList<QHelpSearchQuery> QHelpSearchQueryWidget::query() const
{
    const QHelpSearchQueryWidgetPrivate::QueryHistory &queryHist =
        d->simpleSearch ? d->simpleQueries : d->complexQueries;
    return queryHist.queries.isEmpty() ?
        QList<QHelpSearchQuery>() : queryHist.queries.last();
}

/*!
    \since 4.8

    Sets the QHelpSearchQueryWidget input fields to the values specified by
    \a queryList search field name. Please note that one has to call the search
    engine's search(QList<QHelpSearchQuery> &queryList) function to perform the
    actual search.
*/
void QHelpSearchQueryWidget::setQuery(const QList<QHelpSearchQuery> &queryList)
{
    QList<QLineEdit *> lineEdits;
    lineEdits << d->defaultQuery << d->allQuery << d->atLeastQuery
        << d->similarQuery << d->withoutQuery << d->exactQuery;
    foreach (QLineEdit *lineEdit, lineEdits)
        lineEdit->clear();

    const QLatin1String space(" ");
    foreach (const QHelpSearchQuery &q, queryList) {
        if (QLineEdit *lineEdit = d->lineEditFor(q.fieldName))
            lineEdit->setText(lineEdit->text() + q.wordList.join(space) + space);
    }
    d->searchRequested();
}

/*!
    \reimp
*/
void QHelpSearchQueryWidget::focusInEvent(QFocusEvent *focusEvent)
{
    if (focusEvent->reason() != Qt::MouseFocusReason) {
        d->defaultQuery->selectAll();
        d->defaultQuery->setFocus();
    }
}

/*! \reimp
*/
void QHelpSearchQueryWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
        d->retranslate();
    else
        QWidget::changeEvent(event);
}

QT_END_NAMESPACE
