#ifndef EDA_PARSER_VERILOG_PARSER_H
#define EDA_PARSER_VERILOG_PARSER_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_set>
#include "circuit_model.h"

namespace eda {

// Forward declarations
class VerilogLexer;
class VerilogParser;

/**
 * @brief Token types for Verilog lexical analysis
 */
enum class VerilogTokenType {
    Identifier,   // Variable/module names
    Keyword,      // Verilog keywords
    Number,       // Numeric literals
    Symbol,       // Operators and punctuation
    String,       // String literals
    EndOfInput,   // End of file
    Unknown       // Unrecognized token
};

/**
 * @brief Verilog token structure
 */
struct VerilogToken {
    VerilogTokenType type;
    std::string value;
    int line = 0;
    int column = 0;

    VerilogToken(VerilogTokenType t, const std::string& v)
        : type(t), value(v) {}
};

/**
 * @brief Lexer for Verilog files
 */
class VerilogLexer {
public:
    explicit VerilogLexer(const std::string& input);

    VerilogToken nextToken();
    std::string getWholeExpression();

    int getCurrentLine() const { return line_; }

private:
    std::string input_;
    size_t position_;
    int line_;
    int column_;

    static const std::unordered_set<std::string> keywords;

    void skipWhitespace();
    void skipComment();
    char peek() const;
    char advance();
    bool isAtEnd() const;
    VerilogToken readIdentifier();
    VerilogToken readNumber();
    VerilogToken readString();
};

/**
 * @brief Intermediate representation for Verilog module
 */
struct VerilogModule {
    std::string name;
    std::vector<std::string> ports;
    std::vector<std::string> inputs;
    std::vector<std::string> outputs;
    std::vector<std::string> wires;

    // Gates/instances: <instance_name, <cell_type, <port_name, wire_name>>>
    std::vector<std::tuple<std::string, std::string,
        std::vector<std::pair<std::string, std::string>>>> instances;

    // Assign statements: <lhs, rhs_expression>
    std::vector<std::pair<std::string, std::string>> assigns;
};

/**
 * @brief Verilog netlist parser
 *
 * Parses structural Verilog files and converts to CircuitModel
 */
class VerilogParser {
public:
    VerilogParser();
    ~VerilogParser() = default;

    /**
     * @brief Parse a Verilog file
     * @param filename Path to the .v file
     * @param circuit Output circuit model to populate
     * @return true if parsing succeeded
     */
    bool parse(const std::string& filename, CircuitModel& circuit);

    /**
     * @brief Parse Verilog from string
     * @param content Verilog source code
     * @param circuit Output circuit model to populate
     * @return true if parsing succeeded
     */
    bool parseString(const std::string& content, CircuitModel& circuit);

    /**
     * @brief Get the last error message
     */
    std::string getLastError() const { return last_error_; }

private:
    std::string last_error_;
    CircuitModel* current_circuit_ = nullptr;
    std::unique_ptr<VerilogLexer> lexer_;

    // Parsing methods
    bool parseModule(VerilogModule& module);
    bool parseModuleHeader(VerilogModule& module);
    bool parsePortDeclarations(VerilogModule& module);
    bool parseWireDeclarations(VerilogModule& module);
    bool parseInstances(VerilogModule& module);
    bool parseAssignStatements(VerilogModule& module);

    // Helper methods
    VerilogToken expectToken(VerilogTokenType type);
    VerilogToken expectKeyword(const std::string& keyword);
    bool matchToken(VerilogTokenType type);
    bool matchKeyword(const std::string& keyword);

    // Conversion to CircuitModel
    bool convertToCircuitModel(const VerilogModule& vmodule);

    // Standard cell dimensions (simplified - would come from LEF in real flow)
    std::pair<double, double> getCellDimensions(const std::string& cell_type);
};

} // namespace eda

#endif // EDA_PARSER_VERILOG_PARSER_H
