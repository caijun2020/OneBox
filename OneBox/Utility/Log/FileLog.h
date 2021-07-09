#ifndef FILELOG_H
#define FILELOG_H

#include <QString>

class FileLog
{
public:
    FileLog();
    ~FileLog();

    bool addLogToFile(QString logStr = "None"); // Add Log to file
    bool addLogToFile(const char* logStr, int len);

    void setLogPath(QString newLogPath);    // Set Log Path

private:

    QString logRootPath;
    QString fullLogPath;
};

#endif // FILELOG_H
