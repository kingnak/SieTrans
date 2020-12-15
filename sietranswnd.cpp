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
    m_translationFilterModel = new TranslationFilterModel(m_translationModel, this);
    ui->tblTranslation->setModel(m_translationFilterModel);

    ui->tblTranslation->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tblTranslation->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->tblTranslation->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    ui->tblTranslation->setColumnWidth(2, 20);

    ui->tblTranslation->setContextMenuPolicy(Qt::CustomContextMenu);

    clear();
    connect(ui->txtFilter, &QLineEdit::textChanged, [this](auto v) {
        m_translationFilterModel->setFilterString(v);
    });

    connect(ui->tblTranslation, &QAbstractItemView::doubleClicked, [this](auto idx) {
        if (idx.column() != 2) return;
        auto s = m_translationFilterModel->data(idx, TranslationModel::TranslationStateRole).toInt();
        s = (s+1)%3;
        m_translationFilterModel->setData(idx, s, TranslationModel::TranslationStateRole);
    });

    connect(ui->chkFilterTranslated, &QCheckBox::toggled, [this](bool t) { m_translationFilterModel->filterTranslated(t); } );
    connect(ui->chkFilterProvisional, &QCheckBox::toggled, [this](bool t) { m_translationFilterModel->filterProvisional(t); } );
    connect(ui->chkFilterNotTranslated, &QCheckBox::toggled, [this](bool t) { m_translationFilterModel->filterNonTranslated(t); } );

    connect(ui->tblTranslation, &QWidget::customContextMenuRequested, this, &SieTransWnd::tableContextMenuRequested);

    connect(m_translationModel, &TranslationModel::translationModified, this, [this]{ setWindowModified(true); } );

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
    m_fileHandler.reset();
}

void SieTransWnd::enableEdits(bool enable)
{
    ui->btnSave->setEnabled(enable);
    ui->btnSaveAs->setEnabled(enable);
    ui->btnExportUntranslated->setEnabled(enable);
    ui->txtFilter->setEnabled(enable);
    ui->tblTranslation->setEnabled(enable);
    ui->btnLoadTransDir->setEnabled(enable);
    ui->btnLoadTransFile->setEnabled(enable);
    ui->chkFilterTranslated->setEnabled(enable);
    ui->chkFilterProvisional->setEnabled(enable);
    ui->chkFilterNotTranslated->setEnabled(enable);
}

