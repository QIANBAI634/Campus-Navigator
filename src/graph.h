/**
 * 校园图结构与最短路径算法
 *
 * 核心数据结构：
 * - 邻接表存储图（adjacency list）
 * - 基于 Haversine 公式计算真实地理距离作为边权
 * - Dijkstra 算法求单源最短路径
 *
 * 所有数据结构和算法均为自主实现，不依赖第三方图库。
 */
#ifndef GRAPH_H
#define GRAPH_H

#include <QVector>
#include <QMap>
#include <QSet>
#include <QPair>
#include <QString>
#include <QStringList>
#include <limits>
#include <cmath>
#include <algorithm>
#include "navigationdata.h"

// MSVC 兼容：确保 M_PI 可用
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// 表示一条邻接边
struct Edge {
    int    to;               // 目标节点索引
    double weight;           // 边权（米，基于经纬度计算）
    double congestion = 1.0; // 拥挤度 (0.1~1.0)
    bool   allowBike = true; // 允许自行车
    bool   isShuttle = false;// 电瓶车路线
};

// 路线规划策略
enum RouteStrategy {
    SHORTEST_DISTANCE = 0,  // 最短距离（原 Dijkstra）
    SHORTEST_TIME     = 1,  // 最短时间（含拥挤度）
    BIKE_ONLY         = 2,  // 仅自行车道
    SHUTTLE           = 3   // 电瓶车路线
};

// 附近设施查询结果
struct NearbyFacility {
    int    nodeIndex;   // 设施节点索引
    double distance;    // 实际步行距离（米），基于 Dijkstra 路径
    QString name;       // 设施名称
    QString category;   // 设施类别
};

// 多点路径规划结果
struct MultiStopResult {
    QVector<int> fullPath;      // 完整路径
    QVector<int> visitOrder;    // 途经点在路径中的顺序（索引）
    double       totalDistance; // 总距离（米）
    bool         optimal = true; // true=最短路线枚举, false=按添加顺序
};

/**
 * 校园图类
 * 封装了图构建、最短路径计算、路径显示等功能
 */
class CampusGraph {
public:
    CampusGraph();

    // Dijkstra 算法结果
    struct DijkstraResult {
        QVector<double> dist;   // 从起点到各节点的最短距离
        QVector<int>    prev;   // 前驱节点数组，用于路径重构
    };

    // ========== 图信息 ==========
    int nodeCount()     const { return m_nodeCount; }
    int landmarkCount() const { return m_landmarkCount; }
    int edgeCount()     const { return m_edgeCount; }

    const QVector<NodeInfo>& nodes()        const { return m_nodeList; }
    const QVector<QVector<Edge>>& adjList() const { return m_adj; }

    // 根据名称查找索引
    int indexOf(const QString& name) const {
        return m_nodeIndexMap.value(name, -1);
    }

    // 获取所有地标节点（筛选 type == "landmark"）
    QVector<NodeInfo> getLandmarks() const;

    // ========== 核心算法 ==========

    /**
     * Dijkstra 最短路径算法（基础版：最短距离）
     * @param startIdx 起点索引
     * @param endIdx   终点索引
     * @return 距离数组和前驱数组
     * 时间复杂度：O(V²)
     */
    DijkstraResult dijkstra(int startIdx, int endIdx) const;

    /**
     * Dijkstra 最短路径算法（策略版）
     * @param startIdx  起点索引
     * @param endIdx    终点索引
     * @param strategy  路线策略
     *   - SHORTEST_DISTANCE: 边权=原始距离
     *   - SHORTEST_TIME:     边权=距离/拥挤度（拥挤度越低边权越大）
     *   - BIKE_ONLY:         仅走自行车道（allowBike==true）
     *   - SHUTTLE:           仅走电瓶车路线（isShuttle==true）
     * @return 距离数组和前驱数组
     */
    DijkstraResult dijkstra(int startIdx, int endIdx, RouteStrategy strategy) const;

    /**
     * 根据前驱数组重构最短路径
     * @param prev     前驱数组
     * @param startIdx 起点索引
     * @param endIdx   终点索引
     * @return 路径节点索引序列（从起点到终点）
     */
    QVector<int> reconstructPath(const QVector<int>& prev,
                                 int startIdx, int endIdx) const;

