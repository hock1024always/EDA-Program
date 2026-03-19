#ifndef EDA_VISUALIZER_PLACEMENT_VISUALIZER_H
#define EDA_VISUALIZER_PLACEMENT_VISUALIZER_H

#include "circuit_model.h"
#include <string>
#include <vector>
#include <cstdint>

namespace eda {

/**
 * @brief Color structure for BMP output
 */
struct Color {
    uint8_t r, g, b;

    Color(uint8_t r_, uint8_t g_, uint8_t b_) : r(r_), g(g_), b(b_) {}

    static Color Red() { return Color(255, 0, 0); }
    static Color Green() { return Color(0, 255, 0); }
    static Color Blue() { return Color(0, 0, 255); }
    static Color Yellow() { return Color(255, 255, 0); }
    static Color Cyan() { return Color(0, 255, 255); }
    static Color Magenta() { return Color(255, 0, 255); }
    static Color White() { return Color(255, 255, 255); }
    static Color Black() { return Color(0, 0, 0); }
    static Color Gray(uint8_t v) { return Color(v, v, v); }
};

/**
 * @brief BMP image writer for placement visualization
 */
class BMPWriter {
public:
    BMPWriter(int width, int height);
    ~BMPWriter() = default;

    void setPixel(int x, int y, const Color& color);
    void fillRect(int x, int y, int w, int h, const Color& color);
    void drawRect(int x, int y, int w, int h, const Color& color);
    void drawLine(int x1, int y1, int x2, int y2, const Color& color);

    bool save(const std::string& filename);

private:
    int width_, height_;
    std::vector<uint8_t> data_;

    void writeHeader(std::ofstream& file);
};

/**
 * @brief Placement visualization options
 */
struct VisualizerOptions {
    int image_width = 1200;
    int image_height = 1200;
    int margin = 50;

    bool show_modules = true;
    bool show_nets = false;
    bool show_density = false;
    int density_bins_x = 64;
    int density_bins_y = 64;

    // Colors
    Color movable_color = Color::Red();
    Color fixed_color = Color::Blue();
    Color macro_color = Color::Green();
    Color filler_color = Color::Gray(200);
    Color net_color = Color::Gray(150);
    Color background = Color::White();
};

/**
 * @brief Placement visualizer
 */
class PlacementVisualizer {
public:
    explicit PlacementVisualizer(const CircuitModel& circuit);

    // Export to BMP
    bool exportToBMP(const std::string& filename,
                     const VisualizerOptions& options = VisualizerOptions());

    // Export density heatmap
    bool exportDensityMap(const std::string& filename,
                          int bins_x = 64, int bins_y = 64);

    // Export comparison (before/after)
    bool exportComparison(const std::string& filename,
                          const CircuitModel& circuit_before,
                          const VisualizerOptions& options = VisualizerOptions());

private:
    const CircuitModel& circuit_;

    // Coordinate transformation
    void worldToImage(double wx, double wy, int& ix, int& iy,
                      const VisualizerOptions& options);

    // Drawing methods
    void drawModules(BMPWriter& writer, const VisualizerOptions& options);
    void drawNets(BMPWriter& writer, const VisualizerOptions& options);
    void drawDensity(BMPWriter& writer, const VisualizerOptions& options);

    // Color mapping
    Color densityToColor(double density);
};

} // namespace eda

#endif // EDA_VISUALIZER_PLACEMENT_VISUALIZER_H
