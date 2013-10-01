/***************************************************************************
    copyright     : (C) 1999-2001 Ewald Arnold <kvoctrain@ewald-arnold.de>
                    (C) 2005-2007 Peter Hedlund <peter.hedlund@kdemail.net>
                    (C) 2007-2009 Frederik Gladhorn <gladhorn@kde.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "testentrymanager.h"

#include "entryfilter.h"

#include <prefs.h>
#include <klocale.h>
#include <kconfig.h>

#include <keduvoclesson.h>
#include <keduvocexpression.h>
#include <keduvocdocument.h>

#include <KDebug>
#include <KMessageBox>
#include <QDateTime>

using namespace Practice;

TestEntryManager::TestEntryManager(QWidget* parent)
    :m_parent(parent)
    ,m_fromTranslation(0)
    ,m_toTranslation(1)
    ,m_currentEntry(-1)
    ,m_totalTime(0)
    ,m_randomSequence(QDateTime::currentDateTime().toTime_t())
{
}

TestEntryManager::~TestEntryManager()
{

    qDeleteAll(m_allTestEntries);
}

void TestEntryManager::setDocument(KEduVocDocument* doc)
{
    qDeleteAll(m_allTestEntries);
    m_allTestEntries.clear();
    m_notAskedTestEntries.clear();
    m_currentEntries.clear();

    m_doc = doc;

    // don't crash when trying to start practicing a document containing only one language
    if (m_doc->identifierCount() < 2) {
        KMessageBox::error(0, i18n("The vocabulary collection contains fewer than two languages.", i18n("Could not start practice")));
        return;
    }
    if (Prefs::questionLanguage() >= m_doc->identifierCount() || Prefs::solutionLanguage() >= m_doc->identifierCount()) {
        kDebug() << "Invalid language selection" << m_fromTranslation << " to " << m_toTranslation;
        Prefs::setQuestionLanguage(0);
        Prefs::setSolutionLanguage(1);
    }

    setLanguages(Prefs::questionLanguage(), Prefs::solutionLanguage());
    kDebug() << "Test from: " << m_doc->identifier(m_fromTranslation).name()
        << " to: " << m_doc->identifier(m_toTranslation).name();

    filterTestEntries();
    kDebug() << "Found " << m_allTestEntries.count() << " entries after filtering.";

    m_notAskedTestEntries = m_allTestEntries;

    for ( int i = 0; i < qMin(m_notAskedTestEntries.count(), Prefs::testNumberOfEntries() ); i++ ) {
        m_currentEntries.append( m_notAskedTestEntries.takeAt(0) );
    }
}

QString TestEntryManager::title() const
{
    return m_doc->title();
}

void TestEntryManager::setLanguages(int from, int to)
{
    m_fromTranslation = from;
    m_toTranslation = to;
    TestEntry::setGradeFrom(m_fromTranslation);
    TestEntry::setGradeTo(m_toTranslation);
}

void TestEntryManager::filterTestEntries()
{
    EntryFilter filter(m_parent, m_doc);
    m_allTestEntries = filter.entries();
}

void TestEntryManager::removeCurrentEntryFromPractice()
{
    if (m_currentEntry >= 0) {
        m_currentEntries.takeAt(m_currentEntry);
    }
}

void TestEntryManager::printStatistics()
{
    kDebug() << "Test statistics: ";
    foreach ( TestEntry* entry, m_allTestEntries ) {
        kDebug()
            << " asked: " << entry->statisticCount()
            << " +" << entry->statisticGoodCount() << " -" << entry->statisticBadCount()
            << "Entry: " << entry->entry()->translation(0)->text();
    }
}

int TestEntryManager::totalTime()
{
    // seconds instead of ms
    return m_totalTime / (1000);
}

void TestEntryManager::practiceStarted()
{
    kDebug() << "start practice timer";
    m_time.start();
}

void TestEntryManager::practiceFinished()
{
    m_totalTime = m_time.elapsed();
    kDebug() << "stop practice timer" << m_totalTime << m_time.toString();
}

int TestEntryManager::totalEntryCount()
{
    return m_allTestEntries.count();
}

int TestEntryManager::activeEntryCount()
{
    return m_notAskedTestEntries.count() + m_currentEntries.count();
}

QList<TestEntry*> TestEntryManager::allUnansweredTestEntries()
{
    QList<TestEntry*> allUnansweredEntries;

    allUnansweredEntries.append(m_notAskedTestEntries);
    allUnansweredEntries.append(m_currentEntries);

    return allUnansweredEntries;
}

int TestEntryManager::statisticTotalCorrectFirstAttempt()
{
    int count = 0;
    foreach(TestEntry* entry, m_allTestEntries) {
        if ( entry->correctAtFirstAttempt() ) {
            count++;
        }
    }
    return count;
}

int TestEntryManager::statisticTotalWrong()
{
    int count = 0;
    foreach(TestEntry* entry, m_allTestEntries) {
        if ( entry->statisticBadCount() ) {
            count++;
        }
    }
    return count;
}

int TestEntryManager::statisticTotalUnanswered()
{
    int count = 0;
    foreach(TestEntry* entry, m_allTestEntries) {
        if ( entry->statisticCount() == 0 ) {
            count++;
        }
    }
    return count;
}


TestEntry* TestEntryManager::getNextEntry()
{
    // refill current entries
    while ( m_currentEntries.count() < Prefs::testNumberOfEntries() &&
            m_notAskedTestEntries.count() > 0 ) {
        m_currentEntries.append( m_notAskedTestEntries.takeAt(0) );
    }

    int lastEntry = m_currentEntry;
    // return one of the current entries
    if ( m_currentEntries.count() > 0 ) {
        // one of the current words (by random)
        m_currentEntry = m_randomSequence.getLong(m_currentEntries.count());
        // do not allow to ask the same entry twice in a row
        if ( m_currentEntries.count() > 1 ) {
            while ( m_currentEntry == lastEntry ) {
                m_currentEntry = m_randomSequence.getLong(m_currentEntries.count());
            }
        }

        kDebug() << "nextEntry: " << m_currentEntry << " = " << m_currentEntries.value(m_currentEntry)->entry()->translation(0)->text() << " (" << m_currentEntries.count() + m_notAskedTestEntries.count() << "entries remaining)";

        return m_currentEntries.value(m_currentEntry);
    } else {
        return 0;
    }
}

QStringList TestEntryManager::multipleChoiceAnswers(int numberChoices)
{
    QStringList choices;
    KRandomSequence randomSequence;
    QList<KEduVocExpression*> allEntries = m_doc->lesson()->entries(KEduVocLesson::Recursive);
    int numValidEntries = 0;
    int count = numberChoices;

    // if the current entry has predefined multiple choice entries definied, use them first
    QStringList predefinedChoices = m_currentEntries.at(m_currentEntry)->entry()->translation(Prefs::solutionLanguage())->multipleChoice();
    while (!predefinedChoices.isEmpty() && count > 0) {
        choices.append(predefinedChoices.takeAt(randomSequence.getLong(predefinedChoices.count())));
        count--;
    }

    // find out if we got enough valid entries to fill all the options
    for(int i = 0; i < allEntries.count(); ++i) {
        if(isValidMultipleChoiceAnswer(allEntries.value(i)))
            numValidEntries++;
        if(numValidEntries >= numberChoices)
            break;
    }

    // if we don't have enough valid entries, just try everything and use what we can get
    if (numValidEntries < numberChoices) {
        for (int i = choices.count(); i < allEntries.count(); ++i) {
            KEduVocExpression *exp = allEntries.value(i);

            if (isValidMultipleChoiceAnswer(exp)) {
                choices.append(exp->translation(Prefs::solutionLanguage())->text());
            }
        }
    } else {
        QList<KEduVocExpression*> exprlist;
        while (count > 0) {
            int nr;
            // if there are enough non-empty fields, fill the options only with those
            do {
                nr = randomSequence.getLong(allEntries.count());
            } while (!isValidMultipleChoiceAnswer(allEntries.value(nr)));
            // append if new entry found
            bool newex = true;
            for (int i = 0; newex && i < exprlist.count(); i++) {
                if (exprlist[i] == allEntries.value(nr))
                    newex = false;
            }
            if (newex) {
                count--;
                exprlist.append(allEntries.value(nr));
            }
        }

        for (int i = 0; i < exprlist.count(); i++) {
            choices.append(exprlist[i]->translation(Prefs::solutionLanguage())->text());
        }
    }

    kDebug() << "choices:" << choices;
    return choices;
}

bool TestEntryManager::isValidMultipleChoiceAnswer(KEduVocExpression *e)
{
    // entry is empty
    if (e->translation(Prefs::solutionLanguage())->text().trimmed().isEmpty())
        return false;
    // entry is a synonym of the solution
    if (e->translation(Prefs::solutionLanguage())->synonyms().contains(m_currentEntries.at(m_currentEntry)->entry()->translation(Prefs::solutionLanguage())))
        return false;
    // entry has the same text as the solution
    if (e->translation(Prefs::solutionLanguage())->text().simplified() == m_currentEntries.at(m_currentEntry)->entry()->translation(Prefs::solutionLanguage())->text().simplified())
        return false;
    return true;
}

