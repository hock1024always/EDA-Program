#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

namespace eda {

/**
 * @brief Read entire file into string
 */
std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Cannot open file: " << filename << "\n";
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

/**
 * @brief Write string to file
 */
bool writeFile(const std::string& filename, const std::string& content) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Cannot create file: " << filename << "\n";
        return false;
    }

    file << content;
    return true;
}

/**
 * @brief Check if file exists
 */
bool fileExists(const std::string& filename) {
    return std::filesystem::exists(filename);
}

/**
 * @brief Get file extension
 */
std::string getFileExtension(const std::string& filename) {
    size_t dot_pos = filename.find_last_of('.');
    if (dot_pos != std::string::npos) {
        return filename.substr(dot_pos + 1);
    }
    return "";
}

/**
 * @brief Get directory from path
 */
std::string getDirectory(const std::string& path) {
    size_t last_slash = path.find_last_of("/\\");
    if (last_slash != std::string::npos) {
        return path.substr(0, last_slash + 1);
    }
    return "";
}

/**
 * @brief Get filename from path
 */
std::string getFilename(const std::string& path) {
    size_t last_slash = path.find_last_of("/\\");
    if (last_slash != std::string::npos) {
        return path.substr(last_slash + 1);
    }
    return path;
}

} // namespace eda
