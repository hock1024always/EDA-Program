#ifndef EDA_GUI_CIRCUIT_CANVAS_H
#define EDA_GUI_CIRCUIT_CANVAS_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <memory>

namespace eda {

class CircuitModel;

/**
 * @brief 电路原理图画布
 *
 * 显示电路的逻辑连接关系，类似于EDA01的功能
 */
class CircuitCanvas : public QGraphicsView {
    Q_OBJECT

public:
    explicit CircuitCanvas(QWidget* parent = nullptr);
    ~CircuitCanvas() override = default;

    void setCircuit(CircuitModel* circuit);
    void setShowGrid(bool show);
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
    void drawCircuit();
    void clearScene();

    CircuitModel* circuit_ = nullptr;
    QGraphicsScene* scene_ = nullptr;
    bool show_grid_ = true;
    qreal zoom_factor_ = 1.2;
};

} // namespace eda

#endif // EDA_GUI_CIRCUIT_CANVAS_H
