#include "paginationwidget.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QList>
#include <QSet>
#include <QVariant>
#include <algorithm>

PaginationWidget::PaginationWidget(QWidget *parent)
    : QWidget(parent), m_totalPages(0), m_currentPage(0)
{
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(4);
    setLayout(m_layout);
}

void PaginationWidget::setPageCount(int count)
{
    m_totalPages  = count;
    m_currentPage = 0;
    rebuild();
}

void PaginationWidget::setCurrentPage(int page)
{
    if (page < 0 || page >= m_totalPages) return;
    m_currentPage = page;
    rebuild();
}

int PaginationWidget::currentPage() const
{
    return m_currentPage;
}

// ---------------------------------------------------------------------------
// Build page-button list according to the display rules.
// ---------------------------------------------------------------------------
void PaginationWidget::rebuild()
{
    // Remove all existing child widgets
    QLayoutItem *item;
    while ((item = m_layout->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    if (m_totalPages <= 0) return;

    // Determine the set of page numbers to show
    QList<int> pages;
    if (m_totalPages <= 10) {
        for (int i = 0; i < m_totalPages; ++i) pages.append(i);
    } else {
        QSet<int> pageSet;
        // First two
        pageSet.insert(0);
        pageSet.insert(1);
        // Last two
        pageSet.insert(m_totalPages - 2);
        pageSet.insert(m_totalPages - 1);
        // Current ±2
        for (int d = -2; d <= 2; ++d) {
            int p = m_currentPage + d;
            if (p >= 0 && p < m_totalPages) pageSet.insert(p);
        }
        for (int p : pageSet) pages.append(p);
        std::sort(pages.begin(), pages.end());
    }

    // Emit buttons with "…" labels in gaps
    int prev = -1;
    for (int p : pages) {
        if (prev >= 0 && p > prev + 1) {
            QLabel *dots = new QLabel(QStringLiteral("\u2026"), this);
            dots->setAlignment(Qt::AlignCenter);
            m_layout->addWidget(dots);
        }

        QPushButton *btn = new QPushButton(QString::number(p + 1), this);
        btn->setFixedWidth(36);
        btn->setProperty("page", QVariant(p));
        if (p == m_currentPage) {
            btn->setEnabled(false);
            btn->setFlat(true);
        }
        connect(btn, &QPushButton::clicked,
                this, &PaginationWidget::onPageButtonClicked);
        m_layout->addWidget(btn);
        prev = p;
    }
    m_layout->addStretch();
}

void PaginationWidget::onPageButtonClicked()
{
    QPushButton *btn = qobject_cast<QPushButton *>(sender());
    if (!btn) return;
    int page = btn->property("page").toInt();
    if (page == m_currentPage) return;
    m_currentPage = page;
    rebuild();
    emit pageChanged(page);
}
