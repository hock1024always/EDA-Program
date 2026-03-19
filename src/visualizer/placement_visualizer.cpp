#include "placement_visualizer.h"
#include <fstream>
#include <iostream>
#include <cmath>
#include <algorithm>

namespace eda {

// BMPWriter implementation
BMPWriter::BMPWriter(int width, int height)
    : width_(width), height_(height) {
    // BMP stores pixels bottom-to-top, 3 bytes per pixel (BGR), padded to 4 bytes
    int row_size = ((width * 3 + 3) / 4) * 4;
    data_.resize(row_size * height, 0);
}

void BMPWriter::setPixel(int x, int y, const Color& color) {
    if (x < 0 || x >= width_ || y < 0 || y >= height_) return;

    // BMP is stored bottom-to-top
    int row = height_ - 1 - y;
    int row_size = ((width_ * 3 + 3) / 4) * 4;
    int idx = row * row_size + x * 3;

    if (idx + 2 < static_cast<int>(data_.size())) {
        data_[idx] = color.b;     // Blue
        data_[idx + 1] = color.g; // Green
        data_[idx + 2] = color.r; // Red
    }
}

void BMPWriter::fillRect(int x, int y, int w, int h, const Color& color) {
    for (int dy = 0; dy < h; ++dy) {
        for (int dx = 0; dx < w; ++dx) {
            setPixel(x + dx, y + dy, color);
        }
    }
}

void BMPWriter::drawRect(int x, int y, int w, int h, const Color& color) {
    for (int dx = 0; dx < w; ++dx) {
        setPixel(x + dx, y, color);
        setPixel(x + dx, y + h - 1, color);
    }
    for (int dy = 0; dy < h; ++dy) {
        setPixel(x, y + dy, color);
        setPixel(x + w - 1, y + dy, color);
    }
}

void BMPWriter::drawLine(int x1, int y1, int x2, int y2, const Color& color) {
    // Bresenham's line algorithm
    int dx = std::abs(x2 - x1);
    int dy = std::abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        setPixel(x1, y1, color);

        if (x1 == x2 && y1 == y2) break;

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

void BMPWriter::writeHeader(std::ofstream& file) {
    int row_size = ((width_ * 3 + 3) / 4) * 4;
    int image_size = row_size * height_;
    int file_size = 54 + image_size; // 54 bytes header

    // BMP Header (14 bytes)
    file.put('B'); file.put('M'); // Signature
    writeInt(file, file_size);     // File size
    writeInt(file, 0);             // Reserved
    writeInt(file, 54);            // Offset to pixel data

    // DIB Header (BITMAPINFOHEADER, 40 bytes)
    writeInt(file, 40);            // Header size
    writeInt(file, width_);        // Width
    writeInt(file, height_);       // Height
    writeShort(file, 1);           // Color planes
    writeShort(file, 24);          // Bits per pixel
    writeInt(file, 0);             // Compression (none)
    writeInt(file, image_size);    // Image size
    writeInt(file, 2835);          // X pixels per meter (72 DPI)
    writeInt(file, 2835);          // Y pixels per meter (72 DPI)
    writeInt(file, 0);             // Colors in color table
    writeInt(file, 0);             // Important colors
}

bool BMPWriter::save(const std::string& filename) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Cannot create file: " << filename << "\n";
        return false;
    }

    writeHeader(file);
    file.write(reinterpret_cast<const char*>(data_.data()), data_.size());
    file.close();

    return true;
}

// PlacementVisualizer implementation
PlacementVisualizer::PlacementVisualizer(const CircuitModel& circuit)
    : circuit_(circuit) {}

void PlacementVisualizer::worldToImage(double wx, double wy, int& ix, int& iy,
                                        const VisualizerOptions& options) {
    double scale_x = (options.image_width - 2 * options.margin) / circuit_.die_area.width();
    double scale_y = (options.image_height - 2 * options.margin) / circuit_.die_area.height();
    double scale = std::min(scale_x, scale_y);

    ix = options.margin + static_cast<int>((wx - circuit_.die_area.x_min) * scale);
    iy = options.margin + static_cast<int>((wy - circuit_.die_area.y_min) * scale);
}

Color PlacementVisualizer::densityToColor(double density) {
    // Map density to color gradient (blue -> green -> yellow -> red)
    if (density <= 0.5) {
        // Blue to green
        uint8_t g = static_cast<uint8_t>(density * 2 * 255);
        return Color(0, g, 255);
    } else if (density <= 1.0) {
        // Green to yellow
        uint8_t r = static_cast<uint8_t>((density - 0.5) * 2 * 255);
        return Color(r, 255, 255 - r);
    } else {
        // Yellow to red
        uint8_t g = static_cast<uint8_t>(255 - (density - 1.0) * 255);
        g = std::min(g, static_cast<uint8_t>(255));
        return Color(255, g, 0);
    }
}

