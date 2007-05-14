/***************************************************************************

                        show document statistics

    -----------------------------------------------------------------------

    begin         : Sun Sep 19 20:50:53 MET 1999

    copyright     : (C) 1999-2001 Ewald Arnold <kvoctrain@ewald-arnold.de>
                    (C) 2001 The KDE-EDU team
                    (C) 2005-2007 Peter Hedlund <peter.hedlund@kdemail.net>

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

#ifndef StatisticsDialog_included
#define StatisticsDialog_included

#include <kpagedialog.h>

class KVTTableModel;
class KVTLanguageList;
class StatisticsPage;

class KVTStatisticsDialog : public KPageDialog
{
    Q_OBJECT
public:
    KVTStatisticsDialog(KVTLanguageList &languages, KVTTableModel *model, QWidget *parent);
    ~KVTStatisticsDialog();

protected slots:
    void slotApply();
    void accept();

private:
    KVTTableModel * m_model;
    QList<StatisticsPage *> pageList;
};

#endif // StatisticsDialog_included
