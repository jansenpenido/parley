/***************************************************************************

    $Id$

                     query dialog for articles

    -----------------------------------------------------------------------

    begin                : Fri Dec 3 18:28:18 1999

    copyright            : (C) 1999-2001 Ewald Arnold
                           (C) 2001 The KDE-EDU team
    email                : kvoctrain@ewald-arnold.de

    -----------------------------------------------------------------------

    $Log$
    Revision 1.4  2001/10/28 09:17:06  arnold
    replaced qtarch dialog files with qtdesigner

    Revision 1.3  2001/10/17 21:41:15  waba
    Cleanup & port to Qt3, QTableView -> QTable
    TODO:
    * Fix actions that work on selections
    * Fix sorting
    * Fix language-menu

    Revision 1.2  2001/10/13 11:45:29  coolo
    includemocs and other smaller cleanups. I tried to fix it, but as it's still
    qt2 I can't test :(

    Revision 1.1  2001/10/05 15:45:05  arnold
    import of version 0.7.0pre8 to kde-edu


 ***************************************************************************

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "ArtQueryDlg.h"
#include "QueryDlgBase.h"
#include "MyProgress.h"

#include <kv_resource.h>
#include <kvoctraindoc.h>
#include <QueryManager.h>
#include <eadebug.h>
#include <compat_2x.h>

#include <kapp.h> 

#include <qtimer.h>
#include <qkeycode.h>
#include <qkeycode.h>
#include <qradiobutton.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>



ArtQueryDlg::ArtQueryDlg
(
        SpecFont_t *font,
        QString type,
        int entry,
        int col,
        int query_cycle,
        int query_num,
        int query_startnum,
        kvoctrainExpr *exp,
        kvoctrainDoc  *doc,
        const Article &articles,
        int   mqtime,
        bool _show,
        kvq_timeout_t type_to,
        QWidget *parent,
        char    *name)
	:
	ArtQueryDlgForm( parent, name, true ),
        QueryDlgBase()
{
   qtimer = 0;

   artGroup->insert (natural);
   artGroup->insert (male);
   artGroup->insert (rb_fem);

   connect( stop_it, SIGNAL(clicked()), SLOT(stopItClicked()) );
   connect( dont_know, SIGNAL(clicked()), SLOT(dontKnowClicked()) );
   connect( know_it, SIGNAL(clicked()), SLOT(knowItClicked()) );
   connect( verify, SIGNAL(clicked()), SLOT(verifyClicked()) );
   connect( show_all, SIGNAL(clicked()), SLOT(showAllClicked()) );
   connect( b_edit, SIGNAL(clicked()), SLOT(editClicked()) );
   connect( natural, SIGNAL(clicked()), SLOT(slotNaturalClicked()) );
   connect( male, SIGNAL(clicked()), SLOT(slotMaleClicked()) );
   connect( rb_fem, SIGNAL(clicked()), SLOT(slotFemClicked()) );

   setCaption(kapp->makeStdCaption(i18n("Article training")));
   setQuery (type, entry, col,
             query_cycle, query_num, query_startnum,
             exp, doc, articles, mqtime, _show, type_to);
   setIcon (QPixmap (EA_KDEDATADIR("",  "kvoctrain/mini-kvoctrain.xpm" )));

   if (font != 0 && font->specfont) {
     QFont specfont = font->limitedFont();
     male->setFont (specfont);
     rb_fem->setFont (specfont);
     natural->setFont (specfont);
     orgField->setFont (specfont);
   }
}


void ArtQueryDlg::setQuery(QString type,
                           int entry,
                           int col,
                           int q_cycle,
                           int q_num,
                           int q_start,
                           kvoctrainExpr *exp,
                           kvoctrainDoc  *doc,
                           const Article &art,
                           int mqtime,
                           bool _show,
                           kvq_timeout_t type_to)
{
   type_timeout = type_to;
   kv_exp = exp;
   kv_doc = doc;
   q_row = entry;
   q_ocol = col;
   showCounter = _show,
   timebar->setEnabled(showCounter);
   timelabel->setEnabled(showCounter);
   show_all->setDefault(true);
   articles = art;

   QString s;
   if (col == 0)
     s = exp->getOriginal().stripWhiteSpace();
   else
     s = exp->getTranslation(q_ocol).stripWhiteSpace();

   QString def, indef;
   bool removed = false;

   articles.female(def, indef);
   rb_fem->setText (i18n("&female:\t")+def+" / "+indef);
   rb_fem->setEnabled (!QString(def+indef).isEmpty() );
   if (!removed && s.find(def+" ") == 0) {
     s.remove (0, def.length()+1);
     removed = true;
   }
   if (!removed && s.find(indef+" ") == 0) {
     s.remove (0, indef.length()+1);
     removed = true;
   }

   articles.male(def, indef);
   male->setText (i18n("&male:\t")+def+" / "+indef);
   male->setEnabled (!QString(def+indef).isEmpty() );
   if (!removed && s.find(def+" ") == 0) {
     s.remove (0, def.length()+1);
     removed = true;
   }
   if (!removed && s.find(indef+" ") == 0) {
     s.remove (0, indef.length()+1);
     removed = true;
   }

   articles.natural(def, indef);
   natural->setText (i18n("&natural:\t")+def+" / "+indef);
   natural->setEnabled (!QString(def+indef).isEmpty() );
   if (!removed && s.find(def+" ") == 0) {
     s.remove (0, def.length()+1);
     removed = true;
   }
   if (!removed && s.find(indef+" ") == 0) {
     s.remove (0, indef.length()+1);
     removed = true;
   }

   orgField->setText(s);
   s.setNum (q_cycle);
   progCount->setText (s);

   countbar->setData (q_start, q_start-q_num+1, true);
   countbar->repaint();

   if (mqtime >= 1000) { // more than 1000 milli-seconds
     if (qtimer == 0) {
       qtimer = new QTimer( this );
       connect( qtimer, SIGNAL(timeout()), this, SLOT(timeoutReached()) );
     }

     if (type_timeout != kvq_notimeout) {
       timercount = mqtime/1000;
       timebar->setData (timercount, timercount, false);
       timebar->repaint();
       qtimer->start(1000, TRUE);
     }
     else
       timebar->setEnabled(false);
   }
   else
     timebar->setEnabled(false);
}


void ArtQueryDlg::initFocus() const
{
  rb_fem->setFocus();
}


void ArtQueryDlg::showAllClicked()
{
  resetButton(rb_fem);
  resetButton(male);
  resetButton(natural);

  if (kv_exp->getType (q_ocol) == QM_NOUN  QM_TYPE_DIV  QM_NOUN_F) {
    rb_fem->setChecked (true);
    verifyButton(rb_fem, true);
  }
  else if (kv_exp->getType (q_ocol) == QM_NOUN  QM_TYPE_DIV  QM_NOUN_M) {
    male->setChecked (true);
    verifyButton(male, true);
  }
  else if (kv_exp->getType (q_ocol) == QM_NOUN  QM_TYPE_DIV  QM_NOUN_S) {
    natural->setChecked (true);
    verifyButton(natural, true);
  }
  else
    ;
  dont_know->setDefault(true);
}


void ArtQueryDlg::showMoreClicked()
{
}


void ArtQueryDlg::verifyClicked()
{
  bool known = false;
  if (kv_exp->getType (q_ocol) == QM_NOUN  QM_TYPE_DIV  QM_NOUN_F)
    known = rb_fem->isChecked ();
  else if (kv_exp->getType (q_ocol) == QM_NOUN  QM_TYPE_DIV  QM_NOUN_M)
    known = male->isChecked ();
  else if (kv_exp->getType (q_ocol) == QM_NOUN  QM_TYPE_DIV  QM_NOUN_S)
    known = natural->isChecked ();

  if (rb_fem->isChecked() ) {
    verifyButton(rb_fem, known);
    resetButton(male);
    resetButton(natural);
  }
  else if (male->isChecked() ) {
    verifyButton(male, known);
    resetButton(rb_fem);
    resetButton(natural);
  }
  else if (natural->isChecked() ) {
    verifyButton(natural, known);
    resetButton(male);
    resetButton(rb_fem);
  }

  if (known)
//    know_it->setDefault(true);
    knowItClicked();
  else
    dont_know->setDefault(true);
}


void ArtQueryDlg::knowItClicked()
{
   done (Known);
}


void ArtQueryDlg::timeoutReached()
{
   if (timercount > 0) {
     timercount--;
     timebar->setData (-1, timercount, false);
     timebar->repaint();
     qtimer->start(1000, TRUE);
   }

   if (timercount <= 0) {
     timebar->setData (-1, 0, false);
     timebar->repaint();
     if (type_timeout == kvq_show) {
       showAllClicked();
       verifyClicked();
       dont_know->setDefault(true);
     }
     else if (type_timeout == kvq_cont)
       done (Timeout);
   }
}


void ArtQueryDlg::dontKnowClicked()
{
   done (Unknown);
}


void ArtQueryDlg::stopItClicked()
{
   done (StopIt);
}


void ArtQueryDlg::editClicked()
{

   if (qtimer != 0)
     qtimer->stop();

   emit sigEditEntry (q_row, KV_COL_ORG+q_ocol);
}


void ArtQueryDlg::keyPressEvent( QKeyEvent *e )
{
  switch( e->key() )
  {
    case Key_Escape:
      dontKnowClicked();
    break;
      
    case Key_Return:
    case Key_Enter:
      if (dont_know->isDefault() )
        dontKnowClicked();
      else if (know_it->isDefault() )
        knowItClicked();
      else if (show_all->isDefault() )
        showAllClicked();
      else if (verify->isDefault() )
        verifyClicked();
    break;
      
    default:
      e->ignore();
    break;
  }
}


void ArtQueryDlg::returnPressed()
{
}


void ArtQueryDlg::slotFemClicked()
{
  resetButton(rb_fem);
  resetButton(male);
  resetButton(natural);
  verifyClicked();
}


void ArtQueryDlg::slotMaleClicked()
{
  resetButton(male);
  resetButton(natural);
  resetButton(rb_fem);
  verifyClicked();
}


void ArtQueryDlg::slotNaturalClicked()
{
  resetButton(natural);
  resetButton(male);
  resetButton(rb_fem);
  verifyClicked();
}
#include "ArtQueryDlg.moc"
