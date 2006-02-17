/***************************************************************************

                      main part of kvoctrain

    -----------------------------------------------------------------------

    begin         : Thu Mar 11 20:50:53 MET 1999

    copyright     : (C) 1999-2001 Ewald Arnold <kvoctrain@ewald-arnold.de>
                    (C) 2001 The KDE-EDU team
                    (C) 2004-2006 Peter Hedlund <peter.hedlund@kdemail.net>

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
#include <QTimer>
#include <QPixmap>
#include <QKeyEvent>

#include <kstatusbar.h>
#include <klineedit.h>
#include <kcombobox.h>
#include <kprogressbar.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kprinter.h>
#include <kinputdialog.h>

#include <time.h>
#include <ctype.h>

#include "kvoctrain.h"
#include "entry-dialogs/EntryDlg.h"
#include "docprop-dialogs/DocPropDlg.h"
#include "docprop-dialogs/DocPropLangDlg.h"
#include "statistik-dialogs/StatistikDlg.h"
#include "common-dialogs/kvoctrainprefs.h"
#include "prefs.h"

#define MAX_LESSON       25
#define THRESH_LESSON    KV_MIN_GRADE

/*void kvoctrainApp::slotSaveOptions()
{
   saveOptions(true);
}
*/

kvoctrainApp::~kvoctrainApp()
{
   removeEntryDlg();
   delete header_m;
   delete btimer;
   delete view;
   delete doc;
}


void kvoctrainApp::slotCancelSelection ()
{
  view->getTable()->clearSelection();
}


void kvoctrainApp::slotSelectAll ()
{
  Q3TableSelection ts;
  KVocTrainTable *table = view->getTable();
  table->clearSelection();
  ts.init(0,0);
  ts.expandTo(table->numRows()-1, table->numCols()-1);
  table->addSelection(ts);
}


void kvoctrainApp::slotCurrentCellChanged(int row, int col)
{
  col -= KV_EXTRA_COLS;
  bool noData = false;
  KEduVocExpression *expr = 0;

  statusBar()->clear();
  if (doc->numEntries() <= row || doc->numIdentifiers() <= col || row < 0 || col < 0)
    noData = true;
  else
    expr = doc->entry(row);

  if (rem_label != 0)
    rem_label->setText(i18n("Abbreviation for R)emark","R: %1")
                       .arg(noData ? QString() : expr->remark(col)));
  if (pron_label != 0)
    pron_label->setText(i18n("Abbreviation for P)ronouncation","P: %1")
                        .arg(noData ? QString() : expr->pronounciation(col)));
  if (type_label != 0)
    type_label->setText(i18n("Abbreviation for T)ype of word", "T: %1")
                        .arg(noData ? QString() : QueryManager::typeStr(expr->type(col))));

  if (entryDlg != 0) {
    if (noData)
      entryDlg->setEnabled(EntryDlg::EnableOnlyCommon);
    else {
      if (col == 0)
        entryDlg->setEnabled(EntryDlg::EnableOnlyOriginal);
      else
        entryDlg->setEnabled(EntryDlg::EnableAll);
    }
    slotEditEntry(row, col + KV_EXTRA_COLS);
  }
}


void kvoctrainApp::slotEditRow()
{
  slotEditEntry (view->getTable()->currentRow(), view->getTable()->currentColumn());
}


void kvoctrainApp::slotEditCallBack(int res)
{
//  cout << "secb\n";
  switch (res) {
    case EntryDlg::EditCancel:
      removeEntryDlg();
    break;

    case EntryDlg::EditApply:
      commitEntryDlg(true);
      if (Prefs::smartAppend())
      {
        int row = view->getTable()->currentRow();
        if (row == view->getTable()->numRows()-1)
        {
          int col = view->getTable()->currentColumn();
          if (col < view->getTable()->numCols()-1 && col >= KV_COL_ORG )
          {
            int lesson = doc->entry(row)->lesson();
            if (lesson >= lessons->count())
              lesson = QMAX (0, lessons->count()-1);
            slotChooseLesson(lesson);

            QString exp;
            exp = doc->entry(row)->translation(col+1-KV_COL_ORG);
            if (exp.isEmpty())
              view->getTable()->setCurrentRow(row, col+1);
          }
          else
            slotAppendRow();
        }
      }
    break;

    case EntryDlg::EditUndo:
      int row, col;
      QList<Q3TableSelection> tabsel;
      entryDlg->getCell(row, col, tabsel);
      setDataEntryDlg(row, col);
    break;
  }
}


