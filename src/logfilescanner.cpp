#include "logfilescanner.h"

#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QList>
#include <QPair>
#include <algorithm>

QStringList LogFileScanner::scan(const QString &directory)
{
    QDir dir(directory);
    QFileInfoList files = dir.entryInfoList(QStringList{"*.log"}, QDir::Files);

    // Dump files end with _<20 digits>.log
    static const QRegularExpression dumpRx("_(\\d{20})\\.log$");

    QList<QPair<QString, QString>> dumps; // (timestamp-string, abs-path)
    QStringList active;

    for (const QFileInfo &fi : files) {
        const QRegularExpressionMatch m = dumpRx.match(fi.fileName());
        if (m.hasMatch()) {
            dumps.append(qMakePair(m.captured(1), fi.absoluteFilePath()));
        } else {
            active.append(fi.absoluteFilePath());
        }
    }

    // Sort dumps by timestamp string (lexicographic == chronological)
    std::sort(dumps.begin(), dumps.end());

    QStringList result;
    for (const auto &p : dumps) {
        result.append(p.second);
    }
    result.append(active);
    return result;
}
