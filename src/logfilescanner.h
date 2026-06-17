#pragma once

#include <QStringList>

// Scans a directory for *.log files and returns them sorted oldest-dump-first,
// active (non-dump) files appended last.
//
// Dump file naming: <base>_YYYYMMDDHHMMSSxxxxxx.log  (20-digit timestamp suffix)
// Active file naming: any other *.log
class LogFileScanner
{
public:
    static QStringList scan(const QString &directory);
};
