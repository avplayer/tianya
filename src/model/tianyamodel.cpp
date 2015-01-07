#include <QUrl>
#include "tianyamodel.hpp"

TianyaModel::TianyaModel(QObject* parent)
	: QAbstractTableModel(parent)
{
	qRegisterMetaType<list_info>("list_info");
}

void TianyaModel::update_tianya_list(const list_info& hits_info)
{
	// 计算待插入位置
	auto insert_point = std::find_if(std::begin(m_list_info), std::end(m_list_info), [hits_info](const list_info& a){
		return a.hits < hits_info.hits;
	});

	// 避免重复
	if (insert_point != std::end(m_list_info))
	{
		if(std::end(m_list_info) != std::find_if(std::begin(m_list_info), std::end(m_list_info),
			[hits_info](const list_info& a){return a.post_url == hits_info.post_url;})
		){
			return;
		}
	}else if (m_list_info.size() && m_list_info.rbegin()->post_url == hits_info.post_url)
	{
		return;
	}

	auto offset = insert_point -  m_list_info.begin();

	beginInsertRows(QModelIndex(), offset, offset);
	m_list_info.insert(insert_point, hits_info);
	endInsertRows();
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
	// 向 GUI 提供格式化好的数据
	auto size = m_list_info.size();

	if (index.row() < size)
	{
		if(index.parent().isValid())
			return QVariant();

		const list_info& info = m_list_info[index.row()];
		 //index.row()
		if (role == Qt::DisplayRole)
		{
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
					return  QString::fromStdWString(info.post_time);
				case 5:
					return  QString::fromStdString(info.post_url);
				default:
					return QVariant();
			}
		}
		else if (role == Qt::UserRole+1)
		{
			return QUrl(QString::fromStdString(info.post_url));
		}
		else if (role == Qt::UserRole+2)
		{
			QVariant v;
			v.setValue(info);
			return v;
		}
		else if (role == Qt::UserRole+3)
		{
			switch(index.column())
			{
				case 0:
					return  QString::fromStdWString(info.title);
				case 1:
					return  QString::fromStdWString(info.author);
				default:
					return QVariant();
			}
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

	auto size = m_list_info.size();

	return size;
}

QModelIndex TianyaModel::index(int row, int column, const QModelIndex& parent) const
{
    return  createIndex(row, column);
}
