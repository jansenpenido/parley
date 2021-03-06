/***************************************************************************
    Copyright 2009-2010 Frederik Gladhorn <gladhorn@kde.org>
    Copyright 2009-2010 Daniel Laidig <laidig@kde.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PRACTICE_WRITTENPRACTICEWIDGET_H
#define PRACTICE_WRITTENPRACTICEWIDGET_H

#include "abstractwidget.h"

namespace Ui
{
class WrittenPracticeWidget;
}

namespace Practice
{
class LatexRenderer;

class WrittenPracticeWidget : public Practice::AbstractModeWidget
{
    Q_OBJECT

public:
    WrittenPracticeWidget(GuiFrontend *frontend, QWidget *parent = 0, bool isExampleSentenceMode = false);
    QVariant userInput() Q_DECL_OVERRIDE;

    void setQuestion(const QVariant& question) Q_DECL_OVERRIDE;
    void setSolution(const QVariant& solution) Q_DECL_OVERRIDE;

    void setQuestionFont(const QFont& font) Q_DECL_OVERRIDE;
    void setSolutionFont(const QFont& font) Q_DECL_OVERRIDE;
    void setHint(const QVariant& hint) Q_DECL_OVERRIDE;
    void setFeedback(const QVariant& feedback) Q_DECL_OVERRIDE;
    void setFeedbackState(AbstractFrontend::ResultState feedbackState) Q_DECL_OVERRIDE;
    void setResultState(AbstractFrontend::ResultState resultState) Q_DECL_OVERRIDE;

    void setQuestionSound(const QUrl& soundUrl) Q_DECL_OVERRIDE;
    void setSolutionSound(const QUrl& soundUrl) Q_DECL_OVERRIDE;
    void setSolutionPronunciation(const QString& pronunciationText) Q_DECL_OVERRIDE;
    void setQuestionPronunciation(const QString& pronunciationText) Q_DECL_OVERRIDE;

    void setSynonym(const QString& entry) Q_DECL_OVERRIDE;

public slots:
    void showQuestion() Q_DECL_OVERRIDE;
    void showSolution() Q_DECL_OVERRIDE;
    void continueClicked();
    void showSynonym() Q_DECL_OVERRIDE;

protected:
    Ui::WrittenPracticeWidget* m_ui;

private:
    QString m_solution;
    QString m_synonym;
    AbstractFrontend::ResultState m_resultState;
    AbstractFrontend::ResultState m_feedbackState;
    QList<QWidget*> synonymWidgets;
    LatexRenderer *m_latexRenderer;
    bool isExampleSentenceMode;
};

}

#endif // PRACTICE_WRITTENPRACTICEWIDGET_H
