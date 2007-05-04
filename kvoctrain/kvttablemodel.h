/***************************************************************************

                             kvttablemodel

    -----------------------------------------------------------------------

    copyright     : (C) 2006 Peter Hedlund <peter.hedlund@kdemail.net>

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

#ifndef KVTTABLEMODEL_H
#define KVTTABLEMODEL_H

#include <QAbstractTableModel>
#include <QPixmap>

#include <keduvocdocument.h>
#include <kvtlanguages.h>

#define KV_EXTRA_COLS    2   // add col for lesson number

#define KV_COL_LESS      0   // col: lesson number
#define KV_COL_MARK      1   // col: mark
#define KV_COL_ORG       2   // col: original
#define KV_COL_TRANS     3   // col: first translation

/**
  @author Peter Hedlund <peter.hedlund@kdemail.net>
*/

class KVTTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:

    enum KVTItemDataRole {
        LessonsRole = Qt::UserRole + 1,
        LessonRole,
        StateRole,
        GradeRole
    };

    KVTTableModel(QObject *parent = 0);

    void setDocument(KEduVocDocument * doc);
    KEduVocDocument * document() const
    {
        return m_doc;
    }

    void setLanguages(const KVTLanguages & languages);

    bool insertRows(int row, int count = 1, const QModelIndex &parent = QModelIndex());
    bool removeRows(int row, int count = 1, const QModelIndex &parent = QModelIndex());
    bool insertColumns(int column, int count = 1, const QModelIndex & parent = QModelIndex());
    bool removeColumns(int column, int count = 1, const QModelIndex & parent = QModelIndex());

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;

    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole);

    //void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

private:
    KEduVocDocument * m_doc;
    QPixmap m_pixInactive;
    QPixmap m_pixInQuery;
    KVTLanguages m_languages;

    friend class KVocTrainApp;
    friend class KVTStatisticsDialog;
};

#endif
