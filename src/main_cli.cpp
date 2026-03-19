#include <iostream>
#include <string>
#include <cstring>
#include <memory>
#include "circuit_model.h"
#include "bookshelf_parser.h"
#include "verilog_parser.h"
#include "initial_placer.h"
#include "global_placer.h"
#include "placement_visualizer.h"

using namespace eda;

void printUsage(const char* program) {
    std::cout << "Usage: " << program << " <command> [options] <input_file>\n";
    std::cout << "\nCommands:\n";
    std::cout << "  parse       Parse and display circuit information\n";
    std::cout << "  place       Run placement algorithms\n";
    std::cout << "  visualize   Export placement visualization\n";
    std::cout << "\nOptions:\n";
    std::cout << "  -h, --help              Show this help message\n";
    std::cout << "  -f, --format <format>   Input format: bookshelf, verilog\n";
    std::cout << "  -o, --output <file>     Output file\n";
    std::cout << "  -a, --algorithm <alg>   Placement algorithm: initial, global, both (default: both)\n";
    std::cout << "  -v, --verbose           Verbose output\n";
    std::cout << "  --visualize             Generate visualization images\n";
    std::cout << "\nExamples:\n";
    std::cout << "  " << program << " parse design.aux\n";
    std::cout << "  " << program << " place -a both -o output.pl design.aux\n";
    std::cout << "  " << program << " place --visualize design.aux\n";
    std::cout << "  " << program << " visualize -o image.bmp design.aux\n";
}

bool parseInput(const std::string& input_file, const std::string& format,
                CircuitModel& circuit) {
    if (format == "bookshelf" || input_file.find(".aux") != std::string::npos) {
        std::cout << "Parsing BookShelf format: " << input_file << "\n";
        BookShelfParser parser;
        if (!parser.parse(input_file, circuit)) {
            std::cerr << "Parse error: " << parser.getLastError() << "\n";
            return false;
        }
    }
    else if (format == "verilog" || input_file.find(".v") != std::string::npos) {
        std::cout << "Parsing Verilog format: " << input_file << "\n";
        VerilogParser parser;
        if (!parser.parse(input_file, circuit)) {
            std::cerr << "Parse error: " << parser.getLastError() << "\n";
            return false;
        }
    }
    else {
        std::cerr << "Unknown format. Use -f to specify format.\n";
        return false;
    }
    return true;
}

int cmdParse(int argc, char* argv[], int start_idx) {
    std::string input_file;
    std::string format;
    bool verbose = false;

    for (int i = start_idx; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-f" || arg == "--format") {
            if (i + 1 < argc) format = argv[++i];
        }
        else if (arg == "-v" || arg == "--verbose") {
            verbose = true;
        }
        else if (arg[0] != '-') {
            input_file = arg;
        }
    }

    if (input_file.empty()) {
        std::cerr << "Error: No input file specified\n";
        return 1;
    }

    CircuitModel circuit;
    if (!parseInput(input_file, format, circuit)) {
        return 1;
    }

    // Print statistics
    std::cout << "\n========== Circuit Statistics ==========\n";
    std::cout << "Design name: " << circuit.name << "\n";
    std::cout << "Modules: " << circuit.getNumModules() << "\n";
    std::cout << "  - Movable: " << circuit.getNumMovableModules() << "\n";
    std::cout << "  - Fixed: " << circuit.getNumFixedModules() << "\n";
    std::cout << "Nets: " << circuit.getNumNets() << "\n";
    std::cout << "Pins: " << circuit.getNumPins() << "\n";

    if (!circuit.rows.empty()) {
        std::cout << "Rows: " << circuit.rows.size() << "\n";
        std::cout << "Die area: " << circuit.die_area.width() << " x "
                  << circuit.die_area.height() << "\n";
        std::cout << "Utilization: " << circuit.calcUtilization() * 100.0 << "%\n";
    }

    if (verbose) {
        std::cout << "\nModule list:\n";
        for (const auto& module : circuit.modules) {
            std::cout << "  " << module->name;
            if (!module->cell_type.empty()) {
                std::cout << " [" << module->cell_type << "]";
            }
            std::cout << " (" << module->width << " x " << module->height << ")";
            if (module->is_fixed) std::cout << " [FIXED]";
            std::cout << "\n";
        }
    }

    return 0;
}

