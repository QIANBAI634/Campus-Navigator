/**
 * 旅行日记管理器实现
 * 使用 JSON 文件持久化存储日记数据
 */
#include "diarymanager.h"
#include <QDebug>
#include <QFileInfo>

DiaryManager::DiaryManager()
{
    // 默认存储在可执行文件所在目录下的 data 子目录
    m_storagePath = QDir::currentPath() + "/data";
}

void DiaryManager::setStoragePath(const QString& path)
{
    m_storagePath = path;
    // 确保存储目录存在
    QDir dir;
    if (!dir.exists(m_storagePath)) {
        dir.mkpath(m_storagePath);
    }
    // 创建 photos 子目录
    QString photosDir = m_storagePath + "/photos";
    if (!dir.exists(photosDir)) {
        dir.mkpath(photosDir);
    }
}

// ========== 草稿管理 ==========

QVector<DiaryEntry> DiaryManager::loadDrafts() const
{
    QVector<DiaryEntry> drafts;
    QJsonArray arr = readJsonFile("drafts.json");
    for (const auto& val : arr) {
        drafts.append(DiaryEntry::fromJson(val.toObject()));
    }
    return drafts;
}

bool DiaryManager::saveDrafts(const QVector<DiaryEntry>& drafts) const
{
    QJsonArray arr;
    for (const auto& d : drafts) {
        arr.append(d.toJson());
    }
    return writeJsonFile("drafts.json", arr);
}

bool DiaryManager::addDraft(const DiaryEntry& entry)
{
    QVector<DiaryEntry> drafts = loadDrafts();
    drafts.prepend(entry);  // 最新的在最前面

    // 检查数据大小（粗略估计，超过 4MB 警告）
    QJsonArray arr;
    for (const auto& d : drafts) {
        arr.append(d.toJson());
    }
    QJsonDocument doc(arr);
    if (doc.toJson().size() > 4 * 1024 * 1024) {
        qWarning() << "草稿数据过大，建议删除部分旧草稿或配图";
        return false;
    }

    return saveDrafts(drafts);
}

bool DiaryManager::removeDraft(int index)
{
    QVector<DiaryEntry> drafts = loadDrafts();
    if (index < 0 || index >= drafts.size()) return false;
    drafts.removeAt(index);
    return saveDrafts(drafts);
}

int DiaryManager::draftCount() const
{
    return loadDrafts().size();
}

// ========== 已发布日记管理（全局共享，static 方法） ==========