void SieTransWnd::closeEvent(QCloseEvent *e)
{
    if (!askSave()) {
        e->ignore();
        return;
    }
    QMainWindow::closeEvent(e);
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

void SieTransWnd::on_btnLoadTransFile_clicked()
{
    QSettings s;
    QString fn = QFileDialog::getOpenFileName(this, tr("Übersetzungsdatei öffnen"), s.value("LastTransDir").toString(), tr("Excel Dateien (*.xls)"));
    if (fn.isNull())
        return;

    QFileInfo fi(fn);
    s.setValue("LastTransDir", fi.absolutePath());
    s.sync();

    qApp->setOverrideCursor(Qt::WaitCursor);
    QString err;
    bool res = readTranslationFile(fn, err);

    if (!res) {
        qApp->restoreOverrideCursor();
        QMessageBox::critical(this, tr("Fehler"), tr("Datei konnte nicht gelesen werden: %1").arg(err));
    } else {
        m_translationModel->applyTranslate();
        qApp->restoreOverrideCursor();
    }
}

void SieTransWnd::on_btnSave_clicked()
{
    save();
}

void SieTransWnd::on_btnSaveAs_clicked()
{
    if (!m_fileHandler) return;

    QSettings s;
    auto fn = QFileDialog::getSaveFileName(this, tr("Speichern als"), s.value("LastSaveDir").toString(), tr("Excel Dateien (*.xls)"));
    if (fn.isNull()) {
        return;
    }

    s.setValue("LastSaveDir", fn);
    s.sync();

    save(fn);
}

void SieTransWnd::on_btnExportUntranslated_clicked()
{
    if (!m_fileHandler) return;

    auto opts = QStringList() << tr("Nur fehlende Übersetzungen") << tr("Mit evtl. Übersetzungen");
    bool ok = false;
    auto sel = QInputDialog::getItem(this, tr("Exportieren"), tr("Export-Modus wählen"), opts, 0, false, &ok);

    if (!ok) return;

    QSet<TranslationModel::TranslationState> states;
    states << TranslationModel::TranslationState::NotTranslated;
    if (opts.indexOf(sel) == 1)
        states << TranslationModel::TranslationState::ProvisionalTranslation;

    QSettings s;
    QString fn = QFileDialog::getSaveFileName(this, tr("Exportieren"), s.value("LastExportDir").toString(), tr("Excel Dateien (*.xls)"));
    if (fn.isEmpty()) return;
    s.setValue("LastExportDir", fn);
    s.sync();

    qApp->setOverrideCursor(Qt::WaitCursor);
    QScopedPointer<IExcelHandler> e(createExcelHandler(fn));
    e->newFile(fn, m_fileHandler->getSheetName());
    auto data = m_translationModel->getTranslationData(states);
    e->setColumns(data);
    bool res = e->save();
    qApp->restoreOverrideCursor();

    if (!res) {
        QMessageBox::critical(this, tr("Fehler"), tr("Fehler beim Speichern"));
    }
}

void SieTransWnd::tableContextMenuRequested(const QPoint &p)
{
    auto idx = ui->tblTranslation->indexAt(p);
    if (!idx.isValid()) return;

    auto m = new QMenu;
    auto acts = QList<QAction*>()
        << new QAction(tr("Nicht übersetzt (X)"), m)
        << new QAction(tr("Übersetzt (%1)").arg(QChar(0x2713)), m)
        << new QAction(tr("Evlt. übersetzt (?)"), m);

    m->addAction(acts[0]);
    m->addAction(acts[1]);
    m->addAction(acts[2]);
    auto a = m->exec(ui->tblTranslation->mapToGlobal(p));
    m->deleteLater();

    if (!a) return;

    auto sel = ui->tblTranslation->selectionModel()->selectedIndexes();
    if (a == acts[0]) {
        m_translationFilterModel->updateMultipleTranslationStates(sel, TranslationModel::NotTranslated);
    } else if (a == acts[1]) {
        m_translationFilterModel->updateMultipleTranslationStates(sel, TranslationModel::Translated);
    } else if (a == acts[2]) {
        m_translationFilterModel->updateMultipleTranslationStates(sel, TranslationModel::ProvisionalTranslation);
    }
}

void SieTransWnd::on_btnLoadTransDir_clicked()
{
    QSettings s;
    QString fn = QFileDialog::getExistingDirectory(this, tr("Übersetzungsverzeichnis öffnen"), s.value("LastTransDir").toString());
    if (fn.isNull())
        return;

    QFileInfo fi(fn);
    s.setValue("LastTransDir", fi.absolutePath());
    s.sync();

    qApp->setOverrideCursor(Qt::WaitCursor);
    QStringList err;
    bool res = readTranslationDir(fn, err);
    if (res) {
        m_translationModel->applyTranslate();
    }
    qApp->restoreOverrideCursor();

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
                return false;
            }
            return true;
        default:
            qFatal("Invalid result on askSave");

        }
    }
    return true;
}

bool SieTransWnd::save(const QString &fn)
{
    if (!m_fileHandler) return false;

    qApp->setOverrideCursor(Qt::WaitCursor);
    m_fileHandler->setColumns(m_translationModel->getTranslationData());
    bool res = false;
    if (fn.isNull()) {
        res = m_fileHandler->save();
    } else {
        res = m_fileHandler->saveAs(fn);
        ui->lblCurFile->setText(QDir::toNativeSeparators(fn));
    }
    qApp->restoreOverrideCursor();

    if (!res) {
        QMessageBox::critical(this, tr("Fehler"), tr("Fehler beim Speichern"));
        return false;
    }
    setWindowModified(false);

    return true;
}

bool SieTransWnd::readInputFile(const QString &fn, QString &error)
{
    error.clear();

    QScopedPointer<IExcelHandler> l(createExcelHandler(fn));
    if (!l->loadFile(fn)) {
        error = l->error();
        return false;
    }

    auto cols = l->getColumns();
    m_translationModel->setSourceData(cols);

    m_fileHandler.reset(l.take());
    return true;
}

bool SieTransWnd::readTranslationDir(QDir dir, QStringList &errors)
{
    bool success = false;
    auto l = dir.entryInfoList(QStringList() << "*.xls", QDir::Filter::NoDotAndDotDot | QDir::Filter::Files | QDir::Filter::Dirs);
    for (auto e : l) {
        if (e.isDir()) {
            success |= readTranslationDir(e.dir(), errors);
        } else if (e.isFile()) {
            QString err;
            if (!readTranslationFile(e.absoluteFilePath(), err)) {
                errors << tr("Fehler beim Lesen von %1: %2").arg(QDir::toNativeSeparators(e.absoluteFilePath()), err);
            } else {
                success = true;
            }
        }
    }

    return success;
}

bool SieTransWnd::readTranslationFile(const QString &fn, QString &error)
{
    error.clear();
    QScopedPointer<IExcelHandler> l(createExcelHandler(fn));
    if (!l->loadFile(fn)) {
        error = l->error();
        return false;
    }
    m_translationModel->addTranslations(l->getColumns());
    return true;
}

IExcelHandler *SieTransWnd::createExcelHandler(const QString &fn)
{
    Q_UNUSED(fn);
    return new FileExcelHandler;
}
