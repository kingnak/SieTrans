#ifndef TRANSLATIONMODEL_H
#define TRANSLATIONMODEL_H

#include <QSortFilterProxyModel>
#include "iexcelhandler.h"

class TranslationModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    TranslationModel(QObject *parent = nullptr);

    enum TranslationState {
        NotTranslated,
        ProvisionalTranslation,
        Translated,
    };

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
    QList<IExcelHandler::ColumnData> getTranslationData() const;
    QList<IExcelHandler::ColumnData> getTranslationData(QSet<TranslationState> states) const;
    void updateMultipleTranslationStates(QModelIndexList lst, TranslationState state);

    QMap<TranslationState, int> getTranlationStateCounts() const;

    enum {
        TranslationStateRole = Qt::UserRole
    };

public slots:
    void clear();

signals:
    void translationModified();
    void translationStateModified();

private:
    struct Record {
        IExcelHandler::ColumnData data;
        TranslationState translationState;
    };
    QList<Record> m_data;
    QMap<QString, QString> m_trans;
};

class TranslationFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    TranslationFilterModel(TranslationModel *source, QObject *parent = nullptr);

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

    void updateMultipleTranslationStates(QModelIndexList lst, TranslationModel::TranslationState state);
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

public slots:
    void setFilterString(const QString &filter) { m_str = filter; m_newChanges.clear(); invalidateFilter(); }
    void filterNonTranslated(bool f) { m_fNonTranslated = f; m_newChanges.clear(); invalidateFilter(); }
    void filterProvisional(bool f) { m_fProvisional = f; m_newChanges.clear(); invalidateFilter(); }
    void filterTranslated(bool f) { m_fTranslated = f; m_newChanges.clear(); invalidateFilter(); }

private:
    QString m_str;
    bool m_fNonTranslated = true;
    bool m_fProvisional = true;
    bool m_fTranslated = true;
    QSet<int> m_newChanges;
};

#endif // TRANSLATIONMODEL_H
