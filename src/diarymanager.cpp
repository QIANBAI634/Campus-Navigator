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