    /**
     * 过滤路径，只保留地标节点（隐藏路口节点）
     * @param indices 完整路径的节点索引序列
     * @return 仅包含地标的节点索引序列
     */
    QVector<int> filterLandmarkPath(const QVector<int>& indices) const;

    /**
     * 将路径格式化为可读字符串
     * @param indices 路径节点索引序列
     * @return 如 "西门 → 菜鸟驿站 → 图书馆" 的字符串
     */
    QString formatPathDisplay(const QVector<int>& indices) const;

    /**
     * 查询指定节点附近的服务设施
     * 使用 Dijkstra 计算实际步行距离（非直线距离）
     * @param centerIdx      中心节点索引（当前所在位置）
     * @param maxRange       最大搜索范围（米），默认 500 米
     * @param categoryFilter 设施类别过滤，空字符串表示不过滤
     * @return 按实际距离排序的附近设施列表
     */
    QVector<NearbyFacility> findNearbyFacilities(int centerIdx,
                                                  double maxRange = 500.0,
                                                  const QString& categoryFilter = "") const;

    /**
     * 途经多点最短路径
     * 通过两两 Dijkstra + 全排列枚举求最优访问顺序
     * @param startIdx    起点索引
     * @param stopIndices 途经点索引列表
     * @return 完整路径 + 访问顺序 + 总距离
     */
    MultiStopResult findMultiStopRoute(int startIdx,
                                       const QVector<int>& stopIndices,
                                       bool sequential = false) const;

private:
    QVector<NodeInfo>           m_nodeList;       // 所有节点
    QMap<QString, int>          m_nodeIndexMap;   // 节点名 -> 索引
    QVector<QVector<Edge>>      m_adj;            // 邻接表
    int                         m_nodeCount;
    int                         m_landmarkCount;
    int                         m_edgeCount;

    // 构建图：建立邻接表
    void buildGraph();

    /**
     * Haversine 公式：计算两点间的球面距离
     * @return 距离（米）
     */
    static double haversineDistance(double lng1, double lat1,
                                    double lng2, double lat2);
};

// ============================================================
// 内联实现
// ============================================================

inline CampusGraph::CampusGraph()
    : m_nodeCount(0), m_landmarkCount(0), m_edgeCount(0)
{
    // 复制节点数据
    m_nodeList = nodesData;
    m_nodeCount = m_nodeList.size();

    // 建立名称->索引映射
    for (int i = 0; i < m_nodeCount; ++i) {
        m_nodeIndexMap[m_nodeList[i].name] = i;
        if (m_nodeList[i].type == "landmark") {
            ++m_landmarkCount;
        }
    }

    // 初始化邻接表
    m_adj.resize(m_nodeCount);

    // 构建图
    buildGraph();
}

