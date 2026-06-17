#include "logviewerdialog.h"

#include "logfilescanner.h"
#include "logdataloader.h"
#include "searchworker.h"
#include "paginationwidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDateTimeEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QProgressBar>
#include <QStackedWidget>
#include <QLabel>
#include <QHeaderView>
#include <QCloseEvent>
#include <QColor>
#include <QMessageBox>

static const QStringList TABLE_HEADERS{
    QStringLiteral("时间"),
    QStringLiteral("操作员"),
    QStringLiteral("终端"),
    QStringLiteral("操作对象"),
    QStringLiteral("操作类型"),
    QStringLiteral("操作描述"),
    QStringLiteral("操作结果"),
    QStringLiteral("附件")
};

// ---------------------------------------------------------------------------
LogViewerDialog::LogViewerDialog(const QString &logDirectory, QWidget *parent)
    : QDialog(parent), m_logDir(logDirectory), m_worker(nullptr)
{
    setWindowTitle(QStringLiteral("操作日志查看器"));
    resize(1100, 640);
    buildUi();
}

LogViewerDialog::~LogViewerDialog()
{
    if (m_worker) {
        m_worker->cancel();
        m_worker->wait();
    }
}

// ---------------------------------------------------------------------------
void LogViewerDialog::buildUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(8);

    // ── Top bar: time range + search / cancel ─────────────────────────────
    auto *topBar = new QHBoxLayout;
    topBar->setSpacing(6);

    topBar->addWidget(new QLabel(QStringLiteral("开始时间:"), this));
    m_fromEdit = new QDateTimeEdit(QDateTime::currentDateTime().addDays(-1), this);
    m_fromEdit->setDisplayFormat(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    m_fromEdit->setCalendarPopup(true);
    topBar->addWidget(m_fromEdit);

    topBar->addWidget(new QLabel(QStringLiteral("结束时间:"), this));
    m_toEdit = new QDateTimeEdit(QDateTime::currentDateTime(), this);
    m_toEdit->setDisplayFormat(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    m_toEdit->setCalendarPopup(true);
    topBar->addWidget(m_toEdit);

    m_searchBtn = new QPushButton(QStringLiteral("检索"), this);
    topBar->addWidget(m_searchBtn);

    m_cancelBtn = new QPushButton(QStringLiteral("取消"), this);
    m_cancelBtn->setVisible(false);
    topBar->addWidget(m_cancelBtn);

    topBar->addStretch();
    mainLayout->addLayout(topBar);

    // ── Stacked area ──────────────────────────────────────────────────────
    m_stack = new QStackedWidget(this);

    // Page 0 – empty hint
    m_emptyLabel = new QLabel(QStringLiteral("请选择时间范围后点击检索"), this);
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_stack->addWidget(m_emptyLabel);                   // index 0

    // Page 1 – progress bar
    auto *progressPage   = new QWidget(this);
    auto *progressLayout = new QVBoxLayout(progressPage);
    m_progressBar = new QProgressBar(progressPage);
    m_progressBar->setRange(0, 100);
    m_progressBar->setTextVisible(true);
    progressLayout->addStretch();
    progressLayout->addWidget(m_progressBar);
    progressLayout->addStretch();
    m_stack->addWidget(progressPage);                   // index 1

    // Page 2 – results table + pagination
    auto *resultsPage   = new QWidget(this);
    auto *resultsLayout = new QVBoxLayout(resultsPage);
    resultsLayout->setContentsMargins(0, 0, 0, 0);
    resultsLayout->setSpacing(4);

    m_table = new QTableWidget(0, 8, resultsPage);
    m_table->setHorizontalHeaderLabels(TABLE_HEADERS);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->verticalHeader()->setVisible(false);
    m_table->setAlternatingRowColors(true);
    resultsLayout->addWidget(m_table);

    m_pagination = new PaginationWidget(resultsPage);
    resultsLayout->addWidget(m_pagination);

    m_stack->addWidget(resultsPage);                    // index 2

    mainLayout->addWidget(m_stack);
    m_stack->setCurrentIndex(0);

    // ── Connections ───────────────────────────────────────────────────────
    connect(m_searchBtn, &QPushButton::clicked,
            this, &LogViewerDialog::onSearch);
    connect(m_cancelBtn, &QPushButton::clicked,
            this, &LogViewerDialog::onCancel);
    connect(m_pagination, &PaginationWidget::pageChanged,
            this, &LogViewerDialog::onPageChanged);
}

// ---------------------------------------------------------------------------
void LogViewerDialog::closeEvent(QCloseEvent *event)
{
    onCancel();
    QDialog::closeEvent(event);
}

// ---------------------------------------------------------------------------
void LogViewerDialog::onSearch()
{
    const QDateTime from = m_fromEdit->dateTime();
    const QDateTime to   = m_toEdit->dateTime();

    if (!from.isValid() || !to.isValid()) return;
    if (from > to) {
        QMessageBox::warning(this,
                             QStringLiteral("参数错误"),
                             QStringLiteral("开始时间不能晚于结束时间"));
        return;
    }

    const QStringList files = LogFileScanner::scan(m_logDir);
    if (files.isEmpty()) {
        m_emptyLabel->setText(QStringLiteral("未找到日志文件"));
        m_stack->setCurrentIndex(0);
        return;
    }

    setSearching(true);
    m_progressBar->setValue(0);

    // Cancel any running worker
    if (m_worker) {
        m_worker->cancel();
        m_worker->wait();
        delete m_worker;
    }

    m_worker = new SearchWorker(this);
    m_worker->setup(files, from, to);
    connect(m_worker, &SearchWorker::progress,
            this,     &LogViewerDialog::onProgress);
    connect(m_worker, &SearchWorker::finished,
            this,     &LogViewerDialog::onSearchFinished);
    m_worker->start();
}

void LogViewerDialog::onCancel()
{
    if (m_worker && m_worker->isRunning()) {
        m_worker->cancel();
        m_worker->wait();
    }
    setSearching(false);
    m_emptyLabel->setText(QStringLiteral("请选择时间范围后点击检索"));
    m_stack->setCurrentIndex(0);
}

void LogViewerDialog::onProgress(int percent)
{
    m_progressBar->setValue(percent);
}

void LogViewerDialog::onSearchFinished(QVector<LogLineIndex> indices)
{
    m_indices = indices;
    setSearching(false);

    if (m_indices.isEmpty()) {
        m_emptyLabel->setText(QStringLiteral("未找到符合条件的日志记录"));
        m_stack->setCurrentIndex(0);
        return;
    }

    const int totalPages =
        (m_indices.size() + LogDataLoader::PAGE_SIZE - 1) / LogDataLoader::PAGE_SIZE;
    m_pagination->setPageCount(totalPages);
    showTablePage(0);
    m_stack->setCurrentIndex(2);
}

void LogViewerDialog::onPageChanged(int page)
{
    showTablePage(page);
}

// ---------------------------------------------------------------------------
void LogViewerDialog::showTablePage(int page)
{
    const QVector<OpLogEntry> entries = LogDataLoader::loadPage(m_indices, page);
    m_table->setRowCount(entries.size());

    for (int row = 0; row < entries.size(); ++row) {
        const OpLogEntry &e = entries[row];

        // Helper to create a read-only table item
        auto mkItem = [](const QString &text) {
            auto *it = new QTableWidgetItem(text);
            it->setFlags(it->flags() & ~Qt::ItemIsEditable);
            return it;
        };

        m_table->setItem(row, 0, mkItem(e.rawTimestamp));
        m_table->setItem(row, 1, mkItem(e.username));
        m_table->setItem(row, 2, mkItem(e.terminal));
        m_table->setItem(row, 3, mkItem(e.operationObject));
        m_table->setItem(row, 4, mkItem(e.operationType));
        m_table->setItem(row, 5, mkItem(e.description));

        // Result column with colour highlight
        QString  resultText;
        QColor   resultBg;
        switch (e.result) {
        case RESULT_SUCCESS:
            resultText = QStringLiteral("SUCCESS");
            resultBg   = QColor(0, 180, 0, 90);
            break;
        case RESULT_FAILED:
            resultText = QStringLiteral("FAILED");
            resultBg   = QColor(200, 0, 0, 90);
            break;
        case RESULT_EXECUTING:
            resultText = QStringLiteral("EXECUTING");
            resultBg   = QColor(230, 180, 0, 90);
            break;
        default:
            resultText.clear();
            resultBg = QColor();
            break;
        }
        auto *resultItem = mkItem(resultText);
        if (resultBg.isValid()) resultItem->setBackground(resultBg);
        m_table->setItem(row, 6, resultItem);

        m_table->setItem(row, 7, mkItem(e.addition));
    }
    m_table->resizeColumnsToContents();
    m_table->horizontalHeader()->setStretchLastSection(true);
}

// ---------------------------------------------------------------------------
void LogViewerDialog::setSearching(bool on)
{
    m_searchBtn->setEnabled(!on);
    m_fromEdit->setEnabled(!on);
    m_toEdit->setEnabled(!on);
    m_cancelBtn->setVisible(on);

    if (on) m_stack->setCurrentIndex(1);
}
