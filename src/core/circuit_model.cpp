#include "circuit_model.h"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>

namespace eda {

// Pin implementation
void Pin::updateAbsolutePosition() {
    if (parent_module) {
        position.x = parent_module->position.x + offset.x;
        position.y = parent_module->position.y + offset.y;
    }
}

// Net implementation
void Net::addPin(PinPtr pin) {
    if (pin) {
        pins.push_back(pin);
        pin->net = shared_from_this();
    }
}

void Net::removePin(PinPtr pin) {
    auto it = std::find(pins.begin(), pins.end(), pin);
    if (it != pins.end()) {
        pins.erase(it);
        if (pin) {
            pin->net.reset();
        }
    }
}

double Net::calcHPWL() const {
    if (pins.size() < 2) return 0.0;

    double x_min = pins[0]->position.x;
    double x_max = pins[0]->position.x;
    double y_min = pins[0]->position.y;
    double y_max = pins[0]->position.y;

    for (const auto& pin : pins) {
        x_min = std::min(x_min, pin->position.x);
        x_max = std::max(x_max, pin->position.x);
        y_min = std::min(y_min, pin->position.y);
        y_max = std::max(y_max, pin->position.y);
    }

    return (x_max - x_min) + (y_max - y_min);
}

void Net::updateBoundingBox() {
    if (pins.empty()) return;

    bbox.x_min = pins[0]->position.x;
    bbox.x_max = pins[0]->position.x;
    bbox.y_min = pins[0]->position.y;
    bbox.y_max = pins[0]->position.y;

    for (const auto& pin : pins) {
        bbox.x_min = std::min(bbox.x_min, pin->position.x);
        bbox.x_max = std::max(bbox.x_max, pin->position.x);
        bbox.y_min = std::min(bbox.y_min, pin->position.y);
        bbox.y_max = std::max(bbox.y_max, pin->position.y);
    }
}

PinPtr Net::getDriver() const {
    for (const auto& pin : pins) {
        if (pin->direction == PinDirection::OUTPUT) {
            return pin;
        }
    }
    return nullptr;
}

std::vector<PinPtr> Net::getLoads() const {
    std::vector<PinPtr> loads;
    for (const auto& pin : pins) {
        if (pin->direction == PinDirection::INPUT) {
            loads.push_back(pin);
        }
    }
    return loads;
}

// Module implementation
void Module::addPin(PinPtr pin) {
    if (pin) {
        pins.push_back(pin);
        pin_map[pin->name] = pin;
        pin->parent_module = shared_from_this();
    }
}

PinPtr Module::getPin(const std::string& pin_name) const {
    auto it = pin_map.find(pin_name);
    if (it != pin_map.end()) {
        return it->second;
    }
    return nullptr;
}

Rectangle Module::getBBox() const {
    return Rectangle(position.x, position.y,
                     position.x + width, position.y + height);
}

bool Module::overlaps(const Module& other) const {
    return !(position.x + width <= other.position.x ||
             other.position.x + other.width <= position.x ||
             position.y + height <= other.position.y ||
             other.position.y + other.height <= position.y);
}

// CircuitModel implementation
void CircuitModel::addModule(ModulePtr module) {
    if (module) {
        modules.push_back(module);
        module_map[module->name] = module;
    }
}

ModulePtr CircuitModel::getModule(const std::string& name) const {
    auto it = module_map.find(name);
    if (it != module_map.end()) {
        return it->second;
    }
    return nullptr;
}

void CircuitModel::removeModule(const std::string& name) {
    auto it = module_map.find(name);
    if (it != module_map.end()) {
        auto mod = it->second;
        modules.erase(std::remove(modules.begin(), modules.end(), mod), modules.end());
        module_map.erase(it);
    }
}

void CircuitModel::addNet(NetPtr net) {
    if (net) {
        nets.push_back(net);
        net_map[net->name] = net;
    }
}

NetPtr CircuitModel::getNet(const std::string& name) const {
    auto it = net_map.find(name);
    if (it != net_map.end()) {
        return it->second;
    }
    return nullptr;
}

void CircuitModel::removeNet(const std::string& name) {
    auto it = net_map.find(name);
    if (it != net_map.end()) {
        auto net = it->second;
        nets.erase(std::remove(nets.begin(), nets.end(), net), nets.end());
        net_map.erase(it);
    }
}

void CircuitModel::addRow(const Row& row) {
    rows.push_back(row);
}

size_t CircuitModel::getNumPins() const {
    size_t count = 0;
    for (const auto& module : modules) {
        count += module->pins.size();
    }
    return count;
}

size_t CircuitModel::getNumMovableModules() const {
    size_t count = 0;
    for (const auto& module : modules) {
        if (!module->is_fixed && module->type == ModuleType::STANDARD_CELL) {
            count++;
        }
    }
    return count;
}

size_t CircuitModel::getNumFixedModules() const {
    size_t count = 0;
    for (const auto& module : modules) {
        if (module->is_fixed) {
            count++;
        }
    }
    return count;
}

double CircuitModel::calcTotalHPWL() const {
    double total_hpwl = 0.0;
    for (const auto& net : nets) {
        total_hpwl += net->calcHPWL() * net->weight;
    }
    return total_hpwl;
}

double CircuitModel::calcUtilization() const {
    double module_area = 0.0;
    for (const auto& module : modules) {
        if (!module->is_fixed && module->type == ModuleType::STANDARD_CELL) {
            module_area += module->width * module->height;
        }
    }

    double row_area = 0.0;
    for (const auto& row : rows) {
        row_area += row.width() * row.height;
    }

    if (row_area > 0) {
        return module_area / row_area;
    }
    return 0.0;
}

std::vector<std::vector<double>> CircuitModel::calcDensityMap(
    int num_bins_x, int num_bins_y) const {

    std::vector<std::vector<double>> density(num_bins_x,
        std::vector<double>(num_bins_y, 0.0));

    if (die_area.width() <= 0 || die_area.height() <= 0) {
        return density;
    }

    double bin_width = die_area.width() / num_bins_x;
    double bin_height = die_area.height() / num_bins_y;

    for (const auto& module : modules) {
        if (module->is_fixed) continue;

        int bin_x_min = static_cast<int>((module->position.x - die_area.x_min) / bin_width);
        int bin_x_max = static_cast<int>((module->position.x + module->width - die_area.x_min) / bin_width);
        int bin_y_min = static_cast<int>((module->position.y - die_area.y_min) / bin_height);
        int bin_y_max = static_cast<int>((module->position.y + module->height - die_area.y_min) / bin_height);

        bin_x_min = std::max(0, bin_x_min);
        bin_x_max = std::min(num_bins_x - 1, bin_x_max);
        bin_y_min = std::max(0, bin_y_min);
        bin_y_max = std::min(num_bins_y - 1, bin_y_max);

        double module_area = module->width * module->height;

        for (int x = bin_x_min; x <= bin_x_max; ++x) {
            for (int y = bin_y_min; y <= bin_y_max; ++y) {
                // Calculate overlap area
                double overlap_x_min = std::max(module->position.x,
                    die_area.x_min + x * bin_width);
                double overlap_x_max = std::min(module->position.x + module->width,
                    die_area.x_min + (x + 1) * bin_width);
                double overlap_y_min = std::max(module->position.y,
                    die_area.y_min + y * bin_height);
                double overlap_y_max = std::min(module->position.y + module->height,
                    die_area.y_min + (y + 1) * bin_height);

                if (overlap_x_max > overlap_x_min && overlap_y_max > overlap_y_min) {
                    double overlap_area = (overlap_x_max - overlap_x_min) *
                                          (overlap_y_max - overlap_y_min);
                    density[x][y] += overlap_area / (bin_width * bin_height);
                }
            }
        }
    }

    return density;
}

void CircuitModel::clear() {
    modules.clear();
    nets.clear();
    rows.clear();
    module_map.clear();
    net_map.clear();
    die_area = DieArea();
}

bool CircuitModel::validate() const {
    // Check for duplicate module names
    if (module_map.size() != modules.size()) {
        std::cerr << "Validation error: Duplicate module names detected\n";
        return false;
    }

    // Check for duplicate net names
    if (net_map.size() != nets.size()) {
        std::cerr << "Validation error: Duplicate net names detected\n";
        return false;
    }

    // Check that all modules have valid dimensions
    for (const auto& module : modules) {
        if (module->width < 0 || module->height < 0) {
            std::cerr << "Validation error: Module " << module->name
                      << " has negative dimensions\n";
            return false;
        }
    }

    // Check die area
    if (die_area.width() <= 0 || die_area.height() <= 0) {
        std::cerr << "Validation error: Invalid die area\n";
        return false;
    }

    return true;
}

bool CircuitModel::exportToPl(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << " for writing\n";
        return false;
    }

    file << "UCLA pl 1.0\n";
    file << "# Created by EDA_Integrated\n";
    file << "# Total modules: " << modules.size() << "\n";
    file << "# Total nets: " << nets.size() << "\n\n";

    for (const auto& module : modules) {
        file << std::left << std::setw(30) << module->name << " ";
        file << std::fixed << std::setprecision(6);
        file << module->position.x << " " << module->position.y << " : ";

        if (module->is_fixed) {
            file << "N";
            if (module->type == ModuleType::TERMINAL) {
                file << " /FIXED";
            }
        } else {
            file << "N";
        }
        file << "\n";
    }

    file.close();
    return true;
}

} // namespace eda
