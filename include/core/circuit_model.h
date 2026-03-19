#ifndef EDA_CORE_CIRCUIT_MODEL_H
#define EDA_CORE_CIRCUIT_MODEL_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <Eigen/Sparse>

namespace eda {

// Forward declarations
class Module;
class Pin;
class Net;
class CircuitModel;

using ModulePtr = std::shared_ptr<Module>;
using PinPtr = std::shared_ptr<Pin>;
using NetPtr = std::shared_ptr<Net>;

/**
 * @brief Pin direction enum
 */
enum class PinDirection {
    INPUT,
    OUTPUT,
    INOUT,
    UNKNOWN
};

/**
 * @brief Module type enum
 */
enum class ModuleType {
    STANDARD_CELL,  // Movable standard cell
    MACRO,          // Fixed or movable macro block
    TERMINAL,       // Fixed I/O pin
    FILLER,         // Filler cell for density
    UNKNOWN
};

/**
 * @brief 2D point/position structure
 */
struct Point2D {
    double x = 0.0;
    double y = 0.0;

    Point2D() = default;
    Point2D(double x_, double y_) : x(x_), y(y_) {}

    bool operator==(const Point2D& other) const {
        return x == other.x && y == other.y;
    }

    Point2D operator+(const Point2D& other) const {
        return Point2D(x + other.x, y + other.y);
    }

    Point2D operator-(const Point2D& other) const {
        return Point2D(x - other.x, y - other.y);
    }
};

/**
 * @brief Rectangle/Bounding box structure
 */
struct Rectangle {
    double x_min = 0.0;
    double y_min = 0.0;
    double x_max = 0.0;
    double y_max = 0.0;

    Rectangle() = default;
    Rectangle(double xmin, double ymin, double xmax, double ymax)
        : x_min(xmin), y_min(ymin), x_max(xmax), y_max(ymax) {}

    double width() const { return x_max - x_min; }
    double height() const { return y_max - y_min; }
    double area() const { return width() * height(); }

    Point2D center() const {
        return Point2D((x_min + x_max) / 2.0, (y_min + y_max) / 2.0);
    }

    bool contains(const Point2D& p) const {
        return p.x >= x_min && p.x <= x_max && p.y >= y_min && p.y <= y_max;
    }
};

/**
 * @brief Pin class - represents a connection point on a module
 */
class Pin {
public:
    std::string name;
    PinDirection direction = PinDirection::UNKNOWN;
    Point2D offset;           // Offset from module origin
    Point2D position;         // Absolute position (after placement)
    ModulePtr parent_module;  // Parent module
    NetPtr net;              // Connected net

    Pin(const std::string& name_) : name(name_) {}

    void updateAbsolutePosition();
};

/**
 * @brief Net class - represents a signal network connecting multiple pins
 */
class Net {
public:
    std::string name;
    std::vector<PinPtr> pins;
    double weight = 1.0;

    // Cached bounding box (updated during placement)
    Rectangle bbox;

    Net(const std::string& name_) : name(name_) {}

    void addPin(PinPtr pin);
    void removePin(PinPtr pin);

    // Calculate Half-Perimeter Wirelength (HPWL)
    double calcHPWL() const;

    // Update bounding box based on pin positions
    void updateBoundingBox();

    // Get driver pin (output pin)
    PinPtr getDriver() const;

    // Get load pins (input pins)
    std::vector<PinPtr> getLoads() const;
};

/**
 * @brief Module class - represents a circuit component/cell
 */
class Module {
public:
    std::string name;
    ModuleType type = ModuleType::UNKNOWN;

    // Geometry
    double width = 0.0;
    double height = 0.0;
    Point2D position;  // Lower-left corner position

    // Placement status
    bool is_fixed = false;
    bool is_placed = false;

    // Pins
    std::vector<PinPtr> pins;
    std::map<std::string, PinPtr> pin_map;

    // For standard cells
    std::string cell_type;  // Cell type (e.g., "NAND2", "DFF")

    Module(const std::string& name_) : name(name_) {}

    void addPin(PinPtr pin);
    PinPtr getPin(const std::string& pin_name) const;

    // Get module bounding box
    Rectangle getBBox() const;

    // Get center position
    Point2D getCenter() const {
        return Point2D(position.x + width / 2.0, position.y + height / 2.0);
    }

    void setCenter(const Point2D& center) {
        position.x = center.x - width / 2.0;
        position.y = center.y - height / 2.0;
    }

    // Check if module overlaps with another
    bool overlaps(const Module& other) const;
};

/**
 * @brief Die/Chip area definition
 */
struct DieArea {
    double x_min = 0.0;
    double y_min = 0.0;
    double x_max = 0.0;
    double y_max = 0.0;

    double width() const { return x_max - x_min; }
    double height() const { return y_max - y_min; }
    double area() const { return width() * height(); }

    bool contains(const Point2D& p) const {
        return p.x >= x_min && p.x <= x_max && p.y >= y_min && p.y <= y_max;
    }

    Rectangle toRectangle() const {
        return Rectangle(x_min, y_min, x_max, y_max);
    }
};

/**
 * @brief Row structure for standard cell placement
 */
struct Row {
    double y_coord = 0.0;
    double height = 0.0;
    double x_min = 0.0;
    double x_max = 0.0;
    bool is_horizontal = true;  // true for horizontal rows

    double width() const { return x_max - x_min; }
};

/**
 * @brief CircuitModel - Central data structure for the entire circuit
 *
 * This class serves as the unified internal representation for all
 * input formats (Verilog, BookShelf) and all processing stages.
 */
class CircuitModel {
public:
    // Circuit name
    std::string name;

    // Design elements
    std::vector<ModulePtr> modules;
    std::vector<NetPtr> nets;
    std::vector<Row> rows;

    // Fast lookup maps
    std::map<std::string, ModulePtr> module_map;
    std::map<std::string, NetPtr> net_map;

    // Die area
    DieArea die_area;

    // Technology parameters
    double site_width = 0.0;
    double row_height = 0.0;

public:
    CircuitModel() = default;
    explicit CircuitModel(const std::string& name_) : name(name_) {}

    // Module management
    void addModule(ModulePtr module);
    ModulePtr getModule(const std::string& name) const;
    void removeModule(const std::string& name);

    // Net management
    void addNet(NetPtr net);
    NetPtr getNet(const std::string& name) const;
    void removeNet(const std::string& name);

    // Row management
    void addRow(const Row& row);

    // Statistics
    size_t getNumModules() const { return modules.size(); }
    size_t getNumNets() const { return nets.size(); }
    size_t getNumPins() const;

    size_t getNumMovableModules() const;
    size_t getNumFixedModules() const;

    // Calculate total HPWL
    double calcTotalHPWL() const;

    // Calculate design utilization
    double calcUtilization() const;

    // Calculate density map (for visualization)
    std::vector<std::vector<double>> calcDensityMap(
        int num_bins_x, int num_bins_y) const;

    // Clear all data
    void clear();

    // Validation
    bool validate() const;

    // Export to BookShelf .pl format
    bool exportToPl(const std::string& filename) const;
};

} // namespace eda

#endif // EDA_CORE_CIRCUIT_MODEL_H
