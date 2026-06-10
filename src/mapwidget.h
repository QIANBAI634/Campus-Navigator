/**
 * 地图显示组件
 *
 * 基于 QGraphicsView，以校园地图图片为背景：
 * - 红色圆点标注所有地标节点
 * - 蓝色线条绘制导航路径（包含经过的路口节点）
 * - 点击"完成导航"可清除蓝色路径线
 */
#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QVector>
#include <QLabel>
#include "graph.h"

class MapWidget : public QGraphicsView
{
    Q_OBJECT

public:
    explicit MapWidget(QWidget *parent = nullptr);

    // 加载地图背景图片
    bool loadMap(const QString &imagePath);

    // 设置校园图引用，用于获取节点坐标
    void setGraph(const CampusGraph *graph);

    // 在地图上放置所有地标红点
    void placeLandmarkDots();

    // 绘制导航路径（蓝色线条连接完整路径的所有节点）
    void drawNavigationPath(const QVector<int> &fullPath);

    // 清除导航路径线，恢复默认地图
    void clearNavigationPath();

    // 是否有活跃的导航路径
    bool hasActivePath() const { return m_pathActive; }

protected:
    // 窗口大小改变时自动缩放地图以适配
    void resizeEvent(QResizeEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    QGraphicsScene            *m_scene;
    QGraphicsPixmapItem       *m_mapItem;
    const CampusGraph         *m_graph;

    // 红点项（地标标注）
    QVector<QGraphicsEllipseItem*> m_dotItems;

    // 蓝色路径线项
    QVector<QGraphicsLineItem*>    m_pathLineItems;

    // 起点/终点特殊标注
    QGraphicsEllipseItem      *m_startMarker;
    QGraphicsEllipseItem      *m_endMarker;

    bool  m_pathActive;
    QPixmap m_basePixmap;  // 原始地图图片

    // 自定义悬浮提示标签（白底黑字）
    QLabel* m_tooltipLabel;

    // 映射 QGraphicsEllipseItem* → 节点索引
    QMap<QGraphicsEllipseItem*, int> m_dotIndexMap;

    // 坐标系转换常量（地图图片尺寸 1600×1143）
    static constexpr double MAP_WIDTH  = 1600.0;
    static constexpr double MAP_HEIGHT = 1143.0;

    // 红点半径（在原始图片坐标系下的像素大小）
    static constexpr double DOT_RADIUS = 6.0;
};

#endif // MAPWIDGET_H
