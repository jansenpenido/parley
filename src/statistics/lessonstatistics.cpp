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

// Own
#include "lessonstatistics.h"

// Qt
#include <QApplication>
#include <QHeaderView>
#include <QItemDelegate>
#include <QPainter>

// KDE
#include <KLocalizedString>
#include <KMessageBox>
#include <KInputDialog>
#include <KAction>
#include <KActionCollection>

// Parley
#include "statisticslegendwidget.h"
#include "statisticsmodel.h"
#include "keduvoclesson.h"
#include "prefs.h"


using namespace Editor;


class GradeDelegate: public QItemDelegate
{
public:
    GradeDelegate(QObject *parent = 0)
        : QItemDelegate(parent) {
    }

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option,
		       const QModelIndex &index) const
    {
        QApplication::style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, 0);

        // empty lesson
        if (!index.data(StatisticsModel::TotalCount).toInt()) {
            return;
        }
        drawBackground(painter, option, index);
        painter->drawText(option.rect, Qt::AlignCenter,
			  QString("%1%").arg(index.data(StatisticsModel::TotalPercent).toInt()));
    }

protected:
    void drawBackground(QPainter *painter, const QStyleOptionViewItem &option,
			const QModelIndex &index) const
    {
        QList<QVariant> fractions = index.data(StatisticsModel::LegendFractions).toList();
        StatisticsLegendWidget::paintStatisticsBar(*painter, option.rect, fractions);
    }
};


// ----------------------------------------------------------------


LessonStatisticsView::LessonStatisticsView(QWidget *parent)
    : ContainerView(parent)
{
    header()->setVisible(true);
    header()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignBottom);

    // inherits context menu policy - so action will show up in right click menu
    KAction *removeGradesAction = new KAction(this);
    removeGradesAction->setText(i18n("Remove confidence levels from this unit"));
    removeGradesAction->setIcon(KIcon("edit-clear"));
    removeGradesAction->setWhatsThis(i18n("Remove confidence levels from this unit"));
    removeGradesAction->setToolTip(removeGradesAction->whatsThis());
    removeGradesAction->setStatusTip(removeGradesAction->whatsThis());

    connect(removeGradesAction, SIGNAL(triggered()), SLOT(removeGrades()));
    addAction(removeGradesAction);

    KAction *removeGradesChildrenAction = new KAction(this);
    removeGradesChildrenAction->setText(i18n("Remove confidence levels from this unit and all sub-units"));
    removeGradesChildrenAction->setIcon(KIcon("edit-clear"));
    removeGradesChildrenAction->setWhatsThis(i18n("Remove confidence level from this unit and all sub-units"));
    removeGradesChildrenAction->setToolTip(removeGradesChildrenAction->whatsThis());
    removeGradesChildrenAction->setStatusTip(removeGradesChildrenAction->whatsThis());

    connect(removeGradesChildrenAction, SIGNAL(triggered()), SLOT(removeGradesChildren()));
    addAction(removeGradesChildrenAction);
}

void LessonStatisticsView::setModel(Editor::ContainerModel *model)
{
    ContainerView::setModel(model);

    GradeDelegate *delegate = new GradeDelegate(this);
    for (int i = ContainerModel::FirstDataColumn; i < model->columnCount(QModelIndex()); i++) {
        setItemDelegateForColumn(i, delegate);
        setColumnWidth(i, 150);
    }
//    header()->resizeSections(QHeaderView::ResizeToContents);
    header()->setResizeMode(QHeaderView::Interactive);
    header()->setStretchLastSection(true);
}

void LessonStatisticsView::removeGrades()
{
    QModelIndex selectedIndex = selectionModel()->currentIndex();
    KEduVocLesson *lesson = static_cast<KEduVocLesson*>(selectedIndex.internalPointer());
    lesson->resetGrades(-1, KEduVocContainer::NotRecursive);
}

void LessonStatisticsView::removeGradesChildren()
{
    QModelIndex selectedIndex = selectionModel()->currentIndex();
    KEduVocLesson *lesson = static_cast<KEduVocLesson*>(selectedIndex.internalPointer());
    lesson->resetGrades(-1, KEduVocContainer::Recursive);
}

#include "lessonstatistics.moc"
