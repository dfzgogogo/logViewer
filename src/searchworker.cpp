#include "searchworker.h"
#include "loglineparser.h"

#include <QFile>
#include <QFileInfo>

SearchWorker::SearchWorker(QObject *parent)
    : QThread(parent), m_cancelled(false) {}

void SearchWorker::setup(const QStringList &files,
                         const QDateTime   &from,
                         const QDateTime   &to)
{
    m_files     = files;
    m_from      = from;
    m_to        = to;
    m_cancelled = false;
}

void SearchWorker::cancel()
{
    m_cancelled = true;
}

void SearchWorker::run()
{
    QVector<LogLineIndex> indices;

    // Pre-compute total byte size for progress reporting
    qint64 totalSize = 0;
    for (const QString &fp : m_files) {
        totalSize += QFileInfo(fp).size();
    }

    qint64 processedBytes = 0;
    int    lastPercent    = -1;

    for (const QString &fp : m_files) {
        if (m_cancelled) break;

        QFile file(fp);
        if (!file.open(QIODevice::ReadOnly)) continue;

        while (!file.atEnd()) {
            if (m_cancelled) break;

            const qint64 offset   = file.pos();
            const QByteArray raw  = file.readLine();
            processedBytes       += raw.size();

            const QString line = QString::fromUtf8(raw).trimmed();
            if (line.isEmpty()) continue;

            OpLogEntry entry;
            if (!LogLineParser::parse(line, entry)) continue;

            if (entry.timestamp >= m_from && entry.timestamp <= m_to) {
                indices.append({fp, offset});
            }
        }
        file.close();

        if (totalSize > 0) {
            int pct = static_cast<int>(processedBytes * 100 / totalSize);
            pct = qBound(0, pct, 100);
            if (pct != lastPercent) {
                lastPercent = pct;
                emit progress(pct);
            }
        }
    }

    if (!m_cancelled) {
        emit progress(100);
        emit finished(indices);
    }
}
