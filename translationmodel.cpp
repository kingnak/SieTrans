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
            if (d.translationState == Translated)
                return QChar(0x2713);
            if (d.translationState == ProvisionalTranslation)
                return "?";
            return "";
        case Qt::BackgroundColorRole:
            if (d.translationState == Translated)
                return QColor(qRgb(50, 205, 50));
            if (d.translationState == ProvisionalTranslation)
                return QColor(qRgb(240, 230, 140));
            return QColor(Qt::transparent);
        case Qt::TextAlignmentRole:
            return Qt::AlignCenter;
        case TranslationStateRole:
            return d.translationState;
        }
    }

    return {};
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
            ) {
        return false;
    }

    if (index.column() == 2) {
        if (role == TranslationStateRole) {
            m_data[index.row()].translationState = static_cast<TranslationState> (value.toInt());
            dataChanged(index, index);
            return true;
        }
        return false;
    }

    if (value.toString() == m_data[index.row()].data.second) return false;

    m_data[index.row()].data.second = value.toString();
    m_data[index.row()].translationState = (value.toString().isEmpty()) ? NotTranslated : Translated;
    emit translationModified();
    return true;
}

void TranslationModel::setSourceData(QList<IExcelHandler::ColumnData> data)
{
    beginResetModel();
    m_data.clear();
    for (auto d : data) {
        Record r{d,NotTranslated};
        if (!d.first.isEmpty() && !d.second.isEmpty() && d.first != d.second)
            r.translationState = Translated;
        m_data << r;
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
        if (c.translationState != NotTranslated) continue;
        auto it = m_trans.find(c.data.first);
        if (it != m_trans.end()) {
            c.data.second = *it;
            if (c.data.second == c.data.first || c.data.second.isEmpty())
                c.translationState = ProvisionalTranslation;
            else
                c.translationState = Translated;
            if (minRow < 0) minRow = r;
            maxRow = r;
        }
    }
    if (minRow >= 0) {
        emit dataChanged(index(minRow, 1), index(maxRow, 2));
        emit translationModified();
    }
}

QList<IExcelHandler::ColumnData> TranslationModel::getTranslationData() const
{
    QList<IExcelHandler::ColumnData> ret;
    for (auto c : m_data) {
        ret << c.data;
    }
    return ret;
}

void TranslationModel::updateMultipleTranslationStates(QModelIndexList lst, TranslationModel::TranslationState state)
{
    int min = INT_MAX;
    int max = -1;

    for (auto idx : lst) {
        if (idx.row() < min) min = idx.row();
        if (idx.row() > max) max = idx.row();
        setData(index(idx.row(), 2, idx.parent()), state, TranslationStateRole);
    }

    dataChanged(index(min, 2), index(max, 2));
}

void TranslationModel::clear()
{
    beginResetModel();
    m_data.clear();
    m_trans.clear();
    endResetModel();
}

TranslationFilterModel::TranslationFilterModel(TranslationModel *source, QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setSourceModel(source);
}

bool TranslationFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    auto s1 = sourceModel()->data(sourceModel()->index(source_row, 0, source_parent)).toString();
    auto s2 = sourceModel()->data(sourceModel()->index(source_row, 1, source_parent)).toString();
    auto fl = sourceModel()->data(sourceModel()->index(source_row, 2, source_parent), TranslationModel::TranslationStateRole).toInt();

    // Prevent that changes in state disappear immediately
    if (!m_newChanges.contains(source_row)) {
        if (fl == TranslationModel::Translated && !m_fTranslated) return false;
        if (fl == TranslationModel::NotTranslated && !m_fNonTranslated) return false;
        if (fl == TranslationModel::ProvisionalTranslation && !m_fProvisional) return false;
    }

    if (m_str.isEmpty()) return true;
    if (!s1.contains(m_str, Qt::CaseInsensitive)) return false;
    if (!s2.contains(m_str, Qt::CaseInsensitive)) return false;
    return true;
}

void TranslationFilterModel::updateMultipleTranslationStates(QModelIndexList lst, TranslationModel::TranslationState state)
{
    for (auto &idx : lst) {
        idx = mapToSource(idx);
        m_newChanges << idx.row();
    }
    static_cast<TranslationModel*> (sourceModel())->updateMultipleTranslationStates(lst, state);
}

bool TranslationFilterModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == TranslationModel::TranslationStateRole) {
        m_newChanges << mapToSource(index).row();
    }
    return QSortFilterProxyModel::setData(index, value, role);
}
