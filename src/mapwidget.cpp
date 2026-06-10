/**
 * 地图显示组件实现
 */
#include "mapwidget.h"
#include <QScrollBar>
#include <QResizeEvent>
#include <QMouseEvent>
#include <cmath>

MapWidget::MapWidget(QWidget *parent)
    : QGraphicsView(parent),
      m_scene(nullptr),
      m_mapItem(nullptr),
      m_graph(nullptr),
      m_startMarker(nullptr),
      m_endMarker(nullptr),
      m_pathActive(false),
      m_tooltipLabel(nullptr)
{
    m_scene = new QGraphicsScene(this);
    m_scene->setSceneRect(0, 0, MAP_WIDTH, MAP_HEIGHT);
    setScene(m_scene);

    setRenderHint(QPainter::Antialiasing, true);
    setRenderHint(QPainter::SmoothPixmapTransform, true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setDragMode(QGraphicsView::NoDrag);
    setBackgroundBrush(QBrush(QColor(240, 244, 249)));
    setMinimumHeight(300);
    setMouseTracking(true);

    // 自定义悬浮提示 — 白底黑字（不用系统 tooltip，不受系统主题影响）
    m_tooltipLabel = new QLabel(this);
    m_tooltipLabel->setStyleSheet(
        "QLabel { background: #ffffff; color: #1f2f38;"
        "border: 1px solid #cbdde6; border-radius: 6px;"
        "padding: 5px 10px; font-size: 12px; font-weight: 500; }");
    m_tooltipLabel->hide();
    m_tooltipLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_tooltipLabel->raise();
}

bool MapWidget::loadMap(const QString &imagePath)
{
    if (!m_basePixmap.load(imagePath)) return false;
    m_mapItem = m_scene->addPixmap(m_basePixmap);
    m_mapItem->setZValue(0);
    fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);
    return true;
}

void MapWidget::setGraph(const CampusGraph *graph) { m_graph = graph; }

void MapWidget::placeLandmarkDots()
{
    if (!m_graph) return;

    for (auto *dot : m_dotItems) { m_scene->removeItem(dot); delete dot; }
    m_dotItems.clear();
    m_dotIndexMap.clear();

    static const QMap<QString, QColor> facilityColors = {
        {"出入口", QColor(50,180,50)},    // 出入口 绿
        {"餐饮",       QColor(240,150,30)},    // 餐饮 橙
        {"购物",       QColor(255,140,0)},     // 购物 深橙
        {"住宿",       QColor(100,100,220)},   // 住宿 蓝紫
        {"教学",       QColor(30,130,210)},    // 教学 蓝
        {"行政",       QColor(140,100,80)},    // 行政 棕
        {"运动",       QColor(50,200,200)},    // 运动 青
        {"医疗",       QColor(220,50,50)},     // 医疗 红
        {"学习",       QColor(160,100,200)},   // 学习 紫
        {"休闲",       QColor(250,180,220)},   // 休闲 粉
        {"卫生",       QColor(100,180,220)},   // 卫生 浅蓝
        {"金融",       QColor(220,200,30)},    // 金融 金
        {"邮政",       QColor(180,140,80)},    // 邮政 土黄
    };
    static const QColor defaultColor(200, 200, 200);

    const auto &nodes = m_graph->nodes();
    for (int i = 0; i < nodes.size(); ++i) {
        if (nodes[i].type != "landmark") continue;

        double px = nodes[i].pixelX;
        double py = nodes[i].pixelY;
        QColor color = facilityColors.value(nodes[i].facilityType, defaultColor);

        auto *dot = m_scene->addEllipse(
            px - DOT_RADIUS, py - DOT_RADIUS,
            DOT_RADIUS * 2, DOT_RADIUS * 2,
            QPen(color.darker(140), 1.5), QBrush(color));
        dot->setZValue(2);
        dot->setAcceptHoverEvents(true);
        dot->setCursor(Qt::PointingHandCursor);
        m_dotItems.append(dot);
        m_dotIndexMap[dot] = i;
    }
}

