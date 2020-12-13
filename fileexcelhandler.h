#ifndef FILEEXCELHANDLER_H
#define FILEEXCELHANDLER_H

#include "iexcelhandler.h"
#include "BasicExcel.hpp"
#include <QScopedPointer>

class FileExcelHandler : public IExcelHandler
{
public:
    FileExcelHandler();

    bool loadFile(const QString &filename) override;
    void resetColumns() override;
    QList<ColumnData> getColumns() override;
    void setColumns(QList<ColumnData> cols) override;
    bool saveAs(const QString &filename) override;
    bool save() override;
    QString error() const override { return m_error; }

private:
    bool setError(const QString &e);

    void writeExcel();

    QString m_error;
    QList<ColumnData> m_cols;
    QScopedPointer<YExcel::BasicExcel> m_excel;
};

#endif // FILEEXCELHANDLER_H
