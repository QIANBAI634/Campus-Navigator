/**
 * 旅行日记管理模块
 *
 * 数据结构：
 * - TrackPoint: 单链表节点，存储导航轨迹点
 * - PhotoAttachment: 配图附件元数据
 * - DiaryEntry: 日记条目（含内容、分类、配图、轨迹）
 *
 * 存储方式：
 * - JSON 文件存储日记草稿和已发布日记
 * - 替代 HTML 版本的 IndexedDB + localStorage
 */
#ifndef DIARYMANAGER_H
#define DIARYMANAGER_H

#include <QString>
#include <QStringList>
#include <QVector>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDir>
#include <QFile>
#include <QFileInfo>

// ============================================================
// 日记分类
// ============================================================
namespace DiaryCategory {
    const QString TRAVEL_NOTE        = "游记";
    const QString JOURNEY_REFLECTION = "旅途感悟";
    const QString DAILY_ROUTINE      = "出行日常";
    const QString GUIDE              = "游玩攻略";
    const QString SPOT_COMPLAINT     = "景点吐槽";
    const QString CHECKIN_DIARY      = "打卡随笔";

    inline QStringList allCategories() {
        return {TRAVEL_NOTE, JOURNEY_REFLECTION, DAILY_ROUTINE,
                GUIDE, SPOT_COMPLAINT, CHECKIN_DIARY};
    }
}

// ============================================================
// 轨迹点（单链表节点）
// ============================================================
struct TrackPoint {
    double  lat;
    double  lng;
    QString name;
    QString timestamp;  // ISO 8601 格式

    TrackPoint* next;   // 链表指针

    TrackPoint(double lat_, double lng_, const QString& name_ = "")
        : lat(lat_), lng(lng_), name(name_),
          timestamp(QDateTime::currentDateTime().toString(Qt::ISODate)),
          next(nullptr) {}

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["lat"] = lat;
        obj["lng"] = lng;
        obj["name"] = name;
        obj["timestamp"] = timestamp;
        return obj;
    }

    static TrackPoint* fromJson(const QJsonObject& obj) {
        auto* tp = new TrackPoint(
            obj["lat"].toDouble(),
            obj["lng"].toDouble(),
            obj["name"].toString()
        );
        tp->timestamp = obj["timestamp"].toString();
        return tp;
    }
};

// ============================================================
// 配图附件
// ============================================================
struct PhotoAttachment {
    QString imageFilePath;  // 图片文件路径
    QString caption;        // 图片说明
    int     attachOrder;    // 附件序号

    PhotoAttachment() : attachOrder(0) {}
    PhotoAttachment(const QString& path, const QString& cap, int order)
        : imageFilePath(path), caption(cap), attachOrder(order) {}

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["path"]  = imageFilePath;
        obj["caption"] = caption;
        obj["order"]  = attachOrder;
        return obj;
    }

    static PhotoAttachment fromJson(const QJsonObject& obj) {
        return PhotoAttachment(
            obj["path"].toString(),
            obj["caption"].toString(),
            obj["order"].toInt()
        );
    }
};

// ============================================================
// 日记条目
// ============================================================
struct DiaryEntry {
    qint64                  id;             // 唯一标识（时间戳）
    QString                 title;          // 日记标题
    QString                 content;        // 日记文本内容
    QString                 category;       // 分类
    QString                 campusName;     // 当前景区/校园名称
    QVector<PhotoAttachment> photos;        // 配图列表
    QVector<QJsonObject>     trackPoints;   // 轨迹点（JSON数组）
    QString                 createdAt;      // 创建时间
    QString                 publishedAt;    // 发布时间（草稿为空）
    QString                 userId;         // 用户ID
    QString                 userNickname;   // 用户昵称
    int                     views;          // 浏览次数（热度）
    QMap<QString, int>      userRatings;    // 用户评分：userId → score (1~5)

    DiaryEntry() : id(0), views(0) {
        createdAt = QDateTime::currentDateTime().toString(Qt::ISODate);
    }

    // 平均评分 (0.0 ~ 5.0) — 从 userRatings 实时计算
    double avgRating() const {
        if (userRatings.isEmpty()) return 0.0;
        double sum = 0.0;
        for (int s : userRatings.values()) sum += s;
        return sum / userRatings.size();
    }

    // 评分人次
    int ratingCount() const { return userRatings.size(); }

    // 设置/替换某用户的评分（同一用户只能评一次，后评覆盖前评）
    void setUserRating(const QString& uid, int score) {
        userRatings[uid] = score;
    }

    // 获取某用户的评分（0 表示未评）
    int getUserRating(const QString& uid) const {
        return userRatings.value(uid, 0);
    }