void kvoctrainApp::commitEntryDlg(bool force)
{
//  cout << "ced\n";
   if (entryDlg == 0) {
     kError() << "kvoctrainApp::commitEntryDlg: entryDlg == 0\n";
     return;
   }

   if (!force && entryDlg->isModified() && !Prefs::autoEntryApply()) {
     if( KMessageBox::No == KMessageBox::warningYesNo(this,
                   i18n("The entry dialog contains unsaved changes.\n"
                        "Do you want to apply or discard your changes?"),
                   kapp->makeStdCaption(i18n("Unsaved Changes")),
                   KStdGuiItem::apply(), KStdGuiItem::discard())) {
        return;
     }
   }

   int row, col;
   QList<Q3TableSelection> tabsel;
   entryDlg->getCell(row, col, tabsel);
   int hasSel = tabsel.size() > 1;
   if (tabsel.size() == 1)
     hasSel = (tabsel[0].bottomRow() - tabsel[0].topRow()) > 0;

   fillLessonBox(doc);
   if (!hasSel) {
     KEduVocExpression *expr = doc->entry(row);
     if (col >= KV_COL_ORG) {
       col -= KV_EXTRA_COLS;
       if (col == 0)
         expr->setOriginal(entryDlg->getExpr());
       else
         expr->setTranslation(col, entryDlg->getExpr());

       expr->setRemark (col, entryDlg->getRemark());
       expr->setPronounciation(col, entryDlg->getPronunce());

       expr->setSynonym (col, entryDlg->getSynonym());
       expr->setAntonym (col, entryDlg->getAntonym());
       expr->setExample (col, entryDlg->getExample());
       expr->setUsageLabel (col, entryDlg->getUsageLabel());
       expr->setParaphrase (col, entryDlg->getParaphrase());
       expr->setConjugation (col, entryDlg->getConjugation());
       expr->setComparison(col, entryDlg->getComparison() );
       expr->setMultipleChoice(col, entryDlg->getMultipleChoice() );

       expr->setFauxAmi (col, entryDlg->getFromFauxAmi(), false);
       expr->setFauxAmi (col, entryDlg->getToFauxAmi(), true);
       expr->setGrade(col, entryDlg->getFromGrade(), false);
       expr->setGrade(col, entryDlg->getToGrade(), true);
       expr->setQueryCount(col, entryDlg->getFromQCount(), false);
       expr->setQueryCount(col, entryDlg->getToQCount(), true);
       expr->setBadCount(col, entryDlg->getFromBCount(), false);
       expr->setBadCount(col, entryDlg->getToBCount(), true);
       QDateTime dt;
       dt.setTime_t(entryDlg->getFromDate());
       expr->setQueryDate(col, dt, false);
       dt.setTime_t(entryDlg->getToDate());
       expr->setQueryDate(col, dt, true);
       expr->setType (col, entryDlg->getType());

       for (int j = 0; j <= expr->numTranslations(); j++)
         if (expr->type(j).isEmpty() )
           expr->setType(j, entryDlg->getType());

       for (int j = 0; j <= expr->numTranslations(); j++)
         if (QueryManager::getMainType(expr->type(j))
               !=
             QueryManager::getMainType(entryDlg->getType()) )
           expr->setType(j, entryDlg->getType());
     }
     expr->setLesson(entryDlg->getLesson());
     expr->setActive(entryDlg->getActive());

     entryDlg->setModified(false);
     doc->setModified(true);
     view->getTable()->updateCell(row, col+KV_EXTRA_COLS);
     view->getTable()->updateCell(row, KV_COL_LESS);
   }
   else {
     col -= KV_EXTRA_COLS;
     for (uint ts = 0; ts < tabsel.size(); ++ts) {
       for (int er = tabsel[ts].topRow(); er <= tabsel[ts].bottomRow(); ++er) {
         KEduVocExpression *expr = doc->entry(er);

         if (col >= 0) {
           // only updated "common" props in multimode
           if (entryDlg->fromGradeDirty() )
             expr->setGrade(col, entryDlg->getFromGrade(), false);
           if (entryDlg->toGradeDirty() )
             expr->setGrade(col, entryDlg->getToGrade(), true);

           if (entryDlg->fromQCountDirty() )
             expr->setQueryCount(col, entryDlg->getFromQCount(), false);
           if (entryDlg->toQCountDirty() )
              expr->setQueryCount(col, entryDlg->getToQCount(), true);

           if (entryDlg->fromBCountDirty() )
             expr->setBadCount(col, entryDlg->getFromBCount(), false);
           if (entryDlg->toBCountDirty() )
             expr->setBadCount(col, entryDlg->getToBCount(), true);
           ///@todo port
           /*
           if (entryDlg->fromDateDirty() )
             expr->setQueryDate(col, entryDlg->getFromDate(), false);
           if (entryDlg->toDateDirty() )
             expr->setQueryDate(col, entryDlg->getToDate(), true);
            */
           if (entryDlg->usageDirty() ) {
             for (int j = 0; j <= expr->numTranslations(); j++)
               expr->setUsageLabel (j, entryDlg->getUsageLabel());
           }

           if (entryDlg->typeDirty() )
             for (int j = 0; j <= expr->numTranslations(); j++)
               expr->setType(j, entryDlg->getType());
         }

         if (entryDlg->activeDirty() )
           expr->setActive(entryDlg->getActive());

         if (entryDlg->lessonDirty() )
           expr->setLesson (entryDlg->getLesson());
       }
     }
     entryDlg->setModified(false);
     doc->setModified(true);
     for (uint ts = 0; ts < tabsel.size(); ++ts)
       for (int r = tabsel[ts].topRow(); r <= tabsel[ts].bottomRow(); ++r)
         for (int c = 0; c < view->getTable()->numCols(); ++c)
           view->getTable()->updateCell(r, c);

   }
}


