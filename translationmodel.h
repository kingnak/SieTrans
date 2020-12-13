#ifndef TRANSLATIONMODEL_H
#define TRANSLATIONMODEL_H

#include <QAbstractTableModel>
#include "iexcelhandler.h"

class TranslationModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    TranslationModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &idx) const override;
    int columnCount(const QModelIndex &idx) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &idx) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    void setSourceData(QList<IExcelHandler::ColumnData> data);
    void clearTranslationData();
    void addTranslations(QList<IExcelHandler::ColumnData> translations);
    void applyTranslate();

public slots:
    void clear();

private:
    struct Record {
        IExcelHandler::ColumnData data;
        bool translated;
    };
    QList<Record> m_data;
    QMap<QString, QString> m_trans;
};

#endif // TRANSLATIONMODEL_H
