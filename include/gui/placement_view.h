#ifndef EDA_GUI_PLACEMENT_VIEW_H
#define EDA_GUI_PLACEMENT_VIEW_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <memory>

namespace eda {

class CircuitModel;

/**
 * @brief 布局结果显示视图
 *
 * 显示模块的实际物理布局，支持缩放和平移
 */
class PlacementView : public QGraphicsView {
    Q_OBJECT

public:
    explicit PlacementView(QWidget* parent = nullptr);
    ~PlacementView() override = default;

    void setCircuit(CircuitModel* circuit);
    void setShowGrid(bool show);
    void setShowDensity(bool show);
    void setShowNets(bool show);
    void zoomIn();
    void zoomOut();
    void fitToView();

signals:
    void mouseMoved(const QPoint& scenePos);

protected:
    void wheelEvent(QWheelEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void drawBackground(QPainter* painter, const QRectF& rect) override;

private:
    void drawLayout();
    void drawModules();
    void drawNets();
    void drawDensityMap();
    void clearScene();

    CircuitModel* circuit_ = nullptr;
    QGraphicsScene* scene_ = nullptr;
    bool show_grid_ = true;
    bool show_density_ = false;
    bool show_nets_ = true;
    qreal zoom_factor_ = 1.2;

    // Coordinate transformation
    double scale_x_ = 1.0;
    double scale_y_ = 1.0;
    double offset_x_ = 0.0;
    double offset_y_ = 0.0;
};

} // namespace eda

#endif // EDA_GUI_PLACEMENT_VIEW_H