void kvoctrainApp::createEntryDlg(int row, int col)
{
   if (entryDlg != 0) {
     kError() << "kvoctrainApp::createEntryDlg: entryDlg != 0\n";
     return;
   }

   if ((row < 0) || (col < 0) || (view->getTable()->numRows() <= 0))
     return;

   QString title, text, lang;

   int lesson = doc->entry(row)->lesson();
   if (lesson >= lessons->count())
     lesson = QMAX (0, lessons->count()-1);

   if (col < KV_EXTRA_COLS) {
     title = i18n("Edit General Properties");
     col -= KV_EXTRA_COLS;
     entryDlg = new EntryDlg (
                    this,
                    doc,
                    hasSelection(),
                    true,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    QDateTime(),
                    QDateTime(),
                    QString(),
                    QString(),
                    QString(),
                    lesson,
                    lessons,
                    doc->originalIdentifier(),
                    langset,
                    QString(),
                    doc->entry(row)->type(col),
                    QString(),
                    QString(),
                    QString(),
                    QString(),
                    QString(),
                    QString(),
                    doc->conjugation(0),
                    KEduVocConjugation(),
                    doc->article(0),
                    KEduVocComparison(),
                    KEduVocMultipleChoice(),
                    querymanager,
                    title,
                    doc->entry(row)->isActive(),
                    Prefs::iPAFont());
   }
   else {
     col -= KV_EXTRA_COLS;

     if (col == 0) {
       lang = doc->originalIdentifier();
       text = doc->entry(row)->original();
       title = i18n("Edit Properties for Original");
     }
     else {
       lang = doc->identifier(col);
       text = doc->entry(row)->translation(col);
       title = i18n("Edit Properties of a Translation");
     }

     entryDlg = new EntryDlg (
                    this,
                    doc,
                    hasSelection(),
                    col==0,
                    doc->entry(row)->grade(col, false),
                    doc->entry(row)->grade(col, true),
                    doc->entry(row)->queryCount(col, false),
                    doc->entry(row)->queryCount(col, true),
                    doc->entry(row)->badCount(col, false),
                    doc->entry(row)->badCount(col, true),
                    doc->entry(row)->queryDate(col, false),
                    doc->entry(row)->queryDate(col, true),
                    doc->entry(row)->fauxAmi(col, false),
                    doc->entry(row)->fauxAmi(col, true),
                    text,
                    lesson,
                    lessons,
                    lang,
                    langset,
                    doc->entry(row)->remark(col),
                    doc->entry(row)->type(col),
                    doc->entry(row)->pronounciation(col),
                    doc->entry(row)->synonym(col),
                    doc->entry(row)->antonym(col),
                    doc->entry(row)->example(col),
                    doc->entry(row)->usageLabel(col),
                    doc->entry(row)->paraphrase(col),
                    doc->conjugation(col),
                    doc->entry(row)->conjugation(col),
                    doc->article(col),
                    doc->entry(row)->comparison(col),
                    doc->entry(row)->multipleChoice(col),
                    querymanager,
                    title,
                    doc->entry(row)->isActive(),
                    Prefs::iPAFont());
   }
   connect( entryDlg, SIGNAL(sigEditChoice(int)),
             this, SLOT(slotEditCallBack(int)));

   view->getTable()->setReadOnly(true);

   if (col == 0)
     entryDlg->setEnabled(EntryDlg::EnableOnlyOriginal);
   else
     entryDlg->setEnabled(EntryDlg::EnableAll);

   QList<Q3TableSelection> tabsel;
   entryDlg->setCell(row, col+KV_EXTRA_COLS, tabsel);
   entryDlg->show();
}


void kvoctrainApp::removeEntryDlg()
{
//  cout << "red\n";
  if (entryDlg != 0) {
    commitEntryDlg(false);
    delete entryDlg;
    entryDlg = 0;
  }

  view->getTable()->setReadOnly(false);
}


void kvoctrainApp::slotEditEntry (int row, int col)
{
   if (entryDlg == 0) {
     createEntryDlg(row, col);
     return;
   }

   if (entryDlg->isModified()) {
     commitEntryDlg(false);
   }

   setDataEntryDlg(row, col);
}


void kvoctrainApp::setDataEntryDlg (int row, int col)
{
//  cout << "sded\n";
   if (entryDlg == 0) {
     kError() << "kvoctrainApp::setDataEntryDlg: entryDlg == 0\n";
     return;
   }

   if ((row < 0) || (col < 0) || (view->getTable()->numRows() <= 0))
     return;

   QString text, lang, title;

   KEduVocExpression *expr = doc->entry(row);

   if (expr == 0)
     return; // entry delete in the meantime

   int lesson = expr->lesson();
   if (lesson >= lessons->count())
     lesson = QMAX (0, lessons->count()-1);

   bool hasSel = hasSelection();

   if (col < KV_EXTRA_COLS) {
     title = i18n("Edit General Properties");
     col -= KV_EXTRA_COLS;
     entryDlg->setData(doc,
                       hasSel,
                       true,
                       0,
                       0,
                       0,
                       0,
                       0,
                       0,
                       QDateTime(),
                       QDateTime(),
                       QString(),
                       QString(),
                       QString(),
                       lesson,
                       lessons,
                       doc->originalIdentifier(),
                       langset,
                       QString(),
                       doc->entry(row)->type(0),
                       QString(),
                       QString(),
                       QString(),
                       QString(),
                       QString(),
                       QString(),
                       doc->conjugation(0),
                       KEduVocConjugation(),
                       doc->article(0),
                       KEduVocComparison(),
                       KEduVocMultipleChoice(),
                       querymanager,
                       title,
                       doc->entry(row)->isActive());
   }
   else {
     col -= KV_EXTRA_COLS;

     if (col == 0) {
       title = i18n("Edit Properties for Original");
       lang = doc->originalIdentifier();
       text = doc->entry(row)->original();
     }
     else {
       lang = doc->identifier(col);
       text = doc->entry(row)->translation(col);
       title = i18n("Edit Properties of a Translation");
     }

     entryDlg->setData(doc,
                       hasSel,
                       col==0,
                       doc->entry(row)->grade(col, false),
                       doc->entry(row)->grade(col, true),
                       doc->entry(row)->queryCount(col, false),
                       doc->entry(row)->queryCount(col, true),
                       doc->entry(row)->badCount(col, false),
                       doc->entry(row)->badCount(col, true),
                       doc->entry(row)->queryDate(col, false),
                       doc->entry(row)->queryDate(col, true),
                       doc->entry(row)->fauxAmi(col, false),
                       doc->entry(row)->fauxAmi(col, true),
                       text,
                       lesson,
                       lessons,
                       lang,
                       langset,
                       doc->entry(row)->remark(col),
                       doc->entry(row)->type(col),
                       doc->entry(row)->pronounciation(col),
                       doc->entry(row)->synonym(col),
                       doc->entry(row)->antonym(col),
                       doc->entry(row)->example(col),
                       doc->entry(row)->usageLabel(col),
                       doc->entry(row)->paraphrase(col),
                       doc->conjugation(col),
                       doc->entry(row)->conjugation(col),
                       doc->article(col),
                       doc->entry(row)->comparison(col),
                       doc->entry(row)->multipleChoice(col),
                       querymanager,
                       title,
                       doc->entry(row)->isActive());
   }
   view->getTable()->updateCell(row, col);
   view->getTable()->updateCell(row, KV_COL_LESS);
   QList<Q3TableSelection> tabsel;
   if (hasSel) {
     for (int i = 0; i < view->getTable()->numSelections(); ++i)
       tabsel.push_back(view->getTable()->selection(i));
   }
   entryDlg->setCell(row, col+KV_EXTRA_COLS, tabsel);
}


