/***************************************************************************

                    multiple choice query dialog

    -----------------------------------------------------------------------

    begin         : Thu Nov 25 11:45:53 MET 1999

    copyright     : (C) 1999-2001 Ewald Arnold <kvoctrain@ewald-arnold.de>
                    (C) 2004-2007 Peter Hedlund <peter.hedlund@kdemail.net>
                    (C) 2007 Frederik Gladhorn <frederik.gladhorn@kdemail.net>

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

#include "MCQueryDlg.h"
#include "prefs.h"

#include <QTimer>
#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>
#include <QPushButton>
#include <QKeyEvent>

#include <KLocale>
#include <KDebug>
#include <KRandomSequence>

#include <kvttablemodel.h>

#include <keduvocdocument.h>

#define MAX_MULTIPLE_CHOICE  5  // select one out of x

MCQueryDlg::MCQueryDlg(KEduVocDocument *doc, QWidget *parent) : PracticeDialog(i18n("Multiple Choice"), doc, parent)
{
    mw = new Ui::MCQueryDlgForm();
    mw->setupUi(mainWidget());

    mw->continueButton->setIcon(KIcon("ok"));
    // connecting to SIGNAL nextEntry - emits the signal!
    connect(mw->continueButton, SIGNAL(clicked()), SIGNAL(nextEntry()));

    mw->stopPracticeButton->setIcon( KIcon("list-remove") );
    mw->editEntryButton->setIcon( KIcon("edit") );
    mw->know_it->setIcon(KIcon("go-next"));
    mw->dont_know->setIcon(KIcon("go-next"));

    connect(mw->stopPracticeButton, SIGNAL(clicked()), SLOT(close()));
    connect(mw->editEntryButton, SIGNAL(clicked()), SLOT(editEntry()));

    connect(mw->dont_know, SIGNAL(clicked()), SLOT(skipUnknown()));
    connect(mw->know_it, SIGNAL(clicked()), SLOT(skipKnown()));
    connect(mw->show_all, SIGNAL(clicked()), SLOT(showSolution()));
    connect(mw->rb_trans5, SIGNAL(clicked()), SLOT(verifyClicked()));
    connect(mw->rb_trans4, SIGNAL(clicked()), SLOT(verifyClicked()));
    connect(mw->rb_trans3, SIGNAL(clicked()), SLOT(verifyClicked()));
    connect(mw->rb_trans2, SIGNAL(clicked()), SLOT(verifyClicked()));
    connect(mw->rb_trans1, SIGNAL(clicked()), SLOT(verifyClicked()));

    mw->dont_know->setShortcut(QKeySequence(Qt::Key_Escape));

    mw->countbar->setFormat("%v/%m");
    mw->timebar->setFormat("%v");

    mw->know_it->setVisible(Prefs::iKnow());
    mw->imageGraphicsView->setVisible(false);

    KConfigGroup cg(KGlobal::config(), "MCQueryDlg");
    restoreDialogSize(cg);
}


MCQueryDlg::~MCQueryDlg()
{
    KConfigGroup cg(KGlobal::config(), "MCQueryDlg");
    KDialog::saveDialogSize(cg);
}


void MCQueryDlg::setEntry( TestEntry* entry)
{
    PracticeDialog::setEntry(entry);

    KEduVocExpression *vocExpression = entry->exp;
    mw->timebar->setVisible(Prefs::showCounter());
    mw->timelabel->setVisible(Prefs::showCounter());
    mw->orgField->setFont(Prefs::tableFont());
    mw->orgField->setText(entry->exp->translation(Prefs::fromIdentifier()).text());
    mw->show_all->setDefault(true);

    showContinueButton(false);

    // Query cycle - how often did this show up (?)
    mw->progCount->setText(QString::number(entry->statisticCount()));

    KRandomSequence randomSequence;
    QStringList choices;
    button_ref.clear();
    button_ref.append(qMakePair(mw->rb_trans1, mw->trans1));
    button_ref.append(qMakePair(mw->rb_trans2, mw->trans2));
    button_ref.append(qMakePair(mw->rb_trans3, mw->trans3));
    button_ref.append(qMakePair(mw->rb_trans4, mw->trans4));
    button_ref.append(qMakePair(mw->rb_trans5, mw->trans5));
    randomSequence.randomize(button_ref);
    setWidgetStyle(button_ref[0].first);
    setWidgetStyle(button_ref[1].first);
    setWidgetStyle(button_ref[2].first);
    setWidgetStyle(button_ref[3].first);
    setWidgetStyle(button_ref[4].first);
    setWidgetStyle(button_ref[0].second);
    setWidgetStyle(button_ref[1].second);
    setWidgetStyle(button_ref[2].second);
    setWidgetStyle(button_ref[3].second);
    setWidgetStyle(button_ref[4].second);

    KEduVocMultipleChoice multipleChoice = vocExpression->translation(Prefs::toIdentifier()).multipleChoice();
    for (int i = 0; i < qMin(MAX_MULTIPLE_CHOICE, (int) multipleChoice.size()); ++i) {
        choices.append(multipleChoice.choice(i));
        kDebug() << "Append choice: " << multipleChoice.choice(i);
    }

    if (choices.count() > 1)
        randomSequence.randomize(choices);

    // always include false friend
    QString ff = vocExpression->translation(Prefs::toIdentifier())
        .falseFriend(Prefs::fromIdentifier()).simplified();

    if (!ff.isEmpty()) {
        choices.prepend(ff);
    }

    if (m_doc->entryCount() <= MAX_MULTIPLE_CHOICE) {
        for (int i = choices.count(); i < m_doc->entryCount(); ++i) {
            KEduVocExpression *act = m_doc->entry(i);

            if (act != vocExpression) {
                choices.append(act->translation(Prefs::toIdentifier()).text());
            }
        }
    } else {
        QList<KEduVocExpression*> exprlist;

        int count = MAX_MULTIPLE_CHOICE;
        int numNonEmptyEntries = 0;

        // find out if we got enough non-empty entries to fill all the options
        for(int i = 0; i < m_doc->entryCount(); i++) {
            if(!m_doc->entry(i)->translation(Prefs::toIdentifier()).text().isEmpty())
                numNonEmptyEntries++;
            if(numNonEmptyEntries >= MAX_MULTIPLE_CHOICE)
                break;
        }

        // gather random expressions for the choice
        while (count > 0) {
            int nr;
            // if there are enough non-empty fields, fill the options only with those
            if(numNonEmptyEntries >= MAX_MULTIPLE_CHOICE) {
                do {
                    nr = randomSequence.getLong(m_doc->entryCount());
                } while (m_doc->entry(nr)->translation(Prefs::toIdentifier()).text().isEmpty());
            } else {
                nr = randomSequence.getLong(m_doc->entryCount());
            }
            // append if new expr found
            bool newex = true;
            for (int i = 0; newex && i < exprlist.count(); i++) {
                if (exprlist[i] == m_doc->entry(nr))
                    newex = false;
            }
            if (newex && vocExpression != m_doc->entry(nr)) {
                count--;
                exprlist.append(m_doc->entry(nr));
            }
        }

        for (int i = 0; i < exprlist.count(); i++) {
            choices.append(exprlist[i]->translation(Prefs::toIdentifier()).text());
        }

    }

    choices.prepend(vocExpression->translation(Prefs::toIdentifier()).text());

    for (int i = choices.count(); i < MAX_MULTIPLE_CHOICE; i++)
        choices.append("");

    if (choices.count() > MAX_MULTIPLE_CHOICE)
        choices.erase(choices.begin()+MAX_MULTIPLE_CHOICE, choices.end());

    button_ref[0].first->setEnabled(!choices[0].isEmpty());
    button_ref[1].first->setEnabled(!choices[1].isEmpty());
    button_ref[2].first->setEnabled(!choices[2].isEmpty());
    button_ref[3].first->setEnabled(!choices[3].isEmpty());
    button_ref[4].first->setEnabled(!choices[4].isEmpty());

    button_ref[0].second->setEnabled(!choices[0].isEmpty());
    button_ref[1].second->setEnabled(!choices[1].isEmpty());
    button_ref[2].second->setEnabled(!choices[2].isEmpty());
    button_ref[3].second->setEnabled(!choices[3].isEmpty());
    button_ref[4].second->setEnabled(!choices[4].isEmpty());

    button_ref[0].second->setText(choices[0]);
    button_ref[0].second->setFont(Prefs::tableFont());
    button_ref[1].second->setText(choices[1]);
    button_ref[1].second->setFont(Prefs::tableFont());
    button_ref[2].second->setText(choices[2]);
    button_ref[2].second->setFont(Prefs::tableFont());
    button_ref[3].second->setText(choices[3]);
    button_ref[3].second->setFont(Prefs::tableFont());
    button_ref[4].second->setText(choices[4]);
    button_ref[4].second->setFont(Prefs::tableFont());

    // As long as the buttons are AutoExclusive we cannot uncheck all.
    mw->rb_trans5->setChecked(true);
    mw->rb_trans5->setAutoExclusive ( false );
    mw->rb_trans5->setChecked(false);
    mw->rb_trans5->setAutoExclusive ( true );

    mw->show_all->setFocus();

    imageShowFromEntry( mw->imageGraphicsView, entry );
}


void MCQueryDlg::showSolution()
{
    setWidgetStyle(button_ref[0].first, Default);
    setWidgetStyle(button_ref[1].first, Default);
    setWidgetStyle(button_ref[2].first, Default);
    setWidgetStyle(button_ref[3].first, Default);
    setWidgetStyle(button_ref[4].first, Default);
    setWidgetStyle(button_ref[0].second, Default);
    setWidgetStyle(button_ref[1].second, Default);
    setWidgetStyle(button_ref[2].second, Default);
    setWidgetStyle(button_ref[3].second, Default);
    setWidgetStyle(button_ref[4].second, Default);

    button_ref[0].first->setFocus();
    button_ref[0].first->setChecked(true);
    verifyButton(button_ref[0].first, true, button_ref[0].second);

    showContinueButton(true);

    setAnswerTainted();
}


void MCQueryDlg::verifyClicked()
{
    bool known = button_ref[0].first->isChecked();

    if (button_ref[0].first->isChecked()) {
        verifyButton(button_ref[0].first, known, button_ref[0].second);
    } else if (button_ref[1].first->isChecked()) {
        verifyButton(button_ref[1].first, known, button_ref[1].second);
    } else if (button_ref[2].first->isChecked()) {
        verifyButton(button_ref[2].first, known, button_ref[2].second);
    } else if (button_ref[3].first->isChecked()) {
        verifyButton(button_ref[3].first, known, button_ref[3].second);
    } else if (button_ref[4].first->isChecked()) {
        verifyButton(button_ref[4].first, known, button_ref[4].second);
    }

    ///@todo move the status bar stuff either in or out of the base class

    if (known) {
        resultCorrect();
        showContinueButton(true);
        mw->status->setText(
                getOKComment((int)(((double)mw->countbar->value())
                    /mw->countbar->maximum() * 100.0)));
    } else {
        setAnswerTainted();
        mw->dont_know->setDefault(true);
        mw->status->setText(
                getNOKComment((int)(((double)mw->countbar->value())
                    /mw->countbar->maximum() * 100.0)));
    }
}


void MCQueryDlg::setProgressCounter(int current, int total)
{
    mw->countbar->setMaximum(total);
    mw->countbar->setValue(current);
}

void MCQueryDlg::showContinueButton(bool show)
{
    mw->know_it->setVisible(!show);
    mw->dont_know->setVisible(!show);
    mw->show_all->setVisible(!show);

    mw->continueButton->setVisible(show);

    if ( show ) {
        mw->continueButton->setDefault(true);
    } else {
        mw->dont_know->setDefault(true);
    }
}


/**
 * Used to paint a radio button in result color.
 * @todo rename, rewrite...
 * This doesn't even verify - it needs to be told if the result is correct.
 * @param radio
 * @param is_ok
 * @param widget2
 */
void MCQueryDlg::verifyButton(QRadioButton *radio, bool is_ok, QWidget *widget2)
{
    if (!radio->isEnabled())
        return;

    if (is_ok) {
        setWidgetStyle(radio, PositiveResult);
        if (widget2 != 0) {
            setWidgetStyle(widget2, PositiveResult);
        }
    } else {
        setWidgetStyle(radio, NegativeResult);
        if (widget2 != 0) {
            setWidgetStyle(widget2, NegativeResult);
        }
    }
}


#include "MCQueryDlg.moc"

