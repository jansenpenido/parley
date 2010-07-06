/***************************************************************************

    copyright     : (C) 2007 Frederik Gladhorn <frederik.gladhorn@kdemail.net>

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

#include "configurepracticedialog.h"

#include "advancedpracticeoptions.h"
#include "thresholdoptions.h"
#include "blockoptions.h"

#include <KLocale>

ConfigurePracticeDialog::ConfigurePracticeDialog(KEduVocDocument *doc, QWidget *parent, const QString &name, KConfigSkeleton *config)
    :KConfigDialog(parent, name, config)
{
    m_config = config;

    setCaption(i18nc("@title:window", "Configure Practice"));
    setButtons(Default|Ok|Apply|Cancel|Help);
    setDefaultButton(Ok);

    m_blockOptions = new BlockOptions(this);
    addPage(m_blockOptions, i18nc("@title:group vocabulary can be set to be blocked for a certain amount of time", "Blocking"), "cards-block", i18n("Blocking Settings"), true);

    m_thresholdOptions = new ThresholdOptions(doc, this);
    addPage(m_thresholdOptions, i18nc("@title:group ignore vocabulary based on some properties like word type", "Thresholds"), "practice-setup", i18n("Threshold Settings"), true);

    m_advancedPracticeOptions = new AdvancedPracticeOptions(this);
    addPage(m_advancedPracticeOptions, i18nc("@title:group Configure advanced settings for practicing vocabulary, short title in config dialog.", "Advanced"), "advanced-setup", i18nc("Configure advanced settings for practicing vocabulary.", "Advanced Practice Settings"), true);
    setHelp(QString(),"parley");

    KConfigGroup cg(KSharedConfig::openConfig("parleyrc"), "ConfigurePracticeDialog");
    KDialog::restoreDialogSize(cg);
}

ConfigurePracticeDialog::~ConfigurePracticeDialog()
{
    KConfigGroup cg(KSharedConfig::openConfig("parleyrc"), "ConfigurePracticeDialog");
    KDialog::saveDialogSize(cg);
}


bool ConfigurePracticeDialog::hasChanged()
{
    return  m_thresholdOptions->hasChanged() ||
            m_blockOptions->hasChanged();
}

bool ConfigurePracticeDialog::isDefault()
{
    return m_thresholdOptions->isDefault() &&
           m_blockOptions->isDefault();
}

void ConfigurePracticeDialog::updateSettings()
{
    m_thresholdOptions->updateSettings();
    m_blockOptions->updateSettings();
}

void ConfigurePracticeDialog::updateWidgets()
{
    m_thresholdOptions->updateWidgets();
    m_blockOptions->updateWidgets();
}

void ConfigurePracticeDialog::updateWidgetsDefault()
{
    m_config->useDefaults(true);
    updateWidgets();
    m_config->useDefaults(false);
}

#include "configurepracticedialog.moc"