#pragma once

#include <QVector>
#include "oplogentry.h"

// Loads a single page of log entries from disk by seeking to the stored byte
// offsets.  No global cache is kept; every page fetch reads from the files.
class LogDataLoader
{
public:
    static const int PAGE_SIZE = 50;

    // Load up to PAGE_SIZE entries for the given 0-based page number.
    static QVector<OpLogEntry> loadPage(const QVector<LogLineIndex> &indices,
                                        int page);
};