void PlacementVisualizer::drawModules(BMPWriter& writer,
                                       const VisualizerOptions& options) {
    for (const auto& module : circuit_.modules) {
        int x1, y1, x2, y2;
        worldToImage(module->position.x, module->position.y, x1, y1, options);
        worldToImage(module->position.x + module->width,
                     module->position.y + module->height, x2, y2, options);

        int w = std::max(1, x2 - x1);
        int h = std::max(1, y2 - y1);

        Color color;
        if (module->is_fixed) {
            color = options.fixed_color;
        } else if (module->type == ModuleType::MACRO) {
            color = options.macro_color;
        } else if (module->type == ModuleType::FILLER) {
            color = options.filler_color;
        } else {
            color = options.movable_color;
        }

        writer.fillRect(x1, y1, w, h, color);
        writer.drawRect(x1, y1, w, h, Color::Black());
    }
}

void PlacementVisualizer::drawNets(BMPWriter& writer,
                                    const VisualizerOptions& options) {
    for (const auto& net : circuit_.nets) {
        if (net->pins.size() < 2) continue;

        // Draw connections between pins
        for (size_t i = 0; i < net->pins.size() - 1; ++i) {
            int x1, y1, x2, y2;
            worldToImage(net->pins[i]->position.x, net->pins[i]->position.y,
                        x1, y1, options);
            worldToImage(net->pins[i + 1]->position.x, net->pins[i + 1]->position.y,
                        x2, y2, options);

            writer.drawLine(x1, y1, x2, y2, options.net_color);
        }
    }
}

void PlacementVisualizer::drawDensity(BMPWriter& writer,
                                       const VisualizerOptions& options) {
    auto density_map = circuit_.calcDensityMap(options.density_bins_x,
                                                options.density_bins_y);

    double bin_width = static_cast<double>(options.image_width - 2 * options.margin)
                       / options.density_bins_x;
    double bin_height = static_cast<double>(options.image_height - 2 * options.margin)
                        / options.density_bins_y;

    for (int iy = 0; iy < options.density_bins_y; ++iy) {
        for (int ix = 0; ix < options.density_bins_x; ++ix) {
            double density = density_map[ix][iy];
            Color color = densityToColor(density);

            int px = options.margin + static_cast<int>(ix * bin_width);
            int py = options.margin + static_cast<int>(iy * bin_height);
            int pw = static_cast<int>(bin_width);
            int ph = static_cast<int>(bin_height);

            writer.fillRect(px, py, pw, ph, color);
        }
    }
}

bool PlacementVisualizer::exportToBMP(const std::string& filename,
                                       const VisualizerOptions& options) {
    std::cout << "Exporting placement to " << filename << "...\n";

    BMPWriter writer(options.image_width, options.image_height);

    // Fill background
    writer.fillRect(0, 0, options.image_width, options.image_height,
                    options.background);

    // Draw density heatmap if requested
    if (options.show_density) {
        drawDensity(writer, options);
    }

    // Draw nets if requested
    if (options.show_nets) {
        drawNets(writer, options);
    }

    // Draw modules
    if (options.show_modules) {
        drawModules(writer, options);
    }

    // Draw die area border
    int x1, y1, x2, y2;
    worldToImage(circuit_.die_area.x_min, circuit_.die_area.y_min, x1, y1, options);
    worldToImage(circuit_.die_area.x_max, circuit_.die_area.y_max, x2, y2, options);
    writer.drawRect(x1, y1, x2 - x1, y2 - y1, Color::Black());

    if (!writer.save(filename)) {
        std::cerr << "Failed to save " << filename << "\n";
        return false;
    }

    std::cout << "Exported successfully!\n";
    return true;
}

bool PlacementVisualizer::exportDensityMap(const std::string& filename,
                                            int bins_x, int bins_y) {
    VisualizerOptions options;
    options.show_density = true;
    options.show_modules = false;
    options.show_nets = false;
    options.density_bins_x = bins_x;
    options.density_bins_y = bins_y;

    return exportToBMP(filename, options);
}

bool PlacementVisualizer::exportComparison(const std::string& filename,
                                            const CircuitModel& circuit_before,
                                            const VisualizerOptions& options) {
    // Create side-by-side comparison
    int half_width = options.image_width / 2;

    BMPWriter writer(options.image_width, options.image_height);

    // Fill background
    writer.fillRect(0, 0, options.image_width, options.image_height,
                    options.background);

    // TODO: Implement side-by-side comparison
    // This would require rendering both circuits side by side

    if (!writer.save(filename)) {
        std::cerr << "Failed to save " << filename << "\n";
        return false;
    }

    return true;
}

} // namespace eda
