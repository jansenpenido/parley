/***************************************************************************

    Copyright 2007 Frederik Gladhorn <frederik.gladhorn@kdemail.net>

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
#include "containerview.h"

// Qt
#include <QHeaderView>

// Parley/collection
#include "containermodel.h"


ContainerView::ContainerView(QWidget *parent) : QTreeView(parent)
{
    header()->setStretchLastSection(false);
    header()->setVisible(false);

    setAlternatingRowColors(true);

    // show the actions added by addAction() as right click menu.
    setContextMenuPolicy(Qt::ActionsContextMenu);

    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::DragDrop);
}

void ContainerView::setModel(ContainerModel *model)
{
    QTreeView::setModel(model);
    m_model = model;

    header()->setSectionResizeMode(0, QHeaderView::Stretch);
    header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
}

void ContainerView::slotRename()
{
    edit(selectionModel()->currentIndex());
}