void MapWidget::drawNavigationPath(const QVector<int> &fullPath)
{
    if (!m_graph || fullPath.size() < 2) return;
    clearNavigationPath();

    const auto &nodes = m_graph->nodes();
    for (int i = 0; i < fullPath.size() - 1; ++i) {
        double x1 = nodes[fullPath[i]].pixelX;
        double y1 = nodes[fullPath[i]].pixelY;
        double x2 = nodes[fullPath[i+1]].pixelX;
        double y2 = nodes[fullPath[i+1]].pixelY;
        auto *line = m_scene->addLine(x1, y1, x2, y2,
            QPen(QColor(30, 100, 220), 3.0));
        line->setZValue(1);
        m_pathLineItems.append(line);
    }

    int si = fullPath.first(), ei = fullPath.last();
    m_startMarker = m_scene->addEllipse(
        nodes[si].pixelX - DOT_RADIUS*1.5, nodes[si].pixelY - DOT_RADIUS*1.5,
        DOT_RADIUS*3, DOT_RADIUS*3, QPen(QColor(0,120,0), 2.5), QBrush(QColor(50,200,50)));
    m_startMarker->setZValue(3);
    m_endMarker = m_scene->addEllipse(
        nodes[ei].pixelX - DOT_RADIUS*1.5, nodes[ei].pixelY - DOT_RADIUS*1.5,
        DOT_RADIUS*3, DOT_RADIUS*3, QPen(QColor(180,0,0), 2.5), QBrush(QColor(255,60,60)));
    m_endMarker->setZValue(3);
    m_pathActive = true;
}

void MapWidget::clearNavigationPath()
{
    for (auto *line : m_pathLineItems) { m_scene->removeItem(line); delete line; }
    m_pathLineItems.clear();
    if (m_startMarker) { m_scene->removeItem(m_startMarker); delete m_startMarker; m_startMarker = nullptr; }
    if (m_endMarker) { m_scene->removeItem(m_endMarker); delete m_endMarker; m_endMarker = nullptr; }
    m_pathActive = false;
}

void MapWidget::highlightFacilities(const QVector<int>& nodeIndices)
{
    clearFacilityHighlights();

    if (!m_graph) return;
    const auto& nodes = m_graph->nodes();

    for (int idx : nodeIndices) {
        double px = nodes[idx].pixelX;
        double py = nodes[idx].pixelY;

        // 金色粗圆环标注
        auto* ring = m_scene->addEllipse(
            px - DOT_RADIUS * 2.5, py - DOT_RADIUS * 2.5,
            DOT_RADIUS * 5, DOT_RADIUS * 5,
            QPen(QColor(255, 180, 0), 3.0), QBrush(Qt::transparent));
        ring->setZValue(3);
        m_highlightItems.append(ring);

        // 内部发光
        auto* glow = m_scene->addEllipse(
            px - DOT_RADIUS * 1.8, py - DOT_RADIUS * 1.8,
            DOT_RADIUS * 3.6, DOT_RADIUS * 3.6,
            QPen(Qt::NoPen), QBrush(QColor(255, 200, 50, 80)));
        glow->setZValue(3);
        m_highlightItems.append(glow);
    }
}

void MapWidget::clearFacilityHighlights()
{
    for (auto* item : m_highlightItems) {
        m_scene->removeItem(item);
        delete item;
    }
    m_highlightItems.clear();
}

void MapWidget::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);
}

void MapWidget::mouseMoveEvent(QMouseEvent *event)
{
    QGraphicsView::mouseMoveEvent(event);

    // 检测鼠标悬停在地标圆点上 → 显示白底黑字提示
    QGraphicsEllipseItem* hitDot = nullptr;
    QPointF scenePos = mapToScene(event->pos());

    for (auto* dot : m_dotItems) {
        if (dot->contains(dot->mapFromScene(scenePos))) {
            hitDot = dot;
            break;
        }
    }

    if (hitDot && m_dotIndexMap.contains(hitDot)) {
        int idx = m_dotIndexMap[hitDot];
        const auto& node = m_graph->nodes()[idx];
        QString text = node.name + QString::fromUtf8(" · ") + node.facilityType;
        m_tooltipLabel->setText(text);
        m_tooltipLabel->adjustSize();
        // 定位在鼠标右下方
        QPoint labelPos = event->pos() + QPoint(16, 16);
        // 防止超出地图边界
        if (labelPos.x() + m_tooltipLabel->width() > width())
            labelPos.setX(event->pos().x() - m_tooltipLabel->width() - 8);
        if (labelPos.y() + m_tooltipLabel->height() > height())
            labelPos.setY(event->pos().y() - m_tooltipLabel->height() - 8);
        m_tooltipLabel->move(labelPos);
        m_tooltipLabel->show();
        m_tooltipLabel->raise();
    } else {
        m_tooltipLabel->hide();
    }
}
