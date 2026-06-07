/**
 * 多用户管理器实现
 */
#include "usermanager.h"
#include <QDateTime>
#include <QDebug>

UserManager::UserManager()
{
    m_storagePath = QDir::currentPath() + "/data";
    initDefaultUsers();
    loadUsers();
    // 默认使用第一个用户
    if (!m_users.isEmpty()) {
        m_currentUser = m_users.first();
    }
}

void UserManager::setStoragePath(const QString& path)
{
    m_storagePath = path;
    QDir dir;
    if (!dir.exists(m_storagePath)) {
        dir.mkpath(m_storagePath);
    }
}

void UserManager::initDefaultUsers()
{
    // 如果已经初始化过则跳过
    QString usersFile = m_storagePath + "/users.json";
    if (QFile::exists(usersFile)) return;

    // 预设 10 个用户
    QVector<UserInfo> defaults;
    QStringList names = {"旅行达人小明", "背包客小红", "摄影爱好者阿强",
                         "美食猎人小丽", "骑行侠阿飞", "文艺青年小雅",
                         "户外探险家老张", "历史迷老王", "亲子游小刘", "穷游达人阿杰"};
    QStringList avatars = {"🧑‍💻", "👩‍🎨", "📷", "🍜", "🚴", "🎨",
                           "🏔️", "📚", "👨‍👩‍👧", "🎒"};

    for (int i = 0; i < names.size(); ++i) {
        UserInfo u;
        u.userId    = QString("user_%1").arg(i + 1, 2, 10, QChar('0'));
        u.nickname  = names[i];
        u.avatar    = avatars[i];
        u.createdAt = QDateTime::currentDateTime().toString(Qt::ISODate);
        defaults.append(u);
    }

    QDir dir;
    if (!dir.exists(m_storagePath)) {
        dir.mkpath(m_storagePath);
    }

    QJsonArray arr;
    for (const auto& u : defaults) {
        arr.append(u.toJson());
    }
    QFile file(usersFile);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.write(QJsonDocument(arr).toJson(QJsonDocument::Indented));
        file.close();
    }
}

void UserManager::loadUsers()
{
    m_users.clear();
    QString usersFile = m_storagePath + "/users.json";
    QFile file(usersFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isArray()) return;
    for (const auto& val : doc.array()) {
        m_users.append(UserInfo::fromJson(val.toObject()));
    }
}

void UserManager::saveUsers()
{
    QJsonArray arr;
    for (const auto& u : m_users) {
        arr.append(u.toJson());
    }
    QString usersFile = m_storagePath + "/users.json";
    QFile file(usersFile);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.write(QJsonDocument(arr).toJson(QJsonDocument::Indented));
        file.close();
    }
}

bool UserManager::setCurrentUser(const QString& userId)
{
    for (const auto& u : m_users) {
        if (u.userId == userId) {
            m_currentUser = u;
            return true;
        }
    }
    return false;
}

bool UserManager::addUser(const QString& nickname)
{
    UserInfo u;
    u.userId    = QString("user_%1").arg(m_users.size() + 1, 2, 10, QChar('0'));
    u.nickname  = nickname;
    u.avatar    = "😊";
    u.createdAt = QDateTime::currentDateTime().toString(Qt::ISODate);
    m_users.append(u);
    saveUsers();
    return true;
}

bool UserManager::removeUser(const QString& userId)
{
    for (int i = 0; i < m_users.size(); ++i) {
        if (m_users[i].userId == userId) {
            m_users.removeAt(i);
            saveUsers();
            return true;
        }
    }
    return false;
}

QString UserManager::userDiaryDir(const QString& userId) const
{
    return m_storagePath + "/" + userId;
}
