/***************************************************************************

                       edit common properties

    -----------------------------------------------------------------------

    begin         : Mon Jun 28 21:02:16 1999

    copyright     : (C) 1999-2001 Ewald Arnold <kvoctrain@ewald-arnold.de>
                    (C) 2001 The KDE-EDU team
                    (C) 2005-2007 Peter Hedlund <peter.hedlund@kdemail.net>
                    (C) 2007 Frederik Gladhorn <frederik.gladhorn@kdemail.net>

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

#include "CommonEntryPage.h"

#include <QLineEdit>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QPixmap>

#include <kdebug.h>
#include <klocale.h>
#include <kicon.h>
#include <kdialog.h>

#include "query-dialogs/kvtquery.h"
#include <kvtlanguages.h>
#include "DocPropDlg.h"
#include "PhoneticEntryPage.h"
#include "EntryDlg.h"

CommonEntryPage::CommonEntryPage(KEduVocDocument *doc, QWidget *parent) : QWidget(parent), m_doc(doc)
{
    setupUi(this);

    m_wordTypes.setDocument(m_doc);

    // subdialogs
    connect(b_usageDlg, SIGNAL(clicked()), SLOT(invokeUsageDlg()));
    connect(b_pronDlg, SIGNAL(clicked()), SLOT(invokePronDlg()));
    connect(b_TypeDlg, SIGNAL(clicked()), SLOT(invokeTypeDlg()));

    // in general only the modified state interests us.
    connect(usage_box, SIGNAL(itemSelectionChanged()), this, SLOT(slotDataChanged()));
    // to update the usage label
    connect(usage_box, SIGNAL(itemSelectionChanged()), this, SLOT(slotUsageChanged()));
    connect(lesson_box, SIGNAL(activated(int)), SLOT(slotDataChanged(int)));
    connect(subtype_box, SIGNAL(activated(int)), SLOT(slotDataChanged(int)));
    connect(c_active, SIGNAL(stateChanged(int)), SLOT(slotDataChanged(int)));
    connect(pronounce_line, SIGNAL(textChanged(const QString&)), SLOT(slotDataChanged(const QString&)));
    connect(expr_line, SIGNAL(textChanged(const QString&)), SLOT(slotDataChanged(const QString&)));

    connect(type_box, SIGNAL(activated(int)), SLOT(slotDataChanged(int)));
    // type is tricky - need to update subtype
    connect(type_box, SIGNAL(activated(const QString&)), SLOT(slotUpdateSubTypeBoxContents(const QString&)));



    usage_label->setTitle(i18nc("Usage (area) of an Expression", "&Usage Labels"));
    pronounce_line->setFont(Prefs::iPAFont());

    QIcon list_pm = KIcon("list");
    b_LessDlg->setIcon(list_pm);
    b_LessDlg->setEnabled(false);
    b_TypeDlg->setIcon(list_pm);
    b_usageDlg->setIcon(list_pm);

    QIcon pron_pm = KIcon("character-set");
    b_pronDlg->setIcon(pron_pm);

    subDialog = 0;

    m_currentRow = -1;
    m_currentTranslation = -1;
}


void CommonEntryPage::setData(int row, int col, const QModelIndexList & selection)
{
    m_currentRow = row;
    m_currentTranslation = col;
    m_selection = selection;

    bool editSingleEntry = (m_selection.count() == 1);
    //disable fields that cannot be changed for multiple, enable them if only one entry is edited
    expr_line->setEnabled(editSingleEntry);
    pronounce_line->setEnabled(editSingleEntry);

    KEduVocExpression *firstEntry = m_doc->entry(m_currentRow);
    // set these to the first entry, check if that's ok later
    setLessonBox(firstEntry->lesson());
    setUsageBox(firstEntry->translation(m_currentTranslation).usageLabel());

    c_active->setChecked(firstEntry->isActive());


kDebug() << " TYPE: " << m_doc->entry(m_currentRow)->translation(m_currentTranslation).type() << " WordType: " << m_wordTypes.getMainTypeFromOldFormat(m_doc->entry(m_currentRow)->translation(m_currentTranslation).type()) << " sub: " << m_wordTypes.getSubTypeFromOldFormat(m_doc->entry(m_currentRow)->translation(m_currentTranslation).type());

    updateMainTypeBoxContents();


    QString mainType =
        m_wordTypes.getMainTypeFromOldFormat(
            m_doc->entry(m_currentRow)->
            translation(m_currentTranslation).type());
    QString subType =
        m_wordTypes.getSubTypeFromOldFormat(
            m_doc->entry(m_currentRow)->
            translation(m_currentTranslation).type());

kDebug() << " type index: " << mainType << " sub " << subType;

    type_box->setCurrentIndex( type_box->findText( mainType ) );
    slotUpdateSubTypeBoxContents( mainType );
    subtype_box->setCurrentIndex( subtype_box->findText( subType ) );

    if (editSingleEntry) {
        c_active->setTristate(false);
        // these can only be edited in single mode
        expr_line->setText(m_doc->entry(m_currentRow)->translation(m_currentTranslation).translation());

        pronounce_line->setText(m_doc->entry(m_currentRow)->translation(m_currentTranslation).pronunciation());

    } else { // edit more than one entry
        c_active->setTristate(true);
        expr_line->setText("");
        pronounce_line->setText("");

        // fill enabled fields if equal for all edited entries, otherwise empty.
        foreach ( QModelIndex modelIndex, m_selection) {
            KEduVocExpression *currentEntry = m_doc->entry(modelIndex.row());
            if ( firstEntry->lesson() != currentEntry->lesson() ) {
                lesson_box->setCurrentIndex(-1);
            }

            /// @todo as soon as we have a .subtype() function use that and check for type differences. Now cheating: only set, if type and subtype are equal.
            if ( firstEntry->translation(m_currentTranslation).type()
                    != currentEntry->translation(m_currentTranslation).type()) {
                type_box->setCurrentIndex(-1);
                subtype_box->setCurrentIndex(-1);
            }
            if ( firstEntry->translation(m_currentTranslation).usageLabel()
                    != currentEntry->translation(m_currentTranslation).usageLabel()) {
                setUsageBox("");
            }
            if ( firstEntry->isActive() != currentEntry->isActive() ) {
                kDebug() << "Setting active box to PartiallyChecked.";
                c_active->setCheckState(Qt::PartiallyChecked);
            }
        } // foreach
    } // edit more than one entry
}


void CommonEntryPage::setLessonBox(int lesson)
{
    lesson_box->clear();
    //lesson_box->addItem(m_doc->lessonDescription(0));
    lesson_box->addItems(m_doc->lessonDescriptions());

    //if (lesson >= lesson_box->count())
    //  lesson = 0;
    lesson_box->setCurrentIndex(lesson-1);
}


void CommonEntryPage::setUsageBox(const QString & act_usage)
{
    usages = KVTUsage::getRelation();
    usage_box->clear();
    for (int i = 0; i < (int) usages.size(); i++) {
        usage_box->addItem(usages[i].longStr());
        if (KVTUsage::contains(QString(usages[i].identStr()), act_usage)) {
            usage_box->setCurrentRow(i);
        }
    }
    slotUsageChanged();
}

void CommonEntryPage::slotUsageChanged()
{
    QString s;

    for (int i = 0; i < usage_box->count(); i++) {
        if (usage_box->item(i)->isSelected()) {
            if (!s.isEmpty())
                s += ", ";
            s += usages[i].shortStr();
        }
    }
    usage_line->setText(s);
}


void CommonEntryPage::slotUpdateSubTypeBoxContents(const QString &mainType)
{
    subtype_box->clear();
    subtype_box->addItems( m_wordTypes.getSubTypeList( mainType) );
    subtype_box->setCurrentIndex(-1);
}


void CommonEntryPage::phoneticSelected(wchar_t wc)
{
    pronounce_line->setText( pronounce_line->text() += QChar(wc) );
    emit sigModified();
}


void CommonEntryPage::invokePronDlg()
{
    PhoneticEntryPage * phoneticDlg = new PhoneticEntryPage(Prefs::iPAFont(), this);
    connect(phoneticDlg, SIGNAL(charSelected(wchar_t)), SLOT(phoneticSelected(wchar_t)));
    phoneticDlg->show();
}


void CommonEntryPage::invokeUsageDlg()
{
    QList<int> usageIndex;
    QStringList new_usageStr;

    int old_usages = (int) m_doc->usageDescriptions().size();

    KDialog *subDialog= new KDialog(b_usageDlg);
    subDialog->setCaption(i18nc("usage (area) of an expression", "Edit User-Defined Usage Labels"));
    subDialog->setButtons(KDialog::Ok|KDialog::Cancel);

    subDialog->setDefaultButton(KDialog::Ok);

    connect(subDialog, SIGNAL(finished()), this, SLOT(slotSubDialogClosed()));

    UsageOptPage *usageOptPage = new UsageOptPage(m_doc, this);
    subDialog->setMainWidget(usageOptPage);

    if (subDialog->exec() == QDialog::Accepted) {
        usageOptPage->getUsageLabels(new_usageStr, usageIndex);
        UsageOptPage::cleanUnused(m_doc, usageIndex, old_usages);
        KVTUsage::setUsageNames(new_usageStr);
        //setUsageBox(m_usageCollection);
        m_doc->setUsageDescriptions(new_usageStr);
        m_doc->setModified();
    }
}


void CommonEntryPage::invokeTypeDlg()
{
    QList<int> typeIndex;
    QStringList new_typeStr;

    int old_types = (int) m_doc->typeDescriptions().size();
    KDialog *subDialog = new KDialog(b_TypeDlg);
    subDialog->setCaption(i18n("Edit User Defined Types"));
    subDialog->setButtons(KDialog::Ok|KDialog::Cancel);

    subDialog->setDefaultButton(KDialog::Ok);

    connect(subDialog, SIGNAL(finished()), this, SLOT(slotSubDialogClosed()));

    TypeOptPage *typeOptPage = new TypeOptPage(m_doc, this);
    subDialog->setMainWidget(typeOptPage);

    if (subDialog->exec() == QDialog::Accepted) {
        typeOptPage->getTypeNames(new_typeStr, typeIndex);
        TypeOptPage::cleanUnused(m_doc, typeIndex, old_types);
        KVTQuery::setTypeNames(new_typeStr);
        m_wordTypes.update();
        updateMainTypeBoxContents();
        m_doc->setTypeDescriptions(new_typeStr);
        m_doc->setModified();
    }
}


void CommonEntryPage::slotSubDialogClosed()
{
    if (subDialog) {
        disconnect(subDialog, SIGNAL(finished()), this, SLOT(slotSubDialogClosed()));
        subDialog->deleteLater();
        subDialog = 0L;
    }
}

void CommonEntryPage::commitData()
{
    if ( m_selection.count() == 1 ) {
    // these things are only changed when editing a single entry
        KEduVocExpression *expr = m_doc->entry(m_currentRow);
        if (m_currentTranslation >= 0) {
            expr->translation(m_currentTranslation).setTranslation( expr_line->text() );
            expr->translation(m_currentTranslation).setPronunciation( pronounce_line->text() );
        }
    }

    // things that are changed for multiple or single entries

    foreach(QModelIndex selIndex, m_selection) {
        kDebug() << "Changing multiple entries - entry: " << selIndex.row();
        KEduVocExpression *expr = m_doc->entry(selIndex.row());

        // modified because it can be different for multiple entries and will only be saved if the user changes it. if partially checked no entry will be changed.
        if ( c_active->checkState() != Qt::PartiallyChecked ) {
            expr->setActive( c_active->checkState() == Qt::Checked );
        }
        if ( lesson_box->currentIndex() != -1 ) {
            //m_tableModel->setData(m_tableModel->index(index.m_currentRow(), 0), getLesson(), Qt::EditRole);
            expr->setLesson( lesson_box->currentIndex() + 1 );
        }

        if (m_currentTranslation >= 0) {
//                 if (m_usageIsModified)
//                     for (int j = 0; j < expr->translationCount(); j++)
//                         expr->translation(j).setUsageLabel(m_usageCollection);
            if ( type_box->currentIndex() != -1 ) {
                QString type = m_wordTypes.getOldType( type_box->currentText(),  subtype_box->currentText() );
                // set the type
                expr->translation(m_currentTranslation).setType( type );
                // also set the same type for the other translations
                for (int j = 0; j < expr->translationCount(); j++) {
                    if (expr->translation(j).type().isEmpty())
                        expr->translation(j).setType( type );
                }

                for (int j = 0; j < expr->translationCount(); j++) {
                    if (KVTQuery::getMainType(expr->translation(j).type())
                            !=
                            KVTQuery::getMainType(type)) {
                        expr->translation(j).setType(type);
                    }
                }
            } // type
        }
    }
}

void CommonEntryPage::slotDataChanged()
{
    emit sigModified();
}

void CommonEntryPage::slotDataChanged(int )
{
    emit sigModified();
}

void CommonEntryPage::slotDataChanged(const QString& )
{
    emit sigModified();
}

void CommonEntryPage::updateMainTypeBoxContents()
{
    type_box->clear();
    type_box->addItems( m_wordTypes.getMainTypeList() );

}

#include "CommonEntryPage.moc"
