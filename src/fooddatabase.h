/**
 * 美食数据库
 * 完全独立于日记系统，存储在 data/foods.json
 * 每个餐饮/教学节点按其 facilityType 分配菜系
 */
#ifndef FOODDATABASE_H
#define FOODDATABASE_H

#include <QString>
#include <QStringList>
#include <QVector>
#include <QMap>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QApplication>
#include <algorithm>

// 菜系定义
struct CuisineDef {
    QString     name;
    QStringList defaultFoods;   // 预设美食
    QStringList userFoods;      // 用户自行上传的美食
};

// 美食条目
struct FoodItem {
    QString             name;       // 美食名称
    QString             cuisine;    // 所属菜系
    QString             location;   // 关联节点名（如"学生食堂"）
    QMap<QString, int>  userRatings; // 用户评分：userId → score (1~5)

    // 平均评分
    double avgRating() const {
        if (userRatings.isEmpty()) return 0.0;
        double sum = 0.0;
        for (int s : userRatings.values()) sum += s;
        return sum / userRatings.size();
    }

    int ratingCount() const { return userRatings.size(); }

    // 一人一票
    void setUserRating(const QString& uid, int score) {
        userRatings[uid] = score;
    }

    int getUserRating(const QString& uid) const {
        return userRatings.value(uid, 0);
    }

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["name"]     = name;
        obj["cuisine"]  = cuisine;
        obj["location"] = location;
        QJsonObject rates;
        for (auto it = userRatings.constBegin(); it != userRatings.constEnd(); ++it)
            rates[it.key()] = it.value();
        obj["userRatings"] = rates;
        return obj;
    }

    static FoodItem fromJson(const QJsonObject& obj) {
        FoodItem f;
        f.name     = obj["name"].toString();
        f.cuisine  = obj["cuisine"].toString();
        f.location = obj["location"].toString();
        QJsonObject rates = obj["userRatings"].toObject();
        for (auto it = rates.constBegin(); it != rates.constEnd(); ++it)
            f.userRatings[it.key()] = it.value().toInt();
        return f;
    }
};

// ========== 菜系数据 ==========

// 获取节点对应菜系列表
inline QStringList cuisinesForFacility(const QString& facilityType) {
    if (facilityType == "餐饮")
        return {"中式快餐","西式快餐","地方特色菜","减脂餐","自选菜",
                "高级餐厅","面食","饮品店"};
    if (facilityType == "教学")
        return {"贩卖机食品"};
    return {};
}

// 菜系默认美食数据
inline QMap<QString, QStringList> defaultCuisineFoods() {
    return {
        {"贩卖机食品",  {"零售饮品","零售食品"}},
        {"中式快餐",    {"麻辣烫","米线"}},
        {"西式快餐",    {"汉堡包","薯条","炸鸡","可乐"}},
        {"地方特色菜",  {"叉烧饭","烤鸭饭","西北菜","石锅拌饭"}},
        {"减脂餐",      {"自选减脂餐","减脂套餐"}},
        {"自选菜",      {"自选菜"}},
        {"高级餐厅",    {"火锅","烧烤","烤鱼"}},
        {"面食",        {"拉面","拌面","饺子","抄手"}},
        {"饮品店",      {"奶茶","果茶","咖啡"}},
    };
}

// ========== 美食数据管理（静态方法） ==========

