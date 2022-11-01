# QLogger

In this repository you can find a logger class. Follow the instructions for usage:

1. Create an instance: QLoggerManager *manager = QLoggerManager::getInstance();
2. Add as many destinations as you want:  manager->addDestination(filePathName, module, logLevel);
3. Print the log in the file with: QLog_ followed by Trace/Debug/Info/Warning/Error/Fatal

You can add as much destinations as you want. You also can add several modules for each log file.
