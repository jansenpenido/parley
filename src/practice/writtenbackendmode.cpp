/***************************************************************************
    Copyright 2009 Frederik Gladhorn <gladhorn@kde.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "writtenbackendmode.h"

#include "defaultbackend.h"

#include <KLocalizedString>

using namespace Practice;


WrittenBackendMode::WrittenBackendMode(const Practice::PracticeOptions& practiceOptions, AbstractFrontend* frontend, QObject* parent, TestEntryManager* testEntryManager, KEduVocDocument* doc)
:AbstractBackendMode(practiceOptions, frontend, parent)
,m_testEntryManager(testEntryManager)
,m_doc(doc)
{
    m_validator = new WrittenPracticeValidator(m_practiceOptions.languageTo(),doc);
}

bool WrittenBackendMode::setTestEntry(TestEntry* current)
{
    AbstractBackendMode::setTestEntry(current);
    m_frontend->showQuestion();
    m_lastAnswer.clear();
    m_synonyms.clear();
    m_validator->setEntry(current);
    return true;
}

void WrittenBackendMode::continueAction()
{
    if (m_frontend->resultState() == AbstractFrontend::AnswerCorrect) {
        m_state = SolutionShown; // the user manually toggled the state, so we just count is as correct without showing the solution
        kDebug() << "THE WEIRD THING HAPPENED";
    }
    switch (m_state) {
        case NotAnswered:
        case AnswerWasWrong:
            checkAnswer();
            break;
        case SolutionShown:
            m_currentHint = QString();
            gradeEntryAndContinue();
            break;
    }
}

void WrittenBackendMode::checkAnswer()
{
    QString answer = m_frontend->userInput().toString();
    
    if(answer.isEmpty()) {
        //User gave an empty answer so we want to drop out.
        m_lastAnswer = QString();
        m_state = AnswerWasWrong;
    }

    m_validator->validateAnswer(answer);

    switch(m_state) {
        // right/wrong is only counted on first attempt - when state is NotAnswered.
        case NotAnswered:
            if (m_current->lastErrors().testFlag(TestEntry::Correct)) {
                if (m_current->lastErrors().testFlag(TestEntry::Synonym)){
                    handleSynonym();
                } else {
                    if (m_current->lastErrors().testFlag(TestEntry::CapitalizationMistake)){
                        m_frontend->setFeedback(i18n("Your answer was right on the first attempt, but your capitalization was wrong."));
                    } else if (m_current->lastErrors().testFlag(TestEntry::AccentMistake)){
                        m_frontend->setFeedback(i18n("Your answer was right on the first attempt, but accents were wrong."));
                    } else {
                        m_frontend->setFeedback(i18n("Your answer was right on the first attempt."));
                    }
                    m_frontend->setResultState(AbstractFrontend::AnswerCorrect);
                    m_frontend->setFeedbackState(AbstractFrontend::AnswerCorrect);
                    m_frontend->showSolution();
                
                    m_state = SolutionShown;
                }
            } else {
                handleWrongAnswer();
                m_current->addUserAnswer(answer);
            }
            break;
        case AnswerWasWrong:
            if (m_current->lastErrors().testFlag(TestEntry::Correct)) {
                if (m_current->lastErrors().testFlag(TestEntry::Synonym)){
                    handleSynonym();
                } else {
                    if (m_current->lastErrors().testFlag(TestEntry::CapitalizationMistake)){
                        m_frontend->setFeedback(i18n("Your answer was right... but not on the first try and your capitalization was wrong."));
                    } else if (m_current->lastErrors().testFlag(TestEntry::AccentMistake)){
                        m_frontend->setFeedback(i18n("Your answer was right... but not on the first try and accents were wrong."));
                    } else {
                        m_frontend->setFeedback(i18n("Your answer was right... but not on the first try."));
                    }
                    m_frontend->setResultState(AbstractFrontend::AnswerWrong);
                    m_frontend->setFeedbackState(AbstractFrontend::AnswerCorrect);
                    m_frontend->showSolution();
                    
                    m_state = SolutionShown;
                }
            } else {
                if (answer == m_lastAnswer) {
                    m_frontend->setFeedback(i18n("You did not answer correctly."));
                    m_state = SolutionShown;
                    m_frontend->setResultState(AbstractFrontend::AnswerWrong);
                    m_frontend->setFeedbackState(AbstractFrontend::AnswerWrong);
                    m_frontend->showSolution();
                } else {
                    handleWrongAnswer();
                    m_current->addUserAnswer(answer);
                }
            }
            break;
        case SolutionShown:
            break;
    } 
    m_lastAnswer = answer;
}

void WrittenBackendMode::hintAction()
{
    QString solution = m_current->entry()->translation(m_practiceOptions.languageTo())->text();
    m_currentHint = solution.left(m_currentHint.size() + 1);
    if (m_currentHint.size() == solution.size()) {
        // show solution
        m_frontend->setFeedback(i18n("You revealed the answer by using too many hints."));
        m_frontend->setResultState(AbstractFrontend::AnswerWrong);
        if (m_frontend->userInput().toString() == m_current->entry()->translation(m_practiceOptions.languageTo())->text()) {
            m_frontend->setFeedbackState(AbstractFrontend::AnswerCorrect);
        } else {
            m_frontend->setFeedbackState(AbstractFrontend::AnswerWrong);
        }
        m_frontend->showSolution();
        m_state = SolutionShown;
    } else {
        m_frontend->setHint(i18n("The solution starts with: %1", m_currentHint));
    }
}

void WrittenBackendMode::handleWrongAnswer()
{
    if (m_current->lastErrors().testFlag(TestEntry::Synonym)
        && !Prefs::countSynonymsAsCorrect()){
        m_frontend->setFeedback(i18n("Your answer was wrong as synonyms are not accepted. Please try again."));
    } else if (m_current->lastErrors().testFlag(TestEntry::CapitalizationMistake)
        && !Prefs::ignoreCapitalizationMistakes()){
        m_frontend->setFeedback(i18n("Your answer was wrong as capitalization mistakes are not accepted. Please try again."));
    } else if (m_current->lastErrors().testFlag(TestEntry::AccentMistake)
        && !Prefs::ignoreAccentMistakes()){
        m_frontend->setFeedback(i18n("Your answer was wrong as accent mistakes are not accepted. Please try again."));
    } else {
        m_frontend->setFeedback(i18n("Your answer was wrong. Please try again."));
    }
    m_state = AnswerWasWrong;
}

void WrittenBackendMode::handleSynonym()
{
    const QString answer = m_validator->getCorrectedAnswer();
    if (m_synonyms.contains(answer)) {
        m_frontend->setFeedback(i18n("Your answer was an already entered synonym."));
    } else {
        if (m_current->lastErrors().testFlag(TestEntry::CapitalizationMistake)){
            m_frontend->setFeedback(i18n("Your answer was a synonym and your capitalization was wrong."));
        } else if (m_current->lastErrors().testFlag(TestEntry::AccentMistake)){
            m_frontend->setFeedback(i18n("Your answer was a synonym and accents were wrong."));
        } else {
            m_frontend->setFeedback(i18n("Your answer was a synonym."));
        }
        
        AbstractBackendMode::addSynonym(answer);
        m_frontend->setSynonym(answer);
        m_frontend->showSynonym();
        
        // FIXME that function was nonsense: markSynonymCorrect(answer);
        m_frontend->setResultState(AbstractFrontend::AnswerSynonym);
    }

}
#include "writtenbackendmode.moc"
