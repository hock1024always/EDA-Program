#include "bookshelf_parser.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <limits>

namespace eda {

BookShelfParser::BookShelfParser() = default;

std::string BookShelfParser::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}

bool BookShelfParser::isValidLine(const std::string& line) {
    std::string trimmed = trim(line);
    return !trimmed.empty() && trimmed[0] != '#';
}

std::string BookShelfParser::getFullPath(const std::string& filename) {
    return base_path_ + filename;
}

PinDirection BookShelfParser::convertPinDirection(const std::string& dir) {
    if (dir == "I" || dir == "INPUT") return PinDirection::INPUT;
    if (dir == "O" || dir == "OUTPUT") return PinDirection::OUTPUT;
    if (dir == "B" || dir == "INOUT") return PinDirection::INOUT;
    return PinDirection::UNKNOWN;
}

bool BookShelfParser::parse(const std::string& aux_file, CircuitModel& circuit) {
    std::cout << "Starting BookShelf format parsing...\n";
    std::cout << "AUX file: " << aux_file << "\n\n";

    current_circuit_ = &circuit;
    circuit.clear();

    // Extract base path
    size_t last_slash = aux_file.find_last_of("/\\");
    if (last_slash != std::string::npos) {
        base_path_ = aux_file.substr(0, last_slash + 1);
    }

    // Parse AUX file to get file list
    std::vector<std::string> files;
    if (!parseAuxFile(aux_file, files)) {
        last_error_ = "Failed to parse AUX file";
        std::cerr << "Error: " << last_error_ << "\n";
        return false;
    }

    // Parse each file in order
    for (const auto& file : files) {
        std::string full_path = getFullPath(file);

        if (file.find(".nodes") != std::string::npos) {
            std::cout << "Parsing NODES file: " << file << "\n";
            if (!parseNodesFile(full_path)) {
                last_error_ = "Failed to parse NODES file: " + file;
                std::cerr << "Error: " << last_error_ << "\n";
                return false;
            }
        }
        else if (file.find(".nets") != std::string::npos) {
            std::cout << "Parsing NETS file: " << file << "\n";
            if (!parseNetsFile(full_path)) {
                last_error_ = "Failed to parse NETS file: " + file;
                std::cerr << "Error: " << last_error_ << "\n";
                return false;
            }
        }
        else if (file.find(".scl") != std::string::npos) {
            std::cout << "Parsing SCL file: " << file << "\n";
            if (!parseSclFile(full_path)) {
                last_error_ = "Failed to parse SCL file: " + file;
                std::cerr << "Error: " << last_error_ << "\n";
                return false;
            }
        }
        else if (file.find(".pl") != std::string::npos) {
            std::cout << "Parsing PL file: " << file << "\n";
            if (!parsePlFile(full_path)) {
                last_error_ = "Failed to parse PL file: " + file;
                std::cerr << "Error: " << last_error_ << "\n";
                return false;
            }
        }
        else if (file.find(".wts") != std::string::npos) {
            std::cout << "Parsing WTS file: " << file << " (optional, skipping)\n";
            // WTS files are optional, skip if not needed
        }
    }

    // Update pin positions after all parsing is done
    for (const auto& net : circuit.nets) {
        for (const auto& pin : net->pins) {
            pin->updateAbsolutePosition();
        }
        net->updateBoundingBox();
    }

    std::cout << "\nParsing completed successfully!\n";
    std::cout << "  Modules: " << circuit.getNumModules() << "\n";
    std::cout << "  Nets: " << circuit.getNumNets() << "\n";
    std::cout << "  Pins: " << circuit.getNumPins() << "\n";
    std::cout << "  Rows: " << circuit.rows.size() << "\n";

    return true;
}

bool BookShelfParser::parseAuxFile(const std::string& filename,
                                    std::vector<std::string>& files) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        last_error_ = "Cannot open file: " + filename;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (!isValidLine(line)) continue;

        std::istringstream iss(line);
        std::string token;

        // Skip "RowBasedPlacement :" part
        iss >> token;
        if (token == "RowBasedPlacement") {
            iss >> token; // Skip ":"

            // Read file list
            while (iss >> token) {
                files.push_back(token);

                // Extract design name from first .nodes file
                if (current_circuit_->name.empty() && token.find(".nodes") != std::string::npos) {
                    size_t dot_pos = token.find('.');
                    if (dot_pos != std::string::npos) {
                        current_circuit_->name = token.substr(0, dot_pos);
                    }
                }
            }
        }
    }

    file.close();
    return !files.empty();
}

