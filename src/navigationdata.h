/**
 * 导航数据定义
 * 包含北邮沙河校区所有地标和路口节点，以及道路边数据
 * 数据来源于原 HTML 版本的 nodesData 和 edgesRaw
 */
#ifndef NAVIGATIONDATA_H
#define NAVIGATIONDATA_H

#include <QString>
#include <QVector>
#include <QPair>

// 节点信息结构体
struct NodeInfo {
    QString name;         // 节点名称
    double lng;           // 经度
    double lat;           // 纬度
    QString type;         // 类型: "landmark" / "intersection" / "indoor"
    double pixelX;        // 在地图图片上的像素 X 坐标（1600×1143）
    double pixelY;        // 在地图图片上的像素 Y 坐标（1600×1143）
    QString facilityType; // 服务设施类型（仅 landmark 有效）:
                          // 出入口, 餐饮, 购物, 住宿, 教学, 行政,
                          // 运动, 医疗, 学习, 休闲, 卫生, 金融, 邮政, 交通,
                          // 特殊: 电梯, 房间, 大门, 楼梯
    int    floor;         // 楼层（仅 indoor 有效，0=地面层）
    int    buildingId;    // 所属建筑ID（仅 indoor 有效，0=无）
};

// 边信息（原始数据，存储名称对）
struct EdgeInfo {
    QString from;
    QString to;
};

// 所有节点数据（地标 + 路口）
extern const QVector<NodeInfo> nodesData;

// 所有边数据（无向边，用节点名称表示）
extern const QVector<EdgeInfo> edgesRaw;

#endif // NAVIGATIONDATA_H
