#include "fileexcelhandler.h"
#include "BasicExcel.hpp"
#include <QtCore>

FileExcelHandler::FileExcelHandler()
{

}

bool FileExcelHandler::loadFile(const QString &filename)
{
    m_cols.clear();

    QScopedPointer<YExcel::BasicExcel> b(new YExcel::BasicExcel);
    if (!b->Load(QDir::toNativeSeparators(filename).toLocal8Bit())) {
        return setError(qApp->translate("FileHandler", "Fehler beim Lesen"));
    }

    if (b->GetTotalWorkSheets() == 0) {
        return setError(qApp->translate("FileHandler", "Keine Arbeitsblätter gefunden"));
    }

    m_excel.reset(b.take());
    return true;
}

void FileExcelHandler::resetColumns()
{
    m_cols.clear();
}

QList<IExcelHandler::ColumnData> FileExcelHandler::getColumns()
{
    if (!m_excel) return {};
    if (!m_cols.empty()) return m_cols;

    auto ws = m_excel->GetWorksheet(size_t(0));
    for (size_t r = 0; r < ws->GetTotalRows(); ++r) {
        QString c1 = QString::fromLocal8Bit(ws->Cell(r, 0)->GetString());
        QString c2 = QString::fromLocal8Bit(ws->Cell(r, 1)->GetString());
        m_cols << qMakePair(c1, c2);
    }

    return m_cols;
}

void FileExcelHandler::setColumns(QList<IExcelHandler::ColumnData> cols)
{
    m_cols = cols;
}

bool FileExcelHandler::saveAs(const QString &filename)
{
    if (!m_excel) return false;
    writeExcel();
    return m_excel->SaveAs(QDir::toNativeSeparators(filename).toLocal8Bit());
}

bool FileExcelHandler::save()
{
    if (!m_excel) return false;
    writeExcel();
    return m_excel->Save();
}

bool FileExcelHandler::setError(const QString &e)
{
    m_error = e;
    return false;
}

void FileExcelHandler::writeExcel()
{
    auto ws = m_excel->GetWorksheet(size_t(0));

    for (int r = 0; r < m_cols.size(); ++r) {
        ws->Cell(r, 0)->SetString(m_cols[r].first.toLocal8Bit());
        ws->Cell(r, 1)->SetString(m_cols[r].second.toLocal8Bit());
    }

    // Delete additional old cells
    for (size_t r = m_cols.size(); r < ws->GetTotalRows(); ++r) {
        ws->EraseCell(r, 0);
        ws->EraseCell(r, 1);
    }
}