bool BookShelfParser::parseNodesFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        last_error_ = "Cannot open file: " + filename;
        return false;
    }

    std::string line;
    int num_nodes = 0;
    int num_terminals = 0;
    int node_count = 0;
    int terminal_count = 0;

    while (std::getline(file, line)) {
        if (!isValidLine(line)) continue;

        std::string trimmed = trim(line);

        // Read node count
        if (trimmed.find("NumNodes") == 0) {
            std::istringstream iss(trimmed);
            std::string key, colon;
            iss >> key >> colon >> num_nodes;
            continue;
        }

        // Read terminal count
        if (trimmed.find("NumTerminals") == 0) {
            std::istringstream iss(trimmed);
            std::string key, colon;
            iss >> key >> colon >> num_terminals;
            continue;
        }

        // Skip UCLA format line
        if (trimmed.find("UCLA") == 0) continue;

        // Parse node data: name width height [terminal] [terminal_NI]
        std::istringstream iss(trimmed);
        std::string name;
        double width, height;
        std::string terminal_flag;

        iss >> name >> width >> height;

        auto module = std::make_shared<Module>(name);
        module->width = width;
        module->height = height;

        // Check if terminal
        if (iss >> terminal_flag) {
            if (terminal_flag == "terminal" || terminal_flag == "terminal_NI") {
                module->type = ModuleType::TERMINAL;
                module->is_fixed = true;
                terminal_count++;
            }
        }

        // If not terminal, it's a standard cell
        if (module->type == ModuleType::UNKNOWN) {
            module->type = ModuleType::STANDARD_CELL;
        }

        current_circuit_->addModule(module);
        node_count++;
    }

    file.close();

    if (num_nodes > 0 && node_count != num_nodes) {
        std::cerr << "Warning: Expected " << num_nodes << " nodes, parsed " << node_count << "\n";
    }

    return true;
}

bool BookShelfParser::parseNetsFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        last_error_ = "Cannot open file: " + filename;
        return false;
    }

    std::string line;
    int num_nets = 0;
    int num_pins = 0;
    NetPtr current_net = nullptr;
    int current_degree = 0;

    while (std::getline(file, line)) {
        if (!isValidLine(line)) continue;

        std::string trimmed = trim(line);

        // Read net count
        if (trimmed.find("NumNets") == 0) {
            std::istringstream iss(trimmed);
            std::string key, colon;
            iss >> key >> colon >> num_nets;
            continue;
        }

        // Read pin count
        if (trimmed.find("NumPins") == 0) {
            std::istringstream iss(trimmed);
            std::string key, colon;
            iss >> key >> colon >> num_pins;
            continue;
        }

        // Skip UCLA format line
        if (trimmed.find("UCLA") == 0) continue;

        // Parse net degree: NetDegree : degree netName
        if (trimmed.find("NetDegree") == 0) {
            std::istringstream iss(trimmed);
            std::string key, colon, net_name;
            iss >> key >> colon >> current_degree >> net_name;

            current_net = std::make_shared<Net>(net_name);
            current_circuit_->addNet(current_net);
            continue;
        }

        // Parse pin: moduleName direction : offsetX offsetY
        if (current_net != nullptr && current_degree > 0) {
            std::istringstream iss(trimmed);
            std::string module_name, direction, colon;
            double offset_x, offset_y;

            iss >> module_name >> direction >> colon >> offset_x >> offset_y;

            ModulePtr module = current_circuit_->getModule(module_name);
            if (module) {
                auto pin = std::make_shared<Pin>(module_name + "_" + current_net->name);
                pin->direction = convertPinDirection(direction);
                pin->offset = Point2D(offset_x, offset_y);
                pin->parent_module = module;

                module->addPin(pin);
                current_net->addPin(pin);
            }

            current_degree--;
            if (current_degree == 0) {
                current_net = nullptr;
            }
        }
    }

    file.close();
    return true;
}

