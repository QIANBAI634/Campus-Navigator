/**
 * 主窗口
 * 包含导航界面和旅行日记面板
 * 布局匹配原 HTML 版本的设计风格
 */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QVector>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <QDialog>
#include <QDialogButtonBox>

#include "graph.h"
#include "diarymanager.h"
#include "mapwidget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // ========== 导航相关 ==========
    void onPlanRoute();             // 规划路线按钮
    void onFinishNavigation();      // 完成导航按钮

    // ========== 日记相关 ==========
    void onCategoryChanged();       // 分类按钮切换
    void onTextChanged();           // 日记文本变化（字数统计）
    void onSaveDraft();             // 保存草稿
    void onAttachPhoto();           // 添加配图
    void onLoadDraft(int index);    // 加载草稿
    void onPublishDraft(int index); // 发布草稿
    void onDeleteDraft(int index);  // 删除草稿
    void onDeletePublished(int index); // 删除已发布日记
    void onViewTimeline(int activeIndex = 0); // 查看已发布日记时间线

private:
    // ========== UI 构建 ==========
    void setupUI();
    QWidget* createHeaderPanel();
    QWidget* createNavigationPanel();
    QWidget* createDiaryPanel();
    QWidget* createDraftListPanel();
    QWidget* createPublishedPanel();

    // ========== 辅助方法 ==========
    void populateSelectors();       // 填充起点/终点下拉框
    void updateStats();             // 更新统计信息
    void updateWordCount();         // 更新日记字数
    void refreshDraftList();        // 刷新草稿列表
    void refreshPublishedList();    // 刷新已发布列表
    void syncTrackToDiary();        // 将规划路径同步到日记轨迹
    void applyGlobalStylesheet();   // 应用全局样式

    // ========== 核心数据 ==========
    CampusGraph    m_graph;         // 校园图
    DiaryManager   m_diaryManager;  // 日记管理器
    DiaryEntry     m_currentDiary;  // 当前正在编辑的日记

    // ========== 导航 UI 元素 ==========
    QLabel*        m_statsLabel;
    QComboBox*     m_startSelect;
    QComboBox*     m_endSelect;
    QPushButton*   m_planBtn;
    QPushButton*   m_finishBtn;
    MapWidget*     m_mapWidget;
    QLabel*        m_distanceLabel;
    QLabel*        m_pathDisplay;

    // ========== 日记 UI 元素 ==========
    QVector<QPushButton*> m_categoryBtns;
    QTextEdit*     m_diaryTextarea;
    QLabel*        m_diaryStatus;
    QLabel*        m_diarySummary;
    QPushButton*   m_saveDraftBtn;
    QPushButton*   m_attachPhotoBtn;

    // 草稿列表
    QWidget*       m_draftListPanel;
    QLabel*        m_draftCountBadge;
    QVBoxLayout*   m_draftListLayout;

    // 已发布列表
    QWidget*       m_publishedPanel;
    QLabel*        m_publishedCountBadge;
    QVBoxLayout*   m_publishedListLayout;

    // 滚动区域内的主容器
    QWidget*       m_scrollContent;
};

#endif // MAINWINDOW_H
