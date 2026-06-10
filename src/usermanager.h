/**
 * 多用户管理模块
 *
 * 支持 ≥10 个本地用户，JSON 文件持久化。
 * 每个用户拥有独立的旅行日记数据。
 * 不依赖网络，纯本地存储。
 */
#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <QString>
#include <QStringList>
#include <QVector>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDir>
#include <QFile>

// 用户信息
struct UserInfo {
    QString userId;     // 唯一ID
    QString nickname;   // 昵称
    QString avatar;     // 头像表情
    QString createdAt;  // 创建时间

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["userId"]    = userId;
        obj["nickname"]  = nickname;
        obj["avatar"]    = avatar;
        obj["createdAt"] = createdAt;
        return obj;
    }

    static UserInfo fromJson(const QJsonObject& obj) {
        UserInfo u;
        u.userId    = obj["userId"].toString();
        u.nickname  = obj["nickname"].toString();
        u.avatar    = obj["avatar"].toString();
        u.createdAt = obj["createdAt"].toString();
        return u;
    }
};

class UserManager {
public:
    UserManager();

    void setStoragePath(const QString& path);

    // 获取所有用户
    QVector<UserInfo> allUsers() const { return m_users; }

    // 获取用户数量
    int userCount() const { return m_users.size(); }

    // 登录 / 切换用户
    bool setCurrentUser(const QString& userId);
    UserInfo currentUser() const { return m_currentUser; }
    QString  currentUserId() const { return m_currentUser.userId; }

    // 新建用户
    bool addUser(const QString& nickname);

    // 删除用户
    bool removeUser(const QString& userId);

    // 获取其他用户的日记目录名
    QString userDiaryDir(const QString& userId) const;

private:
    QVector<UserInfo> m_users;
    UserInfo          m_currentUser;
    QString           m_storagePath;

    void loadUsers();
    void saveUsers();
    void initDefaultUsers();
};

#endif // USERMANAGER_H