void kvoctrainApp::slotDocProps ()
{
   int old_lessons = (int) lessons->count();
   int old_types = (int) doc->typeDescriptions().size();
   int old_tenses = (int) doc->tenseDescriptions().size();
   int old_usages = (int) doc->usageDescriptions().size();
   QList<int> old_lessoninquery = doc->lessonsInQuery();

   DocPropsDlg ddlg (doc,
                     0,
                     lessons,
                     doc->title(),
                     doc->author(),
                     doc->license(),
                     doc->docRemark(),
                     doc->typeDescriptions(),
                     doc->tenseDescriptions(),
                     doc->usageDescriptions());

   int res = ddlg.exec();

   if (res == QDialog::Accepted) {
      QList<int> typeIndex;
      QList<int> tenseIndex;
      QList<int> usageIndex;
      QList<int> lessonIndex;
      QStringList new_typeStr;
      QStringList new_tenseStr;
      QStringList new_usageStr;
      QStringList new_lessonStr;
      QList<int> new_lessoninquery;

      doc->enableSorting(ddlg.getSorting());

      doc->setTitle(ddlg.getTitle() );
      doc->setAuthor(ddlg.getAuthor() );
      doc->setLicense(ddlg.getLicense() );
      doc->setDocRemark(ddlg.getDocRemark() );

      slotStatusMsg(i18n("Updating lesson indices..."));
      QApplication::setOverrideCursor( Qt::WaitCursor );

      ddlg.getLesson(lessons, lessonIndex);
      ddlg.getTypeNames(new_typeStr, typeIndex);
      ddlg.getTenseNames(new_tenseStr, tenseIndex);
      ddlg.getUsageLabels(new_usageStr, usageIndex);

      new_lessoninquery = old_lessoninquery;
      LessOptPage::cleanUnused(doc, lessons, lessonIndex, old_lessons, new_lessoninquery);
      for (int i = 1; i < lessons->count(); i++)
        new_lessonStr.push_back(lessons->text(i));

      slotStatusMsg(i18n("Updating type indices..."));
      TypeOptPage::cleanUnused(doc, typeIndex, old_types);
      QueryManager::setTypeNames (new_typeStr);

      slotStatusMsg(i18n("Updating tense indices..."));
      TenseOptPage::cleanUnused(doc, tenseIndex, old_tenses);
      ///@todo port KEduVocConjugation::setTenseNames (new_tenseStr);

      slotStatusMsg(i18n("usage (area) of an expression",
                         "Updating usage label indices..."));
      UsageOptPage::cleanUnused(doc, usageIndex, old_usages);
      UsageManager::setUsageNames (new_usageStr);

      doc->setTypeDescriptions(new_typeStr);
      doc->setTenseDescriptions(new_tenseStr);
      doc->setUsageDescriptions(new_usageStr);
      doc->setLessonDescriptions(new_lessonStr);
      doc->setLessonsInQuery(new_lessoninquery);
      querymanager.setLessonItems(new_lessoninquery);

      doc->setModified();
      view->getTable()->updateContents();

      setWindowTitle(kapp->makeStdCaption(doc->title(), false, doc->isModified()));

      QApplication::restoreOverrideCursor();
      slotStatusMsg(IDS_DEFAULT);
   }
}


void kvoctrainApp::slotDocPropsLang ()
{
   DocPropsLangDlg ldlg (doc, &langset);
   int res = ldlg.exec();

   if (res == QDialog::Accepted) {
      for (int i = 0; i < doc->numIdentifiers(); i++) {
        doc->setArticle(i, ldlg.getArticle(i) );
        doc->setConjugation(i, ldlg.getConjugation(i) );
      }

      doc->setModified();
      view->getTable()->updateContents();
      setWindowTitle(kapp->makeStdCaption(doc->title(), false, doc->isModified()));

      slotStatusMsg(IDS_DEFAULT);
   }
}


void kvoctrainApp::slotModifiedDoc(bool /*mod*/)
{
  setWindowTitle(kapp->makeStdCaption(doc->title(), false, doc->isModified()));
  slotStatusMsg(IDS_DEFAULT);
}


bool kvoctrainApp::hasSelection()
{
  int num = view->getTable()->numSelections();
  if (num < 1) return false;
  if (num > 1) return true;
  Q3TableSelection ts = view->getTable()->selection(0);
  return (ts.bottomRow() - ts.topRow()) > 0;
}


