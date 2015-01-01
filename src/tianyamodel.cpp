#include "syncobj.hpp"
#include "tianyamodel.hpp"

TianyaModel::TianyaModel(QObject* parent)
	: QAbstractTableModel(parent)
{
}

void TianyaModel::update_tianya_list(const std::multimap< int, list_info >& ordered_info)
{
	std::unique_lock<std::mutex> l(m_lock);
	Q_EMIT beginResetModel();

	m_list_info.clear();
	m_list_info.reserve(ordered_info.size());

	// 内部拷贝一份数据
	for (auto it = ordered_info.rbegin(); it!= ordered_info.rend(); ++it)
	{
		m_list_info.push_back(it->second);
	}

	Q_EMIT endResetModel();
}

QVariant TianyaModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal) {
            switch (section)
            {
            case 0:
                return QStringLiteral("标题");
            case 1:
                return QStringLiteral("作者");
            case 2:
                return QStringLiteral("点击");
            case 3:
                return QStringLiteral("回复");
            case 4:
                return QStringLiteral("时间");
            case 5:
                return QStringLiteral("链接");
            }
        }
    }
    return QVariant();

    return QAbstractItemModel::headerData(section, orientation, role);
}


QVariant TianyaModel::data(const QModelIndex& index, int role) const
{
	std::unique_lock<std::mutex> l(m_lock);
	// 向 GUI 提供格式化好的数据
	auto size = m_list_info.size();

	if (role == Qt::DisplayRole && index.row() < size)
	{
		if(index.parent().isValid())
			return QVariant();

		const list_info& info = m_list_info[index.row()];
		 //index.row()

		switch(index.column())
		{
			case 0:
				return  QString::fromStdWString(info.title);
			case 1:
				return  QString::fromStdWString(info.author);
			case 2:
				return  info.hits;
			case 3:
				return  info.replys;
			case 4:
				return  QString::fromStdString(info.post_time);
			case 5:
				return  QString("<a href=\"%1\">%2</a>").arg(QString::fromStdString(info.post_url)).arg(QString::fromStdString(info.post_url));
			default:
				return QVariant();
		}
	}

	return QVariant();
}

int TianyaModel::columnCount(const QModelIndex& parent) const
{
	if(parent.isValid())
		return 0;
	return 6;
}

int TianyaModel::rowCount(const QModelIndex& parent) const
{
	if(parent.isValid())
		return 0;
	std::unique_lock<std::mutex> l(m_lock);

	auto size = m_list_info.size();

	return size;
}
