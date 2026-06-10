/**
 * 景区/校园数据库
 *
 * 存储 200 个旅游目的地元数据：
 * - 39 所 985 高校
 * - 北京 211 高校（去除与 985 重合的）
 * - 北京知名景点、公园、医院、商场补齐至 200 个
 *
 * 所有目的地内部路网使用同一模板（北邮沙河校区图结构）
 * 满足课程设计"景区和校园内部可以一致"的要求
 */
#ifndef CAMPUSDATABASE_H
#define CAMPUSDATABASE_H

#include <QString>
#include <QVector>

// 目的地类别
namespace CampusType {
    const QString UNIVERSITY_985  = "985高校";
    const QString UNIVERSITY_211  = "211高校";
    const QString SCENIC_SPOT     = "景点";
    const QString PARK            = "公园";
    const QString HOSPITAL        = "医院";
    const QString MALL            = "商场";
    const QString CULTURE         = "文化场所";
    const QString TRANSPORT       = "交通枢纽";
}

// 单个景区/校园的信息
struct CampusInfo {
    QString name;       // 名称
    QString type;       // 类别
    QString city;       // 所在城市
    QString district;   // 所在区
    double  heat;       // 热度 (1.0 ~ 5.0)
    double  rating;     // 评价 (1.0 ~ 5.0)
    double  lat;        // 纬度
    double  lng;        // 经度
    QString tags;       // 标签（逗号分隔，用于兴趣推荐）
};

// 获取 200 个景区/校园数据（函数初始化，避免大 initializer list 编译器崩溃）
const QVector<CampusInfo>& getAllCampuses();

// 按名称查找
inline const CampusInfo* findCampus(const QString& name) {
    for (const auto& c : getAllCampuses()) {
        if (c.name == name) return &c;
    }
    return nullptr;
}

#endif // CAMPUSDATABASE_H
