    void log(const std::string& message) {
        ensureDirectoryExists(repoPath);
        std::ofstream logFile(logFilePath, std::ios::app);
        if (logFile.is_open()) {
            logFile << message << std::endl;
        } else {
            std::cerr << "Failed to open log file: " << logFilePath << std::endl;
        }
    }