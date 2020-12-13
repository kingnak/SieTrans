#include "translationmodel.h"
#include <QColor>

TranslationModel::TranslationModel(QObject *parent)
    : QAbstractTableModel(parent)
{

}

int TranslationModel::rowCount(const QModelIndex &idx) const
{
    Q_UNUSED(idx)
    return m_data.size();
}

int TranslationModel::columnCount(const QModelIndex &idx) const
{
    Q_UNUSED(idx)
    return 3;
}

QVariant TranslationModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
        case 0: return tr("Quelle");
        case 1: return tr("Übersetzung");
        case 2: return tr("OK");
        }
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

QVariant TranslationModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid()) return {};
    if (idx.row() < 0 || idx.row() >= m_data.size()) return {};
    if (idx.column() < 0 || idx.column() >= 3) return {};

    auto d = m_data[idx.row()];
    if (idx.column() != 2) {
        switch (role) {
        case Qt::EditRole:
        case Qt::DisplayRole:
            return (idx.column() == 0) ? d.data.first : d.data.second;
        }
    } else {
        switch (role) {
        case Qt::DisplayRole:
            return d.translated ? "X" : "";
        case Qt::BackgroundColorRole:
            return QColor(d.translated ? Qt::darkGreen : Qt::transparent);
        }
    }

    return {};
    //return QAbstractTableModel::data(idx, role);
}

Qt::ItemFlags TranslationModel::flags(const QModelIndex &idx) const
{
    auto f = QAbstractTableModel::flags(idx);
    if (idx.column() == 1) {
        f |= Qt::ItemIsEditable;
    }
    return f;
}

bool TranslationModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()
            || index.column() == 0
            || index.row() < 0
            || index.row() >= m_data.size()
            || role != Qt::EditRole
            ) {
        return false;
    }

    if (value.toString() == m_data[index.row()].data.second) return false;

    m_data[index.row()].data.second = value.toString();
    m_data[index.row()].translated = true;
    return true;
}

void TranslationModel::setSourceData(QList<IExcelHandler::ColumnData> data)
{
    beginResetModel();
    m_data.clear();
    for (auto d : data) {
        m_data << Record{d,false};
    }
    endResetModel();
}

void TranslationModel::clearTranslationData()
{
    m_trans.clear();
}

void TranslationModel::addTranslations(QList<IExcelHandler::ColumnData> translations)
{
    for (auto d : translations) {
        m_trans[d.first] = d.second;
    }
}

void TranslationModel::applyTranslate()
{
    if (m_trans.empty()) return;
    int minRow = -1;
    int maxRow = -1;
    for (int r = 0; r < m_data.size(); ++r) {
        auto &c = m_data[r];
        if (c.translated) continue;
        auto it = m_trans.find(c.data.first);
        if (it != m_trans.end()) {
            c.data.second = *it;
            c.translated = true;
            if (minRow < 0) minRow = r;
            maxRow = r;
        }
    }
    emit dataChanged(index(minRow, 1), index(maxRow, 2));
}

void TranslationModel::clear()
{
    beginResetModel();
    m_data.clear();
    m_trans.clear();
    endResetModel();
}
