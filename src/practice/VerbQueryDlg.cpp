/***************************************************************************

                    query dialog for verbs

    -----------------------------------------------------------------------

    begin         : Fri Dec 3 18:28:18 1999

    copyright     : (C) 1999-2001 Ewald Arnold <kvoctrain@ewald-arnold.de>
                    (C) 2004-2007 Peter Hedlund <peter.hedlund@kdemail.net>

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

#include "VerbQueryDlg.h"

#include <QTimer>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>

#include <KLocale>

#include <kvttablemodel.h>

VerbQueryDlg::VerbQueryDlg(KEduVocDocument *doc, QWidget *parent) : PracticeDialog(i18n("Verb Training"), doc, parent)
{
    mw = new Ui::VerbQueryDlgForm();
    mw->setupUi(mainWidget());

    connect(mw->dont_know, SIGNAL(clicked()), SLOT(dontKnowClicked()));
    connect(mw->know_it, SIGNAL(clicked()), SLOT(knowItClicked()));
    connect(mw->verify, SIGNAL(clicked()), SLOT(verifyClicked()));
    connect(mw->show_all, SIGNAL(clicked()), SLOT(showSolution()));

    connect(mw->p3pmField, SIGNAL(textChanged(const QString&)), SLOT(slotP3pmChanged(const QString&)));
    connect(mw->p3snField, SIGNAL(textChanged(const QString&)), SLOT(slotP3snChanged(const QString&)));
    connect(mw->p3pnField, SIGNAL(textChanged(const QString&)), SLOT(slotP3pnChanged(const QString&)));
    connect(mw->p3smField, SIGNAL(textChanged(const QString&)), SLOT(slotP3smChanged(const QString&)));
    connect(mw->p3pfField, SIGNAL(textChanged(const QString&)), SLOT(slotP3pfChanged(const QString&)));
    connect(mw->p3sfField, SIGNAL(textChanged(const QString&)), SLOT(slotP3sfChanged(const QString&)));
    connect(mw->p2pField, SIGNAL(textChanged(const QString&)), SLOT(slotP2pChanged(const QString&)));
    connect(mw->p2sField, SIGNAL(textChanged(const QString&)), SLOT(slotP2sChanged(const QString&)));
    connect(mw->p1pField, SIGNAL(textChanged(const QString&)), SLOT(slotP1pChanged(const QString&)));
    connect(mw->p1sField, SIGNAL(textChanged(const QString&)), SLOT(slotP1sChanged(const QString&)));

    connect(this, SIGNAL(user1Clicked()), this, SLOT(editEntry()));

    mw->dont_know->setShortcut(QKeySequence(Qt::Key_Escape));

    mw->countbar->setFormat("%v/%m");
    mw->timebar->setFormat("%v");

    KConfigGroup cg(KGlobal::config(), "VerbQueryDialog");
    restoreDialogSize(cg);
}


VerbQueryDlg::~VerbQueryDlg()
{
    KConfigGroup cg(KGlobal::config(), "VerbQueryDialog");
    KDialog::saveDialogSize(cg);
}


void VerbQueryDlg::initFocus()
{
    mw->p1sField->setFocus();
}


void VerbQueryDlg::setEntry(TestEntry* entry)
{
    PracticeDialog::setEntry(entry);

    mw->timebar->setEnabled(Prefs::showCounter());
    mw->timelabel->setEnabled(Prefs::showCounter());
    mw->show_all->setDefault(true);
//     QString s;
//     s.setNum(q_cycle);
//     mw->progCount->setText(s);


///@todo conjugations enable setting personal pronouns
    mw->p1sLabel->setText(m_doc->identifier(Prefs::toIdentifier()).personalPronouns().pers1Singular());
    mw->p2sLabel->setText(m_doc->identifier(Prefs::toIdentifier()).personalPronouns().pers2Singular());
    mw->p3smLabel->setText(m_doc->identifier(Prefs::toIdentifier()).personalPronouns().pers3MaleSingular());
    mw->p3sfLabel->setText(m_doc->identifier(Prefs::toIdentifier()).personalPronouns().pers3FemaleSingular());
    mw->p3snLabel->setText(m_doc->identifier(Prefs::toIdentifier()).personalPronouns().pers3NaturalSingular());

    mw->p1pLabel->setText(m_doc->identifier(Prefs::toIdentifier()).personalPronouns().pers1Plural());
    mw->p2pLabel->setText(m_doc->identifier(Prefs::toIdentifier()).personalPronouns().pers2Plural());
    mw->p3pmLabel->setText(m_doc->identifier(Prefs::toIdentifier()).personalPronouns().pers3MalePlural());
    mw->p3pfLabel->setText(m_doc->identifier(Prefs::toIdentifier()).personalPronouns().pers3FemalePlural());
    mw->p3pnLabel->setText(m_doc->identifier(Prefs::toIdentifier()).personalPronouns().pers3NaturalPlural());

    all_known = true;
    current = -1;
    next();
}


bool VerbQueryDlg::next()
{
    resetAllFields();
    QString tense;

    QString s;
    s = m_entry->exp->translation(Prefs::toIdentifier()).text();
    mw->baseLabel->setText(s);

    if (current < m_entry->exp->translation(Prefs::toIdentifier()).conjugations().count() - 1) {
        current++;
    }

    tense = m_entry->exp->translation(Prefs::toIdentifier()).conjugations().keys().value(current);
    QString msg = i18n("Current tense is: %1.", tense);

    mw->instructionLabel->setText(msg);


    mw->p1sField->setText("");
    mw->p1sField->setEnabled(!m_entry->exp->translation(Prefs::toIdentifier()).conjugations()[tense].pers1Singular().isEmpty());

    mw->p2sField->setText("");
    mw->p2sField->setEnabled(!m_entry->exp->translation(Prefs::toIdentifier()).conjugations()[tense].pers2Singular().isEmpty());

    mw->p3smField->setText("");
    mw->p3smField->setEnabled(!m_entry->exp->translation(Prefs::toIdentifier()).conjugations()[tense].pers3MaleSingular().isEmpty());

    mw->p3sfField->setText("");
    mw->p3sfField->setEnabled(!m_entry->exp->translation(Prefs::toIdentifier()).conjugations()[tense].pers3FemaleSingular().isEmpty());

    mw->p3snField->setText("");
    mw->p3snField->setEnabled(!m_entry->exp->translation(Prefs::toIdentifier()).conjugations()[tense].pers3NaturalSingular().isEmpty());

    mw->p1pField->setText("");
    mw->p1pField->setEnabled(!m_entry->exp->translation(Prefs::toIdentifier()).conjugations()[tense].pers1Plural().isEmpty());

    mw->p2pField->setText("");
    mw->p2pField->setEnabled(!m_entry->exp->translation(Prefs::toIdentifier()).conjugations()[tense].pers2Plural().isEmpty());

    mw->p3pmField->setText("");
    mw->p3pmField->setEnabled(!m_entry->exp->translation(Prefs::toIdentifier()).conjugations()[tense].pers3MalePlural().isEmpty());

    mw->p3pfField->setText("");
    mw->p3pfField->setEnabled(!m_entry->exp->translation(Prefs::toIdentifier()).conjugations()[tense].pers3FemalePlural().isEmpty());

    mw->p3pnField->setText("");
    mw->p3pnField->setEnabled(!m_entry->exp->translation(Prefs::toIdentifier()).conjugations()[tense].pers3NaturalPlural().isEmpty());

    bool common = m_entry->exp->translation(Prefs::toIdentifier()).conjugations()[tense].pers3SingularCommon();
    if (common) {
        mw->p3smField->setEnabled(false);
        mw->p3snField->setEnabled(false);
    }

    common = m_entry->exp->translation(Prefs::toIdentifier()).conjugations()[tense].pers3PluralCommon();
    if (common) {
        mw->p3pmField->setEnabled(false);
        mw->p3pnField->setEnabled(false);
    }

    return false;
}


void VerbQueryDlg::showSolution()
{
    resetAllFields();
    mw->dont_know->setDefault(true);

    QString tense = m_entry->exp->translation(Prefs::toIdentifier()).conjugations().keys().value(current);

    mw->p1sField->setText(m_entry->exp->translation(Prefs::toIdentifier()).conjugations()[tense].pers1Singular());
    mw->p2sField->setText(m_entry->exp->translation(Prefs::toIdentifier()).conjugations()[tense].pers2Singular());
    mw->p3smField->setText(m_entry->exp->translation(Prefs::toIdentifier()).conjugations()[tense].pers3MaleSingular());
    mw->p3sfField->setText(m_entry->exp->translation(Prefs::toIdentifier()).conjugations()[tense].pers3FemaleSingular());
    mw->p3snField->setText(m_entry->exp->translation(Prefs::toIdentifier()).conjugations()[tense].pers3NaturalSingular());

    mw->p1pField->setText(m_entry->exp->translation(Prefs::toIdentifier()).conjugations()[tense].pers1Plural());
    mw->p2pField->setText(m_entry->exp->translation(Prefs::toIdentifier()).conjugations()[tense].pers2Plural());
    mw->p3pmField->setText(m_entry->exp->translation(Prefs::toIdentifier()).conjugations()[tense].pers3MalePlural());
    mw->p3pfField->setText(m_entry->exp->translation(Prefs::toIdentifier()).conjugations()[tense].pers3FemalePlural());
    mw->p3pnField->setText(m_entry->exp->translation(Prefs::toIdentifier()).conjugations()[tense].pers3NaturalPlural());

    verifyField(mw->p1sField, m_entry->exp->translation(Prefs::toIdentifier()).conjugations()[tense].pers1Singular());
    verifyField(mw->p2sField, m_entry->exp->translation(Prefs::toIdentifier()).conjugations()[tense].pers2Singular());
    verifyField(mw->p3sfField, m_entry->exp->translation(Prefs::toIdentifier()).conjugations()[tense].pers3FemaleSingular());

    bool common = m_entry->exp->translation(Prefs::toIdentifier()).conjugations()[tense].pers3SingularCommon();
    if (!common) {
        verifyField(mw->p3smField, m_entry->exp->translation(Prefs::toIdentifier()).conjugations()[tense].pers3MaleSingular());
        verifyField(mw->p3snField, m_entry->exp->translation(Prefs::toIdentifier()).conjugations()[tense].pers3NaturalSingular());
    }

    verifyField(mw->p1pField, m_entry->exp->translation(Prefs::toIdentifier()).conjugations()[tense].pers1Plural());
    verifyField(mw->p2pField, m_entry->exp->translation(Prefs::toIdentifier()).conjugations()[tense].pers2Plural());
    verifyField(mw->p3pfField, m_entry->exp->translation(Prefs::toIdentifier()).conjugations()[tense].pers3FemalePlural());

    common = m_entry->exp->translation(Prefs::toIdentifier()).conjugations()[tense].pers3PluralCommon();
    if (!common) {
        verifyField(mw->p3pmField, m_entry->exp->translation(Prefs::toIdentifier()).conjugations()[tense].pers3MalePlural());
        verifyField(mw->p3pnField, m_entry->exp->translation(Prefs::toIdentifier()).conjugations()[tense].pers3NaturalPlural());
    }

}


void VerbQueryDlg::verifyClicked()
{
    QString tense = m_entry->exp->translation(Prefs::toIdentifier()).conjugations().keys().value(current);

    bool known = true;

    if (!verifyField(mw->p1sField, m_entry->exp->translation(Prefs::toIdentifier()).conjugations()[tense].pers1Singular()))
        known = false;

    if (!verifyField(mw->p2sField, m_entry->exp->translation(Prefs::toIdentifier()).conjugations()[tense].pers2Singular()))
        known = false;

    if (!verifyField(mw->p3sfField, m_entry->exp->translation(Prefs::toIdentifier()).conjugations()[tense].pers3FemaleSingular()))
        known = false;

    bool common = m_entry->exp->translation(Prefs::toIdentifier()).conjugations()[tense].pers3SingularCommon();
    if (!common) {
        if (!verifyField(mw->p3smField, m_entry->exp->translation(Prefs::toIdentifier()).conjugations()[tense].pers3MaleSingular()))
            known = false;

        if (!verifyField(mw->p3snField, m_entry->exp->translation(Prefs::toIdentifier()).conjugations()[tense].pers3NaturalSingular()))
            known = false;
    }

    if (!verifyField(mw->p1pField, m_entry->exp->translation(Prefs::toIdentifier()).conjugations()[tense].pers1Plural()))
        known = false;

    if (!verifyField(mw->p2pField, m_entry->exp->translation(Prefs::toIdentifier()).conjugations()[tense].pers2Plural()))
        known = false;

    if (!verifyField(mw->p3pfField, m_entry->exp->translation(Prefs::toIdentifier()).conjugations()[tense].pers3FemalePlural()))
        known = false;

    common = m_entry->exp->translation(Prefs::toIdentifier()).conjugations()[tense].pers3PluralCommon();
    if (!common) {
        if (!verifyField(mw->p3pmField, m_entry->exp->translation(Prefs::toIdentifier()).conjugations()[tense].pers3MalePlural()))
            known = false;

        if (!verifyField(mw->p3pnField, m_entry->exp->translation(Prefs::toIdentifier()).conjugations()[tense].pers3NaturalPlural()))
            known = false;
    }

    if (known)
        knowItClicked();
    else {
        all_known = false;
        mw->dont_know->setDefault(true);
    }
}


void VerbQueryDlg::resetAllFields()
{
    resetQueryWidget(mw->p1sField);
    resetQueryWidget(mw->p2sField);
    resetQueryWidget(mw->p3sfField);
    resetQueryWidget(mw->p3smField);
    resetQueryWidget(mw->p3snField);

    resetQueryWidget(mw->p1pField);
    resetQueryWidget(mw->p2pField);
    resetQueryWidget(mw->p3pfField);
    resetQueryWidget(mw->p3pmField);
    resetQueryWidget(mw->p3pnField);
}





void VerbQueryDlg::knowItClicked()
{
    resetAllFields();
    if (current >= m_entry->exp->translation(Prefs::toIdentifier()).conjugations().count() - 1) {
        if (all_known)
            emit sigQueryChoice(SkipKnown);
        else
            emit sigQueryChoice(SkipUnknown);
    } else
        next();
}


void VerbQueryDlg::dontKnowClicked()
{
    all_known = false;
    if (current >= m_entry->exp->translation(Prefs::toIdentifier()).conjugations().count() - 1)
        emit sigQueryChoice(SkipUnknown);
    else {
        m_timer->start(1000);
        m_timerCount = Prefs::maxTimePer();
        next();
    }
}


void VerbQueryDlg::slotP3pfChanged(const QString&)
{
    mw->verify->setDefault(true);
    resetQueryWidget(mw->p3pfField);
}


void VerbQueryDlg::slotP3snChanged(const QString&)
{
    mw->verify->setDefault(true);
    resetQueryWidget(mw->p3snField);
}


void VerbQueryDlg::slotP3smChanged(const QString&)
{
    mw->verify->setDefault(true);
    resetQueryWidget(mw->p3smField);
}


void VerbQueryDlg::slotP3pnChanged(const QString&)
{
    mw->verify->setDefault(true);
    resetQueryWidget(mw->p3pnField);
}


void VerbQueryDlg::slotP3sfChanged(const QString&)
{
    mw->verify->setDefault(true);
    resetQueryWidget(mw->p3sfField);
}


void VerbQueryDlg::slotP1sChanged(const QString&)
{
    mw->verify->setDefault(true);
    resetQueryWidget(mw->p1sField);
}


void VerbQueryDlg::slotP2sChanged(const QString&)
{
    mw->verify->setDefault(true);
    resetQueryWidget(mw->p2sField);
}


void VerbQueryDlg::slotP3pmChanged(const QString&)
{
    mw->verify->setDefault(true);
    resetQueryWidget(mw->p3pmField);
}


void VerbQueryDlg::slotP1pChanged(const QString&)
{
    mw->verify->setDefault(true);
    resetQueryWidget(mw->p1pField);
}


void VerbQueryDlg::slotP2pChanged(const QString&)
{
    mw->verify->setDefault(true);
    resetQueryWidget(mw->p2pField);
}

void VerbQueryDlg::setProgressCounter(int current, int total)
{
    mw->countbar->setMaximum(total);
    mw->countbar->setValue(current);
}

#include "VerbQueryDlg.moc"