    // 获取预览文本（前50个字符）
    QString preview() const {
        if (content.length() <= 50) return content;
        return content.left(50) + "...";
    }

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["id"]         = id;
        obj["title"]      = title;
        obj["content"]    = content;
        obj["category"]   = category;
        obj["campusName"] = campusName;
        obj["createdAt"]  = createdAt;
        obj["publishedAt"] = publishedAt;
        obj["userId"]     = userId;
        obj["userNickname"] = userNickname;
        obj["preview"]    = preview();
        obj["views"]      = views;

        // userRatings → JSON
        QJsonObject ratingsObj;
        for (auto it = userRatings.constBegin(); it != userRatings.constEnd(); ++it)
            ratingsObj[it.key()] = it.value();
        obj["userRatings"] = ratingsObj;

        QJsonArray photoArr;
        for (const auto& p : photos) {
            photoArr.append(p.toJson());
        }
        obj["photos"] = photoArr;

        QJsonArray trackArr;
        for (const auto& tp : trackPoints) {
            trackArr.append(tp);
        }
        obj["trackPoints"] = trackArr;
        obj["trackPointCount"] = trackPoints.size();

        return obj;
    }

    static DiaryEntry fromJson(const QJsonObject& obj) {
        DiaryEntry entry;
        entry.id           = static_cast<qint64>(obj["id"].toDouble());
        entry.title        = obj["title"].toString();
        entry.content      = obj["content"].toString();
        entry.category     = obj["category"].toString();
        entry.campusName   = obj["campusName"].toString();
        entry.createdAt    = obj["createdAt"].toString();
        entry.publishedAt  = obj["publishedAt"].toString();
        entry.userId       = obj["userId"].toString();
        entry.userNickname = obj["userNickname"].toString();
        entry.views        = obj["views"].toInt();

        // userRatings from JSON
        QJsonObject ratingsObj = obj["userRatings"].toObject();
        for (auto it = ratingsObj.constBegin(); it != ratingsObj.constEnd(); ++it)
            entry.userRatings[it.key()] = it.value().toInt();

        QJsonArray photoArr = obj["photos"].toArray();
        for (const auto& p : photoArr) {
            entry.photos.append(PhotoAttachment::fromJson(p.toObject()));
        }

        QJsonArray trackArr = obj["trackPoints"].toArray();
        for (const auto& t : trackArr) {
            entry.trackPoints.append(t.toObject());
        }

        return entry;
    }
};

// ============================================================
// 日记管理器
// ============================================================
class DiaryManager {
public:
    DiaryManager();

    // ========== 日记存储路径设置 ==========
    void setStoragePath(const QString& path);
    QString storagePath() const { return m_storagePath; }

    // ========== 草稿管理 ==========
    QVector<DiaryEntry> loadDrafts() const;
    bool saveDrafts(const QVector<DiaryEntry>& drafts) const;
    bool addDraft(const DiaryEntry& entry);
    bool removeDraft(int index);
    int  draftCount() const;

    // ========== 已发布日记管理（全局共享） ==========
    static QVector<DiaryEntry> loadPublished(const QString& dataDir);
    static bool savePublished(const QString& dataDir, const QVector<DiaryEntry>& published);
    bool publishDraft(int draftIndex, const QString& dataDir,
                      const QString& campusName, const QString& nickname);
    static bool removePublished(const QString& dataDir, int index);
    static int  publishedCount(const QString& dataDir);

    // 增加浏览次数（热度）
    static bool incrementViews(const QString& dataDir, int index);

    // 设置用户评分 (1~5)，同一用户后评覆盖前评
    static bool setUserRating(const QString& dataDir, int index,
                              const QString& userId, int score);

    // ========== 日记搜索/推荐（核心算法）==========

    // 堆 Top-K 选择：按热度(views)或评分(avgRating)选出前K条
    static QVector<DiaryEntry> topKDiaries(const QVector<DiaryEntry>& src, int k,
                                            bool byHeat);

    // 按目的地搜索 + 排序
    static QVector<DiaryEntry> searchByDestination(const QVector<DiaryEntry>& src,
                                                    const QString& keyword,
                                                    bool byHeat);

    // 按标题精确查找（基于 QHash，O(1) 均摊）
    static int findDiaryByTitle(const QVector<DiaryEntry>& src,
                                const QString& title);

    // ========== 图片管理 ==========
    QString copyPhotoToStorage(const QString& sourcePath);

private:
    QString m_storagePath;

    // 辅助：读写 JSON 文件
    bool writeJsonFile(const QString& filename, const QJsonArray& array) const;
    QJsonArray readJsonFile(const QString& filename) const;
};

#endif // DIARYMANAGER_H
