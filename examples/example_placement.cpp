#include <iostream>
#include <memory>
#include "circuit_model.h"
#include "bookshelf_parser.h"
#include "initial_placer.h"
#include "global_placer.h"
#include "placement_visualizer.h"

using namespace eda;

void printUsage(const char* program) {
    std::cout << "Usage: " << program << " <aux_file> [options]\n";
    std::cout << "\nOptions:\n";
    std::cout << "  --initial-only    Run only initial placement\n";
    std::cout << "  --global-only     Run only global placement\n";
    std::cout << "  --visualize       Export visualization images\n";
    std::cout << "  --output <dir>    Output directory for results\n";
}

int main(int argc, char* argv[]) {
    std::cout << "EDA Integrated - Placement Example\n";
    std::cout << "==================================\n\n";

    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::string aux_file = argv[1];
    bool run_initial = true;
    bool run_global = true;
    bool visualize = false;
    std::string output_dir = ".";

    // Parse options
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--initial-only") {
            run_initial = true;
            run_global = false;
        } else if (arg == "--global-only") {
            run_initial = false;
            run_global = true;
        } else if (arg == "--visualize") {
            visualize = true;
        } else if (arg == "--output" && i + 1 < argc) {
            output_dir = argv[++i];
        }
    }

    // Parse input
    CircuitModel circuit;
    BookShelfParser parser;

    std::cout << "Parsing BookShelf design: " << aux_file << "\n";
    if (!parser.parse(aux_file, circuit)) {
        std::cerr << "Failed to parse: " << parser.getLastError() << "\n";
        return 1;
    }

    std::cout << "\nDesign Statistics:\n";
    std::cout << "  Modules: " << circuit.getNumModules() << "\n";
    std::cout << "  Movable: " << circuit.getNumMovableModules() << "\n";
    std::cout << "  Fixed: " << circuit.getNumFixedModules() << "\n";
    std::cout << "  Nets: " << circuit.getNumNets() << "\n";
    std::cout << "  Die Area: " << circuit.die_area.width() << " x "
              << circuit.die_area.height() << "\n";

    // Save initial placement
    if (visualize) {
        PlacementVisualizer visualizer(circuit);
        visualizer.exportToBMP(output_dir + "/initial_placement.bmp");
    }

    // Run initial placement
    if (run_initial) {
        std::cout << "\n>>> Running Initial Placement (Kraftwerk2A)\n";
        InitialPlacer initial_placer(circuit);

        if (!initial_placer.run()) {
            std::cerr << "Initial placement failed: " << initial_placer.getLastError() << "\n";
            return 1;
        }

        initial_placer.getStats().print();

        if (visualize) {
            PlacementVisualizer visualizer(circuit);
            visualizer.exportToBMP(output_dir + "/after_initial_placement.bmp");
        }
    }

    // Run global placement
    if (run_global) {
        std::cout << "\n>>> Running Global Placement (ePlace-MS)\n";
        GlobalPlacer global_placer(circuit);
        global_placer.setTargetOverflow(0.1);
        global_placer.setMaxIterations(200);

        // Progress callback
        global_placer.setProgressCallback([](int iter, double hpwl, double overflow) {
            std::cout << "  Progress: iter=" << iter
                      << " HPWL=" << hpwl
                      << " overflow=" << overflow * 100.0 << "%\n";
        });

        if (!global_placer.run()) {
            std::cerr << "Global placement failed: " << global_placer.getLastError() << "\n";
            return 1;
        }

        global_placer.getStats().print();

        if (visualize) {
            PlacementVisualizer visualizer(circuit);
            visualizer.exportToBMP(output_dir + "/after_global_placement.bmp");
            visualizer.exportDensityMap(output_dir + "/density_map.bmp", 64, 64);
        }
    }

    // Export final placement
    std::string output_pl = output_dir + "/final_placement.pl";
    std::cout << "\nExporting placement to: " << output_pl << "\n";
    if (circuit.exportToPl(output_pl)) {
        std::cout << "Export successful!\n";
    } else {
        std::cerr << "Export failed!\n";
    }

    // Final statistics
    std::cout << "\nFinal Statistics:\n";
    std::cout << "  Total HPWL: " << circuit.calcTotalHPWL() << "\n";
    std::cout << "  Utilization: " << circuit.calcUtilization() * 100.0 << "%\n";

    return 0;
}