int cmdPlace(int argc, char* argv[], int start_idx) {
    std::string input_file;
    std::string format;
    std::string output_file;
    std::string algorithm = "both";
    bool visualize = false;

    for (int i = start_idx; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-f" || arg == "--format") {
            if (i + 1 < argc) format = argv[++i];
        }
        else if (arg == "-o" || arg == "--output") {
            if (i + 1 < argc) output_file = argv[++i];
        }
        else if (arg == "-a" || arg == "--algorithm") {
            if (i + 1 < argc) algorithm = argv[++i];
        }
        else if (arg == "--visualize") {
            visualize = true;
        }
        else if (arg[0] != '-') {
            input_file = arg;
        }
    }

    if (input_file.empty()) {
        std::cerr << "Error: No input file specified\n";
        return 1;
    }

    CircuitModel circuit;
    if (!parseInput(input_file, format, circuit)) {
        return 1;
    }

    std::cout << "\nInitial HPWL: " << circuit.calcTotalHPWL() << "\n";

    // Run initial placement
    if (algorithm == "initial" || algorithm == "both") {
        std::cout << "\n>>> Running Initial Placement (Kraftwerk2A)\n";
        InitialPlacer placer(circuit);
        if (!placer.run()) {
            std::cerr << "Initial placement failed: " << placer.getLastError() << "\n";
            return 1;
        }
        placer.getStats().print();
    }

    // Run global placement
    if (algorithm == "global" || algorithm == "both") {
        std::cout << "\n>>> Running Global Placement (ePlace-MS)\n";
        GlobalPlacer placer(circuit);
        if (!placer.run()) {
            std::cerr << "Global placement failed: " << placer.getLastError() << "\n";
            return 1;
        }
        placer.getStats().print();
    }

    // Export visualization
    if (visualize) {
        std::cout << "\n>>> Exporting visualization...\n";
        PlacementVisualizer visualizer(circuit);
        visualizer.exportToBMP("placement_result.bmp");
        visualizer.exportDensityMap("density_map.bmp");
    }

    // Export placement
    if (!output_file.empty()) {
        std::cout << "\nExporting placement to: " << output_file << "\n";
        if (circuit.exportToPl(output_file)) {
            std::cout << "Export successful!\n";
        } else {
            std::cerr << "Export failed!\n";
        }
    }

    return 0;
}

int cmdVisualize(int argc, char* argv[], int start_idx) {
    std::string input_file;
    std::string format;
    std::string output_file = "visualization.bmp";

    for (int i = start_idx; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-f" || arg == "--format") {
            if (i + 1 < argc) format = argv[++i];
        }
        else if (arg == "-o" || arg == "--output") {
            if (i + 1 < argc) output_file = argv[++i];
        }
        else if (arg[0] != '-') {
            input_file = arg;
        }
    }

    if (input_file.empty()) {
        std::cerr << "Error: No input file specified\n";
        return 1;
    }

    CircuitModel circuit;
    if (!parseInput(input_file, format, circuit)) {
        return 1;
    }

    std::cout << "\n>>> Exporting visualization...\n";
    PlacementVisualizer visualizer(circuit);

    VisualizerOptions options;
    options.show_modules = true;
    options.show_density = true;

    if (visualizer.exportToBMP(output_file, options)) {
        std::cout << "Visualization saved to: " << output_file << "\n";
    } else {
        std::cerr << "Failed to save visualization!\n";
        return 1;
    }

    return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::string command = argv[1];

    if (command == "-h" || command == "--help") {
        printUsage(argv[0]);
        return 0;
    }
    else if (command == "parse") {
        return cmdParse(argc, argv, 2);
    }
    else if (command == "place") {
        return cmdPlace(argc, argv, 2);
    }
    else if (command == "visualize") {
        return cmdVisualize(argc, argv, 2);
    }
    else {
        // Backward compatibility: treat as parse command
        return cmdParse(argc, argv, 1);
    }
}
