#ifndef IEXCELLOADER_H
#define IEXCELLOADER_H

#include <QList>

class IExcelHandler
{
    Q_DISABLE_COPY(IExcelHandler)
public:
    IExcelHandler() = default;
    virtual ~IExcelHandler() = default;

    virtual bool loadFile(const QString &filename) = 0;
    virtual QList<QPair<QString, QString>> getColumns() = 0;
    virtual QString error() const = 0;
};

#endif // IEXCELLOADER_H
