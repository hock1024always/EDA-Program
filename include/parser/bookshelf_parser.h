#ifndef EDA_PARSER_BOOKSHELF_PARSER_H
#define EDA_PARSER_BOOKSHELF_PARSER_H

#include <string>
#include <vector>
#include <memory>
#include "circuit_model.h"

namespace eda {

/**
 * @brief BookShelf format parser
 *
 * Parses industry-standard BookShelf format files used in VLSI placement benchmarks.
 * Supports: .aux, .nodes, .nets, .scl, .pl, .wts files
 */
class BookShelfParser {
public:
    BookShelfParser();
    ~BookShelfParser() = default;

    /**
     * @brief Parse a BookShelf design from .aux file
     * @param aux_file Path to the .aux file
     * @param circuit Output circuit model to populate
     * @return true if parsing succeeded
     */
    bool parse(const std::string& aux_file, CircuitModel& circuit);

    /**
     * @brief Get the last error message
     */
    std::string getLastError() const { return last_error_; }

private:
    std::string base_path_;
    std::string last_error_;
    CircuitModel* current_circuit_ = nullptr;

    // Helper functions
    std::string trim(const std::string& str);
    bool isValidLine(const std::string& line);
    std::string getFullPath(const std::string& filename);

    // File parsers
    bool parseAuxFile(const std::string& filename, std::vector<std::string>& files);
    bool parseNodesFile(const std::string& filename);
    bool parseNetsFile(const std::string& filename);
    bool parseSclFile(const std::string& filename);
    bool parsePlFile(const std::string& filename);
    bool parseWtsFile(const std::string& filename);

    // Pin direction conversion
    PinDirection convertPinDirection(const std::string& dir);
};

} // namespace eda

#endif // EDA_PARSER_BOOKSHELF_PARSER_H
