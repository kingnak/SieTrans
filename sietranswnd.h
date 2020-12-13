#ifndef SIETRANSWND_H
#define SIETRANSWND_H

#include <QMainWindow>

namespace Ui { class SieTransWnd; }
class TranslationModel;
class QSortFilterProxyModel;

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

private slots:
    void on_btnLoadIn_clicked();
    void on_btnLoadTrans_clicked();

private:
    bool askSave();
    bool save();
    bool readInputFile(const QString &fn, QString &error);
    bool readTranslationDir(const QString &dir, QStringList &errors);

private:
    Ui::SieTransWnd *ui;
    TranslationModel *m_translationModel;
    QSortFilterProxyModel *m_translationFilterModel;
};
#endif // SIETRANSWND_H
