#ifndef IEXCELLOADER_H
#define IEXCELLOADER_H

#include <QList>

class IExcelHandler
{
    Q_DISABLE_COPY(IExcelHandler)
public:
    typedef QPair<QString, QString> ColumnData;

    IExcelHandler() = default;
    virtual ~IExcelHandler() = default;

    virtual bool loadFile(const QString &filename) = 0;
    virtual void resetColumns() = 0;
    virtual QList<ColumnData> getColumns() = 0;
    virtual void setColumns(QList<ColumnData> cols) = 0;
    virtual bool save() = 0;
    virtual bool saveAs(const QString &filename) = 0;

    virtual QString error() const = 0;
};

#endif // IEXCELLOADER_H
