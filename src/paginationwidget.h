#pragma once

#include <QWidget>

class QHBoxLayout;

// Compact page-navigation widget.
//
// Display rules:
//   total pages <= 10  →  show every page button
//   total pages >  10  →  first 2  +  current±2  +  last 2,
//                          remaining gaps shown as "…"
class PaginationWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PaginationWidget(QWidget *parent = nullptr);

    void setPageCount(int count);      // resets to page 0
    void setCurrentPage(int page);     // 0-based
    int  currentPage() const;

signals:
    void pageChanged(int page);        // 0-based

private slots:
    void onPageButtonClicked();

private:
    void rebuild();

    int          m_totalPages;
    int          m_currentPage;
    QHBoxLayout *m_layout;
};
