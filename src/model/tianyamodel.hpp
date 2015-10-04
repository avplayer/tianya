#pragma once

#include <tianya_list.hpp>
#include <QAbstractItemModel>

Q_DECLARE_METATYPE(list_info)

class TianyaModel : public QAbstractTableModel
{
	Q_OBJECT
public:

    explicit TianyaModel(QObject* parent = 0);
	virtual ~TianyaModel(){}

	Q_INVOKABLE void update_tianya_list(const list_info& hits_info);

public:
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
private:
	std::vector<list_info> m_list_info;
};