bool BookShelfParser::parseSclFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        last_error_ = "Cannot open file: " + filename;
        return false;
    }

    std::string line;
    int num_rows = 0;
    bool in_row = false;
    Row current_row;

    double min_x = std::numeric_limits<double>::max();
    double max_x = std::numeric_limits<double>::lowest();
    double min_y = std::numeric_limits<double>::max();
    double max_y = std::numeric_limits<double>::lowest();

    while (std::getline(file, line)) {
        if (!isValidLine(line)) continue;

        std::string trimmed = trim(line);

        // Read row count
        if (trimmed.find("NumRows") == 0) {
            std::istringstream iss(trimmed);
            std::string key, colon;
            iss >> key >> colon >> num_rows;
            continue;
        }

        // Skip UCLA format line
        if (trimmed.find("UCLA") == 0) continue;

        // Start new row
        if (trimmed.find("CoreRow") == 0) {
            in_row = true;
            current_row = Row();
            continue;
        }

        // End of row
        if (trimmed.find("End") == 0 && in_row) {
            current_circuit_->addRow(current_row);
            in_row = false;
            continue;
        }

        // Parse row attributes
        if (in_row) {
            std::istringstream iss(trimmed);
            std::string key, colon;
            iss >> key >> colon;

            if (key == "Coordinate") {
                iss >> current_row.y_coord;
                min_y = std::min(min_y, current_row.y_coord);
                max_y = std::max(max_y, current_row.y_coord);
            }
            else if (key == "Height") {
                iss >> current_row.height;
            }
            else if (key == "Sitewidth") {
                double site_width;
                iss >> site_width;
                current_circuit_->site_width = site_width;
            }
            else if (key == "Sitespacing") {
                double site_spacing;
                iss >> site_spacing;
            }
            else if (key == "SubrowOrigin") {
                double origin;
                std::string num_sites_key, colon2;
                int num_sites;
                iss >> origin >> num_sites_key >> colon2 >> num_sites;

                current_row.x_min = origin;
                current_row.x_max = origin + num_sites * current_circuit_->site_width;

                min_x = std::min(min_x, current_row.x_min);
                max_x = std::max(max_x, current_row.x_max);
            }
        }
    }

    // Set die area based on rows
    if (!current_circuit_->rows.empty()) {
        double row_height = current_circuit_->rows[0].height;
        current_circuit_->row_height = row_height;
        current_circuit_->die_area.x_min = min_x;
        current_circuit_->die_area.y_min = min_y;
        current_circuit_->die_area.x_max = max_x;
        current_circuit_->die_area.y_max = max_y + row_height;
    }

    file.close();
    return true;
}

bool BookShelfParser::parsePlFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        last_error_ = "Cannot open file: " + filename;
        return false;
    }

    std::string line;

    while (std::getline(file, line)) {
        if (!isValidLine(line)) continue;

        std::string trimmed = trim(line);

        // Skip UCLA format line
        if (trimmed.find("UCLA") == 0) continue;

        // Parse position: moduleName x y : orientation [/FIXED]
        std::istringstream iss(trimmed);
        std::string module_name, colon, orientation;
        double x, y;
        std::string fixed_flag;

        iss >> module_name >> x >> y >> colon >> orientation;

        ModulePtr module = current_circuit_->getModule(module_name);
        if (module) {
            // BookShelf uses center position, we store lower-left
            module->setCenter(Point2D(x, y));
            module->is_placed = true;

            // Check if fixed
            if (iss >> fixed_flag && fixed_flag == "/FIXED") {
                module->is_fixed = true;
            }
        }
    }

    file.close();
    return true;
}

bool BookShelfParser::parseWtsFile(const std::string& filename) {
    // WTS files are typically empty or contain net weights
    // For now, we skip detailed parsing as weights default to 1.0
    std::ifstream file(filename);
    if (!file.is_open()) {
        // WTS is optional, don't fail if not present
        return true;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (!isValidLine(line)) continue;

        // Parse net weights if present
        // Format: netName weight
        std::istringstream iss(line);
        std::string net_name;
        double weight;

        if (iss >> net_name >> weight) {
            NetPtr net = current_circuit_->getNet(net_name);
            if (net) {
                net->weight = weight;
            }
        }
    }

    file.close();
    return true;
}

} // namespace eda
