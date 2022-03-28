/**
 * @file main.cpp
 * @class main
 * @author Benoît MOUFLIN
 * @date 2020-11-02
 *
 * @brief The main function
 *
 * @module QLoggerTest
 * @note
 *
 * Copyright (c) 2020 NEXO. All rights reserved.
 * https://nexo-sa.com
 */
#include <QCoreApplication>

#include "QLogger.h"

#include <QDebug>
#include <QTimer>
#include <QDir>

using namespace QLogger;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    qInfo() << "--- QLoggerTest ---";
    qInfo() << "# Welcome";

    static const QString l_file1("test1.log");
    static const QString l_file2("test2.log");

    static const QString l_module1("QLoggerTest");
    static const QString l_module2("TestiiTest");

    QLoggerManager *l_manager = QLoggerManager::getInstance();

    // --- Default features ---

    // Create destination with a given file name
    l_manager->addDestination(l_file1, l_module1, LogLevel::Debug);

    QLog_Debug(l_module1, QStringLiteral("This is a debug log message 0."));
    QLog_Debug(l_module1, QStringLiteral("This is a debug log message 1.."));
    QLog_Debug(l_module1, QStringLiteral("This is a debug log message 2..."));
    QLog_Debug(l_module1, QStringLiteral("This is a debug log message 3...."));

    // Try to create another module of the same name but with a different file name and level - ignoring
    l_manager->addDestination(l_file2, l_module1, LogLevel::Debug);
    // The log message is written into the file1
    QLog_Debug(l_module1, QStringLiteral("This is a debug log message 0."));

    // The module doesn't exist yet - messges are enqueued
    QLog_Debug(l_module2, QStringLiteral("This is a TestiiTest."));
    // Create the corresponding module
    l_manager->addDestination(l_file2, l_module2, LogLevel::Debug);
    QLog_Debug(l_module2, QStringLiteral("This is a TestiiTest two.."));


    // --- New features ---
    static const QString l_file3("test3.log");

    static const QString l_module3("QLoggerTest2");
    static const QString l_module4("TestiiTest2");

    // Create module at the default destination folder and full mode (console and file)
    l_manager->addDestination(l_file3, l_module3, LogLevel::Debug, QString(), LogMode::Full);

    QLog_Debug(l_module3, QStringLiteral("This is a debug log message 0."));
    QLog_Debug(l_module3, QStringLiteral("This is a debug log message 1.."));
    QLog_Debug(l_module3, QStringLiteral("This is a debug log message 2..."));
    QLog_Debug(l_module3, QStringLiteral("This is a debug log message 3...."));

    // The module doesn't exist yet - messges are enqueued
    QLog_Debug(l_module4, QStringLiteral("This is a TestiiTest."));
    // Create the corresponding module with auto-generated filename, default destination folder with a custom log message display
    l_manager->addDestination(QString(), l_module4, LogLevel::Debug, QString(), LogMode::Full
                              , LogFileDisplay::Number, LogMessageDisplay::DateTime|LogMessageDisplay::Message);
    QLog_Debug(l_module4, QStringLiteral("This is a TestiiTest two.."));


    QTimer::singleShot(2500, &a, []() {
        qInfo() << "# Done.";
        exit(0);
    } );

    return a.exec();
}

