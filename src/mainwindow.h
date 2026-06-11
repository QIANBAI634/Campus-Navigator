/**
 * 主窗口 — 四标签页底部导航布局
 * 类似微信UI: 导航 | 推荐 | 查询 | 日记
 */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QScrollArea>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QVector>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <QDialog>
#include <QDialogButtonBox>
#include <QButtonGroup>
#include <QEvent>
#include <QLineEdit>
#include <QListWidget>

#include "graph.h"
#include "diarymanager.h"
#include "mapwidget.h"
#include "campusdatabase.h"
#include "usermanager.h"
#include "fooddatabase.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    // ========== 标签切换 ==========
    void switchTab(int index);

    // ========== 导航相关 ==========
    void onPlanRoute();
    void onFinishNavigation();
    void onCampusChanged(int index);
    void onUserChanged(int index);
    void onPlanMultiStop();
    void onAddStop();
    void onClearStops();

    // ========== 场所查询 ==========
    void onQueryNearbyFacilities();

    // ========== 旅游推荐 ==========
    void onRecommendTop();
    void onSearchCampus();

    // ========== 美食推荐 ==========
    void onFoodTopK();               // 美食 Top-10
    void onFoodSearch();             // 美食搜索
    void onFoodRate(int index, int score); // 美食评分
    void onFoodAdd();                // 添加菜品

    // ========== 日记相关 ==========
    void onCategoryChanged();
    void onTextChanged();
    void onSaveDraft();
    void onAttachPhoto();
    void onLoadDraft(int index);
    void onPublishDraft(int index);
    void onDeleteDraft(int index);
    void onDeletePublished(int index);
    void onViewTimeline(int activeIndex = 0);
    void showDiaryDetail(int index);
    void onRecommendDiaries();          // 日记 Top-K
    void onSearchDiaryTitle();          // 精确查标题
    void onSearchDiaryDest();           // 按目的地查

private:
    // ========== UI 构建 ==========
    void setupUI();
    QWidget* createTopBar();
    QWidget* createBottomBar();
    QWidget* createPageNavigation();
    QWidget* createPageRecommend();
    QWidget* createPageSearch();
    QWidget* createPageDiary();

    QWidget* createNavigationPanel();
    QWidget* createDiarySearchPanel();    // 日记搜索/推荐面板
    QWidget* createRecommendPanel();
    QWidget* createFoodPanel();          // 美食推荐面板
    QWidget* createNearbyFacilityPanel();
    QWidget* createDiaryPanel();
    QWidget* createDraftListPanel();
    QWidget* createPublishedPanel();

    // ========== 辅助方法 ==========
    void populateSelectors();
    void updateStats();
    void updateWordCount();
    void refreshDraftList();
    void refreshPublishedList();
    void syncTrackToDiary();
    void applyGlobalStylesheet();
    void updateTabStyle(int active);

    // ========== 核心数据 ==========
    CampusGraph    m_graph;
    DiaryManager   m_diaryManager;
    DiaryEntry     m_currentDiary;
    UserManager    m_userManager;

    // ========== 顶栏 UI ==========
    QComboBox*     m_campusSelect;
    QComboBox*     m_userSelect;
    QLabel*        m_statsLabel = nullptr;

    // ========== QStackedWidget + 页面 ==========
    QStackedWidget* m_stack;

    // ========== 底部标签按钮 ==========
    QPushButton*   m_tabNav;
    QPushButton*   m_tabRec;
    QPushButton*   m_tabSearch;
    QPushButton*   m_tabDiary;

    // ========== 导航 UI 元素 ==========
    QComboBox*     m_startSelect;
    QComboBox*     m_endSelect;
    QComboBox*     m_strategySelect;
    QPushButton*   m_planBtn;
    QPushButton*   m_finishBtn;
    QPushButton*   m_addStopBtn;
    QPushButton*   m_multiPlanBtn;
    QPushButton*   m_clearStopBtn;
    QListWidget*   m_stopList;
    QVector<int>   m_stopIndices;
    MapWidget*     m_mapWidget;
    MapWidget*     m_searchMapWidget = nullptr;  // 查询页独立地图
    QLabel*        m_distanceLabel;
    QLabel*        m_pathDisplay;

    // ========== 旅游推荐 UI 元素 ==========
    QComboBox*     m_recSortSelect;
    QComboBox*     m_recTypeSelect;
    QLineEdit*     m_recSearchInput;
    QPushButton*   m_recBtn;
    QPushButton*   m_recSearchBtn;
    QLabel*        m_recResultLabel;
    QVBoxLayout*   m_recResultLayout;

    // ========== 美食推荐 UI 元素 ==========
    QComboBox*     m_foodSortSelect = nullptr;
    QComboBox*     m_foodCuisineSelect = nullptr;
    QLineEdit*     m_foodSearchInput = nullptr;
    QPushButton*   m_foodTopKBtn = nullptr;
    QPushButton*   m_foodAddBtn = nullptr;
    QLabel*        m_foodResultLabel = nullptr;
    QVBoxLayout*   m_foodResultLayout = nullptr;

    // ========== 场所查询 UI 元素 ==========
    QComboBox*     m_facilityCenterSelect;
    QComboBox*     m_facilityRangeSelect;
    QComboBox*     m_facilityCategorySelect;
    QPushButton*   m_queryNearbyBtn;
    QLabel*        m_nearbyResultLabel;
    QVBoxLayout*   m_nearbyResultLayout;
    int            m_activeCenterIdx;

    // ========== 日记 UI 元素 ==========
    QVector<QPushButton*> m_categoryBtns;
    QTextEdit*     m_diaryTextarea;
    QLabel*        m_diaryStatus;
    QLabel*        m_diarySummary;
    QPushButton*   m_saveDraftBtn;
    QPushButton*   m_attachPhotoBtn;
    // 日记搜索/推荐
    QPushButton*   m_diaryRecBtn = nullptr;
    QPushButton*   m_diarySearchDestBtn = nullptr;
    QPushButton*   m_diarySearchTitleBtn = nullptr;
    QLineEdit*     m_diarySearchDestInput = nullptr;
    QLineEdit*     m_diarySearchTitleInput = nullptr;
    QLabel*        m_diarySearchResultLabel = nullptr;
    QVBoxLayout*   m_diarySearchResultLayout = nullptr;

    QWidget*       m_draftListPanel;
    QLabel*        m_draftCountBadge;
    QVBoxLayout*   m_draftListLayout;

    QWidget*       m_publishedPanel;
    QLabel*        m_publishedCountBadge;
    QVBoxLayout*   m_publishedListLayout;
};

#endif // MAINWINDOW_H
