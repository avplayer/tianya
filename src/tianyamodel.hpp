#pragma once

#include <tianya_list.hpp>
#include <QAbstractItemModel>
#include <thread>
#include <mutex>

class TianyaModel : public QAbstractTableModel
{
	Q_OBJECT
public:

    explicit TianyaModel(QObject* parent = 0);

	void update_tianya_list(const std::multimap<int, list_info>& ordered_info);

public:
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    virtual int columnCount(const QModelIndex& parent) const Q_DECL_OVERRIDE;
    virtual int rowCount(const QModelIndex& parent) const Q_DECL_OVERRIDE;

private:
	mutable std::mutex m_lock;
	std::vector<list_info> m_list_info;
};