void kvoctrainApp::slotRemoveRow()
{
  if (!hasSelection()) {
    if( KMessageBox::Continue == KMessageBox::warningContinueCancel(this,
                  i18n("Do you really want to delete the selected entry?\n"),
                  kapp->makeStdCaption(""),KStdGuiItem::del()))
    {
      KVocTrainTable *table = view->getTable();
      doc->removeEntry(table->currentRow());
      doc->setModified();
      table->updateContents();
    }
  }
  else {
    if(KMessageBox::Continue == KMessageBox::warningContinueCancel(this,
                  i18n("Do you really want to delete the selected range?\n"),
                  kapp->makeStdCaption(""),KStdGuiItem::del()))
    {
      KVocTrainTable *table = view->getTable();

      int numRows = table->numRows();
      // Must count backwards otherwise entry-numbering goes wrong when
      // deleting.
      for (int i = numRows-1; i >= 0; i--)
        if (table->isRowSelected(i) )
          doc->removeEntry(i);
      doc->setModified();
      table->updateContents();
    }
  }
  editRemoveSelectedArea->setEnabled(view->getTable()->numRows() > 0);
}


void kvoctrainApp::slotAppendRow ()
{
  KEduVocExpression expr;
  expr.setLesson(act_lesson);
  doc->appendEntry(&expr);
  doc->setModified();
  int row = doc->numEntries()-1;
  view->getTable()->setRowHeight(row, view->getTable()->fontMetrics().lineSpacing() );
  view->getTable()->setCurrentRow(row, KV_COL_ORG);
  view->getTable()->updateContents(row, KV_COL_ORG);
  view->getTable()->clearSelection();
  view->getTable()->selectRow(row);
  editRemoveSelectedArea->setEnabled(view->getTable()->numRows() > 0);
}


void kvoctrainApp::keyReleaseEvent( QKeyEvent *e )
{
  switch( e->key() ) {
    case Qt::Key_Shift:  shiftActive = false;
    break;

    case Qt::Key_Alt:  altActive = false;
    break;

    case Qt::Key_Control:  controlActive = false;
    break;
  }
}


void kvoctrainApp::keyPressEvent( QKeyEvent *e )
{
  ///@todo
  ///port
  /*
  controlActive = (e->state() & ControlButton) != 0;
  shiftActive = (e->state() & ShiftButton) != 0;
  altActive = (e->state() & AltButton) != 0;
  */
  switch( e->key() ) {
    case Qt::Key_Plus:
      if (controlActive) {
        int less = lessons->currentItem();
        if (less == lessons->count()-1)
          lessons->setCurrentItem(0);
        else
          lessons->setCurrentItem(less+1);
        slotChooseLesson(lessons->currentItem());
      }
    break;

    case Qt::Key_Minus:
      if (controlActive) {
        int less = lessons->currentItem();
        if (less == 0)
          lessons->setCurrentItem(lessons->count()-1);
        else
          lessons->setCurrentItem(less-1);
        slotChooseLesson(lessons->currentItem());
      }
    break;

    case Qt::Key_Shift:  shiftActive = true;
    break;

    case Qt::Key_Alt:  altActive = true;
    break;

    case Qt::Key_Control:  controlActive = true;
    break;

    case Qt::Key_Tab:
      if (view->getTable()->hasFocus() )  {
        searchLine->setFocus();
        searchLine->selectAll();
      }
      else
        view->getTable()->setFocus();
    break;

    case Qt::Key_Backtab:
      if (searchLine->hasFocus() )
        view->getTable()->setFocus();
      else {
        searchLine->setFocus();
        searchLine->selectAll();
      }
    break;

    case Qt::Key_Delete:
      slotRemoveRow();
    break;

    case Qt::Key_Insert: {
      slotAppendRow();
    }
    break;

    default:
      bool found = false;
      if (!found)
        e->ignore();
  }
  slotStatusMsg(IDS_DEFAULT);
}


void kvoctrainApp::slotChooseLesson(int idx)
{
  act_lesson = idx;
  doc->setCurrentLesson(idx);
  doc->setModified(true);
}


void kvoctrainApp::slotCreateLesson(int header)
{
  QList<int> sel;
  doc->setModified();
  for (int i = 0; i < view->getTable()->count(); i++) {
    KEduVocExpression *kv = view->getTable()->getRow(i);
    kv->setLesson(0);
    if (kv->grade(header) > THRESH_LESSON && !kv->translation(header).isEmpty() )
      sel.push_back(i);
  }

  int cnt = 0;
  while (cnt < MAX_LESSON && sel.size() != 0) {
    int nr = random.getLong(sel.size());
    KEduVocExpression *kv = view->getTable()->getRow(sel[nr]);
    // include non-lesson and non-empty string
    if (kv->lesson() == 0) {
      kv->setLesson(1);
      sel.erase (sel.begin() + nr);
      cnt++;
    }
  }
  view->getTable()->updateContents();
}


void kvoctrainApp::slotShowStatist()
{
   StatistikDlg sdlg (langset, doc);
   sdlg.exec();
}


void kvoctrainApp::slotCleanVocabulary ()
{
   prepareProgressBar();
   QApplication::setOverrideCursor( Qt::WaitCursor );
   int num = doc->cleanUp();
   QApplication::restoreOverrideCursor();
   removeProgressBar();

   slotStatusMsg(IDS_DEFAULT);

   if (num != 0) {
     view->setView(doc, langset);
     QString s =
        i18n("1 entry with the same content has been found and removed.",
             "%n entries with the same content have been found and removed.", num);

     KMessageBox::information(this,
       s,
       kapp->makeStdCaption(i18n("Clean Up")));
   }
}


