#include "loglineparser.h"

// ---------------------------------------------------------------------------
// Extract all bracket-enclosed fields from a log line.
// Handles nested brackets (though the log format does not use them).
// ---------------------------------------------------------------------------
QStringList LogLineParser::extractFields(const QString &line)
{
    QStringList fields;
    const int n = line.length();
    int i = 0;
    while (i < n) {
        if (line[i] != '[') { ++i; continue; }
        int depth = 1;
        int start = i + 1;
        ++i;
        while (i < n && depth > 0) {
            if      (line[i] == '[') ++depth;
            else if (line[i] == ']') --depth;
            if (depth > 0) ++i;
            else break;
        }
        fields.append(line.mid(start, i - start));
        ++i;
    }
    return fields;
}

OperationResult LogLineParser::parseResult(const QString &s)
{
    if (s == QLatin1String("RESULT_SUCCESS"))   return RESULT_SUCCESS;
    if (s == QLatin1String("RESULT_FAILED"))    return RESULT_FAILED;
    if (s == QLatin1String("RESULT_EXECUTING")) return RESULT_EXECUTING;
    return DEFAULT;
}

bool LogLineParser::parse(const QString &line, OpLogEntry &entry)
{
    QStringList fields = extractFields(line);
    if (fields.size() < 8) return false;

    // Field 0: timestamp, e.g. "2026-06-06 15:49:01.870951"
    // QDateTime supports only millisecond precision; we parse the first 23 chars
    // and keep the full string for display.
    const QString &tsStr = fields[0];
    QDateTime ts;
    if (tsStr.length() >= 23) {
        ts = QDateTime::fromString(tsStr.left(23), "yyyy-MM-dd HH:mm:ss.zzz");
    }
    if (!ts.isValid()) return false;

    entry.rawTimestamp    = tsStr;
    entry.timestamp       = ts;
    entry.username        = fields[1];
    entry.terminal        = fields[2];
    entry.operationObject = fields[3];
    entry.operationType   = fields[4];
    entry.description     = fields[5];
    entry.result          = parseResult(fields[6]);
    entry.addition        = fields[7];
    return true;
}