inline void CampusGraph::buildGraph()
{
    m_edgeCount = 0;

    // 伪随机哈希，用于生成拥挤度
    auto edgeHash = [](int a, int b) -> double {
        unsigned h = static_cast<unsigned>(a * 2654435761ULL + b * 1597334677ULL);
        return 0.3 + (h % 71) * 0.01;  // 0.30 ~ 1.00
    };

    // 定义电瓶车路线（固定路段）
    QSet<QPair<QString,QString>> shuttleEdges;
    QStringList shuttlePairs = {
        "西门","2",  "2","8",  "8","30",  "30","学生食堂",
        "学生食堂","南门",  "8","11",  "11","南门",  "11","14",
        "14","医务室",  "8","7",  "7","医务室",  "11","图书馆"
    };
    for (int i = 0; i < shuttlePairs.size(); i += 2) {
        shuttleEdges.insert({shuttlePairs[i], shuttlePairs[i+1]});
        shuttleEdges.insert({shuttlePairs[i+1], shuttlePairs[i]});
    }

    for (const auto& edge : edgesRaw) {
        int fromIdx = m_nodeIndexMap.value(edge.from, -1);
        int toIdx   = m_nodeIndexMap.value(edge.to, -1);
        if (fromIdx < 0 || toIdx < 0) continue;

        const auto& fromNode = m_nodeList[fromIdx];
        const auto& toNode   = m_nodeList[toIdx];

        double w = haversineDistance(fromNode.lng, fromNode.lat,
                                     toNode.lng, toNode.lat);

        bool isShuttle = shuttleEdges.contains({edge.from, edge.to});
        double cong = edgeHash(fromIdx, toIdx);

        // 无向边：双向添加（所有边均可自行车）
        m_adj[fromIdx].push_back({toIdx, w, cong, true, isShuttle});
        m_adj[toIdx].push_back({fromIdx, w, cong, true, isShuttle});
        ++m_edgeCount;
    }

    // 自动拆分长边
    const int TARGET_EDGES = 200;
    int splitNodeId = 35;

    while (m_edgeCount < TARGET_EDGES) {
        double maxW = 0.0;
        int maxU = -1, maxV = -1;
        double maxCong = 1.0;
        bool   maxBike = true;
        bool   maxShuttle = false;
        for (int u = 0; u < m_adj.size(); ++u) {
            for (int j = 0; j < m_adj[u].size(); ++j) {
                if (m_adj[u][j].to > u && m_adj[u][j].weight > maxW) {
                    maxW = m_adj[u][j].weight;
                    maxU = u;
                    maxV = m_adj[u][j].to;
                    maxCong = m_adj[u][j].congestion;
                    maxBike = m_adj[u][j].allowBike;
                    maxShuttle = m_adj[u][j].isShuttle;
                }
            }
        }
        if (maxU < 0 || maxW < 2.0) break;

        NodeInfo midNode;
        midNode.name  = QString::number(splitNodeId++);
        midNode.type  = "intersection";
        midNode.lng   = (m_nodeList[maxU].lng + m_nodeList[maxV].lng) / 2.0;
        midNode.lat   = (m_nodeList[maxU].lat + m_nodeList[maxV].lat) / 2.0;
        midNode.pixelX = (m_nodeList[maxU].pixelX + m_nodeList[maxV].pixelX) / 2.0;
        midNode.pixelY = (m_nodeList[maxU].pixelY + m_nodeList[maxV].pixelY) / 2.0;
        midNode.floor = 0;
        midNode.buildingId = 0;

        int midIdx = m_nodeList.size();
        m_nodeList.append(midNode);
        m_nodeIndexMap[midNode.name] = midIdx;
        ++m_nodeCount;

        auto removeEdge = [](QVector<Edge>& vec, int target) {
            for (int k = 0; k < vec.size(); ++k) {
                if (vec[k].to == target) { vec.removeAt(k); return; }
            }
        };
        removeEdge(m_adj[maxU], maxV);
        removeEdge(m_adj[maxV], maxU);

        double w1 = haversineDistance(m_nodeList[maxU].lng, m_nodeList[maxU].lat,
                                       m_nodeList[midIdx].lng, m_nodeList[midIdx].lat);
        double w2 = haversineDistance(m_nodeList[midIdx].lng, m_nodeList[midIdx].lat,
                                       m_nodeList[maxV].lng, m_nodeList[maxV].lat);

        m_adj.append(QVector<Edge>());
        m_adj[maxU].push_back({midIdx, w1, maxCong, maxBike, maxShuttle});
        m_adj[midIdx].push_back({maxU, w1, maxCong, maxBike, maxShuttle});
        m_adj[midIdx].push_back({maxV, w2, maxCong, maxBike, maxShuttle});
        m_adj[maxV].push_back({midIdx, w2, maxCong, maxBike, maxShuttle});
        ++m_edgeCount;
    }
}

inline double CampusGraph::haversineDistance(double lng1, double lat1,
                                              double lng2, double lat2)
{
    const double R = 6371000.0;  // 地球半径（米）
    auto toRad = [](double deg) { return deg * M_PI / 180.0; };

    double dLat = toRad(lat2 - lat1);
    double dLon = toRad(lng2 - lng1);
    double a = std::sin(dLat / 2.0) * std::sin(dLat / 2.0) +
               std::cos(toRad(lat1)) * std::cos(toRad(lat2)) *
               std::sin(dLon / 2.0) * std::sin(dLon / 2.0);
    double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
    return std::round(R * c);
}

