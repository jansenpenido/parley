/***************************************************************************
    Copyright 2008 Frederik Gladhorn <frederik.gladhorn@kdemail.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "conjugationwidget.h"

#include <keduvocdocument.h>
#include <keduvocexpression.h>
#include <keduvocwordtype.h>
#include <KMessageBox>

using namespace Editor;

ConjugationWidget::ConjugationWidget(QWidget *parent) : QWidget(parent)
{
    m_doc = 0;
    m_entry = 0;
    m_identifier = -1;

    setupUi(this);

    connect(makeVerbButton, SIGNAL(clicked()), SLOT(slotMakeVerb()));
    connect(nextTenseButton, SIGNAL(clicked()), SLOT(slotNextTense()));
    connect(tenseComboBox, SIGNAL(activated(int)), SLOT(slotTenseSelected(int)));
    connect(tenseComboBox->lineEdit(), SIGNAL(editingFinished()), SLOT(tenseEditingFinished()));

    showMakeVerbWidgets();
    makeVerbButton->setEnabled(false);

    m_conjugationLineEdits[KEduVocWordFlag::First | KEduVocWordFlag::Singular]
        = singularFirstPersonLineEdit;
    m_conjugationLineEdits[KEduVocWordFlag::Second | KEduVocWordFlag::Singular]
        = singularSecondPersonLineEdit;

    m_conjugationLineEdits[KEduVocWordFlag::Third | KEduVocWordFlag::Masculine | KEduVocWordFlag::Singular]
        = singularThirdMalePersonLineEdit;
    m_conjugationLineEdits[KEduVocWordFlag::Third | KEduVocWordFlag::Feminine | KEduVocWordFlag::Singular]
        = singularThirdFemalePersonLineEdit;
    m_conjugationLineEdits[KEduVocWordFlag::Third | KEduVocWordFlag::Neuter | KEduVocWordFlag::Singular]
        = singularThirdNeutralPersonLineEdit;

    m_conjugationLineEdits[KEduVocWordFlag::First | KEduVocWordFlag::Dual]
        = dualFirstPersonLineEdit;
    m_conjugationLineEdits[KEduVocWordFlag::Second | KEduVocWordFlag::Dual]
        = dualSecondPersonLineEdit;

    m_conjugationLineEdits[KEduVocWordFlag::Third | KEduVocWordFlag::Masculine | KEduVocWordFlag::Dual]
        = dualThirdMalePersonLineEdit;
    m_conjugationLineEdits[KEduVocWordFlag::Third | KEduVocWordFlag::Feminine | KEduVocWordFlag::Dual]
        = dualThirdFemalePersonLineEdit;
    m_conjugationLineEdits[KEduVocWordFlag::Third | KEduVocWordFlag::Neuter | KEduVocWordFlag::Dual]
        = dualThirdNeutralPersonLineEdit;

    m_conjugationLineEdits[KEduVocWordFlag::First | KEduVocWordFlag::Plural]
        = pluralFirstPersonLineEdit;
    m_conjugationLineEdits[KEduVocWordFlag::Second | KEduVocWordFlag::Plural]
        = pluralSecondPersonLineEdit;

    m_conjugationLineEdits[KEduVocWordFlag::Third | KEduVocWordFlag::Masculine | KEduVocWordFlag::Plural]
        = pluralThirdMalePersonLineEdit;
    m_conjugationLineEdits[KEduVocWordFlag::Third | KEduVocWordFlag::Feminine | KEduVocWordFlag::Plural]
        = pluralThirdFemalePersonLineEdit;
    m_conjugationLineEdits[KEduVocWordFlag::Third | KEduVocWordFlag::Neuter | KEduVocWordFlag::Plural]
        = pluralThirdNeutralPersonLineEdit;

    foreach(const KEduVocWordFlags &index, m_conjugationLineEdits.keys()) {
        connect(m_conjugationLineEdits.value(index), SIGNAL(textChanged(const QString&)), SLOT(textChanged(const QString&)));
    }
}


void ConjugationWidget::textChanged(const QString& text)
{
    int valueIndex = m_conjugationLineEdits.values().indexOf(qobject_cast<KLineEdit*>(sender()));
    int key = m_conjugationLineEdits.keys().value(valueIndex);
    m_entry->translation(m_identifier)->conjugation(tenseComboBox->currentText()).setConjugation(text, (KEduVocWordFlag::Flags)key);
}


void ConjugationWidget::slotTenseSelected(int sel)
{
    Q_UNUSED(sel);
    updateEntries();
}


void ConjugationWidget::updateEntries()
{
    foreach(int key, m_conjugationLineEdits.keys()) {
        m_conjugationLineEdits.value((KEduVocWordFlag::Flags)key)->setText(m_entry->translation(m_identifier)->conjugation(tenseComboBox->currentText()).conjugation((KEduVocWordFlag::Flags)key).text());
    }
}

void ConjugationWidget::setTranslation(KEduVocExpression * entry, int identifier)
{
    tenseComboBox->clear();
    tenseComboBox->completionObject()->clear();

    m_entry = entry;
    m_identifier = identifier;

    if (!m_doc || !entry) {
        setEnabled(false);
        showMakeVerbWidgets();
        makeVerbButton->setEnabled(false);
        return;
    }

    // init tenses per language
    tenseComboBox->addItems(m_doc->identifier(identifier).tenseList());
    tenseComboBox->completionObject()->insertItems(m_doc->identifier(identifier).tenseList());
    tenseComboBox->setCurrentIndex(0);

    setEnabled(true);
    if (entry->translation(m_identifier)->wordType()
            && entry->translation(m_identifier)->wordType()->wordType() & KEduVocWordFlag::Verb) {
        // if it's a verb already, hide the make verb button and start editing it
        showConjugationEditWidgets();
        updateEntries();
    } else {
        // hide widgets
        makeVerbButton->setEnabled(true);
        showMakeVerbWidgets();
        makeVerbButton->setText(i18n("\"%1\" is a verb", m_entry->translation(m_identifier)->text()));
    }
}

void ConjugationWidget::setDocument(KEduVocDocument * doc)
{
    m_doc = doc;
    tenseComboBox->clear();
    tenseComboBox->completionObject()->clear();
}

void ConjugationWidget::slotNextTense()
{
    if (tenseComboBox->currentIndex() + 1 < tenseComboBox->count()) {
        tenseComboBox->setCurrentIndex(tenseComboBox->currentIndex() + 1);
    } else {
        tenseComboBox->setCurrentIndex(0);
    }
    updateEntries();
}

void ConjugationWidget::slotMakeVerb()
{
    if(!m_doc) {
        return;
    }

    // find an adverb container
    KEduVocWordType* container = m_doc->wordTypeContainer()->childOfType(KEduVocWordFlag::Verb);
    if (container) {
        m_entry->translation(m_identifier)->setWordType(container);
        showConjugationEditWidgets();
    } else {
        ///@todo better message
        KMessageBox::information(this, i18n("Could not determine word type of verbs"));
    }
    setTranslation(m_entry, m_identifier);
}

void ConjugationWidget::showMakeVerbWidgets()
{
    makeVerbButton->setVisible(true);
    showWidgets(false, false, false, false, false, false, false);
}

void ConjugationWidget::showConjugationEditWidgets()
{
    makeVerbButton->setVisible(false);
    updateVisiblePersons();
}

void ConjugationWidget::updateVisiblePersons()
{
    if (m_identifier < 0) {
        showWidgets(false, false, false, false, false, false, false);
        return;
    }

    bool dualVisible = m_doc->identifier(m_identifier).personalPronouns().dualExists();
    bool maleFemaleDifferent = m_doc->identifier(m_identifier).personalPronouns().maleFemaleDifferent();
    bool neutralExists = m_doc->identifier(m_identifier).personalPronouns().neutralExists();

    showWidgets(true, true, dualVisible, true, maleFemaleDifferent, maleFemaleDifferent, neutralExists || (!maleFemaleDifferent));
    
    // set up the personal pronouns
    KEduVocPersonalPronoun pron = m_doc->identifier(m_identifier).personalPronouns();

    singularFirstPersonLabel->setText(pron.personalPronoun( KEduVocWordFlag::First | KEduVocWordFlag::Singular ));
    singularSecondPersonLabel->setText(pron.personalPronoun( KEduVocWordFlag::Second | KEduVocWordFlag::Singular ));
    singularThirdMalePersonLabel->setText(pron.personalPronoun( KEduVocWordFlag::Third | KEduVocWordFlag::Masculine| KEduVocWordFlag::Singular ));
    singularThirdFemalePersonLabel->setText(pron.personalPronoun( KEduVocWordFlag::Third | KEduVocWordFlag::Feminine| KEduVocWordFlag::Singular ));
    singularThirdNeutralPersonLabel->setText(pron.personalPronoun( KEduVocWordFlag::Third | KEduVocWordFlag::Neuter| KEduVocWordFlag::Singular ));

    dualFirstPersonLabel->setText(pron.personalPronoun( KEduVocWordFlag::First| KEduVocWordFlag::Dual ));
    dualSecondPersonLabel->setText(pron.personalPronoun( KEduVocWordFlag::Second| KEduVocWordFlag::Dual ));
    dualThirdMalePersonLabel->setText(pron.personalPronoun( KEduVocWordFlag::Third | KEduVocWordFlag::Masculine| KEduVocWordFlag::Dual ));
    dualThirdFemalePersonLabel->setText(pron.personalPronoun( KEduVocWordFlag::Third | KEduVocWordFlag::Feminine| KEduVocWordFlag::Dual ));
    dualThirdNeutralPersonLabel->setText(pron.personalPronoun( KEduVocWordFlag::Third | KEduVocWordFlag::Neuter| KEduVocWordFlag::Dual ));

    pluralFirstPersonLabel->setText(pron.personalPronoun( KEduVocWordFlag::First| KEduVocWordFlag::Plural ));
    pluralSecondPersonLabel->setText(pron.personalPronoun( KEduVocWordFlag::Second| KEduVocWordFlag::Plural ));
    pluralThirdMalePersonLabel->setText(pron.personalPronoun( KEduVocWordFlag::Third | KEduVocWordFlag::Masculine| KEduVocWordFlag::Plural ));
    pluralThirdFemalePersonLabel->setText(pron.personalPronoun( KEduVocWordFlag::Third | KEduVocWordFlag::Feminine| KEduVocWordFlag::Plural ));
    pluralThirdNeutralPersonLabel->setText(pron.personalPronoun( KEduVocWordFlag::Third | KEduVocWordFlag::Neuter| KEduVocWordFlag::Plural ));
}

void ConjugationWidget::showWidgets(bool tenses, bool singular, bool dual, bool plural, bool maleVisible, bool femaleVisible, bool neuterVisible)
{
    tenselabel->setVisible(tenses);
    tenseComboBox->setVisible(tenses);
    nextTenseButton->setVisible(tenses);

    singularLabel->setVisible(singular);
    singularFirstPersonLabel->setVisible(singular);
    singularFirstPersonLineEdit->setVisible(singular);
    singularSecondPersonLabel->setVisible(singular);
    singularSecondPersonLineEdit->setVisible(singular);

    singularThirdMalePersonLabel->setVisible(singular && maleVisible);
    singularThirdMalePersonLineEdit->setVisible(singular && maleVisible);
    singularThirdFemalePersonLabel->setVisible(singular && femaleVisible);
    singularThirdFemalePersonLineEdit->setVisible(singular && femaleVisible);
    singularThirdNeutralPersonLabel->setVisible(singular && neuterVisible);
    singularThirdNeutralPersonLineEdit->setVisible(singular && neuterVisible);

    dualLabel->setVisible(dual);
    dualFirstPersonLabel->setVisible(dual);
    dualFirstPersonLineEdit->setVisible(dual);
    dualSecondPersonLabel->setVisible(dual);
    dualSecondPersonLineEdit->setVisible(dual);

    dualThirdMalePersonLabel->setVisible(dual && maleVisible);
    dualThirdMalePersonLineEdit->setVisible(dual && maleVisible);
    dualThirdFemalePersonLabel->setVisible(dual && femaleVisible);
    dualThirdFemalePersonLineEdit->setVisible(dual && femaleVisible);
    dualThirdNeutralPersonLabel->setVisible(dual && neuterVisible);
    dualThirdNeutralPersonLineEdit->setVisible(dual && neuterVisible);

    pluralLabel->setVisible(plural);
    pluralFirstPersonLabel->setVisible(plural);
    pluralFirstPersonLineEdit->setVisible(plural);
    pluralSecondPersonLabel->setVisible(plural);
    pluralSecondPersonLineEdit->setVisible(plural);

    pluralThirdMalePersonLabel->setVisible(plural && maleVisible);
    pluralThirdMalePersonLineEdit->setVisible(plural && maleVisible);
    pluralThirdFemalePersonLabel->setVisible(plural && femaleVisible);
    pluralThirdFemalePersonLineEdit->setVisible(plural && femaleVisible);
    pluralThirdNeutralPersonLabel->setVisible(plural && neuterVisible);
    pluralThirdNeutralPersonLineEdit->setVisible(plural && neuterVisible);
}

void ConjugationWidget::tenseEditingFinished()
{
    const QStringList& oldTenses = m_doc->identifier(m_identifier).tenseList();
    if (!oldTenses.contains(tenseComboBox->currentText())) {
        // add a new tense
        m_doc->identifier(m_identifier).setTense(oldTenses.count(), tenseComboBox->currentText());
        // put it into the completion
        tenseComboBox->completionObject()->addItem(tenseComboBox->currentText());
    }
}


#include "conjugationwidget.moc"