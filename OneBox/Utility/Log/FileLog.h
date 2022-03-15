/**********************************************************************
PACKAGE:        Log
FILE:           FileLog.h
COPYRIGHT (C):  All rights reserved.

PURPOSE:        Write log to a file
**********************************************************************/

#ifndef FILELOG_H
#define FILELOG_H

#include <QString>

class FileLog
{
public:
    FileLog();
    ~FileLog();

    /*-----------------------------------------------------------------------
    FUNCTION:		addLogToFile
    PURPOSE:		Add log string to file
    ARGUMENTS:		QString logStr -- log string
    RETURNS:		true, successful
                    false, failed
    -----------------------------------------------------------------------*/
    bool addLogToFile(QString logStr = "");

    /*-----------------------------------------------------------------------
    FUNCTION:		addLogToFile
    PURPOSE:		Add log string to file
    ARGUMENTS:		const char* logStr -- log buffer pointer
                    int len            -- log length
    RETURNS:		true, successful
                    false, failed
    -----------------------------------------------------------------------*/
    bool addLogToFile(const char* logStr, int len);

    /*-----------------------------------------------------------------------
    FUNCTION:		setLogPath
    PURPOSE:		Set the path of log file
    ARGUMENTS:		QString newLogPath -- log path with QString format
    RETURNS:		None
    -----------------------------------------------------------------------*/
    void setLogPath(QString newLogPath);    // Set Log Path

private:

    QString logRootPath;
    QString fullLogPath;
};

#endif // FILELOG_H
