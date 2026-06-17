#pragma once

#include "oplogentry.h"
#include <QString>
#include <QStringList>

// Parses a single log line of the format:
// [timestamp] [username] [terminal] [object] [type] [description] [result] [addition]
// Fields may be empty ([]).
class LogLineParser
{
public:
    // Returns true and fills 'entry' on success; returns false on malformed input.
    static bool parse(const QString &line, OpLogEntry &entry);

private:
    static QStringList extractFields(const QString &line);
    static OperationResult parseResult(const QString &s);
};
