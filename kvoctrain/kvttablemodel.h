//
// C++ Interface: kvttablemodel
//
// Description: 
//
//
// Author:  (C) 2006 Peter Hedlund <peter.hedlund@kdemail.net>
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef KVTTABLEMODEL_H
#define KVTTABLEMODEL_H

#include <QAbstractTableModel>

#include <keduvocdocument.h>

/**
  @author Peter Hedlund <peter.hedlund@kdemail.net>
*/
class KVTTableModel : public QAbstractTableModel
{
Q_OBJECT
public:
  KVTTableModel(QObject *parent = 0);

  void setDocument(KEduVocDocument * doc);

  int rowCount(const QModelIndex &parent) const;
  int columnCount(const QModelIndex &parent) const;

  QVariant data(const QModelIndex &index, int role) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;

  Qt::ItemFlags flags(const QModelIndex &index) const;
  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
  bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole);

private:
  KEduVocDocument * m_doc;
};

#endif
