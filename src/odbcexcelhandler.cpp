#include "odbcexcelhandler.h"
#include <QtCore>
#include <QtSql>

OdbcExcelHandler::OdbcExcelHandler()
{
    QString n = QString::fromLocal8Bit(std::tmpnam(nullptr));
    QFileInfo fi(n);
    m_connName = "xls_conn_" + fi.fileName();
}

OdbcExcelHandler::~OdbcExcelHandler()
{
    QSqlDatabase::database(m_connName).close();
    QSqlDatabase::removeDatabase(m_connName);
}

bool OdbcExcelHandler::loadFile(const QString &filename)
{
    m_sheetName.clear();
    if (!QFile::exists(filename)) {
        return setError(qApp->translate("ODBCLoader", "Datei %1 nicht gefunden").arg(filename));
    }

    auto db = QSqlDatabase::addDatabase("QODBC", m_connName);
    db.setDatabaseName("DRIVER={Microsoft Excel Driver (*.xls, *.xlsx, *.xlsm, *.xlsb)};DBQ=" + QDir::toNativeSeparators(filename));
    if (!db.open()) {
        return setError(qApp->translate("ODBCLoader", "Datei %1 kann nicht gelesen werden: %2").arg(filename, db.lastError().text()));
    }

    auto tbls = db.tables(QSql::AllTables);
    if (tbls.isEmpty()) {
        return setError(qApp->translate("ODBCLoader", "Keine Arbeitsblätter gefunden"));
    }

    m_sheetName = tbls[0];
    return true;
}

QList<QPair<QString, QString> > OdbcExcelHandler::getColumns()
{
    auto db = QSqlDatabase::database(m_connName);
    if (!db.isOpen()) {
        return {};
    }

    QList<QPair<QString, QString>> res;
    QSqlQuery q("SELECT * FROM ["+m_sheetName+"]", db);
    while (q.next()) {
        res << qMakePair(q.value(0).toString(), q.value(1).toString());
    }

    return res;
}

bool OdbcExcelHandler::setError(const QString &e)
{
    m_error = e;
    return false;
}


