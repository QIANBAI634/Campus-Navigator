/**
 * 地图显示组件实现
 */
#include "mapwidget.h"
#include <QScrollBar>
#include <QResizeEvent>
#include <cmath>

MapWidget::MapWidget(QWidget *parent)
    : QGraphicsView(parent),
      m_scene(nullptr),
      m_mapItem(nullptr),
      m_graph(nullptr),
      m_startMarker(nullptr),
      m_endMarker(nullptr),
      m_pathActive(false)
{
    // 创建场景
    m_scene = new QGraphicsScene(this);
    m_scene->setSceneRect(0, 0, MAP_WIDTH, MAP_HEIGHT);
    setScene(m_scene);

    // 视图设置：抗锯齿、无滚动条（地图自动适配窗口）
    setRenderHint(QPainter::Antialiasing, true);
    setRenderHint(QPainter::SmoothPixmapTransform, true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setDragMode(QGraphicsView::NoDrag);

    // 背景色（未加载地图时显示）
    setBackgroundBrush(QBrush(QColor(240, 244, 249)));

    // 视图最小高度
    setMinimumHeight(300);
}

bool MapWidget::loadMap(const QString &imagePath)
{
    if (!m_basePixmap.load(imagePath)) {
        return false;
    }

    // 创建地图图片项
    m_mapItem = m_scene->addPixmap(m_basePixmap);
    m_mapItem->setZValue(0);  // 最底层

    // 缩放以适应视图
    fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);

    return true;
}

void MapWidget::setGraph(const CampusGraph *graph)
{
    m_graph = graph;
}

void MapWidget::placeLandmarkDots()
{
    if (!m_graph) return;

    // 先清除旧的标记
    for (auto *dot : m_dotItems) {
        m_scene->removeItem(dot);
        delete dot;
    }
    m_dotItems.clear();

    // 为每个地标放置红点
    const auto &nodes = m_graph->nodes();
    for (int i = 0; i < nodes.size(); ++i) {
        if (nodes[i].type != "landmark") continue;

        double px = nodes[i].pixelX;
        double py = nodes[i].pixelY;

        // 红点
        auto *dot = m_scene->addEllipse(
            px - DOT_RADIUS, py - DOT_RADIUS,
            DOT_RADIUS * 2, DOT_RADIUS * 2,
            QPen(Qt::darkRed, 1.5),
            QBrush(QColor(220, 40, 40))
        );
        dot->setZValue(2);  // 在地图上方
        // 存储节点索引用于 tooltip
        dot->setToolTip(nodes[i].name);
        m_dotItems.append(dot);
    }
}

void MapWidget::drawNavigationPath(const QVector<int> &fullPath)
{
    if (!m_graph || fullPath.size() < 2) return;

    // 先清除旧路径
    clearNavigationPath();

    const auto &nodes = m_graph->nodes();

    // 从路径中依次取相邻节点画线段
    for (int i = 0; i < fullPath.size() - 1; ++i) {
        int idxA = fullPath[i];
        int idxB = fullPath[i + 1];

        double x1 = nodes[idxA].pixelX;
        double y1 = nodes[idxA].pixelY;
        double x2 = nodes[idxB].pixelX;
        double y2 = nodes[idxB].pixelY;

        auto *line = m_scene->addLine(
            x1, y1, x2, y2,
            QPen(QColor(30, 100, 220), 3.0)  // 蓝色，线宽3像素
        );
        line->setZValue(1);  // 在地图上面，红点下面
        m_pathLineItems.append(line);
    }

    // 高亮起点（绿色大点）
    int startIdx = fullPath.first();
    m_startMarker = m_scene->addEllipse(
        nodes[startIdx].pixelX - DOT_RADIUS * 1.5,
        nodes[startIdx].pixelY - DOT_RADIUS * 1.5,
        DOT_RADIUS * 3, DOT_RADIUS * 3,
        QPen(QColor(0, 120, 0), 2.5),
        QBrush(QColor(50, 200, 50))
    );
    m_startMarker->setZValue(3);

    // 高亮终点（红色大点）
    int endIdx = fullPath.last();
    m_endMarker = m_scene->addEllipse(
        nodes[endIdx].pixelX - DOT_RADIUS * 1.5,
        nodes[endIdx].pixelY - DOT_RADIUS * 1.5,
        DOT_RADIUS * 3, DOT_RADIUS * 3,
        QPen(QColor(180, 0, 0), 2.5),
        QBrush(QColor(255, 60, 60))
    );
    m_endMarker->setZValue(3);

    m_pathActive = true;
}

void MapWidget::clearNavigationPath()
{
    // 清除所有蓝线
    for (auto *line : m_pathLineItems) {
        m_scene->removeItem(line);
        delete line;
    }
    m_pathLineItems.clear();

    // 清除起点/终点标记
    if (m_startMarker) {
        m_scene->removeItem(m_startMarker);
        delete m_startMarker;
        m_startMarker = nullptr;
    }
    if (m_endMarker) {
        m_scene->removeItem(m_endMarker);
        delete m_endMarker;
        m_endMarker = nullptr;
    }

    m_pathActive = false;
}

void MapWidget::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    // 窗口大小变化时重新缩放以适应
    fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);
}