void kvoctrainApp::slotCreateRandom()
{
   bool ok = FALSE;
   int res = KInputDialog::getInteger(i18n( "Entries in Lesson" ),
                i18n( "Enter number of entries in lesson:" ), Prefs::entriesPerLesson(), 1, 1000, 1, &ok, this );
   if (!ok)
     return;

   Prefs::setEntriesPerLesson(res);

   slotStatusMsg(i18n("Creating random lessons..."));
   QApplication::setOverrideCursor( Qt::WaitCursor );

   QList<KEduVocExpression*> randomList;
   for (int i = 0; i < doc->numEntries(); i++) {
     KEduVocExpression *expr = doc->entry(i);
     if (expr->lesson() == 0)
       randomList.push_back(expr);
   }

   if (randomList.size () != 0) {
     int less_no = lessons->count() /* +1 anyway */ ;
     QString s;
     s.setNum (less_no);
     s.insert (0, "- ");
     lessons->insertItem (s);
     int less_cnt = Prefs::entriesPerLesson();
     while (randomList.size () != 0) {
       if (--less_cnt <= 0) {
         less_cnt = Prefs::entriesPerLesson();
         less_no++;
         s.setNum (less_no);
         s.insert (0, "- ");
         lessons->insertItem (s);
       }
       int nr = random.getLong(randomList.size());
       randomList[nr]->setLesson(less_no);
       randomList.erase(randomList.begin() + nr);
     }

     QStringList new_lessonStr;
     for (int i = 1; i < lessons->count(); i++)
       new_lessonStr.push_back(lessons->text(i));
     doc->setLessonDescriptions(new_lessonStr);
     view->getTable()->updateContents();
     doc->setModified ();
   }
   QApplication::restoreOverrideCursor();
   slotStatusMsg(IDS_DEFAULT);
}


void kvoctrainApp::slotGeneralOptions()
{
  slotGeneralOptionsPage(-1);
}


void kvoctrainApp::slotGeneralOptionsPage(int index)
{
  KVocTrainPrefs* dialog = new KVocTrainPrefs(langset, doc, lessons, &querymanager, this, "settings",  Prefs::self() );
  connect(dialog, SIGNAL(settingsChanged()), this, SLOT(slotApplyPreferences()));
  if (index >= 0)
    dialog->selectPage(index);
  dialog->show();
}


void kvoctrainApp::slotApplyPreferences()
{
  if (pron_label)
    pron_label->setFont(Prefs::iPAFont());
  view->getTable()->setFont(Prefs::tableFont());
  view->getTable()->updateContents();

  readLanguages();
  // update header buttons
  for (int i = 0; i < (int) doc->numIdentifiers(); i++)
  {
    QString sid = i>0 ? doc->identifier(i): doc->originalIdentifier();
    int idx = langset.indexShortId(sid);
    QString pm = "";
    QString lid = sid;
    if (idx >= 0)
    {
      lid = langset.longId(idx);
      pm = langset.PixMapFile(idx);
    }
    view->setHeaderProp(i + KV_EXTRA_COLS, lid, pm);
  }
}


void kvoctrainApp::slotAppendLang(int header_and_cmd)
{
   int lang_id = (header_and_cmd >> 16) & 0xFF;

   if (lang_id == 0xFF) {
     QString msg = i18n("To append a new language which is not listed in "
                        "the submenu, you must first add its data in the "
                        "general options dialog.\n"
                        "Should this dialog be invoked now?");
    if( KMessageBox::Yes == KMessageBox::questionYesNo(this,
                  msg,
                  kapp->makeStdCaption(""),i18n("Invoke Dialog"), i18n("Do Not Invoke")))
     {
       slotGeneralOptionsPage(1);
     }
     return;
   }

   if (lang_id >= (int) langset.size())
     return;
   doc->appendIdentifier("");
   int num = doc->numEntries()-1;
   for (int i = 0; i < (int) num; i++) {
      KEduVocExpression *expr = doc->entry(i);
      expr->setType (num, expr->type(0));
   }

   doc->setIdentifier(doc->numIdentifiers()-1, langset.shortId(lang_id));
   view->setView(doc, langset);
   doc->setModified();
}


void kvoctrainApp::slotInitSearch()
{
  searchpos = 0;
  searchstr = "";
}


void kvoctrainApp::slotSearchNext()
{
  slotResumeSearch(searchstr);
}


void kvoctrainApp::slotResumeSearch(const QString& s)
{
  if (s.length() == 0) {
    slotInitSearch();
    return;
  }

  slotStatusMsg(i18n("Searching expression..."));
  QApplication::setOverrideCursor( Qt::WaitCursor );

  // new word or shortend word
  if (s.length() < searchstr.length() )
    searchpos = 0;

  // search in current col from current row till end
  // SHIFT means start search from beginning of word
  bool word_beg = controlActive;
  int idx = doc->search(s, view->getTable()->currentColumn()-KV_EXTRA_COLS, searchpos, -1, word_beg);
  if (idx >= 0) {
    view->getTable()->clearSelection();
    view->getTable()->setCurrentRow(idx, view->getTable()->currentColumn());
    searchpos = idx+1;
  }
  else { // try again from beginning up to current pos
    int idx = doc->search(s, view->getTable()->currentColumn()-KV_EXTRA_COLS, 0, searchpos, word_beg);
    if (idx >= 0) {
      view->getTable()->clearSelection();
      view->getTable()->setCurrentRow(idx, view->getTable()->currentColumn());
      searchpos = idx+1;
    }
    else
      searchpos = 0;
  }

  searchstr = s;
  QApplication::restoreOverrideCursor();
  slotStatusMsg(IDS_DEFAULT);
}


