#pragma once

#include <QThread>
#include <QVector>
#include <QDateTime>
#include <QStringList>
#include <atomic>

#include "oplogentry.h"

// Asynchronous log-search worker.
// Reads each log file line-by-line, parses timestamps, and emits the byte-offset
// index for every line that falls within [from, to].
// The full entry is NOT cached; only (filePath, byteOffset) pairs are stored.
class SearchWorker : public QThread
{
    Q_OBJECT
public:
    explicit SearchWorker(QObject *parent = nullptr);

    void setup(const QStringList &files,
               const QDateTime   &from,
               const QDateTime   &to);

    // Thread-safe: call from any thread to request early termination.
    void cancel();

signals:
    void progress(int percent);                        // 0-100
    void finished(QVector<LogLineIndex> indices);      // emitted on success

protected:
    void run() override;

private:
    QStringList          m_files;
    QDateTime            m_from;
    QDateTime            m_to;
    std::atomic<bool>    m_cancelled;
};
