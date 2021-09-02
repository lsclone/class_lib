#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <chrono>
#include <string>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <iostream>

namespace reg {

    /*
    * @brief    export current datetime
    * @return   string, format is yyyy_mm_dd_hh_mm_ss.
    */
    std::string GetTimeStamp() {
        std::stringstream stamp;
        auto now = std::chrono::system_clock::now();
        time_t now_t = std::chrono::system_clock::to_time_t(now);
        auto tm = std::localtime(&now_t);
        stamp << std::setfill('0') << std::setw(4) << tm->tm_year + 1900 << "_"
            << std::setfill('0') << std::setw(2) << tm->tm_mon + 1 << "_"
            << std::setfill('0') << std::setw(2) << tm->tm_mday << "_"
            << std::setfill('0') << std::setw(2) << tm->tm_hour << "_"
            << std::setfill('0') << std::setw(2) << tm->tm_min << "_"
            << std::setfill('0') << std::setw(2) << tm->tm_sec;
        return stamp.str();
    }

    /*
    * @brief    write log (with file maximum size detection)
    * @sample   reg::Log("date: %s, timestemp: %s, errorcode: %u", "10/15", "12:30", 5);
    */
    void Log(const char* format, ...) {
        const int BuffLen = 4096;
        const int FileMaxSize = 10 * 1024 * 1024;

        std::ofstream out;
        // ios::app : append content to the end of file.
        out.open("log.txt", ios::out | ios::app);
        if (!out.is_open()) {
            //throw std::exception("[LOG]open file failed.");
            return;
        }

        out.seekp(0, ios::end);
        if (out.tellp() >= FileMaxSize) {
            // step1: close log file
            out.close();
            // step2: rename log file to log_yyyy_mm_dd_hh_mm_ss.txt.
            std::string hestory = "log_" + GetTimeStamp() + ".txt";
            if (std::rename("log.txt", hestory.data()) != 0) {
                //throw std::exception("[LOG]rename file failed.");
                return;
            }
            // step3: open a new log file
            out.open("log.txt", ios::out | ios::app);
            if (!out.is_open()) {
                //throw std::exception("[LOG]open file failed.");
                return;
            }
        }

        va_list args;
        va_start(args, format);

        char buff[BuffLen] = { 0 };
        //sprintf(buff, format, args); // export garbage characters.
        _vsprintf_l(buff, format, NULL, args);
        out << buff << std::endl;
        out.close();

        va_end(args);
    }

} // namespace reg

/*
* @brief    write log (no file maximum size detection)
* @param    [in]filename    log file name
* @param    [in]format      content to export to log file.
* @sample   MacroSimpLog("log.txt", "date: " << "01/01" << ", timestemp: " << "01:01");
*/
#define MacroSimpLog(filename, format) { \
    std::ofstream out(filename, ios::out | ios::app); \
    if (out.is_open()) { \
        out << format << std::endl; \
        out.close(); \
    } \
}

/*
* @brief    export current datetime
* @return   string, format is yyyy_mm_dd_hh_mm_ss.
*
* @sample   std::string time_stamp;
*           MarcorTimeStamp();
*/
#define MarcorTimeStamp() { \
    std::stringstream stamp; \
    auto now = std::chrono::system_clock::now(); \
    time_t now_t = std::chrono::system_clock::to_time_t(now); \
    auto tm = std::localtime(&now_t); \
    stamp << std::setfill('0') << std::setw(4) << tm->tm_year + 1900 << "_" \
        << std::setfill('0') << std::setw(2) << tm->tm_mon + 1 << "_" \
        << std::setfill('0') << std::setw(2) << tm->tm_mday << "_" \
        << std::setfill('0') << std::setw(2) << tm->tm_hour << "_" \
        << std::setfill('0') << std::setw(2) << tm->tm_min << "_" \
        << std::setfill('0') << std::setw(2) << tm->tm_sec; \
    time_stamp = stamp.str(); \
}

/*
* @brief    write log (with file maximum size detection)
* @param    [in]filename    log file name
* @param    [in]format      content to export to log file.
* @sample   MacroLog("log.txt", "date: " << "01/01" << ", timestemp: " << "01:01");
*/
#define MacroLog(filename, format) { \
    std::ofstream out; \
    out.open(filename, ios::out | ios::app); \
    if (out.is_open()) { \
        out.seekp(0, ios::end); \
        if (out.tellp() >= 10 * 1024 * 1024) { \
            out.close(); \
            std::string time_stamp; \
            MarcorTimeStamp(); \
            std::string hestory(filename); \
            size_t pos = hestory.find_last_of("."); \
            hestory.insert(pos, "_" + time_stamp); \
            if (std::rename(filename, hestory.data()) == 0) { \
                out.open(filename, ios::out | ios::app); \
                if (out.is_open()) { \
                    out << format << std::endl; \
                    out.close(); \
                } \
            } \
        } \
        else { \
            out << format << std::endl; \
            out.close(); \
        } \
    } \
}
