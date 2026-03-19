#include "circuit_canvas.h"
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
#include <cmath>

namespace eda {

CircuitCanvas::CircuitCanvas(QWidget* parent)
    : QGraphicsView(parent) {
    scene_ = new QGraphicsScene(this);
    setScene(scene_);

    setRenderHint(QPainter::Antialiasing, true);
    setDragMode(QGraphicsView::ScrollHandDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);

    setBackgroundBrush(QBrush(Qt::white));
}

void CircuitCanvas::setCircuit(CircuitModel* circuit) {
    circuit_ = circuit;
    drawCircuit();
}

void CircuitCanvas::setShowGrid(bool show) {
    show_grid_ = show;
    viewport()->update();
}

void CircuitCanvas::zoomIn() {
    scale(zoom_factor_, zoom_factor_);
}

void CircuitCanvas::zoomOut() {
    scale(1.0 / zoom_factor_, 1.0 / zoom_factor_);
}

void CircuitCanvas::fitToView() {
    if (scene_) {
        fitInView(scene_->itemsBoundingRect(), Qt::KeepAspectRatio);
    }
}

void CircuitCanvas::wheelEvent(QWheelEvent* event) {
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

void CircuitCanvas::mouseMoveEvent(QMouseEvent* event) {
    QPointF scene_pos = mapToScene(event->pos());
    emit mouseMoved(scene_pos.toPoint());
    QGraphicsView::mouseMoveEvent(event);
}

void CircuitCanvas::drawBackground(QPainter* painter, const QRectF& rect) {
    QGraphicsView::drawBackground(painter, rect);

    if (!show_grid_) return;

    // Draw grid
    qreal grid_size = 20.0;
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

void CircuitCanvas::drawCircuit() {
    clearScene();

    if (!circuit_) return;

    // Simple tree layout for modules
    int x_spacing = 100;
    int y_spacing = 80;
    int x_offset = 50;
    int y_offset = 50;

    // Group modules by type for better layout
    std::vector<ModulePtr> inputs, outputs, gates;

    for (const auto& module : circuit_->modules) {
        if (module->type == ModuleType::TERMINAL) {
            if (module->name.find("in") != std::string::npos ||
                module->name.find("IN") != std::string::npos) {
                inputs.push_back(module);
            } else {
                outputs.push_back(module);
            }
        } else {
            gates.push_back(module);
        }
    }

    // Draw inputs on left
    int y_pos = y_offset;
    for (const auto& module : inputs) {
        auto* rect = new QGraphicsRectItem(0, y_pos, 60, 30);
        rect->setBrush(QBrush(QColor(100, 150, 255)));
        rect->setPen(QPen(Qt::black));
        scene_->addItem(rect);

        auto* text = new QGraphicsTextItem(QString::fromStdString(module->name));
        text->setPos(5, y_pos + 8);
        scene_->addItem(text);

        y_pos += y_spacing;
    }

    // Draw gates in middle columns
    x_pos = x_offset + 100;
    y_pos = y_offset;
    int col = 0;

    for (const auto& module : gates) {
        if (col > 0 && col % 5 == 0) {
            x_pos += x_spacing;
            y_pos = y_offset;
        }

        auto* rect = new QGraphicsRectItem(x_pos, y_pos, 80, 40);
        rect->setBrush(QBrush(QColor(255, 200, 100)));
        rect->setPen(QPen(Qt::black));
        scene_->addItem(rect);

        auto* text = new QGraphicsTextItem(QString::fromStdString(module->name));
        text->setPos(x_pos + 5, y_pos + 12);
        scene_->addItem(text);

        y_pos += y_spacing;
        col++;
    }

    // Draw outputs on right
    x_pos = x_offset + 300;
    y_pos = y_offset;
    for (const auto& module : outputs) {
        auto* rect = new QGraphicsRectItem(x_pos, y_pos, 60, 30);
        rect->setBrush(QBrush(QColor(100, 255, 150)));
        rect->setPen(QPen(Qt::black));
        scene_->addItem(rect);

        auto* text = new QGraphicsTextItem(QString::fromStdString(module->name));
        text->setPos(x_pos + 5, y_pos + 8);
        scene_->addItem(text);

        y_pos += y_spacing;
    }

    // Draw nets as lines (simplified)
    for (const auto& net : circuit_->nets) {
        if (net->pins.size() < 2) continue;

        for (size_t i = 0; i < net->pins.size() - 1; ++i) {
            // Just draw straight lines between first and last pin for now
            auto* line = new QGraphicsLineItem(
                0, y_offset + i * y_spacing + 15,
                x_pos, y_offset + i * y_spacing + 15);
            line->setPen(QPen(QColor(150, 150, 150), 2));
            scene_->addItem(line);
            break; // Only draw one line per net for simplicity
        }
    }

    // Adjust scene rect
    scene_->setSceneRect(scene_->itemsBoundingRect().adjusted(-20, -20, 20, 20));
}

void CircuitCanvas::clearScene() {
    scene_->clear();
}

} // namespace eda
