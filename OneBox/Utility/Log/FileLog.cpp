#include "FileLog.h"
#include <QDateTime>
#include <QTime>
#include <QFile>
#include <QTextStream>
#include <QDebug>

FileLog::FileLog()
{
    logRootPath.clear();
    fullLogPath.clear();
    logRootPath.append("./Log/");
}

FileLog::~FileLog()
{
}

bool FileLog::addLogToFile(QString logStr)
{
    bool ret = false;
    QDateTime time = QDateTime::currentDateTime();
    QString timeStr = time.toString("yyyy-MM-dd");  // Time format yyyy-MM-dd
    timeStr.append(".txt"); // Set Log file format as text

    // Init full log path
    fullLogPath.clear();
    fullLogPath.append(logRootPath).append(timeStr);

    QFile logFile(fullLogPath);
    if(logFile.open(QFile::WriteOnly | QFile::Append))
    {
        QTextStream out(&logFile);
        out << logStr.append("\r\n"); // For each log, append an Enter separator
        logFile.close();
        ret = true;
    }

    return ret;
}

bool FileLog::addLogToFile(const char* logStr, int len)
{
    bool ret = false;
    return ret;
}

void FileLog::setLogPath(QString newLogPath)
{
    logRootPath.clear();
    logRootPath.append(newLogPath);
}