inline CampusGraph::DijkstraResult
CampusGraph::dijkstra(int startIdx, int endIdx) const
{
    const double INF = std::numeric_limits<double>::infinity();

    QVector<double> dist(m_nodeCount, INF);
    QVector<int>    prev(m_nodeCount, -1);
    QVector<bool>   visited(m_nodeCount, false);

    dist[startIdx] = 0.0;

    for (int i = 0; i < m_nodeCount; ++i) {
        // 找到未访问节点中距离最小的
        int    u       = -1;
        double minDist = INF;
        for (int j = 0; j < m_nodeCount; ++j) {
            if (!visited[j] && dist[j] < minDist) {
                minDist = dist[j];
                u = j;
            }
        }

        // 无可达节点或已到达终点，提前结束
        if (u == -1 || u == endIdx) break;

        visited[u] = true;

        // 松弛操作
        for (const auto& edge : m_adj[u]) {
            int    v = edge.to;
            double w = edge.weight;
            if (!visited[v] && dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                prev[v] = u;
            }
        }
    }

    return {dist, prev};
}

inline CampusGraph::DijkstraResult
CampusGraph::dijkstra(int startIdx, int endIdx, RouteStrategy strategy) const
{
    const double INF = std::numeric_limits<double>::infinity();
    QVector<double> dist(m_nodeCount, INF);
    QVector<int>    prev(m_nodeCount, -1);
    QVector<bool>   visited(m_nodeCount, false);

    dist[startIdx] = 0.0;

    for (int i = 0; i < m_nodeCount; ++i) {
        int    u       = -1;
        double minDist = INF;
        for (int j = 0; j < m_nodeCount; ++j) {
            if (!visited[j] && dist[j] < minDist) {
                minDist = dist[j];
                u = j;
            }
        }
        if (u == -1 || u == endIdx) break;
        visited[u] = true;

        for (const auto& edge : m_adj[u]) {
            if (visited[edge.to]) continue;

            // 策略过滤
            if (strategy == BIKE_ONLY && !edge.allowBike) continue;
            if (strategy == SHUTTLE   && !edge.isShuttle)  continue;

            // 边权计算
            double cost;
            switch (strategy) {
            case SHORTEST_TIME:
                // 时间 = 距离 / 拥挤度（拥挤度越低越慢）
                cost = edge.weight / edge.congestion;
                break;
            default:
                cost = edge.weight;
                break;
            }

            if (dist[u] + cost < dist[edge.to]) {
                dist[edge.to] = dist[u] + cost;
                prev[edge.to] = u;
            }
        }
    }
    return {dist, prev};
}

inline QVector<int> CampusGraph::reconstructPath(const QVector<int>& prev,
                                                  int startIdx,
                                                  int endIdx) const
{
    QVector<int> path;
    if (prev[endIdx] == -1 && endIdx != startIdx) {
        return path;  // 不可达
    }

    int cur = endIdx;
    while (cur != -1) {
        path.prepend(cur);
        if (cur == startIdx) break;
        cur = prev[cur];
    }

    return (path.isEmpty() || path[0] != startIdx) ? QVector<int>() : path;
}

inline QVector<int> CampusGraph::filterLandmarkPath(const QVector<int>& indices) const
{
    QVector<int> result;
    for (int idx : indices) {
        if (m_nodeList[idx].type == "landmark") {
            result.append(idx);
        }
    }
    return result;
}

inline QString CampusGraph::formatPathDisplay(const QVector<int>& indices) const
{
    if (indices.isEmpty()) {
        return QString::fromUtf8("⚠️ 无法找到连通路径");
    }
    QStringList names;
    for (int idx : indices) {
        names.append(m_nodeList[idx].name);
    }
    return names.join("  →  ");
}

