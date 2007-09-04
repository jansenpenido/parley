/***************************************************************************

                             kvtnewdocumentwizardlanguagepage

    -----------------------------------------------------------------------

    copyright     : (C) 2007 Frederik Gladhorn <frederik.gladhorn@kdemail.net>

    -----------------------------------------------------------------------

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "editlanguagedialogpage.h"

#include "languagesettings.h"

#include <keduvocdocument.h>

#include <KStandardDirs>
#include <KLocale>
#include <QLabel>

/**
 *
 * @param parent
 */
EditLanguageDialogPage::EditLanguageDialogPage(KEduVocDocument* doc, int identifierIndex, QWidget * parent)
    : QWidget(parent)
{
    m_doc = doc;
    m_identifierIndex = identifierIndex;
    setupUi(this);

    initialize();

    identifierNameLineEdit->setText(m_doc->identifier(m_identifierIndex).name());


}


EditLanguageDialogPage::~EditLanguageDialogPage()
{
}


void EditLanguageDialogPage::initialize()
{
    // language codes
    QStringList codes = KGlobal::locale()->allLanguagesList();

    int current = codes.indexOf(KGlobal::locale()->language());

    QStringList languageNames;
    foreach (QString code, codes){
        localeComboBox->addItem( KGlobal::locale()->languageCodeToName(code), code);
    }

    localeComboBox->setCurrentIndex(localeComboBox->findData(
        m_doc->identifier(m_identifierIndex).locale()));

    // icons
    LanguageSettings currentSettings(m_doc->identifier(m_identifierIndex).locale());
    currentSettings.readConfig();
    QString currentIcon = currentSettings.icon();

    iconComboBox->addItem(i18n("No icon"));

    QStringList countrylist = KGlobal::locale()->allCountriesList();
    countrylist.sort();

    int currentIndex = 0;

    foreach(QString code, countrylist) {
        QString country = KGlobal::dirs()->findResource("locale",
                QString("l10n/%1/entry.desktop").arg(code));
        KConfig entry(country, KConfig::OnlyLocal);
        KConfigGroup group = entry.group("KCM Locale");
        QString name = group.readEntry("Name", i18n("without name"));

        QString pixmap = country;
        pixmap.truncate(pixmap.lastIndexOf('/'));
        pixmap += "/flag.png";

        iconComboBox->addItem(QIcon(pixmap), name, pixmap);
    }


    iconComboBox->setCurrentIndex(iconComboBox->findData(currentIcon));
}


void EditLanguageDialogPage::commitData()
{
    QString locale = localeComboBox->itemData(localeComboBox->currentIndex()).toString();
    m_doc->identifier(m_identifierIndex).setLocale( locale );
    m_doc->identifier(m_identifierIndex).setName( identifierNameLineEdit->text() );

    QString icon = iconComboBox->itemData(iconComboBox->currentIndex()).toString();

    LanguageSettings settings(locale);
    settings.setIcon(icon);
    settings.setKeyboardLayout("");
    settings.writeConfig();

    kDebug() << "commitData: " << m_identifierIndex << " locale: " << locale << " icon: " << icon;
}

#include "editlanguagedialogpage.moc"
