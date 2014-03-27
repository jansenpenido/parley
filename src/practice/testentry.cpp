/***************************************************************************
    Copyright 2007 Frederik Gladhorn <frederik.gladhorn@kdemail.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "testentry.h"

#include "prefs.h"

#if 0
int TestEntry::m_gradeFrom = 0;
int TestEntry::m_gradeTo = 1;
#endif

TestEntry::TestEntry(KEduVocExpression *entry)
    : m_entry(entry)
    , m_statisticCount(0)
    , m_statisticGoodCount(0)
    , m_statisticBadCount(0)
    , m_answeredCorrectInSequence(0)
    , m_correctAtFirstAttempt(false)
      //, m_practiceFinished(false)
    , m_changeGrades(false)
    , m_isUnseenQuestion(false)
    , m_lastPercentage(0.0)
    , m_lastError(0)
    , m_languageFrom(98)        // Bogus number to make sure that it's set correctly later
    , m_languageTo(99)
{}

#if 0
void TestEntry::setGradeTo(int to)
{
    m_gradeTo = to;
}

void TestEntry::setGradeFrom(int from)
{
    m_gradeFrom = from;
}
#else
void TestEntry::setLanguageFrom(int from)
{
    m_languageFrom = from;
}

void TestEntry::setLanguageTo(int to)
{
    m_languageTo = to;
}
#endif

int TestEntry::answeredCorrectInSequence()
{
    return m_answeredCorrectInSequence;
}

int TestEntry::statisticCount()
{
    return m_statisticCount;
}

int TestEntry::statisticBadCount()
{
    return m_statisticBadCount;
}

int TestEntry::statisticGoodCount()
{
    return m_statisticGoodCount;
}

void TestEntry::updateStatisticsRightAnswer(grade_t currentGrade)
{
    m_statisticCount++;
    m_statisticGoodCount++;
    m_answeredCorrectInSequence++;
    m_isUnseenQuestion = false;

    // Check if this is the first time the user is seeing this question (not practiced).
    if (currentGrade == KV_NORM_GRADE) {
        m_isUnseenQuestion = true;
    }

    if ((!Prefs::altLearn()) || m_answeredCorrectInSequence == 3) {
        m_changeGrades = true;
    }

    // Make changes in statistics if answered correctly and not answered wrong in current test
    if (!Prefs::altLearn() && m_statisticBadCount == 0) {
        m_correctAtFirstAttempt = true;
    } else {
        // alt learn: 3 times right
        if (answeredCorrectInSequence() == 3  && m_statisticBadCount == 0) {
            m_correctAtFirstAttempt = true;
        }
    }

}

bool TestEntry::changeGrades()
{
    return m_changeGrades;
}

void TestEntry::updateStatisticsWrongAnswer(grade_t currentGrade)
{
    m_statisticCount++;
    m_statisticBadCount++;
    m_answeredCorrectInSequence = 0;
    m_changeGrades = true;
    m_isUnseenQuestion = false;

    if (currentGrade == KV_NORM_GRADE) {
	m_isUnseenQuestion = true;
    }
}

#if 0
int TestEntry::gradeFrom()
{
    return m_gradeFrom;
}

int TestEntry::gradeTo()
{
    return m_gradeTo;
}
#else
int TestEntry::languageFrom() const
{
    return m_languageFrom;
}

int TestEntry::languageTo() const
{
    return m_languageTo;
}
#endif

bool TestEntry::isUnseenQuestion() const
{
    return m_isUnseenQuestion;
}

bool TestEntry::correctAtFirstAttempt()
{
    return m_correctAtFirstAttempt;
}

void TestEntry::setLastErrors(TestEntry::ErrorTypes errorTypes)
{
    m_lastError = errorTypes;
}

TestEntry::ErrorTypes TestEntry::lastErrors()
{
    return m_lastError;
}

void TestEntry::setLastPercentage(double percent)
{
    m_lastPercentage = percent;
}

double TestEntry::lastPercentage()
{
    return m_lastPercentage;
}

KEduVocExpression * TestEntry::entry()
{
    return m_entry;
}

QString TestEntry::conjugationTense() const
{
    return m_conjugationTense;
}

void TestEntry::setConjugationTense(const QString& tense)
{
    m_conjugationTense = tense;
}

QList<KEduVocWordFlags> TestEntry::conjugationPronouns() const
{
    return m_conjugationPronouns;
}

void TestEntry::setConjugationPronouns(const QList<KEduVocWordFlags>& flags)
{
    m_conjugationPronouns = flags;
}
