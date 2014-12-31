#pragma once

#include <tianya_list.hpp>
#include <QAbstractItemModel>

class TianyaModel : public QAbstractTableModel
{
    Q_OBJECT

public:
	TianyaModel(tianya&);

public:
    virtual QVariant data(const QModelIndex& index, int role) const;
    virtual int columnCount(const QModelIndex& parent) const;
    virtual int rowCount(const QModelIndex& parent) const;
    virtual QModelIndex parent(const QModelIndex& child) const;

private:
};