inline MultiStopResult
CampusGraph::findMultiStopRoute(int startIdx,
                                 const QVector<int>& stopIndices,
                                 bool sequential) const
{
    MultiStopResult result;
    result.optimal = !sequential;
    result.totalDistance = std::numeric_limits<double>::infinity();

    if (stopIndices.isEmpty()) return result;

    // 1. 计算所有地点的两两最短距离（含起点）
    QVector<int> allNodes;
    allNodes.append(startIdx);
    for (int s : stopIndices) allNodes.append(s);
    int N = allNodes.size();

    QVector<QVector<double>> distMatrix(N, QVector<double>(N, -1));
    QVector<QVector<QVector<int>>> pathMatrix(N,
        QVector<QVector<int>>(N));

    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            if (i == j) { distMatrix[i][j] = 0; continue; }
            auto [d, p] = dijkstra(allNodes[i], allNodes[j]);
            distMatrix[i][j] = d[allNodes[j]];
            pathMatrix[i][j] = reconstructPath(p, allNodes[i], allNodes[j]);
        }
    }

    // 2. 确定访问顺序
    QVector<int> visitOrder;

    if (sequential) {
        // 按添加顺序：1, 2, 3, ..., N-1
        for (int k = 1; k < N; ++k) visitOrder.append(k);
    } else {
        // 全排列枚举最优访问顺序（K ≤ 5）
        QVector<int> perm;
        for (int k = 1; k < N; ++k) perm.append(k);
        double bestDist = std::numeric_limits<double>::infinity();

        do {
            double total = distMatrix[0][perm[0]];
            for (int i = 0; i < perm.size() - 1; ++i)
                total += distMatrix[perm[i]][perm[i+1]];

            if (total < bestDist) {
                bestDist = total;
                visitOrder = perm;
            }
        } while (std::next_permutation(perm.begin(), perm.end()));
    }

    if (visitOrder.isEmpty()) return result;

    result.visitOrder = visitOrder;

    // 3. 计算总距离（不返回起点）
    result.totalDistance = distMatrix[0][visitOrder[0]];
    for (int i = 0; i < visitOrder.size() - 1; ++i)
        result.totalDistance += distMatrix[visitOrder[i]][visitOrder[i+1]];

    // 4. 拼接完整路径
    result.fullPath.clear();
    result.fullPath.append(pathMatrix[0][visitOrder[0]]);
    for (int i = 0; i < visitOrder.size() - 1; ++i) {
        QVector<int> seg = pathMatrix[visitOrder[i]][visitOrder[i+1]];
        if (!seg.isEmpty()) seg.removeFirst();
        result.fullPath.append(seg);
    }

    if (!std::isfinite(result.totalDistance)) {
        result.fullPath.clear();
        result.visitOrder.clear();
    }

    return result;
}

inline QVector<NodeInfo> CampusGraph::getLandmarks() const
{
    QVector<NodeInfo> result;
    for (const auto& node : m_nodeList) {
        if (node.type == "landmark") {
            result.append(node);
        }
    }
    return result;
}

inline QVector<NearbyFacility> CampusGraph::findNearbyFacilities(
    int centerIdx, double maxRange, const QString& categoryFilter) const
{
    QVector<NearbyFacility> results;

    if (centerIdx < 0 || centerIdx >= m_nodeCount) return results;

    // 运行 Dijkstra 计算从 center 到所有节点的实际路径距离
    auto [dist, prev] = dijkstra(centerIdx, -1);

    for (int i = 0; i < m_nodeCount; ++i) {
        // 只考虑地标节点（设施），排除中心节点自身
        if (i == centerIdx) continue;
        if (m_nodeList[i].type != "landmark") continue;

        double d = dist[i];
        if (!std::isfinite(d) || d > maxRange) continue;

        const QString& ft = m_nodeList[i].facilityType;
        // 类别过滤：空字符串表示不过滤
        if (!categoryFilter.isEmpty() && ft != categoryFilter) continue;

        results.append({i, d, m_nodeList[i].name, ft});
    }

    // 按距离升序排序（稳定排序保留同类内的相对顺序）
    std::stable_sort(results.begin(), results.end(),
        [](const NearbyFacility& a, const NearbyFacility& b) {
            return a.distance < b.distance;
        });

    return results;
}

#endif // GRAPH_H
