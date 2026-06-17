#pragma once

#include <QDialog>
#include <QVector>
#include "oplogentry.h"

class QDateTimeEdit;
class QPushButton;
class QTableWidget;
class QProgressBar;
class QStackedWidget;
class QLabel;
class SearchWorker;
class PaginationWidget;

// Main operation-log viewer dialog.
//
// Usage:
//   LogViewerDialog dlg("/var/log/ops", parent);
//   dlg.exec();
class LogViewerDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LogViewerDialog(const QString &logDirectory,
                             QWidget       *parent = nullptr);
    ~LogViewerDialog() override;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onSearch();
    void onCancel();
    void onProgress(int percent);
    void onSearchFinished(QVector<LogLineIndex> indices);
    void onPageChanged(int page);

private:
    void buildUi();
    void showTablePage(int page);
    void setSearching(bool on);

    // ── data ──────────────────────────────────────────────────────────────
    QString               m_logDir;
    SearchWorker         *m_worker;
    QVector<LogLineIndex> m_indices;

    // ── widgets ───────────────────────────────────────────────────────────
    QDateTimeEdit    *m_fromEdit;
    QDateTimeEdit    *m_toEdit;
    QPushButton      *m_searchBtn;
    QPushButton      *m_cancelBtn;

    QStackedWidget   *m_stack;   // index 0=empty  1=progress  2=results
    QLabel           *m_emptyLabel;
    QProgressBar     *m_progressBar;
    QTableWidget     *m_table;
    PaginationWidget *m_pagination;
};
