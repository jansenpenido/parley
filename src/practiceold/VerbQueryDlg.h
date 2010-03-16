/***************************************************************************

                    query dialog for verbs

    -----------------------------------------------------------------------

    begin          : Fri Dec 3 18:28:18 1999

    copyright      : (C) 1999-2001 Ewald Arnold <kvoctrain@ewald-arnold.de>
                     (C) 2005, 2007 Peter Hedlund <peter.hedlund@kdemail.net>

    -----------------------------------------------------------------------

 ***************************************************************************

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef VerbQueryDlg_included
#define VerbQueryDlg_included

#include "ui_VerbQueryDlgForm.h"
#include "practicedialog.h"

class VerbQueryDlg : public PracticeDialog
{
    Q_OBJECT

public:
    VerbQueryDlg(KEduVocDocument *doc, QWidget *parent);

    ~VerbQueryDlg();

    void setEntry(TestEntry* entry);
    void setProgressCounter(int current, int total);

private slots:
    void showSolution();
    void verifyClicked();

private:
    void setupConjugationLineEditMap();
    void setupPersonalPronouns();
    void clearLineEdits();
    void setupTense(const QString& tense);

    void setStatusText(const QString &) {}
    QProgressBar* timebar() { return mw->timebar; }

    /// the tense that is asked right now
    QString m_currentTense;

    /// the tenses of the current entry that have not been asked
    QStringList m_tenses;

    /// all tenses that are to be practiced
    QStringList m_activeTenses;

    bool m_hasDualConjugations;

    QMap < int, KLineEdit* > m_conjugationWidgets;

    Ui::VerbQueryDlgForm * mw;
};

#endif // VerbQueryDlg_included