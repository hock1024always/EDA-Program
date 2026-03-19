#include "placement_view.h"
#include "circuit_model.h"
#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QColor>
#include <cmath>

namespace eda {

PlacementView::PlacementView(QWidget* parent)
    : QGraphicsView(parent) {
    scene_ = new QGraphicsScene(this);
    setScene(scene_);

    setRenderHint(QPainter::Antialiasing, true);
    setDragMode(QGraphicsView::ScrollHandDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);

    setBackgroundBrush(QBrush(Qt::white));
}

void PlacementView::setCircuit(CircuitModel* circuit) {
    circuit_ = circuit;
    drawLayout();
}

void PlacementView::setShowGrid(bool show) {
    show_grid_ = show;
    viewport()->update();
}

void PlacementView::setShowDensity(bool show) {
    show_density_ = show;
    drawLayout();
}

void PlacementView::setShowNets(bool show) {
    show_nets_ = show;
    drawLayout();
}

void PlacementView::zoomIn() {
    scale(zoom_factor_, zoom_factor_);
}

void PlacementView::zoomOut() {
    scale(1.0 / zoom_factor_, 1.0 / zoom_factor_);
}

void PlacementView::fitToView() {
    if (scene_) {
        fitInView(scene_->itemsBoundingRect(), Qt::KeepAspectRatio);
    }
}

void PlacementView::wheelEvent(QWheelEvent* event) {
    if (event->modifiers() & Qt::ControlModifier) {
        // Zoom with Ctrl+Wheel
        if (event->angleDelta().y() > 0) {
            zoomIn();
        } else {
            zoomOut();
        }
        event->accept();
    } else {
        // Scroll normally
        QGraphicsView::wheelEvent(event);
    }
}

void PlacementView::mouseMoveEvent(QMouseEvent* event) {
    QPointF scene_pos = mapToScene(event->pos());
    emit mouseMoved(scene_pos.toPoint());
    QGraphicsView::mouseMoveEvent(event);
}

void PlacementView::drawBackground(QPainter* painter, const QRectF& rect) {
    QGraphicsView::drawBackground(painter, rect);

    if (!show_grid_) return;

    // Draw coordinate grid
    qreal grid_size = 50.0;
    QPen pen(QColor(220, 220, 220));
    pen.setWidthF(0.5);
    painter->setPen(pen);

    // Vertical lines
    qreal left = std::floor(rect.left() / grid_size) * grid_size;
    qreal right = rect.right();
    for (qreal x = left; x <= right; x += grid_size) {
        painter->drawLine(QPointF(x, rect.top()), QPointF(x, rect.bottom()));
    }

    // Horizontal lines
    qreal top = std::floor(rect.top() / grid_size) * grid_size;
    qreal bottom = rect.bottom();
    for (qreal y = top; y <= bottom; y += grid_size) {
        painter->drawLine(QPointF(rect.left(), y), QPointF(rect.right(), y));
    }
}

void PlacementView::drawLayout() {
    clearScene();

    if (!circuit_) return;

    // Setup coordinate transformation
    if (circuit_->die_area.width() > 0 && circuit_->die_area.height() > 0) {
        QRectF view_rect = viewport()->rect();
        double view_aspect = view_rect.width() / view_rect.height();
        double die_aspect = circuit_->die_area.width() / circuit_->die_area.height();

        double scale;
        if (view_aspect > die_aspect) {
            // Fit to height
            scale = view_rect.height() / circuit_->die_area.height();
        } else {
            // Fit to width
            scale = view_rect.width() / circuit_->die_area.width();
        }

        scale_x_ = scale * 0.8; // Leave some margin
        scale_y_ = scale * 0.8;
        offset_x_ = (view_rect.width() - circuit_->die_area.width() * scale_x_) / 2;
        offset_y_ = (view_rect.height() - circuit_->die_area.height() * scale_y_) / 2;
    }

    // Draw components
    if (show_density_) {
        drawDensityMap();
    }

    drawModules();

    if (show_nets_) {
        drawNets();
    }

    // Draw die area boundary
    double x1 = circuit_->die_area.x_min * scale_x_ + offset_x_;
    double y1 = circuit_->die_area.y_min * scale_y_ + offset_y_;
    double x2 = circuit_->die_area.x_max * scale_x_ + offset_x_;
    double y2 = circuit_->die_area.y_max * scale_y_ + offset_y_;

    auto* boundary = scene_->addRect(x1, y1, x2 - x1, y2 - y1);
    boundary->setPen(QPen(QColor(0, 0, 0), 2));

    // Adjust scene rect
    scene_->setSceneRect(scene_->itemsBoundingRect().adjusted(-20, -20, 20, 20));
}

void PlacementView::drawModules() {
    if (!circuit_) return;

    for (const auto& module : circuit_->modules) {
        double x = module->position.x * scale_x_ + offset_x_;
        double y = module->position.y * scale_y_ + offset_y_;
        double w = module->width * scale_x_;
        double h = module->height * scale_y_;

        QColor color;
        if (module->is_fixed) {
            color = QColor(100, 100, 255); // Blue for fixed
        } else if (module->type == ModuleType::MACRO) {
            color = QColor(100, 255, 100); // Green for macros
        } else if (module->type == ModuleType::FILLER) {
            color = QColor(200, 200, 200); // Gray for fillers
        } else {
            color = QColor(255, 100, 100); // Red for movable cells
        }

        auto* rect = scene_->addRect(x, y, w, h);
        rect->setBrush(QBrush(color));
        rect->setPen(QPen(Qt::black, 1));

        // Add module name (only for larger modules)
        if (w > 20 && h > 15) {
            auto* text = scene_->addText(QString::fromStdString(module->name));
            text->setPos(x + 2, y + 2);
            text->setScale(0.8);
        }
    }
}

void PlacementView::drawNets() {
    if (!circuit_) return;

    QPen net_pen(QColor(150, 150, 150), 1);
    net_pen.setStyle(Qt::DashLine);

    for (const auto& net : circuit_->nets) {
        if (net->pins.size() < 2) continue;

        // Draw lines between consecutive pins
        for (size_t i = 0; i < net->pins.size() - 1; ++i) {
            double x1 = net->pins[i]->position.x * scale_x_ + offset_x_;
            double y1 = net->pins[i]->position.y * scale_y_ + offset_y_;
            double x2 = net->pins[i + 1]->position.x * scale_x_ + offset_x_;
            double y2 = net->pins[i + 1]->position.y * scale_y_ + offset_y_;

            auto* line = scene_->addLine(x1, y1, x2, y2, net_pen);
            line->setZValue(-1); // Behind modules
        }
    }
}

void PlacementView::drawDensityMap() {
    if (!circuit_) return;

    // Simple density visualization using rectangles
    int bins_x = 32;
    int bins_y = 32;

    auto density_map = circuit_->calcDensityMap(bins_x, bins_y);

    double bin_width = circuit_->die_area.width() / bins_x * scale_x_;
    double bin_height = circuit_->die_area.height() / bins_y * scale_y_;

    for (int iy = 0; iy < bins_y; ++iy) {
        for (int ix = 0; ix < bins_x; ++ix) {
            double density = density_map[ix][iy];

            // Map density to color (0=blue, 1=green, >1=red)
            QColor color;
            if (density <= 0.5) {
                int blue = 255;
                int green = static_cast<int>(density * 2 * 255);
                color = QColor(0, green, blue, 100);
            } else if (density <= 1.0) {
                int green = 255;
                int red = static_cast<int>((density - 0.5) * 2 * 255);
                color = QColor(red, green, 0, 100);
            } else {
                int red = 255;
                int green = static_cast<int>(255 - (density - 1.0) * 100);
                green = std::max(0, green);
                color = QColor(red, green, 0, 150);
            }

            double x = (circuit_->die_area.x_min + ix * circuit_->die_area.width() / bins_x) * scale_x_ + offset_x_;
            double y = (circuit_->die_area.y_min + iy * circuit_->die_area.height() / bins_y) * scale_y_ + offset_y_;

            auto* rect = scene_->addRect(x, y, bin_width, bin_height);
            rect->setBrush(QBrush(color));
            rect->setPen(QPen(Qt::transparent));
            rect->setZValue(-2); // Behind everything
        }
    }
}

void PlacementView::clearScene() {
    scene_->clear();
}

} // namespace eda
