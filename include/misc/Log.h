/**
 * @file Log.h
 * @brief Usage: Configure global variable structlog and then use like: LOG(LOGLEVEL) << "Your messages";
 * @author Alberto Lepe <dev@alepe.com> <https://stackoverflow.com/a/32262143/18689693>
 * @author Alexander Howel <howelale@b-tu.de>
 */

#ifndef LOG_H
#define LOG_H

#include <iostream>

enum typelog
{
    INFO,
    WARN,
    ERROR,
    DEBUG
};

struct structlog
{
    bool headers = true;
    typelog level = ERROR;
};

extern structlog LOGCFG;

using namespace std;

class LOG
{
public:
    LOG() {}
    LOG(typelog type)
    {
        msglevel = type;
        if (LOGCFG.headers)
        {
            operator<<("[" + getLabel(type) + "] ");
        }
    }
    ~LOG()
    {
        if (opened)
        {
            cout << endl;
        }
        opened = false;
    }
    template <class T>
    LOG &operator<<(const T &msg)
    {
        if (msglevel <= LOGCFG.level)
        {
            cout << msg;
            opened = true;
        }
        return *this;
    }

private:
    bool opened = false;
    typelog msglevel = DEBUG;
    inline string getLabel(typelog type)
    {
        string label;
        switch (type)
        {
        case DEBUG:
            label = "DEBUG";
            break;
        case INFO:
            label = "INFO";
            break;
        case WARN:
            label = "WARN";
            break;
        case ERROR:
            label = "ERROR";
            break;
        }
        return label;
    }
};

#endif /* LOG_H */