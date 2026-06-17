#include "logdataloader.h"
#include "loglineparser.h"

#include <QFile>

QVector<OpLogEntry> LogDataLoader::loadPage(const QVector<LogLineIndex> &indices,
                                             int page)
{
    QVector<OpLogEntry> entries;
    const int start = page * PAGE_SIZE;
    const int end   = qMin(start + PAGE_SIZE, indices.size());

    // Group consecutive indices that share the same file to avoid
    // reopening the same file for every row.
    QString   openPath;
    QFile     file;

    for (int i = start; i < end; ++i) {
        const LogLineIndex &idx = indices[i];

        if (idx.filePath != openPath) {
            if (file.isOpen()) file.close();
            file.setFileName(idx.filePath);
            if (!file.open(QIODevice::ReadOnly)) {
                openPath.clear();
                continue;
            }
            openPath = idx.filePath;
        }

        if (!file.seek(idx.byteOffset)) continue;

        const QString line = QString::fromUtf8(file.readLine()).trimmed();
        OpLogEntry entry;
        if (LogLineParser::parse(line, entry)) {
            entries.append(entry);
        }
    }

    if (file.isOpen()) file.close();
    return entries;
}