QVector<DiaryEntry> DiaryManager::loadPublished(const QString& dataDir)
{
    QVector<DiaryEntry> published;
    QString fullPath = dataDir + "/published.json";
    QFile file(fullPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return published;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    for (const auto& val : doc.array())
        published.append(DiaryEntry::fromJson(val.toObject()));
    return published;
}

bool DiaryManager::savePublished(const QString& dataDir,
                                  const QVector<DiaryEntry>& published)
{
    QDir dir;
    if (!dir.exists(dataDir)) dir.mkpath(dataDir);

    QJsonArray arr;
    for (const auto& p : published) arr.append(p.toJson());
    QFile file(dataDir + "/published.json");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
    file.write(QJsonDocument(arr).toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

bool DiaryManager::publishDraft(int draftIndex, const QString& dataDir,
                                 const QString& campusName, const QString& nickname)
{
    QVector<DiaryEntry> drafts = loadDrafts();
    if (draftIndex < 0 || draftIndex >= drafts.size()) return false;

    DiaryEntry entry = drafts[draftIndex];
    entry.publishedAt  = QDateTime::currentDateTime().toString(Qt::ISODate);
    entry.campusName   = campusName;
    entry.userNickname = nickname;

    QVector<DiaryEntry> published = loadPublished(dataDir);
    published.prepend(entry);

    drafts.removeAt(draftIndex);

    return savePublished(dataDir, published) && saveDrafts(drafts);
}

bool DiaryManager::removePublished(const QString& dataDir, int index)
{
    QVector<DiaryEntry> published = loadPublished(dataDir);
    if (index < 0 || index >= published.size()) return false;
    published.removeAt(index);
    return savePublished(dataDir, published);
}

int DiaryManager::publishedCount(const QString& dataDir)
{
    return loadPublished(dataDir).size();
}

bool DiaryManager::incrementViews(const QString& dataDir, int index)
{
    QVector<DiaryEntry> published = loadPublished(dataDir);
    if (index < 0 || index >= published.size()) return false;
    published[index].views += 1;
    return savePublished(dataDir, published);
}

bool DiaryManager::setUserRating(const QString& dataDir, int index,
                                  const QString& userId, int score)
{
    QVector<DiaryEntry> published = loadPublished(dataDir);
    if (index < 0 || index >= published.size()) return false;
    if (score < 1 || score > 5) return false;
    published[index].setUserRating(userId, score);
    return savePublished(dataDir, published);
}

// ========== 日记搜索/推荐 核心算法 ==========

namespace {

// 小顶堆下沉 — 与旅游推荐共用同一个堆框架
void diaryHeapSiftDown(QVector<DiaryEntry>& heap, int i, int heapSize, bool byHeat)
{
    while (true) {
        int left  = 2 * i + 1;
        int right = 2 * i + 2;
        int smallest = i;
        auto key = [byHeat](const DiaryEntry& d) {
            return byHeat ? static_cast<double>(d.views) : d.avgRating();
        };
        if (left  < heapSize && key(heap[left])  < key(heap[smallest])) smallest = left;
        if (right < heapSize && key(heap[right]) < key(heap[smallest])) smallest = right;
        if (smallest == i) break;
        std::swap(heap[i], heap[smallest]);
        i = smallest;
    }
}

} // anonymous namespace

QVector<DiaryEntry> DiaryManager::topKDiaries(const QVector<DiaryEntry>& src,
                                                int k, bool byHeat)
{
    if (src.isEmpty()) return {};
    k = qMin(k, src.size());

    QVector<DiaryEntry> heap;
    for (int i = 0; i < k; ++i) heap.append(src[i]);
    for (int i = k / 2 - 1; i >= 0; --i)
        diaryHeapSiftDown(heap, i, k, byHeat);

    auto key = [byHeat](const DiaryEntry& d) {
        return byHeat ? static_cast<double>(d.views) : d.avgRating();
    };

    for (int i = k; i < src.size(); ++i) {
        if (key(src[i]) > key(heap[0])) {
            heap[0] = src[i];
            diaryHeapSiftDown(heap, 0, k, byHeat);
        }
    }

    for (int i = k - 1; i >= 1; --i) {
        std::swap(heap[0], heap[i]);
        diaryHeapSiftDown(heap, 0, i, byHeat);
    }
    std::reverse(heap.begin(), heap.end());
    return heap;
}

QVector<DiaryEntry> DiaryManager::searchByDestination(
    const QVector<DiaryEntry>& src, const QString& keyword, bool byHeat)
{
    QVector<DiaryEntry> matched;
    // 匹配 campusName 或任一轨迹点名
    for (const auto& d : src) {
        bool hit = d.campusName.contains(keyword, Qt::CaseInsensitive);
        if (!hit) {
            for (const auto& tp : d.trackPoints) {
                QString nm = tp["name"].toString();
                if (nm.contains(keyword, Qt::CaseInsensitive)) {
                    hit = true; break;
                }
            }
        }
        if (hit) matched.append(d);
    }

    // 按热度/评分降序排序
    std::sort(matched.begin(), matched.end(),
        [byHeat](const DiaryEntry& a, const DiaryEntry& b) {
            if (byHeat) return a.views > b.views;
            return a.avgRating() > b.avgRating();
        });
    return matched;
}

int DiaryManager::findDiaryByTitle(const QVector<DiaryEntry>& src,
                                     const QString& title)
{
    // 构建 QHash 索引 → O(1) 查找
    QHash<QString, int> index;
    for (int i = 0; i < src.size(); ++i)
        index[src[i].title] = i;
    return index.value(title.trimmed(), -1);
}

// ========== 图片管理 ==========

QString DiaryManager::copyPhotoToStorage(const QString& sourcePath)
{
    QDir dir;
    QString photosDir = m_storagePath + "/photos";
    if (!dir.exists(photosDir)) {
        dir.mkpath(photosDir);
    }

    QFileInfo fi(sourcePath);
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString destName = "photo_" + timestamp + "_" + fi.fileName();
    QString destPath = photosDir + "/" + destName;

    if (QFile::copy(sourcePath, destPath)) {
        return destPath;
    }
    // 如果复制失败，返回原始路径
    qWarning() << "复制图片失败:" << sourcePath << "->" << destPath;
    return sourcePath;
}

// ========== JSON 文件读写辅助 ==========

bool DiaryManager::writeJsonFile(const QString& filename,
                                  const QJsonArray& array) const
{
    QDir dir;
    if (!dir.exists(m_storagePath)) {
        dir.mkpath(m_storagePath);
    }

    QString fullPath = m_storagePath + "/" + filename;
    QFile file(fullPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "无法写入文件:" << fullPath;
        return false;
    }

    QJsonDocument doc(array);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

QJsonArray DiaryManager::readJsonFile(const QString& filename) const
{
    QString fullPath = m_storagePath + "/" + filename;
    QFile file(fullPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // 文件不存在时返回空数组（正常情况）
        return QJsonArray();
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "JSON 解析错误:" << parseError.errorString();
        return QJsonArray();
    }

    return doc.array();
}
