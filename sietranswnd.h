#ifndef SIETRANSWND_H
#define SIETRANSWND_H

#include <QMainWindow>
#include <QDir>

namespace Ui { class SieTransWnd; }
class TranslationModel;
class TranslationFilterModel;
class IExcelHandler;

class SieTransWnd : public QMainWindow
{
    Q_OBJECT

public:
    SieTransWnd(QWidget *parent = nullptr);
    ~SieTransWnd();

public slots:
    void clear();
    void enableEdits(bool enable = true);
    void disableEdits(bool disable = true) { enableEdits(!disable); }

protected:
    void closeEvent(QCloseEvent *e) override;

private slots:
    void on_btnLoadIn_clicked();
    void on_btnLoadTransDir_clicked();
    void on_btnLoadTransFile_clicked();
    void on_btnSave_clicked();
    void on_btnSaveAs_clicked();
    void tableContextMenuRequested(const QPoint &p);

private:
    bool askSave();
    bool save(const QString &fn = {});
    bool readInputFile(const QString &fn, QString &error);
    bool readTranslationDir(QDir dir, QStringList &errors);
    bool readTranslationFile(const QString &fn, QString &error);

    IExcelHandler *createExcelHandler(const QString &fn);

private:
    Ui::SieTransWnd *ui;
    TranslationModel *m_translationModel;
    TranslationFilterModel *m_translationFilterModel;
    QScopedPointer<IExcelHandler> m_fileHandler;
};
#endif // SIETRANSWND_H