void kvoctrainApp::slotStatusMsg(const QString &/*text*/)
{
//
}


void kvoctrainApp::aboutToShowLearn()
{
  learn_menu->clear();

  QList<QString> set_names;
  for (int i = 0; i < (int) langset.size(); i++) {
    if(langset.longId(i).isEmpty() )
      set_names.push_back(langset.shortId(i));
    else
      set_names.push_back(langset.longId(i));
  }

  QList<QString> main_names;
  for (int j = 0; j < (int) doc->numIdentifiers(); j++) {
   int i;
   QString did = j == 0 ? doc->originalIdentifier() : doc->identifier(j);
   if ((i = langset.indexShortId(did)) >= 0)
     main_names.push_back(langset.longId(i));
   else
     main_names.push_back(did);
  }

  for (int header = 0; header < (int) doc->numIdentifiers(); header++) {
    // show pixmap and long name if available
    int j;
    header_m = new QMenu();
    if (header != 0 ) {
      header_m->insertItem(SmallIconSet("run_query"), i18n("Create Random &Query"), (header << 16) | IDH_START_QUERY);
      header_m->insertItem(SmallIconSet("run_multi"), i18n("Create &Multiple Choice"), (header << 16) | IDH_START_MULTIPLE);

      header_m->setItemEnabled((header << 16) | IDH_START_MULTIPLE, doc->numIdentifiers() > 1);
      header_m->setItemEnabled((header << 16) | IDH_START_QUERY,  doc->numIdentifiers() > 1);
      header_m->insertSeparator();

      header_m->insertItem(i18n("&Verbs"), (header << 16) | IDH_START_VERB);
      header_m->insertItem(i18n("&Articles"), (header << 16) | IDH_START_ARTICLE);
      header_m->insertItem(i18n("&Comparison Forms"), (header << 16) | IDH_START_ADJECTIVE);
      header_m->insertSeparator();
      header_m->insertItem(i18n("S&ynonyms"), (header << 16) | IDH_START_SYNONYM);
      header_m->insertItem(i18n("A&ntonyms"), (header << 16) | IDH_START_ANTONYM);
      header_m->insertItem(i18n("E&xamples"), (header << 16) | IDH_START_EXAMPLE);
      header_m->insertItem(i18n("&Paraphrase"), (header << 16) | IDH_START_PARAPHRASE);
    }
    else {
      QMenu *query_m = new QMenu();
      QMenu *multiple_m = new QMenu();

      for (int i = 1; i < (int) doc->numIdentifiers(); i++) {
        // show pixmap and long name if available
        int j;
        if((j = langset.indexShortId(doc->identifier(i))) >= 0
           && !langset.PixMapFile(j).isEmpty()
           && !langset.longId(j).isEmpty() ) {
          query_m->insertItem(QPixmap(langset.PixMapFile(j)), i18n("From %1").arg(main_names[i]),
              (i << (16+8)) |  IDH_START_QUERY);  // hack: IDs => header-ids + cmd
        }
        else {
          query_m->insertItem(i18n("From %1").arg(doc->identifier(i)), (i << (16+8)) |  IDH_START_QUERY);
        }
      }

      header_m->insertItem(SmallIconSet("run_query"), i18n("Create Random &Query"), query_m, (3 << 16) | IDH_NULL);
      connect (query_m, SIGNAL(activated(int)), this, SLOT(slotHeaderCallBack(int)));
      connect (query_m, SIGNAL(highlighted(int)), this, SLOT(slotHeaderStatus(int)));

      for (int i = 1; i < (int) doc->numIdentifiers(); i++) {
        // show pixmap and long name if available
        int j;
        if((j = langset.indexShortId(doc->identifier(i))) >= 0
           && !langset.PixMapFile(j).isEmpty()
           && !langset.longId(j).isEmpty() ) {
          multiple_m->insertItem(QPixmap(langset.PixMapFile(j)), i18n("From %1").arg(main_names[i]),
              (i << (16+8)) |  IDH_START_MULTIPLE);  // hack: IDs => header-ids + cmd
        }
        else {
          multiple_m->insertItem(i18n("From %1").arg(doc->identifier(i)), (i << (16+8)) |  IDH_START_MULTIPLE);
        }
      }
      header_m->insertItem(SmallIconSet("run_multi"), i18n("Create &Multiple Choice"), multiple_m, (4 << 16) | IDH_NULL);
      header_m->insertSeparator();

      header_m->insertItem(i18n("Train &Verbs"), (header << 16) | IDH_START_VERB);
      header_m->insertItem(i18n("&Article Training"), (header << 16) | IDH_START_ARTICLE);
      header_m->insertItem(i18n("&Comparison Training"), (header << 16) | IDH_START_ADJECTIVE);
      header_m->insertSeparator();
      header_m->insertItem(i18n("&Synonyms"), (header << 16) | IDH_START_SYNONYM);
      header_m->insertItem(i18n("&Antonyms"), (header << 16) | IDH_START_ANTONYM);
      header_m->insertItem(i18n("E&xamples"), (header << 16) | IDH_START_EXAMPLE);
      header_m->insertItem(i18n("&Paraphrase"), (header << 16) | IDH_START_PARAPHRASE);

      connect (multiple_m, SIGNAL(activated(int)), this, SLOT(slotHeaderCallBack(int)));
      connect (multiple_m, SIGNAL(highlighted(int)), this, SLOT(slotHeaderStatus(int)));
    }

    connect (header_m, SIGNAL(activated(int)), this, SLOT(slotHeaderCallBack(int)));
    connect (header_m, SIGNAL(highlighted(int)), this, SLOT(slotHeaderStatus(int)));

    QString id = header == 0 ? doc->originalIdentifier() : doc->identifier(header);
    if((j = langset.indexShortId(id)) >= 0
       && !langset.PixMapFile(j).isEmpty()
       && !langset.longId(j).isEmpty() ) {
      learn_menu->insertItem(QPixmap(langset.PixMapFile(j)), main_names[header], header_m, IDH_NULL);
    }
    else {
      learn_menu->insertItem(id, header_m, IDH_NULL);
    }
  }

  learn_menu->insertSeparator();
  learn_menu->insertItem(SmallIconSet("run_query"), i18n("Resume &Query"), ID_RESUME_QUERY );
  learn_menu->insertItem(SmallIconSet("run_multi"), i18n("Resume &Multiple Choice"), ID_RESUME_MULTIPLE );

  learn_menu->setItemEnabled(ID_RESUME_QUERY,  query_num != 0);
  learn_menu->setItemEnabled(ID_RESUME_MULTIPLE,  query_num != 0);

}

