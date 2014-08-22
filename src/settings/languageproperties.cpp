/***************************************************************************
    Copyright 2008 Frederik Gladhorn <gladhorn@kde.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "languageproperties.h"

#include "languagepropertiespage.h"
#include "languagesettings.h"

#include <keduvocdocument.h>

#include <KMessageBox>
#include <QLabel>
#include <KLocale>

LanguageProperties::LanguageProperties(KEduVocDocument* doc, QWidget * parent)
    : KPageDialog(parent), m_doc(doc)
{
    setCaption(i18n("Edit Languages"));
    setFaceType(List);
    setButtons(User1 | User2 | Ok | Cancel);

    setButtonText(User2, i18n("Add language"));
    setButtonIcon(User2, KIcon("list-add"));
    setButtonText(User1, i18n("Remove language"));
    setButtonIcon(User1, KIcon("list-remove"));

    connect(this, SIGNAL(user2Clicked()), this, SLOT(slotAppendIdentifier()));
    connect(this, SIGNAL(user1Clicked()), this, SLOT(slotDeleteIdentifier()));

    for (int i = 0; i < m_doc->identifierCount(); i++) {
        createPage(i);
    }
}

KPageWidgetItem*  LanguageProperties::createPage(int i)
{
    LanguagePropertiesPage* page = new LanguagePropertiesPage(m_doc, i, this);

    QString name = i18n("New Language");

    // check if this language already exists in the doc
    if (m_doc->identifierCount() <= i) {
      m_doc->appendIdentifier();
    }
    if (m_doc->identifierCount() > i) {
        name = m_doc->identifier(i).name();
        LanguageSettings currentSettings(m_doc->identifier(i).locale());
        currentSettings.readConfig();
    }

    KPageWidgetItem* editPage = new KPageWidgetItem(page, name);
    editPage->setHeader(i18nc("Edit language properties", "Properties for %1", name));

    m_pages.append(editPage);
    addPage(editPage);

    editPage->setIcon(KIcon("set-language"));

    connect(page->identifierNameLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(pageNameChanged(const QString&)));
    connect(this, SIGNAL(accepted()), page, SLOT(accept()));

    return editPage;
}

void LanguageProperties::accept()
{
    KEduVocDocument *doc = m_doc;
    int deleted = 0;

    for (int index = 0; index < m_pages.count(); index++) {
        if (m_pages.value(index)->isEnabled()) {
            if (index - deleted >= doc->identifierCount()) {
                // add a language
                doc->appendIdentifier();
            }
            LanguagePropertiesPage * page = static_cast<LanguagePropertiesPage *>(m_pages.value(index)->widget());
            page->setLanguageIdentifierIndex(index - deleted);
        } else {
            // page is disabled, delete the language
            if (index < doc->identifierCount()) {
                if (KMessageBox::warningYesNo(this, i18n("Really delete language: %1?", doc->identifier(index - deleted).name()), i18n("Remove Language")) == KMessageBox::Yes) {
                    doc->removeIdentifier(index - deleted);
                    deleted++;
                }
            }
        }
    }

    KDialog::accept();
}


void LanguageProperties::slotAppendIdentifier()
{
    // if a page that was "removed" is selected, simply enable it again to not delete it.
    if (currentPage() && !currentPage()->isEnabled()) {
        currentPage()->setEnabled(true);
        return;
    }

    int i = m_pages.count();
    KPageWidgetItem* newPage = createPage(i);
    setCurrentPage(newPage);
}

void LanguageProperties::slotDeleteIdentifier()
{
    currentPage()->setEnabled(false);
}

void LanguageProperties::pageNameChanged(const QString & newName)
{
    currentPage()->setName(newName);
}

#include "languageproperties.moc"
