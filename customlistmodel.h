#ifndef CUSTOMLISTMODEL_H
#define CUSTOMLISTMODEL_H

#include <QStringListModel>
#include <QStringList>
class CustomListModel : public QStringListModel
{
public:
    explicit CustomListModel(QObject *parent = nullptr);

    // QAbstractItemModel interface
    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);

private:
    QSet<QPersistentModelIndex> checkedItems;
public slots:
   // readDataFrom
};

#endif // CUSTOMLISTMODEL_H
