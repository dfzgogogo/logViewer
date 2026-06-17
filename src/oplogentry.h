#pragma once

#include <QString>
#include <QDateTime>

enum OperationResult : int {
    DEFAULT          = 0,
    RESULT_FAILED    = 1,
    RESULT_SUCCESS   = 2,
    RESULT_EXECUTING = 3
};

// Lightweight index: file path + byte offset of the line
struct LogLineIndex {
    QString filePath;
    qint64  byteOffset;
};

// Fully parsed log entry (loaded on demand per page)
struct OpLogEntry {
    QDateTime       timestamp;
    QString         username;
    QString         terminal;
    QString         operationObject;
    QString         operationType;
    QString         description;
    OperationResult result;
    QString         addition;
    // Raw timestamp string preserving microsecond digits for display
    QString         rawTimestamp;
};
