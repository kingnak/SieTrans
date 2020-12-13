#ifndef ODBCEXCELLOADER_H
#define ODBCEXCELLOADER_H

#include "iexcelhandler.h"

class OdbcExcelHandler : public IExcelHandler
{
public:
    OdbcExcelHandler();
    ~OdbcExcelHandler();

    bool loadFile(const QString &filename) override;
    QList<QPair<QString, QString> > getColumns() override;
    QString error() const override { return m_error; }

private:
    bool setError(const QString &e);
    QString m_connName;
    QString m_error;
    QString m_sheetName;
};

#endif // ODBCEXCELLOADER_H
