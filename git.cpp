#include <iostream>
#include <filesystem>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <chrono>  // For timestamp
#include <vector>
#include <sstream>
#include <algorithm>
#include <set>
#include <functional>




// Class representing a single commit in the repository
class Commit {
public:
    std::string id;
    std::string timestamp;
    std::string author;
    std::string message;

    // Serialize the commit data into a string format
    std::string serialize() const {
        return id + "|" + timestamp + "|" + author + "|" + message;
    }

    // Deserialize a string back into a Commit object
    static Commit deserialize(const std::string& data) {
        std::istringstream iss(data);
        Commit commit;
        std::string part;

        // Extract and set the various fields of the commit
        // Extract id
        std::getline(iss, part, '|');
        commit.id = part;

        // Extract timestamp
        std::getline(iss, part, '|');
        commit.timestamp = part;

        // Extract author
        std::getline(iss, part, '|');
        commit.author = part;

        // Extract message
        std::getline(iss, part, '|');
        commit.message = part;

        return commit;
    }
};

//Class representing the MiniGit functionality
class MiniGit {
private:
    std::vector<Commit> commitHistory;  //Store the history of commits
    std::string historyFilePath = "./.mygit/commit_history.txt";
    std::string repoPath = "./.mygit";

    // Ensure that the specified directory exists
    void ensureDirectoryExists(const std::string& path) {
        if (!std::filesystem::exists(path)) {
            std::filesystem::create_directory(path);
            std::cout << "Created directory: " << path << std::endl;
        }
    }

    // Save the commit history to a file
    void saveCommitHistory() {
        ensureDirectoryExists(repoPath); // Ensure .mygit exists
        std::ofstream file(historyFilePath, std::ios::out | std::ios::trunc);
        if (!file) {
            std::cerr << "Error: Unable to open file for writing: " << historyFilePath << std::endl;
            return;
        }
        for (const auto& commit : commitHistory) {
            std::string serializedCommit = commit.serialize();
            //std::cout << "Debug: Writing to file: " << serializedCommit << std::endl; // Debug print
            file << serializedCommit << std::endl;
        }
        //std::cout << "Commit history saved to " << historyFilePath << std::endl;
    }


    // Load the commit history from a file
    void loadCommitHistory() {
        std::ifstream file(historyFilePath);
        if (!file) {
            std::cerr << "Error: Unable to open file for reading: " << historyFilePath << std::endl;
            return;
        }
        std::string line;
        while (std::getline(file, line)) {
            //std::cout << "Debug: Read from file: " << line << std::endl; // Debug print
            commitHistory.push_back(Commit::deserialize(line));
        }
        //std::cout << "Commit history loaded." << std::endl;
    }

    // Display the commit history
    void viewCommitHistory() {
        if (commitHistory.empty()) {
            std::cout << "No commits have been made yet." << std::endl;
            return;
        }

        std::cout << "Commit History:" << std::endl;
        for (const auto& commit : commitHistory) {
            std::cout << "Commit ID: " << commit.id << std::endl;
            std::cout << "Timestamp: " << commit.timestamp << std::endl;
            std::cout << "Author: " << commit.author << std::endl;
            std::cout << "Message: " << commit.message << std::endl;
            std::cout << std::endl;
        }
    }

    // Generate a hash value for a file's contents
    size_t hashFile(const std::string& filePath) {
        std::ifstream file(filePath);
        std::string fileContent((std::istreambuf_iterator<char>(file)),
                                 std::istreambuf_iterator<char>());
        return std::hash<std::string>{}(fileContent);
    }


public:
    // Constructor
    MiniGit() {
        try {
            loadCommitHistory();
        } catch (const std::exception& e) {
            std::cerr << "Exception during initialization: " << e.what() << std::endl;
        }
    }

    // Destructor
   ~MiniGit() {
        try {
            saveCommitHistory();
        } catch (const std::exception& e) {
            std::cerr << "Exception during destruction: " << e.what() << std::endl;
        }
    }


    // Initialize a new repository
    void initRepository() {
        if (!std::filesystem::exists(repoPath)) {
            std::filesystem::create_directory(repoPath);
            std::filesystem::create_directory(repoPath + "/commits");
            std::filesystem::create_directory(repoPath + "/staging");
            std::cout << "Repository initialized." << std::endl;
        } else {
            std::cout << "Repository already exists." << std::endl;
        }
    }

// Add a file to the staging area
void addFile(const std::string& filename) {
    try{
    std::string stagingPath = "./.mygit/staging/" + filename;
    std::string originalPath = "./" + filename;

    // Check if the file exists in the project directory
    if (!std::filesystem::exists(originalPath)) {
        std::cout << "File does not exist in the project directory." << std::endl;
        return;
    }
    // If the file exists in staging, append changes to it
    if (std::filesystem::exists(stagingPath)) {
        // Open the original file and the staging file
        std::ifstream src(originalPath, std::ios::binary);
        std::ofstream dst(stagingPath, std::ios::binary | std::ios::app);

        // Check if the original file has content to append
        if (src.peek() != std::ifstream::traits_type::eof()) {
            // Append content from the original file to the staging file
            dst << src.rdbuf();
            std::cout << "Appended content to file in staging: " << filename << std::endl;
        } else {
            std::cout << "No new content to append for: " << filename << std::endl;
        }
    } else {
        // If the file is not in staging, add it as a new file
        std::filesystem::copy(originalPath, stagingPath, std::filesystem::copy_options::overwrite_existing);
        std::cout << "Added new file to staging: " << filename << std::endl;
    }
    }catch (const std::exception& e) {
            std::cerr << "Exception in addFile: " << e.what() << std::endl;
        }
    }

    
   