namespace FoodData {

// 获取 foods.json 路径
inline QString foodsPath() {
    return QApplication::applicationDirPath() + "/data/foods.json";
}

// 加载所有美食
inline QVector<FoodItem> loadAll() {
    QVector<FoodItem> items;
    QFile file(foodsPath());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return items;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    for (const auto& v : doc.array())
        items.append(FoodItem::fromJson(v.toObject()));
    return items;
}

// 保存所有美食
inline bool saveAll(const QVector<FoodItem>& items) {
    QDir dir;
    if (!dir.exists(QApplication::applicationDirPath() + "/data"))
        dir.mkpath(QApplication::applicationDirPath() + "/data");
    QJsonArray arr;
    for (const auto& f : items) arr.append(f.toJson());
    QFile file(foodsPath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
    file.write(QJsonDocument(arr).toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

// 初始化：如果 foods.json 为空，从节点数据生成默认美食
inline void initDefault() {
    if (!loadAll().isEmpty()) return;

    QVector<FoodItem> items;
    // 从 graph 节点数据读取（通过外部传入，或在此处 hardcode 餐饮节点名）
    // 这里通过一个中间辅助：外部在创建 graph 后调用 regenerateAll
}

// 根据节点列表重新生成默认美食（每次切换 campus 不重置，仅首次初始化）
inline void regenerateAll(const QVector<QString>& foodNodes,
                           const QMap<QString, QString>& nodeFacilityMap) {
    QMap<QString, QStringList> defaults = defaultCuisineFoods();

    QVector<FoodItem> items;
    for (const auto& nodeName : foodNodes) {
        QString ft = nodeFacilityMap.value(nodeName);
        QStringList cuisines = cuisinesForFacility(ft);
        for (const auto& cuisine : cuisines) {
            QStringList foods = defaults.value(cuisine);
            for (const auto& food : foods) {
                FoodItem fi;
                fi.name     = food;
                fi.cuisine  = cuisine;
                fi.location = nodeName;
                items.append(fi);
            }
        }
    }
    saveAll(items);
}

// 用户添加菜品
inline FoodItem addUserFood(const QString& name, const QString& cuisine,
                             const QString& location) {
    FoodItem fi;
    fi.name     = name;
    fi.cuisine  = cuisine;
    fi.location = location;
    auto items = loadAll();
    items.append(fi);
    saveAll(items);
    return fi;
}

// 设置评分
inline bool setRating(int foodIndex, const QString& userId, int score) {
    auto items = loadAll();
    if (foodIndex < 0 || foodIndex >= items.size()) return false;
    if (score < 1 || score > 5) return false;
    items[foodIndex].setUserRating(userId, score);
    return saveAll(items);
}

// 小顶堆下沉（复用于 FoodItem 的 Top-K）
namespace {
void foodHeapSiftDown(QVector<FoodItem>& heap, int i, int heapSize,
                       bool byHeat) {
    while (true) {
        int left  = 2 * i + 1;
        int right = 2 * i + 2;
        int smallest = i;
        auto key = [byHeat](const FoodItem& f) {
            return byHeat ? static_cast<double>(f.ratingCount())
                          : f.avgRating();
        };
        if (left < heapSize && key(heap[left]) < key(heap[smallest]))
            smallest = left;
        if (right < heapSize && key(heap[right]) < key(heap[smallest]))
            smallest = right;
        if (smallest == i) break;
        std::swap(heap[i], heap[smallest]);
        i = smallest;
    }
}
} // anonymous namespace

// 堆 Top-K
inline QVector<FoodItem> topK(const QVector<FoodItem>& src, int k,
                               bool byHeat,
                               const QString& cuisineFilter,
                               const QString& search) {
    QVector<FoodItem> filtered;
    for (const auto& f : src) {
        if (!cuisineFilter.isEmpty() && f.cuisine != cuisineFilter) continue;
        if (!search.isEmpty()) {
            QString hay = f.name + "," + f.cuisine + "," + f.location;
            if (!hay.contains(search, Qt::CaseInsensitive)) continue;
        }
        filtered.append(f);
    }
    if (filtered.isEmpty()) return {};
    k = qMin(k, filtered.size());

    QVector<FoodItem> heap;
    for (int i = 0; i < k; ++i) heap.append(filtered[i]);
    for (int i = k / 2 - 1; i >= 0; --i)
        foodHeapSiftDown(heap, i, k, byHeat);

    auto key = [byHeat](const FoodItem& f) {
        return byHeat ? static_cast<double>(f.ratingCount()) : f.avgRating();
    };

    for (int i = k; i < filtered.size(); ++i) {
        if (key(filtered[i]) > key(heap[0])) {
            heap[0] = filtered[i];
            foodHeapSiftDown(heap, 0, k, byHeat);
        }
    }

    for (int i = k - 1; i >= 1; --i) {
        std::swap(heap[0], heap[i]);
        foodHeapSiftDown(heap, 0, i, byHeat);
    }
    return heap;
}

// 获取所有菜系名称（用于下拉框）
inline QStringList allCuisineNames() {
    return {"贩卖机食品","中式快餐","西式快餐","地方特色菜",
            "减脂餐","自选菜","高级餐厅","面食","饮品店"};
}

} // namespace FoodData

#endif // FOODDATABASE_H
