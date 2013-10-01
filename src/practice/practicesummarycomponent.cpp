/***************************************************************************
    Copyright 2007-2009 Frederik Gladhorn <gladhorn@kde.org>
 ***************************************************************************

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "practicesummarycomponent.h"
#include "prefs.h"

#include "parleyactions.h"

#include <QTableWidgetItem>
#include <QTextDocument>
#include <QTextDocumentWriter>
#include <QTextCursor>
#include <QTextTable>
#include <KMessageBox>
#include <keduvocexpression.h>
#include <KConfigGroup>
#include <KActionCollection>
#include <KColorScheme>
#include <KToolBar>
#include <KFileDialog>

using namespace Practice; 

class PracticeSummaryComponent::SortedAttemptTableWidgetItem: public QTableWidgetItem
{
    virtual bool operator<(const QTableWidgetItem &other) const
    {
        if (data(Qt::DisplayRole).toInt() == other.data(Qt::DisplayRole).toInt()) {
            return data(Qt::UserRole).toInt() < other.data(Qt::UserRole).toInt();
        }
        return data(Qt::DisplayRole).toInt() < other.data(Qt::DisplayRole).toInt();
    }
};

PracticeSummaryComponent::PracticeSummaryComponent(TestEntryManager* testEntryManager, QWidget* parent)
    :KXmlGuiWindow(parent)
    ,m_testEntryManager(testEntryManager)
{
    // KXmlGui
    setXMLFile("practicesummaryui.rc");
    setObjectName("Statistics");
    
    QWidget *mainWidget = new QWidget(this);
    setupUi(mainWidget);
    setCentralWidget(mainWidget);

    initActions(parent);
    
    setupDetailsTable();
    summaryBar->setStatistics(m_testEntryManager->statisticTotalCorrectFirstAttempt(), m_testEntryManager->statisticTotalWrong(), m_testEntryManager->statisticTotalUnanswered());

    int total = m_testEntryManager->statisticTotalCorrectFirstAttempt() + m_testEntryManager->statisticTotalWrong();
    int minutes = m_testEntryManager->totalTime() / 60;
    int seconds = m_testEntryManager->totalTime() % 60;
    
    testSummaryLabel->setText(i18nc("number of words, minutes, seconds", "You practiced %1 in %2 and %3.",
                                    i18np("one word", "%1 words", total),
                                    i18np("one minute", "%1 minutes", minutes),
                                    i18np("one second", "%1 seconds", seconds)));
    
    KConfigGroup cfg(KSharedConfig::openConfig("parleyrc"), objectName());
    applyMainWindowSettings(cfg);
}

PracticeSummaryComponent::~PracticeSummaryComponent()
{
    KConfigGroup cfg(KSharedConfig::openConfig("parleyrc"), objectName());
    saveMainWindowSettings(cfg);
}

void PracticeSummaryComponent::initActions(QWidget* parleyMainWindow)
{
    ParleyActions::create(ParleyActions::EnterEditMode, parleyMainWindow, SLOT(showEditor()), actionCollection());
    ParleyActions::create(ParleyActions::StartPractice, parleyMainWindow, SLOT(showPracticeConfiguration()), actionCollection());
    ParleyActions::create(ParleyActions::ExportPracticeResults, this, SLOT(exportResults()), actionCollection());
    actionCollection()->action("practice_start")->setText(i18n("Practice Again"));
    actionCollection()->action("practice_start")->setToolTip(i18n("Practice Again"));
}

void PracticeSummaryComponent::setupDetailsTable()
{
    tableWidget->setRowCount(m_testEntryManager->totalEntryCount());
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

    Qt::ItemFlags flags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    KColorScheme scheme(QPalette::Active);
    QPalette correctPalette = QApplication::palette();
    correctPalette.setColor(QPalette::WindowText, scheme.foreground(KColorScheme::PositiveText).color());
    correctPalette.setColor(QPalette::Text, scheme.foreground(KColorScheme::PositiveText).color());

    QPalette wrongPalette = QApplication::palette();
    wrongPalette.setColor(QPalette::WindowText, scheme.foreground(KColorScheme::NegativeText).color());
    wrongPalette.setColor(QPalette::Text, scheme.foreground(KColorScheme::NegativeText).color());

    int i = 0;
    // TODO headers with languages
    // TODO some colors, maybe an indicator icon wether the word was right/wrong
    foreach(TestEntry* entry, m_testEntryManager->allTestEntries()) {
        QTableWidgetItem* itemFrom = new QTableWidgetItem(
                entry->entry()->translation(TestEntry::gradeFrom())->text());
        QTableWidgetItem* itemTo = new QTableWidgetItem(
                entry->entry()->translation(TestEntry::gradeTo())->text());
        if (entry->statisticGoodCount() > 0) {
            itemTo->setForeground(correctPalette.foreground());
        }

        QTableWidgetItem* itemUserAnswer = new QTableWidgetItem(
                entry->userAnswers().join("; "));
        itemUserAnswer->setForeground(wrongPalette.foreground());

        SortedAttemptTableWidgetItem* itemAttempts = new SortedAttemptTableWidgetItem();
        itemAttempts->setData(Qt::DisplayRole, entry->statisticCount());
        itemAttempts->setData(Qt::UserRole, entry->statisticBadCount());
        itemAttempts->setTextAlignment(Qt::AlignRight);

        itemFrom->setFlags(flags);
        itemTo->setFlags(flags);
        itemUserAnswer->setFlags(flags);
        itemAttempts->setFlags(flags);

        if (entry->correctAtFirstAttempt()) {
            itemUserAnswer->setIcon(KIcon("dialog-ok-apply"));
        } else if (entry->statisticGoodCount() > 0) {
            itemUserAnswer->setIcon(KIcon("task-attempt"));
        } else if (entry->statisticCount() > 0) {
            itemUserAnswer->setIcon(KIcon("dialog-error"));
        } else {
            itemUserAnswer->setIcon(KIcon("task-attempt"));
        }

        tableWidget->setItem(i, 0, itemAttempts);
        tableWidget->setItem(i, 1, itemFrom);
        tableWidget->setItem(i, 2, itemTo);
        tableWidget->setItem(i, 3, itemUserAnswer);
        ++i;
    }

    tableWidget->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    tableWidget->setSortingEnabled(true);
    tableWidget->sortItems(0, Qt::DescendingOrder);
}

void PracticeSummaryComponent::exportResults()
{
    QString filter = "*.html|" + i18n("HTML Files") + "\n*.odt|" + i18n("OpenDocument text files");
    QString fileName = KFileDialog::getSaveFileName(KUrl("kfiledialog:///practice_export"), filter);

    if (fileName.isEmpty()) {
        return;
    }

    QTextDocument doc;
    doc.setHtml("<html><head><title>" + i18n("Practice results") + "</title></body></html>");
    QTextCursor cursor(&doc);

    cursor.insertHtml("<h1>" + m_testEntryManager->title() + "</h1><br />");

    cursor.insertText(i18n("Answered questions: %1\n", m_testEntryManager->totalEntryCount()));
    cursor.insertText(i18n("Correct answers: %1\n", m_testEntryManager->statisticTotalCorrectFirstAttempt()));
    cursor.insertText(i18n("Wrong answers: %1\n", m_testEntryManager->statisticTotalWrong()));

    QTextTableFormat tableFormat;
    tableFormat.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);
    tableFormat.setCellPadding(1);
    tableFormat.setAlignment(Qt::AlignLeft);
    QTextTable *table = cursor.insertTable(1, 4, tableFormat);
    table->cellAt(0, 0).firstCursorPosition().insertHtml(i18n("<b>Attempts</b>"));
    table->cellAt(0, 1).firstCursorPosition().insertHtml(i18n("<b>Question</b>"));
    table->cellAt(0, 2).firstCursorPosition().insertHtml(i18n("<b>Correct answer</b>"));
    table->cellAt(0, 3).firstCursorPosition().insertHtml(i18n("<b>Your errors</b>"));

    foreach(TestEntry* entry, m_testEntryManager->allTestEntries()) {
        table->appendRows(1);
        int newRow = table->rows() - 1;
        table->cellAt(newRow, 0).firstCursorPosition().insertText(QString::number(entry->statisticCount()));
        table->cellAt(newRow, 1).firstCursorPosition().insertText(entry->entry()->translation(TestEntry::gradeFrom())->text());
        table->cellAt(newRow, 2).firstCursorPosition().insertText(entry->entry()->translation(TestEntry::gradeTo())->text());
        table->cellAt(newRow, 3).firstCursorPosition().insertText(entry->userAnswers().join("; "));
    }

    QTextDocumentWriter writer(fileName);

    if (!writer.write(&doc)) {
        KMessageBox::error (this, i18n("Could not write to %1", fileName),
                            i18n("Could not write file"));
        return;
    }
}

#include "practicesummarycomponent.moc"

