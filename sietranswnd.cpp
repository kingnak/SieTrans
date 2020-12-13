#include "sietranswnd.h"
#include "ui_sietranswnd.h"
#include <QtWidgets>
#include "fileexcelhandler.h"
#include "translationmodel.h"

SieTransWnd::SieTransWnd(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SieTransWnd)
{
    ui->setupUi(this);
    m_translationModel = new TranslationModel(this);
    m_translationFilterModel = new QSortFilterProxyModel(this);
    m_translationFilterModel->setSourceModel(m_translationModel);
    ui->tblTranslation->setModel(m_translationFilterModel);

    ui->tblTranslation->setColumnWidth(2, 20);

    clear();
    connect(ui->txtFilter, &QLineEdit::textChanged, [this](auto v) {
        m_translationFilterModel->setFilterFixedString(v);
    });
}

SieTransWnd::~SieTransWnd()
{
    delete ui;
}

void SieTransWnd::clear()
{
    ui->txtFilter->clear();
    ui->lblCurFile->clear();
    m_translationModel->clear();
    setWindowModified(false);
    disableEdits();
}

void SieTransWnd::enableEdits(bool enable)
{
    ui->btnSave->setEnabled(enable);
    ui->btnSaveAs->setEnabled(enable);
    ui->btnExportUntranslated->setEnabled(enable);
    ui->txtFilter->setEnabled(enable);
    ui->tblTranslation->setEnabled(enable);
}

void SieTransWnd::on_btnLoadIn_clicked()
{
    if (!askSave()) return;
    clear();

    QSettings s;
    QString fn = QFileDialog::getOpenFileName(this, tr("Eingabedatei öffnen"), s.value("LastOpenDir").toString(), tr("Excel Dateien (*.xls)"));
    if (fn.isNull())
        return;

    QFileInfo fi(fn);
    s.setValue("LastOpenDir", fi.dir().absolutePath());
    s.sync();

    qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
    QString err;
    bool res = readInputFile(fn, err);
    qApp->restoreOverrideCursor();

    if (!res) {
        QMessageBox::critical(this, tr("Fehler"), tr("Datei konnte nicht gelesen werden: %1").arg(err));
        return;
    }

    ui->lblCurFile->setText(QDir::toNativeSeparators(fi.absoluteFilePath()));
    enableEdits();
}

void SieTransWnd::on_btnLoadTrans_clicked()
{
    QSettings s;
    QString fn = QFileDialog::getOpenFileName(this, tr("Übersetzungsverzeichnis öffnen"), s.value("LastTransDir").toString(), {}, nullptr, QFileDialog::Option::ShowDirsOnly);
    if (fn.isNull())
        return;

    QFileInfo fi(fn);
    s.setValue("LastTransDir", fi.absolutePath());
    s.sync();

    QStringList err;
    bool res = readTranslationDir(fn, err);
    if (!res || !err.isEmpty()) {
        QMessageBox m(res ? QMessageBox::Icon::Warning : QMessageBox::Icon::Critical,
                      tr("Fehler"),
                      err.join("\n"),
                      QMessageBox::StandardButton::Ok,
                      this);
        m.exec();
    }
}

bool SieTransWnd::askSave()
{
    if (this->isWindowModified()) {
        QMessageBox::StandardButton res = QMessageBox::question(this, tr("Speichern?"), tr("Sollen die aktuellen Änderungen gespeichert werden?"), QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No | QMessageBox::StandardButton::Cancel);
        switch (res) {
        case QMessageBox::StandardButton::Cancel:
            return false;
        case QMessageBox::StandardButton::No:
            return true;
        case QMessageBox::StandardButton::Yes:
            if (!save()) {
                QMessageBox::critical(this, tr("Fehler"), tr("Fehler beim Speichern"));
                return false;
            }
            return true;
        default:
            qFatal("Invalid result on askSave");

        }
    }
    return true;
}

bool SieTransWnd::save()
{
    return true;
}

bool SieTransWnd::readInputFile(const QString &fn, QString &error)
{
    error.clear();

    FileExcelHandler l;
    if (!l.loadFile(fn)) {
        error = l.error();
        return false;
    }

    auto cols = l.getColumns();
    m_translationModel->setSourceData(cols);

    return true;
}

bool SieTransWnd::readTranslationDir(const QString &dir, QStringList &errors)
{
    errors.clear();
    return true;
}