    std::string formatTimestamp(std::chrono::system_clock::time_point tp) {
        std::time_t time = std::chrono::system_clock::to_time_t(tp);
        std::string timestamp = std::ctime(&time);

        // Trim the newline character from the end of the string
        timestamp.erase(std::remove(timestamp.begin(), timestamp.end(), '\n'), timestamp.end());
        return timestamp;
    }


// Commit the staged changes
void commit(const std::string& author, const std::string& message) {
    try{
   auto now = std::chrono::system_clock::now();
    std::string commitID = std::to_string(now.time_since_epoch().count());
    std::string commitPath = "./.mygit/commits/" + commitID;
    std::filesystem::create_directory(commitPath);

    std::string stagingPath = "./.mygit/staging";
    std::string projectPath = "./"; // Assuming project files are in the current directory

    // Automatically add modified files to staging
    for (const auto& entry : std::filesystem::directory_iterator(projectPath)) {
        if (!entry.is_directory()) {
            std::string filename = entry.path().filename().string();
            std::string filePathInProject = projectPath + filename;
            std::string filePathInStaging = stagingPath + "/" + filename;

            // If the file exists in the last commit and has been modified, add it to staging
            if (std::filesystem::exists(filePathInStaging) &&
                hashFile(filePathInStaging) != hashFile(filePathInProject)) {
                std::filesystem::copy(filePathInProject, filePathInStaging, std::filesystem::copy_options::overwrite_existing);
            }
        }
    }

    // Commit the files in staging
    for (const auto& entry : std::filesystem::directory_iterator(stagingPath)) {
        std::string filename = entry.path().filename().string();
        std::filesystem::copy(entry.path(), commitPath + "/" + filename, std::filesystem::copy_options::overwrite_existing);
    }

    Commit newCommit;
    newCommit.id = commitID;
    newCommit.timestamp = formatTimestamp(now);
    newCommit.author = author;
    newCommit.message = message;

    commitHistory.push_back(newCommit);
    saveCommitHistory();

    std::cout << "Committed as " << commitID << std::endl;
    }catch (const std::exception& e) {
            std::cerr << "Exception in commit: " << e.what() << std::endl;
        }
    }
    
 


// Check the status of the staging area
void gitStatus() {
    try{
    std::string stagingPath = "./.mygit/staging";
    std::string commitsPath = "./.mygit/commits";
    std::set<std::string> committedFiles;

    bool isStagingEmpty = std::filesystem::is_empty(stagingPath);
    bool uncommittedFilesFound = false;

    // Collect all filenames from commit folders
    for (const auto& commitFolder : std::filesystem::directory_iterator(commitsPath)) {
        for (const auto& file : std::filesystem::directory_iterator(commitFolder.path())) {
            committedFiles.insert(file.path().filename().string());
        }
    }
 

    // Check for files in staging that are not in any commits folder
    for (const auto& file : std::filesystem::directory_iterator(stagingPath)) {
        std::string filename = file.path().filename().string();
        if (committedFiles.find(filename) == committedFiles.end()) {
            // File is in staging but not committed
            std::cout << "Uncommitted file: " << filename << std::endl;            
            uncommittedFilesFound = true;

        } else {
            // File is in staging and has a committed version, check if modified
            std::string filePathInStaging = stagingPath + "/" + filename;
            std::string filePathInProject = "./" + filename;
            if (hashFile(filePathInStaging) != hashFile(filePathInProject)) {
                std::cout << "Modified and uncommitted file: " << filename << std::endl;
                uncommittedFilesFound = true;
            }
        }
    }
    if (isStagingEmpty || !uncommittedFilesFound) {
        std::cout << "No files in staging area to commit." << std::endl;
    }
    }catch (const std::exception& e) {
            std::cerr << "Exception in gitStatus: " << e.what() << std::endl;
        }

}


    
    // Process user commands
    void processCommand(int argc, char* argv[]) {
        try{if (argc < 2) {
            std::cout << "Usage: " << argv[0] << " <command> [args]" << std::endl;
            return;
        }

        std::string command = argv[1];

        if (command == "init") {
            initRepository();
        } else if (command == "addFile") {
            if (argc < 3) {
                std::cout << "Usage: " << argv[0] << " addFile <filename>" << std::endl;
            } else {
                std::string filename = argv[2];
                addFile(filename);
            }
        } else if (command == "commit") {
            if (argc < 4) {
                std::cout << "Usage: " << argv[0] << " commit <author> <message>" << std::endl;
            } else {
                std::string author = argv[2];
                std::string message;
                for (int i = 3; i < argc; ++i) {
                    message += std::string(argv[i]) + (i < argc - 1 ? " " : "");
                }
                commit(author, message);
            }
        } else if (command == "viewCommitHistory") {
            viewCommitHistory();
        } else if (command == "status") {
            gitStatus();
        } else {
            std::cout << "Unknown command : " << command << std::endl;
        }

        }catch (const std::exception& e) {
            std::cerr << "Exception in processCommand: " << e.what() << std::endl;
        }
    }

        
    };

int main(int argc, char* argv[]) {
    try {
        MiniGit git;
        if (argc >= 2) {
            git.processCommand(argc, argv);
        } else {
            std::cout << "Usage: " << argv[0] << " <command> [args]" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Unhandled Exception: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}