void kvoctrainApp::aboutToShowVocabAppendLanguage()
{
  if (doc != 0)
  {
    vocabAppendLanguage->clear();
    QMenu * add_m = vocabAppendLanguage->popupMenu();

    QStringList names;
    for (int i = 0; i < (int) langset.size(); i++)
    {
      if(langset.longId(i).isEmpty() )
        names.append(langset.shortId(i));
      else
        names.append(langset.longId(i));
    }

    for (int i = 0; i < (int) langset.size(); i++)
    {
      if(!langset.PixMapFile(i).isEmpty() && !langset.longId(i).isEmpty())
        add_m->insertItem(QPixmap(langset.PixMapFile(i)), names[i], (i << 16) | IDH_APPEND);
      else
        add_m->insertItem(names[i], (i << 16) | IDH_APPEND);
    }

    add_m->insertItem(i18n("Another Language..."), (0xFF << 16) | IDH_APPEND);
  }
}


void kvoctrainApp::aboutToShowVocabSetLanguage()
{
  if (doc != 0)
  {
    vocabSetLanguage->clear();
    QMenu * set_m = vocabSetLanguage->popupMenu();

    QStringList names;
    for (int i = 0; i < (int) langset.size(); i++)
    {
      if(langset.longId(i).isEmpty() )
        names.append(langset.shortId(i));
      else
        names.append(langset.longId(i));
    }

    for (int header = 0; header < doc->numIdentifiers(); ++header )
    {
      // select one of the available languages for the column
      QMenu *langs_m = new QMenu();
        // hack: ID => header-id + language

      for (int i = 0; i < (int) langset.size(); i++) {
        if(!langset.PixMapFile(i).isEmpty() && !langset.longId(i).isEmpty())
          langs_m->insertItem(QPixmap(langset.PixMapFile(i)), names[i], (header << 16) | (i << (16+8)) | IDH_SET_LANG);
        else
          langs_m->insertItem(names[i], (header << 16) | (i << (16+8)) | IDH_SET_LANG);
      }

      connect (langs_m, SIGNAL(activated(int)), this, SLOT(slotSetHeaderProp(int)));
      connect (langs_m, SIGNAL(highlighted(int)), this, SLOT(slotHeaderStatus(int)));

      if (header == 0)
        set_m->insertItem(i18n("&Original"), langs_m, (2 << 16) | IDH_NULL);
      else {
        if (doc->numIdentifiers() <= 2)
          set_m->insertItem(i18n("&Translation"), langs_m, (2 << 16) | IDH_NULL);
        else
          set_m->insertItem(i18n("&%1. Translation").arg(header), langs_m, (2 << 16) | IDH_NULL);
      }
    }
  }
}


void kvoctrainApp::aboutToShowVocabRemoveLanguage()
{
  if (doc != 0)
  {
    vocabRemoveLanguage->clear();
    QMenu * remove_m = vocabRemoveLanguage->popupMenu();

    QStringList names;
    for (int j = 1; j < (int) doc->numIdentifiers(); j++)
    {
      int i;
      if ((i = langset.indexShortId(doc->identifier(j))) >= 0)
        names.append(langset.longId(i));
      else
        names.append(doc->identifier(j));
    }

    for (int i = 1; i < (int) doc->numIdentifiers(); i++)
    {
      // show pixmap and long name if available
      int j;
      if((j = langset.indexShortId(doc->identifier(i))) >= 0
          && !langset.PixMapFile(j).isEmpty()
          && !langset.longId(j).isEmpty() )
        remove_m->insertItem(QPixmap(langset.PixMapFile(j)), names[i-1], (i << 16) |  IDH_REMOVE);  // hack: IDs => header-ids + cmd
      else
        remove_m->insertItem(doc->identifier(i), (i << 16) | IDH_REMOVE);
    }
  }
}


void kvoctrainApp::slotStatusHelpMsg(const QString &text)
{
  ///////////////////////////////////////////////////////////////////
  // change status message of whole statusbar temporary (text, msec)
  if (pbar == 0 || !pbar->isVisible() )
    statusBar()->message(text, 3000);
}

void kvoctrainApp::slotFilePrint()
{
  slotStatusMsg(i18n("Printing..."));
  KPrinter printer;
  printer.setFullPage(true);
  if (printer.setup(this))
  {
    view->print(&printer);
  }

  slotStatusMsg(i18n("Ready"));
}

#include "kvoctrain.moc"


