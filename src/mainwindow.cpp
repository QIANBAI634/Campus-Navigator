/**
 * 主窗口实现
 * 包含完整的导航交互和旅行日记管理界面
 */
#include "mainwindow.h"

#include <QApplication>
#include <QScreen>
#include <QScrollBar>
#include <QFileInfo>
#include <QPixmap>
#include <QFrame>
#include <QButtonGroup>
#include <QGridLayout>
#include <QMouseEvent>

// ============================================================
// 构造函数 & 析构函数
// ============================================================

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 设置窗口属性
    setWindowTitle("北邮沙河 · 智行导航  |  校园导航系统");
    resize(720, 860);

    // 居中显示（确保不超出屏幕顶部）
    if (auto* screen = QGuiApplication::primaryScreen()) {
        QRect sg = screen->availableGeometry();
        int x = qMax(sg.x(), sg.x() + (sg.width()  - width())  / 2);
        int y = qMax(sg.y(), sg.y() + (sg.height() - height()) / 2);
        move(x, y);
    }

    // 设置数据存储路径
    m_diaryManager.setStoragePath(QApplication::applicationDirPath() + "/data");

    // 初始化当前日记
    m_currentDiary.id = QDateTime::currentMSecsSinceEpoch();
    m_currentDiary.category = DiaryCategory::TRAVEL_NOTE;
    m_currentDiary.userId = "current_user";
    m_activeCenterIdx = -1;

    // 构建 UI
    applyGlobalStylesheet();
    setupUI();
    populateSelectors();
    updateStats();
    updateWordCount();
    refreshDraftList();
    refreshPublishedList();   // 启动时加载已发布日记列表
}

MainWindow::~MainWindow() {}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    // 朋友圈时间线中点击图片 → 打开大图预览
    if (event->type() == QEvent::MouseButtonPress) {
        QLabel *label = qobject_cast<QLabel*>(obj);
        if (label && label->property("photoPath").isValid()) {
            QString photoPath = label->property("photoPath").toString();
            QPixmap pm(photoPath);
            if (!pm.isNull()) {
                QDialog *viewer = new QDialog(this);
                viewer->setWindowTitle("🖼️ 图片预览");
                viewer->setStyleSheet("QDialog { background: black; }");
                viewer->resize(800, 600);

                QVBoxLayout *vl = new QVBoxLayout(viewer);
                vl->setContentsMargins(0, 0, 0, 0);

                QLabel *imgLabel = new QLabel();
                imgLabel->setPixmap(pm.scaled(780, 580,
                    Qt::KeepAspectRatio, Qt::SmoothTransformation));
                imgLabel->setAlignment(Qt::AlignCenter);
                imgLabel->setStyleSheet("background: black;");

                // 点击任意位置关闭
                imgLabel->setCursor(Qt::PointingHandCursor);
                imgLabel->installEventFilter(viewer);
                imgLabel->setProperty("closeOnClick", true);

                vl->addWidget(imgLabel);
                viewer->exec();
                viewer->deleteLater();
            }
            return true;
        }
        // 大图预览中点击关闭
        if (label && label->property("closeOnClick").isValid()) {
            if (QDialog *dlg = qobject_cast<QDialog*>(label->parent())) {
                dlg->accept();
            }
            return true;
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

// ============================================================
// 全局样式表
// ============================================================

void MainWindow::applyGlobalStylesheet()
{
    setStyleSheet(QStringLiteral(
        "QMainWindow {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "    stop:0 #e9f0f5, stop:1 #d9e2ec);"
        "}"
        "QScrollArea {"
        "  border: none;"
        "  background: transparent;"
        "}"
        "QComboBox {"
        "  background: white;"
        "  border: 1px solid #cbdde6;"
        "  border-radius: 20px;"
        "  padding: 10px 16px;"
        "  font-size: 14px;"
        "  font-weight: 500;"
        "  color: #0a2b3e;"
        "  min-width: 160px;"
        "}"
        "QComboBox:hover { border-color: #2c7da0; }"
        "QComboBox:focus { border-color: #0f5b7a; }"
        "QComboBox::drop-down {"
        "  border: none;"
        "  width: 30px;"
        "}"
        "QComboBox QAbstractItemView {"
        "  background: white;"
        "  border: 1px solid #cbdde6;"
        "  border-radius: 8px;"
        "  selection-background-color: #e9f2f5;"
        "  selection-color: #0a2b3e;"
        "}"
        "QPushButton {"
        "  border-radius: 20px;"
        "  padding: 10px 18px;"
        "  font-size: 13px;"
        "  font-weight: 600;"
        "}"
        "QTextEdit {"
        "  border: 1px solid #cbdde6;"
        "  border-radius: 16px;"
        "  padding: 12px 14px;"
        "  font-size: 14px;"
        "  color: #1f2f38;"
        "  background: white;"
        "}"
        "QTextEdit:focus {"
        "  border-color: #0f5b7a;"
        "}"
        "QLabel#heroTitle {"
        "  font-size: 22px;"
        "  font-weight: 700;"
        "  color: white;"
        "}"
        "QLabel#heroSubtitle {"
        "  font-size: 13px;"
        "  color: rgba(255,255,255,0.8);"
        "}"
        // 下拉框滚动条白底
        "QScrollBar:vertical {"
        "  background: #f0f4f9;"
        "  width: 10px;"
        "  border-radius: 5px;"
        "}"
        "QScrollBar::handle:vertical {"
        "  background: #cbdde6;"
        "  border-radius: 5px;"
        "  min-height: 20px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "  background: #a0b8c8;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "  height: 0px;"
        "}"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
        "  background: none;"
        "}"
    ));
}

// ============================================================
// UI 构建
// ============================================================

void MainWindow::setupUI()
{
    // 中央容器
    QWidget* central = new QWidget(this);
    central->setStyleSheet("background: #d9e2ec;");
    QVBoxLayout* mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 顶栏
    mainLayout->addWidget(createTopBar());

    // QStackedWidget — 4 个页面
    m_stack = new QStackedWidget();
    m_stack->setStyleSheet("QStackedWidget { background: transparent; }");
    m_stack->addWidget(createPageNavigation());  // index 0
    m_stack->addWidget(createPageRecommend());    // index 1
    m_stack->addWidget(createPageSearch());       // index 2
    m_stack->addWidget(createPageDiary());        // index 3
    mainLayout->addWidget(m_stack, 1);  // stretch=1 填满剩余空间

    // 底部标签栏
    mainLayout->addWidget(createBottomBar());

    setCentralWidget(central);

    // 默认选中导航
    switchTab(0);
}

// ============================================================
// 顶栏（精简版）
// ============================================================

QWidget* MainWindow::createTopBar()
{
    QWidget* bar = new QWidget();
    bar->setFixedHeight(52);
    bar->setStyleSheet(
        "background: #0a2b3e;");

    QHBoxLayout* layout = new QHBoxLayout(bar);
    layout->setContentsMargins(16, 4, 16, 4);
    layout->setSpacing(10);

    // 标题
    QLabel* title = new QLabel("📍 校园导航");
    title->setStyleSheet(
        "font-size:15px; font-weight:700; color: white; background:transparent;");
    layout->addWidget(title);

    layout->addStretch();

    // 景区选择
    QLabel* campusLabel = new QLabel("🏛️");
    campusLabel->setStyleSheet("color: rgba(255,255,255,0.8); font-size:13px;");
    layout->addWidget(campusLabel);

    m_campusSelect = new QComboBox();
    m_campusSelect->setMinimumWidth(160);
    m_campusSelect->setStyleSheet(
        "QComboBox { background: white; border-radius: 12px;"
        "padding: 4px 10px; font-size: 11px; color: #1f2f38; }"
        "QComboBox:hover { background: #f0f4f9; }"
        "QComboBox QAbstractItemView {"
        "  background: white; color: #1f2f38; max-height: 300px;"
        "  selection-background-color: #e9f2f5; selection-color: #0a2b3e; }");
    const auto& campuses = getAllCampuses();
    for (int i = 0; i < campuses.size(); ++i)
        m_campusSelect->addItem(campuses[i].name, i);
    connect(m_campusSelect, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onCampusChanged);
    layout->addWidget(m_campusSelect, 1);

    // 用户切换
    QLabel* userLabel = new QLabel("👤");
    userLabel->setStyleSheet("color: rgba(255,255,255,0.8); font-size:13px;");
    layout->addWidget(userLabel);

    m_userSelect = new QComboBox();
    m_userSelect->setMaximumWidth(110);
    m_userSelect->setStyleSheet(
        "QComboBox { background: white; border-radius: 12px;"
        "padding: 4px 8px; font-size: 11px; color: #1f2f38; }"
        "QComboBox:hover { background: #f0f4f9; }"
        "QComboBox QAbstractItemView {"
        "  background: white; color: #1f2f38;"
        "  selection-background-color: #e9f2f5; selection-color: #0a2b3e; }");
    for (const auto& u : m_userManager.allUsers())
        m_userSelect->addItem(u.avatar + " " + u.nickname, u.userId);
    connect(m_userSelect, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onUserChanged);
    layout->addWidget(m_userSelect);

    return bar;
}

// ============================================================
// 底部标签栏
// ============================================================

QWidget* MainWindow::createBottomBar()
{
    QWidget* bar = new QWidget();
    bar->setFixedHeight(60);
    bar->setStyleSheet("background: white; border-top: 1px solid #e0e0e0;");

    QHBoxLayout* layout = new QHBoxLayout(bar);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    struct TabDef { QString icon; QString text; QPushButton*& btn; };
    TabDef tabs[] = {
        {"🧭", "导航",   m_tabNav},
        {"🎯", "推荐",   m_tabRec},
        {"🔍", "查询",   m_tabSearch},
        {"📝", "日记",   m_tabDiary},
    };

    for (int i = 0; i < 4; ++i) {
        QPushButton* btn = new QPushButton(
            QString("%1\n%2").arg(tabs[i].icon).arg(tabs[i].text));
        btn->setCheckable(true);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(
            "QPushButton {"
            "  background: white; border: none; border-top: 3px solid transparent;"
            "  font-size: 10px; color: #999; padding: 6px 0 4px 0;"
            "}"
            "QPushButton:hover { color: #1f6d49; }"
            "QPushButton:checked {"
            "  color: #1f6d49; font-weight: 700;"
            "  border-top: 3px solid #1f6d49;"
            "}");
        tabs[i].btn = btn;

        int idx = i;
        connect(btn, &QPushButton::clicked, this, [this, idx]() {
            switchTab(idx);
        });

        layout->addWidget(btn);
    }

    return bar;
}

// ============================================================
// 4个页面（各自包裹在 QScrollArea 中）
// ============================================================

QWidget* MainWindow::createPageNavigation()
{
    QScrollArea* sa = new QScrollArea();
    sa->setWidgetResizable(true);
    sa->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    sa->setFrameShape(QFrame::NoFrame);
    sa->setStyleSheet(
        "QScrollArea { border: none; background: transparent; }"
        "QScrollBar:vertical { background: #f0f4f9; width: 8px; border-radius: 4px; }"
        "QScrollBar::handle:vertical { background: #cbdde6; border-radius: 4px; min-height: 20px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }");

    QWidget* page = new QWidget();
    page->setStyleSheet("background: #d9e2ec;");
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(16, 8, 16, 8);
    layout->addWidget(createNavigationPanel());
    layout->addStretch();

    sa->setWidget(page);
    return sa;
}

QWidget* MainWindow::createPageRecommend()
{
    QScrollArea* sa = new QScrollArea();
    sa->setWidgetResizable(true);
    sa->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    sa->setFrameShape(QFrame::NoFrame);
    sa->setStyleSheet(
        "QScrollArea { border: none; background: transparent; }"
        "QScrollBar:vertical { background: #f0f4f9; width: 8px; border-radius: 4px; }"
        "QScrollBar::handle:vertical { background: #cbdde6; border-radius: 4px; min-height: 20px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }");

    QWidget* page = new QWidget();
    page->setStyleSheet("background: #d9e2ec;");
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(16, 8, 16, 8);
    layout->addWidget(createRecommendPanel());
    layout->addStretch();

    sa->setWidget(page);
    return sa;
}

QWidget* MainWindow::createPageSearch()
{
    QScrollArea* sa = new QScrollArea();
    sa->setWidgetResizable(true);
    sa->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    sa->setFrameShape(QFrame::NoFrame);
    sa->setStyleSheet(
        "QScrollArea { border: none; background: transparent; }"
        "QScrollBar:vertical { background: #f0f4f9; width: 8px; border-radius: 4px; }"
        "QScrollBar::handle:vertical { background: #cbdde6; border-radius: 4px; min-height: 20px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }");

    QWidget* page = new QWidget();
    page->setStyleSheet("background: #d9e2ec;");
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(16, 8, 16, 8);
    layout->addWidget(createNearbyFacilityPanel());
    layout->addStretch();

    sa->setWidget(page);
    return sa;
}

QWidget* MainWindow::createPageDiary()
{
    QScrollArea* sa = new QScrollArea();
    sa->setWidgetResizable(true);
    sa->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    sa->setFrameShape(QFrame::NoFrame);
    sa->setStyleSheet(
        "QScrollArea { border: none; background: transparent; }"
        "QScrollBar:vertical { background: #f0f4f9; width: 8px; border-radius: 4px; }"
        "QScrollBar::handle:vertical { background: #cbdde6; border-radius: 4px; min-height: 20px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }");

    QWidget* page = new QWidget();
    page->setStyleSheet("background: #d9e2ec;");
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(16, 8, 16, 8);
    layout->addWidget(createDiaryPanel());
    layout->addWidget(createDraftListPanel());
    layout->addWidget(createPublishedPanel());
    layout->addStretch();

    sa->setWidget(page);
    return sa;
}

// ============================================================
// 标签切换
// ============================================================

void MainWindow::switchTab(int index)
{
    m_stack->setCurrentIndex(index);
    updateTabStyle(index);
}

void MainWindow::updateTabStyle(int active)
{
    QPushButton* btns[] = {m_tabNav, m_tabRec, m_tabSearch, m_tabDiary};
    for (int i = 0; i < 4; ++i) {
        btns[i]->setChecked(i == active);
    }
}

// ============================================================
// ============================================================
// 导航面板
// ============================================================

QWidget* MainWindow::createNavigationPanel()
{
    QWidget* panel = new QWidget();
    panel->setStyleSheet("padding: 24px 28px;");

    QVBoxLayout* layout = new QVBoxLayout(panel);
    layout->setSpacing(16);

    // ---- 起点选择器 ----
    QWidget* startBox = new QWidget();
    startBox->setStyleSheet(
        "background: #f8fafc;"
        "border-radius: 24px;"
        "padding: 8px 16px;"
        "border: 1px solid #e2edf2;"
    );
    QHBoxLayout* startLayout = new QHBoxLayout(startBox);
    QLabel* startLabel = new QLabel("🚩 起点");
    startLabel->setStyleSheet(
        "font-weight:600; color:#1e4a6b; background:#e9f2f5;"
        "border-radius:20px; padding:6px 16px; font-size:13px;");
    m_startSelect = new QComboBox();
    m_startSelect->setMinimumWidth(200);
    startLayout->addWidget(startLabel);
    startLayout->addWidget(m_startSelect, 1);
    layout->addWidget(startBox);

    // ---- 终点选择器 ----
    QWidget* endBox = new QWidget();
    endBox->setStyleSheet(
        "background: #f8fafc;"
        "border-radius: 24px;"
        "padding: 8px 16px;"
        "border: 1px solid #e2edf2;"
    );
    QHBoxLayout* endLayout = new QHBoxLayout(endBox);
    QLabel* endLabel = new QLabel("🏁 终点");
    endLabel->setStyleSheet(
        "font-weight:600; color:#1e4a6b; background:#e9f2f5;"
        "border-radius:20px; padding:6px 16px; font-size:13px;");
    m_endSelect = new QComboBox();
    m_endSelect->setMinimumWidth(200);
    endLayout->addWidget(endLabel);
    endLayout->addWidget(m_endSelect, 1);
    layout->addWidget(endBox);

    // ---- 路线策略选择器 ----
    QWidget* strategyBox = new QWidget();
    strategyBox->setStyleSheet(
        "background: #f8fafc; border-radius: 24px; padding: 8px 16px;"
        "border: 1px solid #e2edf2;");
    QHBoxLayout* strategyLayout = new QHBoxLayout(strategyBox);

    QLabel* strategyLabel = new QLabel("🧭 策略");
    strategyLabel->setStyleSheet(
        "font-weight:600; color:#1e4a6b; background:#e9f2f5;"
        "border-radius:20px; padding:6px 16px; font-size:13px;");
    strategyLayout->addWidget(strategyLabel);

    m_strategySelect = new QComboBox();
    m_strategySelect->addItems({"最短距离", "最短时间(含拥挤度)", "仅自行车道", "电瓶车路线"});
    m_strategySelect->setStyleSheet(
        "QComboBox { background: white; border: 1px solid #cbdde6;"
        " border-radius: 20px; padding: 8px 14px; font-size: 13px; color: #1f2f38; }"
        "QComboBox:hover { border-color: #2c7da0; }"
        "QComboBox QAbstractItemView { background: white; color: #1f2f38;"
        " selection-background-color: #e9f2f5; selection-color: #0a2b3e; }");
    strategyLayout->addWidget(m_strategySelect, 1);

    // 途经点按钮（添加到途经点列表）
    m_addStopBtn = new QPushButton("📌 添加途经点");
    m_addStopBtn->setStyleSheet(
        "QPushButton { background: #e9f2f5; color: #1e4a6b; border: 1px solid #cbdde6;"
        " border-radius: 20px; padding: 8px 16px; font-size: 12px; font-weight:600; }"
        "QPushButton:hover { background: #d9e5f0; }");
    m_addStopBtn->setCursor(Qt::PointingHandCursor);
    connect(m_addStopBtn, &QPushButton::clicked, this, &MainWindow::onAddStop);
    strategyLayout->addWidget(m_addStopBtn);

    layout->addWidget(strategyBox);

    // 途经点列表
    m_stopList = new QListWidget();
    m_stopList->setMaximumHeight(72);
    m_stopList->setStyleSheet(
        "QListWidget { background: white; border: 1px solid #cbdde6;"
        " border-radius: 14px; padding: 4px 8px; font-size: 12px; color: #1e4a6b; }"
        "QListWidget::item { background: #e9f2f5; border-radius: 10px;"
        " margin: 2px 0; padding: 2px 8px; }");
    m_stopList->hide();
    layout->addWidget(m_stopList);

    // ---- 按钮行：规划 + 多点规划 + 完成导航 ----
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(12);

    m_planBtn = new QPushButton("✨ 规划最短路径 ✨");
    m_planBtn->setStyleSheet(
        "QPushButton {"
        "  background: #1f6d49;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 30px;"
        "  padding: 14px 24px;"
        "  font-size: 16px;"
        "  font-weight: 600;"
        "}"
        "QPushButton:hover { background: #0e5437; }"
        "QPushButton:pressed { background: #0a3d28; }"
    );
    m_planBtn->setCursor(Qt::PointingHandCursor);
    connect(m_planBtn, &QPushButton::clicked, this, &MainWindow::onPlanRoute);
    btnLayout->addWidget(m_planBtn, 3);

    m_finishBtn = new QPushButton("✅ 完成导航");
    m_finishBtn->setStyleSheet(
        "QPushButton {"
        "  background: #0f5b7a;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 30px;"
        "  padding: 14px 20px;"
        "  font-size: 14px;"
        "  font-weight: 600;"
        "}"
        "QPushButton:hover { background: #0a3d52; }"
        "QPushButton:pressed { background: #06293a; }"
    );
    m_finishBtn->setCursor(Qt::PointingHandCursor);
    m_finishBtn->setEnabled(false);
    connect(m_finishBtn, &QPushButton::clicked, this, &MainWindow::onFinishNavigation);
    btnLayout->addWidget(m_finishBtn, 1);

    layout->addLayout(btnLayout);

    // 途经点按钮行（在多主按钮下方）
    QHBoxLayout* multiRow = new QHBoxLayout();
    multiRow->setSpacing(8);

    m_multiPlanBtn = new QPushButton("🗺️ 规划多点路径");
    m_multiPlanBtn->setStyleSheet(
        "QPushButton { background: #f59e0b; color: white; border: none;"
        " border-radius: 24px; padding: 9px 16px; font-size: 12px; font-weight: 600; }"
        "QPushButton:hover { background: #d97706; }");
    m_multiPlanBtn->setCursor(Qt::PointingHandCursor);
    m_multiPlanBtn->setEnabled(false);
    connect(m_multiPlanBtn, &QPushButton::clicked, this, &MainWindow::onPlanMultiStop);
    multiRow->addWidget(m_multiPlanBtn);

    m_clearStopBtn = new QPushButton("清空途经点");
    m_clearStopBtn->setStyleSheet(
        "QPushButton { background: #fff2e6; color: #c2410c; border: 1px solid #fcd5ce;"
        " border-radius: 24px; padding: 9px 14px; font-size: 12px; font-weight:600; }"
        "QPushButton:hover { background: #ffe0cc; }");
    m_clearStopBtn->setCursor(Qt::PointingHandCursor);
    m_clearStopBtn->setEnabled(false);
    connect(m_clearStopBtn, &QPushButton::clicked, this, &MainWindow::onClearStops);
    multiRow->addWidget(m_clearStopBtn);
    multiRow->addStretch();

    layout->addLayout(multiRow);

    // ---- 地图组件 ----
    m_mapWidget = new MapWidget();
    m_mapWidget->setGraph(&m_graph);
    // 依次尝试多个路径加载地图图片
    QStringList mapPaths = {
        QApplication::applicationDirPath() + "/assets/北邮校园地图.jpg",
        QApplication::applicationDirPath() + "/../assets/北邮校园地图.jpg",
        QApplication::applicationDirPath() + "/../../assets/北邮校园地图.jpg",
    };
    for (const QString& path : mapPaths) {
        if (m_mapWidget->loadMap(path)) break;
    }
    m_mapWidget->placeLandmarkDots();
    layout->addWidget(m_mapWidget);

    // ---- "显示标签类型" 按钮 (地图右下角浮层风格) ----
    QWidget* legendBtnRow = new QWidget();
    legendBtnRow->setStyleSheet("background: transparent;");
    QHBoxLayout* legendBtnLayout = new QHBoxLayout(legendBtnRow);
    legendBtnLayout->setContentsMargins(0, 4, 0, 4);
    legendBtnLayout->addStretch();
    QPushButton* legendBtn = new QPushButton("🏷️ 显示标签类型");
    legendBtn->setStyleSheet(
        "QPushButton {"
        "  background: white; border: 1px solid #cbdde6; border-radius: 16px;"
        "  padding: 6px 14px; font-size: 12px; color: #1e4a6b; font-weight: 500; }"
        "QPushButton:hover { background: #e9f2f5; border-color: #2c7da0; }");
    legendBtn->setCursor(Qt::PointingHandCursor);
    // 点击弹窗
    connect(legendBtn, &QPushButton::clicked, this, [this]() {
        QDialog* dlg = new QDialog(this);
        dlg->setWindowTitle("🏷️ 服务设施标签类型");
        dlg->resize(420, 360);
        dlg->setStyleSheet("QDialog { background: #f8fafc; }");
        QVBoxLayout* dl = new QVBoxLayout(dlg);
        dl->setSpacing(10);
        QLabel* dlTitle = new QLabel("地图上的彩色圆点对应以下设施类型：");
        dlTitle->setStyleSheet("font-size:13px; color:#0f5b7a; font-weight:600; padding:4px 0;");
        dl->addWidget(dlTitle);
        QGridLayout* grid = new QGridLayout();
        grid->setSpacing(8);
        QVector<QPair<QString, QPair<QString, QColor>>> facilityList = {
            {"出入口", {"🏠", QColor(50,180,50)}},   {"餐饮", {"🍽️", QColor(240,150,30)}},
            {"购物", {"🛒", QColor(255,140,0)}},    {"住宿", {"🛏️", QColor(100,100,220)}},
            {"教学", {"📖", QColor(30,130,210)}},   {"行政", {"🏛️", QColor(140,100,80)}},
            {"运动", {"⚽", QColor(50,200,200)}},   {"医疗", {"🏥", QColor(220,50,50)}},
            {"学习", {"📚", QColor(160,100,200)}},  {"休闲", {"☕", QColor(250,180,220)}},
            {"卫生", {"🚻", QColor(100,180,220)}},  {"金融", {"💰", QColor(220,200,30)}},
            {"邮政", {"📮", QColor(180,140,80)}},
        };
        for (int i = 0; i < facilityList.size(); ++i) {
            QWidget* item = new QWidget();
            item->setStyleSheet("background: white; border-radius: 10px; padding: 4px;");
            QHBoxLayout* il = new QHBoxLayout(item);
            il->setContentsMargins(8, 6, 8, 6);
            // 颜色圆点
            QLabel* dot = new QLabel("●");
            dot->setStyleSheet(QString("color: %1; font-size: 20px;").arg(facilityList[i].second.second.name()));
            il->addWidget(dot);
            // emoji + 名称
            QLabel* name = new QLabel(QString("%1 %2").arg(facilityList[i].second.first, facilityList[i].first));
            name->setStyleSheet("font-size: 14px; color:#1f2f38; font-weight:500;");
            il->addWidget(name);
            il->addStretch();
            int row = i / 3, col = i % 3;
            grid->addWidget(item, row, col);
        }
        dl->addLayout(grid);
        QPushButton* closeBtn = new QPushButton("关闭");
        closeBtn->setStyleSheet(
            "QPushButton { background: #1f6d49; color: white; border-radius: 16px;"
            "padding: 8px 24px; font-size: 13px; }"
            "QPushButton:hover { background: #0e5437; }");
        connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);
        dl->addWidget(closeBtn, 0, Qt::AlignCenter);
        dlg->exec();
        dlg->deleteLater();
    });
    legendBtnLayout->addWidget(legendBtn);
    layout->addWidget(legendBtnRow);

    // ---- 结果显示区域 ----
    QWidget* resultBox = new QWidget();
    resultBox->setStyleSheet(
        "background: #f0f4f9;"
        "border-radius: 24px;"
        "padding: 16px 20px;"
        "border: 1px solid #dce5ec;"
    );
    QVBoxLayout* resultLayout = new QVBoxLayout(resultBox);

    m_distanceLabel = new QLabel("📏 路径距离");
    m_distanceLabel->setStyleSheet(
        "background: #1e4a6b; color: white;"
        "border-radius: 20px; padding: 5px 16px;"
        "font-size: 12px; font-weight: 500;");
    resultLayout->addWidget(m_distanceLabel);

    QLabel* pathTitle = new QLabel("🗺️ 推荐路线 (仅显示主要建筑)");
    pathTitle->setStyleSheet(
        "font-weight:700; color:#0f5b7a; font-size:13px; margin-top:8px;");
    resultLayout->addWidget(pathTitle);

    m_pathDisplay = new QLabel("✨ 选择起点和终点，点击规划路线 ✨");
    m_pathDisplay->setWordWrap(true);
    m_pathDisplay->setStyleSheet(
        "background: #fefce8;"
        "border-left: 4px solid #1f6d49;"
        "border-radius: 14px;"
        "padding: 12px 16px;"
        "font-size: 13px;"
        "color: #1f2f38;"
    );
    resultLayout->addWidget(m_pathDisplay);

    layout->addWidget(resultBox);

    return panel;
}

// ============================================================
// 堆 Top-K 选择（核心算法）
// 时间复杂度 O(n·logK)，不用全排序即可得到前 K 个结果
// ============================================================

namespace {

// 小顶堆 siftDown，用于 Top-K 维护
void heapSiftDown(QVector<CampusInfo>& heap, int i, int heapSize,
                  bool sortByHeat)  // true=按热度, false=按评分
{
    while (true) {
        int left  = 2 * i + 1;
        int right = 2 * i + 2;
        int smallest = i;
        auto key = [sortByHeat](const CampusInfo& c) {
            return sortByHeat ? c.heat : c.rating;
        };
        if (left  < heapSize && key(heap[left])  < key(heap[smallest])) smallest = left;
        if (right < heapSize && key(heap[right]) < key(heap[smallest])) smallest = right;
        if (smallest == i) break;
        std::swap(heap[i], heap[smallest]);
        i = smallest;
    }
}

// 用小顶堆选 Top-K（不经过全排序）
QVector<CampusInfo> topKCampuses(const QVector<CampusInfo>& src, int k,
                                  bool sortByHeat,
                                  const QString& typeFilter,
                                  const QString& keyword)
{
    // 1. 先筛选
    QVector<CampusInfo> filtered;
    for (const auto& c : src) {
        if (!typeFilter.isEmpty() && c.type != typeFilter) continue;
        if (!keyword.isEmpty()) {
            QString hay = c.name + "," + c.type + "," + c.city + "," + c.district + "," + c.tags;
            if (!hay.contains(keyword, Qt::CaseInsensitive)) continue;
        }
        filtered.append(c);
    }
    if (filtered.isEmpty()) return {};

    // 2. Top-K 小顶堆
    auto key = [sortByHeat](const CampusInfo& c) {
        return sortByHeat ? c.heat : c.rating;
    };
    k = qMin(k, filtered.size());

    QVector<CampusInfo> heap;
    for (int i = 0; i < k; ++i) heap.append(filtered[i]);
    // 建堆 O(k)
    for (int i = k / 2 - 1; i >= 0; --i)
        heapSiftDown(heap, i, k, sortByHeat);

    // 扫描剩余元素 O((n-k)·logk)
    for (int i = k; i < filtered.size(); ++i) {
        if (key(filtered[i]) > key(heap[0])) {
            heap[0] = filtered[i];
            heapSiftDown(heap, 0, k, sortByHeat);
        }
    }

    // 3. 堆排序提取，降序排列 O(k·logk)
    for (int i = k - 1; i >= 1; --i) {
        std::swap(heap[0], heap[i]);
        heapSiftDown(heap, 0, i, sortByHeat);
    }
    // 现在 heap 是从最小到最大，反转
    std::reverse(heap.begin(), heap.end());
    return heap;
}

} // anonymous namespace

// ============================================================
// 旅游推荐面板
// ============================================================

QWidget* MainWindow::createRecommendPanel()
{
    QWidget* panel = new QWidget();
    panel->setStyleSheet(
        "background: #f8fafc;"
        "border-radius: 24px;"
        "padding: 20px;"
        "margin: 12px 28px;"
        "border: 1px solid #e2edf2;"
    );
    QVBoxLayout* layout = new QVBoxLayout(panel);
    layout->setSpacing(12);

    // 标题
    QLabel* title = new QLabel("🎯 旅游推荐");
    title->setStyleSheet("font-size:16px; font-weight:700; color:#0f5b7a; background:transparent;");
    layout->addWidget(title);

    // 第一行：排序方式 + 类别过滤
    QHBoxLayout* row1 = new QHBoxLayout();
    row1->setSpacing(12);

    QLabel* sortLabel = new QLabel("排序");
    sortLabel->setStyleSheet(
        "font-weight:600; color:#1e4a6b; background:#e9f2f5;"
        "border-radius:20px; padding:6px 14px; font-size:12px;");
    row1->addWidget(sortLabel);

    m_recSortSelect = new QComboBox();
    m_recSortSelect->addItems({"按热度", "按评分"});
    m_recSortSelect->setMaximumWidth(110);
    row1->addWidget(m_recSortSelect);

    QLabel* typeLabel = new QLabel("类别");
    typeLabel->setStyleSheet(
        "font-weight:600; color:#1e4a6b; background:#e9f2f5;"
        "border-radius:20px; padding:6px 14px; font-size:12px;");
    row1->addWidget(typeLabel);

    m_recTypeSelect = new QComboBox();
    m_recTypeSelect->addItem("全部");
    m_recTypeSelect->addItems({
        "985高校", "211高校", "景点", "公园", "医院", "商场", "文化场所", "交通枢纽"
    });
    m_recTypeSelect->setMaximumWidth(130);
    row1->addWidget(m_recTypeSelect);

    row1->addStretch();

	    // 推荐 Top-10 按钮
	    m_recBtn = new QPushButton("🏆 Top-10 榜");
	    m_recBtn->setMinimumWidth(140);
	    m_recBtn->setStyleSheet(
	        "QPushButton {"
	        "  background: #1f6d49; color: white; border: none;"
	        "  border-radius: 20px; padding: 10px 16px;"
	        "  font-size: 13px; font-weight: 600; }"
	        "QPushButton:hover { background: #0e5437; }");
    m_recBtn->setCursor(Qt::PointingHandCursor);
    connect(m_recBtn, &QPushButton::clicked, this, &MainWindow::onRecommendTop);
    row1->addWidget(m_recBtn);

    layout->addLayout(row1);

    // 第二行：搜索
    QHBoxLayout* row2 = new QHBoxLayout();
    row2->setSpacing(12);

    QLabel* searchLabel = new QLabel("🔎 搜索");
    searchLabel->setStyleSheet(
        "font-weight:600; color:#1e4a6b; background:#e9f2f5;"
        "border-radius:20px; padding:6px 14px; font-size:12px;");
    row2->addWidget(searchLabel);

    m_recSearchInput = new QLineEdit();
    m_recSearchInput->setPlaceholderText("输入名称 / 类别 / 标签关键字...");
    m_recSearchInput->setStyleSheet(
        "QLineEdit { background: white; border: 1px solid #cbdde6;"
        "border-radius: 20px; padding: 8px 16px; font-size: 13px; color: #1f2f38; }"
        "QLineEdit:focus { border-color: #0f5b7a; }");
    row2->addWidget(m_recSearchInput, 1);

    m_recSearchBtn = new QPushButton("搜索");
    m_recSearchBtn->setStyleSheet(
        "QPushButton {"
        "  background: #0f5b7a; color: white; border: none;"
        "  border-radius: 20px; padding: 8px 18px;"
        "  font-size: 13px; font-weight: 600; }"
        "QPushButton:hover { background: #0a3d52; }");
    m_recSearchBtn->setCursor(Qt::PointingHandCursor);
    connect(m_recSearchBtn, &QPushButton::clicked, this, &MainWindow::onSearchCampus);
    row2->addWidget(m_recSearchBtn);

    layout->addLayout(row2);

    // 结果标题
    m_recResultLabel = new QLabel("💡 点击「Top-10 推荐」查看热门景区，或输入关键字搜索");
    m_recResultLabel->setWordWrap(true);
    m_recResultLabel->setStyleSheet(
        "background: #fefce8; border-left: 4px solid #1f6d49;"
        "border-radius: 14px; padding: 10px 16px; font-size: 13px; color: #1f2f38;");
    layout->addWidget(m_recResultLabel);

    // 结果列表容器
    QWidget* resultContainer = new QWidget();
    resultContainer->setStyleSheet("background: transparent;");
    m_recResultLayout = new QVBoxLayout(resultContainer);
    m_recResultLayout->setContentsMargins(0, 0, 0, 0);
    m_recResultLayout->setSpacing(6);
    layout->addWidget(resultContainer);

    return panel;
}

// ============================================================
// 旅游推荐槽函数
// ============================================================

void MainWindow::onRecommendTop()
{
    onSearchCampus();  // 推荐 = 无关键字的搜索 + Top-10
}

void MainWindow::onSearchCampus()
{
    // 清空旧结果
    QLayoutItem* child;
    while ((child = m_recResultLayout->takeAt(0)) != nullptr) {
        if (child->widget()) child->widget()->deleteLater();
        delete child;
    }

    const auto& campuses = getAllCampuses();
    if (campuses.isEmpty()) return;

    bool byHeat  = (m_recSortSelect->currentText() == "按热度");
    QString type = m_recTypeSelect->currentText();
    if (type == "全部") type.clear();
    QString kw   = m_recSearchInput->text().trimmed();

    // 堆 Top-10 选择（不经过全排序）
    QVector<CampusInfo> result = topKCampuses(campuses, 10, byHeat, type, kw);

    if (result.isEmpty()) {
        m_recResultLabel->setStyleSheet(
            "background:#fff2e6; color:#c2410c;"
            "border-radius:14px; padding:10px 16px; font-size:13px;");
        m_recResultLabel->setText("📭 没有找到匹配的景区/学校");
        return;
    }

    // 标题
    QString modeStr = byHeat ? "热度" : "评分";
    QString typeStr = type.isEmpty() ? "全部" : type;
    QString kwStr   = kw.isEmpty() ? "" : QString(" · 关键字「%1」").arg(kw);
    m_recResultLabel->setStyleSheet(
        "background:#fefce8; border-left:4px solid #1f6d49;"
        "border-radius:14px; padding:10px 16px; font-size:13px; color:#1f2f38;");
    m_recResultLabel->setText(
        QString("🏆 Top-%1 · 按%2 · %3%4")
            .arg(result.size()).arg(modeStr).arg(typeStr).arg(kwStr));

    // 逐项显示 — 用单行 HTML 富文本确保完美对齐
    static const QMap<QString, QString> typeEmoji = {
        {"985高校", "🎓"}, {"211高校", "🏫"}, {"景点", "🏯"}, {"公园", "🌳"},
        {"医院", "🏥"}, {"商场", "🛍️"}, {"文化场所", "🎭"}, {"交通枢纽", "🚉"}
    };

    for (int i = 0; i < result.size(); ++i) {
        const auto& c = result[i];
        QString medal = (i == 0) ? "🥇" : (i == 1) ? "🥈" : (i == 2) ? "🥉"
            : QString("%1 ").arg(i + 1, 2);  // 右对齐占2位

        // 单行 HTML：排名 | emoji 名称 · 城市 | 🔥热度 | ⭐评分
        QString html = QString(
            "<table width='100%' cellspacing='0' cellpadding='4'"
            " style='font-size:13px; color:#1f2f38;'>"
            "<tr>"
            "<td width='32' align='center' style='font-weight:700;'>%1</td>"
            "<td width='28' align='center'>%2</td>"
            "<td align='left' style='font-weight:600; color:#1e4a6b;'>%3 · %4 · %5</td>"
            "<td width='52' align='right' style='color:#c2410c;'>🔥%6</td>"
            "<td width='52' align='right' style='color:#f59e0b;'>⭐%7</td>"
            "</tr></table>"
        ).arg(medal, typeEmoji.value(c.type, "📍"),
              c.name, c.city, c.type,
              QString::number(c.heat, 'f', 1),
              QString::number(c.rating, 'f', 1));

        QLabel* rowLabel = new QLabel(html);
        rowLabel->setTextFormat(Qt::RichText);
        rowLabel->setWordWrap(false);
        rowLabel->setStyleSheet(
            "QLabel { background: white; border: 1px solid #e2edf2;"
            " border-radius: 10px; padding: 4px 10px; }");
        m_recResultLayout->addWidget(rowLabel);
    }
}

// ============================================================
// 附近设施查询面板
// ============================================================

QWidget* MainWindow::createNearbyFacilityPanel()
{
    QWidget* panel = new QWidget();
    panel->setStyleSheet(
        "background: #f8fafc;"
        "border-radius: 24px;"
        "padding: 20px;"
        "margin: 12px 28px;"
        "border: 1px solid #e2edf2;"
    );
    QVBoxLayout* layout = new QVBoxLayout(panel);
    layout->setSpacing(12);

    // 标题行
    QHBoxLayout* titleRow = new QHBoxLayout();
    QLabel* title = new QLabel("🔍 附近设施查询");
    title->setStyleSheet("font-size:16px; font-weight:700; color:#0f5b7a; background:transparent;");
    titleRow->addWidget(title);

    // 提示：点击"完成导航"后默认从终点查询
    QLabel* tip = new QLabel("基于实际步行距离（非直线距离）");
    tip->setStyleSheet("font-size:11px; color:#7f9aaa; background:transparent;");
    titleRow->addWidget(tip);
    titleRow->addStretch();
    layout->addLayout(titleRow);

    // 第一行：当前位置 + 搜索范围
    QHBoxLayout* row1 = new QHBoxLayout();
    row1->setSpacing(12);

    QLabel* centerLabel = new QLabel("📍 所在位置");
    centerLabel->setStyleSheet(
        "font-weight:600; color:#1e4a6b; background:#e9f2f5;"
        "border-radius:20px; padding:6px 14px; font-size:12px;");
    row1->addWidget(centerLabel);

    m_facilityCenterSelect = new QComboBox();
    m_facilityCenterSelect->setMinimumWidth(180);
    m_facilityCenterSelect->setToolTip("选择你当前所在的地标位置");
    row1->addWidget(m_facilityCenterSelect, 2);

    QLabel* rangeLabel = new QLabel("📏 范围");
    rangeLabel->setStyleSheet(
        "font-weight:600; color:#1e4a6b; background:#e9f2f5;"
        "border-radius:20px; padding:6px 14px; font-size:12px;");
    row1->addWidget(rangeLabel);

    m_facilityRangeSelect = new QComboBox();
    m_facilityRangeSelect->addItems({"100m", "200m", "300m", "500m", "800m", "1000m", "全部"});
    m_facilityRangeSelect->setCurrentIndex(3); // 默认500m
    m_facilityRangeSelect->setMaximumWidth(100);
    row1->addWidget(m_facilityRangeSelect);

    layout->addLayout(row1);

    // 第二行：类别过滤 + 查询按钮
    QHBoxLayout* row2 = new QHBoxLayout();
    row2->setSpacing(12);

    QLabel* catLabel = new QLabel("🏷️ 类别");
    catLabel->setStyleSheet(
        "font-weight:600; color:#1e4a6b; background:#e9f2f5;"
        "border-radius:20px; padding:6px 14px; font-size:12px;");
    row2->addWidget(catLabel);

    m_facilityCategorySelect = new QComboBox();
    m_facilityCategorySelect->addItem("全部类别");
    m_facilityCategorySelect->addItems({
        "出入口", "餐饮", "购物", "住宿", "教学", "行政",
        "运动", "医疗", "学习", "休闲", "卫生", "金融", "邮政", "交通"
    });
    row2->addWidget(m_facilityCategorySelect, 2);

    m_queryNearbyBtn = new QPushButton("🔍 查询附近设施");
    m_queryNearbyBtn->setStyleSheet(
        "QPushButton {"
        "  background: #1f6d49;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 24px;"
        "  padding: 10px 20px;"
        "  font-size: 14px;"
        "  font-weight: 600;"
        "}"
        "QPushButton:hover { background: #0e5437; }"
        "QPushButton:pressed { background: #0a3d28; }"
    );
    m_queryNearbyBtn->setCursor(Qt::PointingHandCursor);
    connect(m_queryNearbyBtn, &QPushButton::clicked,
            this, &MainWindow::onQueryNearbyFacilities);
    row2->addWidget(m_queryNearbyBtn);

    layout->addLayout(row2);

    // 结果列表区域
    m_nearbyResultLabel = new QLabel("💡 选择一个位置，点击查询，查看附近设施");
    m_nearbyResultLabel->setWordWrap(true);
    m_nearbyResultLabel->setStyleSheet(
        "background: #fefce8;"
        "border-left: 4px solid #1f6d49;"
        "border-radius: 14px;"
        "padding: 10px 16px;"
        "font-size: 13px;"
        "color: #1f2f38;"
    );
    layout->addWidget(m_nearbyResultLabel);

    // 结果列表容器（动态生成）
    QWidget* resultContainer = new QWidget();
    resultContainer->setStyleSheet("background: transparent;");
    m_nearbyResultLayout = new QVBoxLayout(resultContainer);
    m_nearbyResultLayout->setContentsMargins(0, 0, 0, 0);
    m_nearbyResultLayout->setSpacing(6);
    layout->addWidget(resultContainer);

    return panel;
}

// ============================================================
// 日记面板
// ============================================================

QWidget* MainWindow::createDiaryPanel()
{
    QWidget* panel = new QWidget();
    panel->setStyleSheet(
        "background: #f8fafc;"
        "border-radius: 24px;"
        "padding: 20px;"
        "margin: 0 28px;"
        "border: 1px solid #e2edf2;"
    );
    QVBoxLayout* layout = new QVBoxLayout(panel);
    layout->setSpacing(12);

    // 标题
    QLabel* title = new QLabel("📝 旅行日记");
    title->setStyleSheet("font-size:16px; font-weight:700; color:#0f5b7a;");
    layout->addWidget(title);

    // 分类按钮 — 2行3列网格布局，确保每个按钮足够宽
    QGridLayout* catLayout = new QGridLayout();
    catLayout->setSpacing(10);
    QStringList cats = DiaryCategory::allCategories();
    for (int i = 0; i < cats.size(); ++i) {
        QPushButton* btn = new QPushButton(cats[i]);
        btn->setCheckable(true);
        btn->setMinimumHeight(40);
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        btn->setStyleSheet(
            "QPushButton {"
            "  background: white;"
            "  border: 1px solid #cbdde6;"
            "  border-radius: 12px;"
            "  padding: 10px 0px;"
            "  font-size: 16px;"
            "  font-weight: 500;"
            "  color: #1e4a6b;"
            "}"
            "QPushButton:hover { background: #e9f2f5; border-color: #2c7da0; }"
            "QPushButton:checked {"
            "  background: #1f6d49;"
            "  color: white;"
            "  border-color: #1f6d49;"
            "}"
        );
        connect(btn, &QPushButton::clicked, this, &MainWindow::onCategoryChanged);
        m_categoryBtns.append(btn);
        catLayout->addWidget(btn, i / 3, i % 3);
    }
    // 默认选中第一个（游记）
    if (!m_categoryBtns.isEmpty()) {
        m_categoryBtns[0]->setChecked(true);
    }
    layout->addLayout(catLayout);

    // 日记文本区域
    m_diaryTextarea = new QTextEdit();
    m_diaryTextarea->setPlaceholderText(
        "在这里写下你的旅途感悟...\n\n提示：规划路径后，轨迹会自动同步到日记中");
    m_diaryTextarea->setMinimumHeight(120);
    m_diaryTextarea->setMaximumHeight(200);
    connect(m_diaryTextarea, &QTextEdit::textChanged,
            this, &MainWindow::onTextChanged);
    layout->addWidget(m_diaryTextarea);

    // 状态栏 + 按钮
    QHBoxLayout* statusLayout = new QHBoxLayout();

    m_diaryStatus = new QLabel("📄 新日记 · 0 字");
    m_diaryStatus->setStyleSheet("font-size:11px; color:#5e7a8c;");
    statusLayout->addWidget(m_diaryStatus);

    statusLayout->addStretch();

    m_attachPhotoBtn = new QPushButton("🖼️ 配图");
    m_attachPhotoBtn->setStyleSheet(
        "QPushButton {"
        "  background: white;"
        "  border: 1px solid #cbdde6;"
        "  border-radius: 20px;"
        "  padding: 6px 14px;"
        "  font-size: 12px;"
        "  font-weight: 500;"
        "  color: #1e4a6b;"
        "}"
        "QPushButton:hover { background: #e9f2f5; }"
    );
    m_attachPhotoBtn->setCursor(Qt::PointingHandCursor);
    connect(m_attachPhotoBtn, &QPushButton::clicked,
            this, &MainWindow::onAttachPhoto);
    statusLayout->addWidget(m_attachPhotoBtn);

    m_saveDraftBtn = new QPushButton("💾 保存草稿");
    m_saveDraftBtn->setStyleSheet(
        "QPushButton {"
        "  background: #f59e0b;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 20px;"
        "  padding: 6px 14px;"
        "  font-size: 12px;"
        "  font-weight: 600;"
        "}"
        "QPushButton:hover { background: #d97706; }"
    );
    m_saveDraftBtn->setCursor(Qt::PointingHandCursor);
    connect(m_saveDraftBtn, &QPushButton::clicked,
            this, &MainWindow::onSaveDraft);
    statusLayout->addWidget(m_saveDraftBtn);

    layout->addLayout(statusLayout);

    // 摘要提示区
    m_diarySummary = new QLabel();
    m_diarySummary->setStyleSheet(
        "background: white;"
        "border-radius: 12px;"
        "padding: 10px 14px;"
        "font-size: 11px;"
        "color: #3a5b6e;"
    );
    m_diarySummary->setWordWrap(true);
    m_diarySummary->hide();
    layout->addWidget(m_diarySummary);

    return panel;
}

// ============================================================
// 草稿列表面板
// ============================================================

QWidget* MainWindow::createDraftListPanel()
{
    m_draftListPanel = new QWidget();
    m_draftListPanel->setStyleSheet(
        "background: #f8fafc;"
        "border-radius: 20px;"
        "padding: 16px;"
        "margin: 12px 28px;"
        "border: 1px solid #e2edf2;"
    );

    QVBoxLayout* outerLayout = new QVBoxLayout(m_draftListPanel);

    QHBoxLayout* headerLayout = new QHBoxLayout();
    QLabel* title = new QLabel("📚 我的草稿");
    title->setStyleSheet("font-size:14px; font-weight:700; color:#0f5b7a;");
    headerLayout->addWidget(title);

    m_draftCountBadge = new QLabel("0");
    m_draftCountBadge->setStyleSheet(
        "background:#1f6d49; color:white; border-radius:12px;"
        "padding:2px 10px; font-size:11px;");
    headerLayout->addWidget(m_draftCountBadge);
    headerLayout->addStretch();
    outerLayout->addLayout(headerLayout);

    m_draftListLayout = new QVBoxLayout();
    outerLayout->addLayout(m_draftListLayout);

    m_draftListPanel->hide();
    return m_draftListPanel;
}

// ============================================================
// 已发布列表面板
// ============================================================

QWidget* MainWindow::createPublishedPanel()
{
    m_publishedPanel = new QWidget();
    m_publishedPanel->setStyleSheet(
        "background: #f0f7f4;"
        "border-radius: 20px;"
        "padding: 16px;"
        "margin: 12px 28px;"
        "border: 1px solid #c8e0d2;"
    );

    QVBoxLayout* outerLayout = new QVBoxLayout(m_publishedPanel);

    QHBoxLayout* headerLayout = new QHBoxLayout();
    QLabel* title = new QLabel("📰 已发布日记");
    title->setStyleSheet("font-size:14px; font-weight:700; color:#1f6d49;");
    headerLayout->addWidget(title);

    m_publishedCountBadge = new QLabel("0");
    m_publishedCountBadge->setStyleSheet(
        "background:#1f6d49; color:white; border-radius:12px;"
        "padding:2px 10px; font-size:11px;");
    headerLayout->addWidget(m_publishedCountBadge);
    headerLayout->addStretch();
    outerLayout->addLayout(headerLayout);

    m_publishedListLayout = new QVBoxLayout();
    outerLayout->addLayout(m_publishedListLayout);

    m_publishedPanel->hide();
    return m_publishedPanel;
}

// ============================================================
// 辅助方法
// ============================================================

void MainWindow::populateSelectors()
{
    m_startSelect->clear();
    m_endSelect->clear();

    // 添加默认提示项
    m_startSelect->addItem("—— 选择起点地标 ——", QVariant());
    m_endSelect->addItem("—— 选择终点地标 ——", QVariant());

    QVector<NodeInfo> landmarks = m_graph.getLandmarks();
    for (const auto& lm : landmarks) {
        m_startSelect->addItem(lm.name, lm.name);
        m_endSelect->addItem(lm.name, lm.name);
    }

    // 填充附近设施查询的"所在位置"选择器
    m_facilityCenterSelect->addItem("—— 选择当前位置 ——", QVariant());
    for (const auto& lm : landmarks) {
        m_facilityCenterSelect->addItem(lm.name, lm.name);
    }
}

void MainWindow::updateStats()
{
    m_statsLabel->setText(
        QString("📍 地标 %1     🔗 路段 %2")
            .arg(m_graph.landmarkCount())
            .arg(m_graph.edgeCount())
    );
}

void MainWindow::updateWordCount()
{
    QString cat = m_currentDiary.category.isEmpty()
                      ? DiaryCategory::TRAVEL_NOTE
                      : m_currentDiary.category;
    int len = m_diaryTextarea->toPlainText().length();
    m_diaryStatus->setText(QString("📄 %1 · %2 字").arg(cat).arg(len));
}

// ============================================================
// 导航槽函数
// ============================================================

void MainWindow::onPlanRoute()
{
    // 先清除上一次的导航路径
    m_mapWidget->clearNavigationPath();
    m_finishBtn->setEnabled(false);

    QString startName = m_startSelect->currentData().toString();
    QString endName   = m_endSelect->currentData().toString();

    if (startName.isEmpty() || endName.isEmpty()) {
        m_pathDisplay->setStyleSheet(
            "background:#fff2e6; color:#c2410c;"
            "border-radius:14px; padding:12px 16px; font-size:13px;");
        m_pathDisplay->setText("❌ 请完整选择起点和终点");
        m_distanceLabel->setText("📏 路径距离");
        return;
    }

    if (startName == endName) {
        m_distanceLabel->setText("📏 路径距离: 0 米");
        m_pathDisplay->setStyleSheet(
            "background:#fefce8; border-left:4px solid #1f6d49;"
            "border-radius:14px; padding:12px 16px; font-size:13px;");
        m_pathDisplay->setText(QString("📍 %1 (起点即终点)").arg(startName));
        return;
    }

    int startIdx = m_graph.indexOf(startName);
    int endIdx   = m_graph.indexOf(endName);

    if (startIdx < 0 || endIdx < 0) {
        m_pathDisplay->setStyleSheet(
            "background:#fff2e6; color:#c2410c;"
            "border-radius:14px; padding:12px 16px; font-size:13px;");
        m_pathDisplay->setText("节点不存在，请重新选择");
        return;
    }

    // 获取策略
    RouteStrategy strategy = static_cast<RouteStrategy>(
        m_strategySelect->currentIndex());

    // 策略名称
    static const char* stratNames[] = {"最短距离", "最短时间", "自行车道", "电瓶车"};
    const char* stratName = stratNames[m_strategySelect->currentIndex()];

    // 执行 Dijkstra 算法（带策略）
    auto [dist, prev] = m_graph.dijkstra(startIdx, endIdx, strategy);
    double totalDist = dist[endIdx];

    if (!std::isfinite(totalDist)) {
        m_distanceLabel->setText("📏 路径距离: 不可达");
        m_pathDisplay->setStyleSheet(
            "background:#fff2e6; color:#c2410c;"
            "border-radius:14px; padding:12px 16px; font-size:13px;");
        m_pathDisplay->setText(
            QString("🚫 路径不可达 (%1策略)").arg(stratName));
        return;
    }

    // 重构路径
    QVector<int> fullPath = m_graph.reconstructPath(prev, startIdx, endIdx);
    if (fullPath.isEmpty()) {
        m_distanceLabel->setText("📏 路径距离: 不可达");
        m_pathDisplay->setText("路径重建失败");
        return;
    }

    // 过滤并显示
    QVector<int> landmarkPath = m_graph.filterLandmarkPath(fullPath);
    QString displayStr = m_graph.formatPathDisplay(landmarkPath);

    // 标签：区分距离/时间
    QString metricLabel = (strategy == SHORTEST_TIME) ? "最短时间" : "最短距离";
    QString metricUnit  = (strategy == SHORTEST_TIME) ? "s"       : "米";
    m_distanceLabel->setText(
        QString("📏 %1: %2 %3 (%4策略)")
            .arg(metricLabel).arg(static_cast<int>(totalDist)).arg(metricUnit).arg(stratName));

    m_pathDisplay->setStyleSheet(
        "background:#fefce8; border-left:4px solid #1f6d49;"
        "border-radius:14px; padding:12px 16px; font-size:13px;"
        "color:#1f2f38;");
    m_pathDisplay->setText(QString("🚶 %1").arg(displayStr));

    // 在地图上绘制蓝色导航路径（完整路径，含路口）
    m_mapWidget->drawNavigationPath(fullPath);
    m_finishBtn->setEnabled(true);

    // 自动将路径同步到日记轨迹
    syncTrackToDiary();
}

void MainWindow::syncTrackToDiary()
{
    // 从当前路径显示中提取地标序列，存入 m_currentDiary 的轨迹
    QString startName = m_startSelect->currentData().toString();
    QString endName   = m_endSelect->currentData().toString();

    if (startName.isEmpty() || endName.isEmpty()) return;
    if (startName == endName) return;

    int startIdx = m_graph.indexOf(startName);
    int endIdx   = m_graph.indexOf(endName);
    if (startIdx < 0 || endIdx < 0) return;

    auto [dist, prev] = m_graph.dijkstra(startIdx, endIdx);
    if (!std::isfinite(dist[endIdx])) return;

    QVector<int> fullPath = m_graph.reconstructPath(prev, startIdx, endIdx);
    if (fullPath.isEmpty()) return;

    // 保存完整路径的轨迹点
    m_currentDiary.trackPoints.clear();
    for (int idx : fullPath) {
        const auto& node = m_graph.nodes()[idx];
        QJsonObject tp;
        tp["lat"]  = node.lat;
        tp["lng"]  = node.lng;
        tp["name"] = node.name;
        tp["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        m_currentDiary.trackPoints.append(tp);
    }

    m_diarySummary->setText(
        QString("🗺️ 已同步路线轨迹 · 共 %1 个节点（含路口）")
            .arg(fullPath.size()));
    m_diarySummary->show();
}

// ============================================================
// 完成导航槽函数
// ============================================================

void MainWindow::onFinishNavigation()
{
    m_mapWidget->clearNavigationPath();
    m_finishBtn->setEnabled(false);
    m_mapWidget->clearFacilityHighlights();
}

// ============================================================
// 途经多点 + 添加/清空途经点
// ============================================================

void MainWindow::onAddStop()
{
    QString endName = m_endSelect->currentData().toString();
    if (endName.isEmpty()) return;

    int idx = m_graph.indexOf(endName);
    if (idx < 0) return;

    // 不能添加已在列表中的点
    if (m_stopIndices.contains(idx)) return;

    m_stopIndices.append(idx);
    m_stopList->addItem(QString("📍 %1").arg(endName));
    m_stopList->show();

    // 启用/禁用按钮
    m_multiPlanBtn->setEnabled(m_stopIndices.size() >= 1);
    m_clearStopBtn->setEnabled(true);
}

void MainWindow::onClearStops()
{
    m_stopIndices.clear();
    m_stopList->clear();
    m_stopList->hide();
    m_multiPlanBtn->setEnabled(false);
    m_clearStopBtn->setEnabled(false);
}

void MainWindow::onPlanMultiStop()
{
    m_mapWidget->clearNavigationPath();
    m_mapWidget->clearFacilityHighlights();
    m_finishBtn->setEnabled(false);

    if (m_stopIndices.isEmpty()) {
        m_pathDisplay->setStyleSheet(
            "background:#fff2e6; color:#c2410c;"
            "border-radius:14px; padding:12px 16px; font-size:13px;");
        m_pathDisplay->setText("❌ 请先添加途经点");
        return;
    }

    QString startName = m_startSelect->currentData().toString();
    if (startName.isEmpty()) {
        m_pathDisplay->setText("❌ 请先选择起点");
        return;
    }

    int startIdx = m_graph.indexOf(startName);
    if (startIdx < 0) {
        m_pathDisplay->setText("❌ 起点节点不存在");
        return;
    }

    // 调用多点路径规划（TSP + 全排列）
    MultiStopResult result = m_graph.findMultiStopRoute(startIdx, m_stopIndices);

    if (result.fullPath.isEmpty() || !std::isfinite(result.totalDistance)) {
        m_distanceLabel->setText("📏 多点路径: 不可达");
        m_pathDisplay->setStyleSheet(
            "background:#fff2e6; color:#c2410c;"
            "border-radius:14px; padding:12px 16px; font-size:13px;");
        m_pathDisplay->setText("🚫 无法规划途经多点的连通路径");
        return;
    }

    // 过滤显示
    QVector<int> landmarkPath = m_graph.filterLandmarkPath(result.fullPath);
    QString displayStr = m_graph.formatPathDisplay(landmarkPath);

    m_distanceLabel->setText(
        QString("📏 多点路径: %1 米 (途经 %2 个景点)")
            .arg(static_cast<int>(result.totalDistance))
            .arg(m_stopIndices.size()));

    m_pathDisplay->setStyleSheet(
        "background:#fefce8; border-left:4px solid #f59e0b;"
        "border-radius:14px; padding:12px 16px; font-size:13px; color:#1f2f38;");
    m_pathDisplay->setText(QString("🗺️ %1").arg(displayStr));

    // 绘制路径
    m_mapWidget->drawNavigationPath(result.fullPath);
    m_finishBtn->setEnabled(true);
}

// ============================================================
// 附近设施查询槽函数
// ============================================================

void MainWindow::onQueryNearbyFacilities()
{
    // 清除上次的高亮
    m_mapWidget->clearFacilityHighlights();

    // 清空上次结果
    QLayoutItem* child;
    while ((child = m_nearbyResultLayout->takeAt(0)) != nullptr) {
        if (child->widget()) child->widget()->deleteLater();
        delete child;
    }

    QString centerName = m_facilityCenterSelect->currentData().toString();
    if (centerName.isEmpty()) {
        m_nearbyResultLabel->setStyleSheet(
            "background:#fff2e6; color:#c2410c;"
            "border-radius:14px; padding:10px 16px; font-size:13px;");
        m_nearbyResultLabel->setText("❌ 请先选择当前位置");
        return;
    }

    int centerIdx = m_graph.indexOf(centerName);
    if (centerIdx < 0) {
        m_nearbyResultLabel->setText("⚠️ 未找到该位置");
        return;
    }

    // 解析范围
    QString rangeText = m_facilityRangeSelect->currentText();
    double maxRange = 1e9;  // "全部"
    if (rangeText != "全部") {
        maxRange = rangeText.chopped(1).toDouble();  // "500m" → 500
    }

    // 解析类别过滤
    QString catFilter = m_facilityCategorySelect->currentText();
    if (catFilter == "全部类别") catFilter.clear();

    // 执行查询
    QVector<NearbyFacility> results = m_graph.findNearbyFacilities(
        centerIdx, maxRange, catFilter);

    // 显示结果
    if (results.isEmpty()) {
        m_nearbyResultLabel->setStyleSheet(
            "background:#fefce8; border-left:4px solid #f59e0b;"
            "border-radius:14px; padding:10px 16px; font-size:13px; color:#1f2f38;");
        m_nearbyResultLabel->setText(
            QString("📭 在 %1 范围内未找到%2设施")
                .arg(rangeText)
                .arg(catFilter.isEmpty() ? "" : catFilter + " "));
        return;
    }

    // 统计信息
    m_nearbyResultLabel->setStyleSheet(
        "background:#fefce8; border-left:4px solid #1f6d49;"
        "border-radius:14px; padding:10px 16px; font-size:13px; color:#1f2f38;");
    m_nearbyResultLabel->setText(
        QString("✅ 找到 %1 个%2设施（%3 范围内，实际步行距离）")
            .arg(results.size())
            .arg(catFilter.isEmpty() ? "" : catFilter + " ")
            .arg(rangeText));

    // 构建结果条目
    QVector<int> highlightIndices;
    for (int i = 0; i < results.size(); ++i) {
        const auto& f = results[i];
        highlightIndices.append(f.nodeIndex);

        QWidget* row = new QWidget();
        row->setStyleSheet(
            "background: white; border-radius: 12px; padding: 8px 14px;"
            "border: 1px solid #e2edf2;");
        QHBoxLayout* rl = new QHBoxLayout(row);
        rl->setContentsMargins(0, 0, 0, 0);
        rl->setSpacing(8);

        // 序号 + emoji
        static const QMap<QString, QString> emojiMap = {
            {"出入口", "🏠"}, {"餐饮", "🍽️"}, {"购物", "🛒"}, {"住宿", "🛏️"},
            {"教学", "📖"}, {"行政", "🏛️"}, {"运动", "⚽"}, {"医疗", "🏥"},
            {"学习", "📚"}, {"休闲", "☕"}, {"卫生", "🚻"}, {"金融", "💰"},
            {"邮政", "📮"}, {"交通", "🚌"}
        };
        QString emoji = emojiMap.value(f.category, "📍");

        QLabel* numLabel = new QLabel(QString("#%1").arg(i + 1));
        numLabel->setStyleSheet(
            "font-weight:700; font-size:11px; color:#1f6d49; background:transparent;"
            "min-width:24px;");
        rl->addWidget(numLabel);

        QLabel* emojiLabel = new QLabel(emoji);
        emojiLabel->setStyleSheet("font-size:16px; background:transparent;");
        rl->addWidget(emojiLabel);

        QLabel* nameLabel = new QLabel(f.name);
        nameLabel->setStyleSheet(
            "font-weight:600; font-size:13px; color:#1e4a6b; background:transparent;");
        rl->addWidget(nameLabel, 1);

        QLabel* cat = new QLabel(f.category);
        cat->setStyleSheet(
            "background:#e9f2f5; border-radius:10px; padding:2px 10px;"
            "font-size:10px; color:#1e4a6b;");
        rl->addWidget(cat);

        QLabel* dist = new QLabel(QString("🚶 %1m").arg(static_cast<int>(f.distance)));
        dist->setStyleSheet(
            "font-weight:700; font-size:12px; color:#0f5b7a; background:transparent;");
        rl->addWidget(dist);

        m_nearbyResultLayout->addWidget(row);
    }

    // 在地图上高亮
    m_mapWidget->highlightFacilities(highlightIndices);
}

void MainWindow::onCampusChanged(int index)
{
    if (index < 0) return;
    int campusIdx = m_campusSelect->currentData().toInt();
    const auto& campuses = getAllCampuses();
    if (campusIdx < 0 || campusIdx >= campuses.size()) return;

    const CampusInfo& ci = campuses[campusIdx];
    // 更新统计信息显示
    updateStats();

    // 更新窗口标题
    setWindowTitle(QString("%1 · 校园导航系统").arg(ci.name));
}

void MainWindow::onUserChanged(int index)
{
    if (index < 0) return;
    QString userId = m_userSelect->currentData().toString();
    m_userManager.setCurrentUser(userId);
    m_currentDiary.userId = userId;

    // 更新日记管理器存储路径
    m_diaryManager.setStoragePath(
        QApplication::applicationDirPath() + "/data/" + userId);
}

// ============================================================
// 日记槽函数
// ============================================================

void MainWindow::onCategoryChanged()
{
    // clicked 信号在 checkable 按钮状态变更后触发
    // 强制确保点击的按钮被选中，其余取消
    QPushButton* sender = qobject_cast<QPushButton*>(QObject::sender());
    if (!sender) return;

    for (QPushButton* btn : m_categoryBtns) {
        btn->blockSignals(true);
        btn->setChecked(btn == sender);
        btn->blockSignals(false);
    }
    m_currentDiary.category = sender->text();
    updateWordCount();
}

void MainWindow::onTextChanged()
{
    // 同步文本到当前日记对象
    m_currentDiary.content = m_diaryTextarea->toPlainText();
    updateWordCount();
}

void MainWindow::onSaveDraft()
{
    // 确保内容同步
    m_currentDiary.content = m_diaryTextarea->toPlainText();
    m_currentDiary.createdAt = QDateTime::currentDateTime().toString(Qt::ISODate);
    m_currentDiary.id = QDateTime::currentMSecsSinceEpoch();

    bool ok = m_diaryManager.addDraft(m_currentDiary);
    if (ok) {
        m_saveDraftBtn->setText("✅ 已保存");
        QTimer::singleShot(2000, this, [this]() {
            m_saveDraftBtn->setText("💾 保存草稿");
        });

        // 清空编辑区
        m_currentDiary = DiaryEntry();
        m_currentDiary.id = QDateTime::currentMSecsSinceEpoch();
        m_currentDiary.category = DiaryCategory::TRAVEL_NOTE;
        m_currentDiary.userId = "current_user";
        m_diaryTextarea->clear();
        if (!m_categoryBtns.isEmpty()) {
            m_categoryBtns[0]->setChecked(true);
            for (int i = 1; i < m_categoryBtns.size(); ++i) {
                m_categoryBtns[i]->setChecked(false);
            }
        }
        updateWordCount();
        refreshDraftList();
        refreshPublishedList();
        m_diarySummary->setText(
            QString("✅ 草稿已保存 · 共 %1 条 · 已清空编辑区")
                .arg(m_diaryManager.draftCount()));
        m_diarySummary->show();
    } else {
        m_diarySummary->setText("⚠️ 保存失败：数据过大，请删除部分旧草稿或配图");
        m_diarySummary->show();
    }
}

void MainWindow::onAttachPhoto()
{
    QStringList files = QFileDialog::getOpenFileNames(
        this,
        "选择配图",
        QString(),
        "图片文件 (*.png *.jpg *.jpeg *.bmp *.gif);;所有文件 (*.*)"
    );

    if (files.isEmpty()) return;

    for (const QString& filePath : files) {
        // 复制到存储目录
        QString storedPath = m_diaryManager.copyPhotoToStorage(filePath);

        QFileInfo fi(filePath);
        PhotoAttachment photo(storedPath, fi.fileName(),
                              m_currentDiary.photos.size() + 1);
        m_currentDiary.photos.append(photo);
    }

    m_diarySummary->setText(
        QString("🖼️ 已添加 %1 张配图 · 当前共 %2 张")
            .arg(files.size())
            .arg(m_currentDiary.photos.size()));
    m_diarySummary->show();
}

void MainWindow::onLoadDraft(int index)
{
    QVector<DiaryEntry> drafts = m_diaryManager.loadDrafts();
    if (index < 0 || index >= drafts.size()) return;

    m_currentDiary = drafts[index];
    m_diaryTextarea->setPlainText(m_currentDiary.content);

    // 设置分类按钮
    for (QPushButton* btn : m_categoryBtns) {
        btn->setChecked(btn->text() == m_currentDiary.category);
    }

    updateWordCount();
    m_diarySummary->setText(
        QString("📖 已加载草稿 · 🖼️ %1 张图 · 🗺️ %2 个轨迹点")
            .arg(m_currentDiary.photos.size())
            .arg(m_currentDiary.trackPoints.size()));
    m_diarySummary->show();
}

void MainWindow::onPublishDraft(int index)
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认发布",
        "确定发布这条日记吗？发布后会从草稿中移除。",
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) return;

    // 获取当前景区名和用户昵称
    QString campusName;
    int ci = m_campusSelect->currentData().toInt();
    const auto& camps = getAllCampuses();
    if (ci >= 0 && ci < camps.size()) campusName = camps[ci].name;
    QString nickname = m_userManager.currentUser().nickname;
    QString globalDir = QApplication::applicationDirPath() + "/data";

    if (m_diaryManager.publishDraft(index, globalDir, campusName, nickname)) {
        refreshDraftList();
        refreshPublishedList();
        m_diarySummary->setText("📤 日记已发布！");
        m_diarySummary->show();
    }
}

void MainWindow::onDeleteDraft(int index)
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认删除",
        "确定删除这条草稿吗？",
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) return;

    if (m_diaryManager.removeDraft(index)) {
        refreshDraftList();
        m_diarySummary->setText(
            QString("🗑️ 草稿已删除 · 剩余 %1 条")
                .arg(m_diaryManager.draftCount()));
        m_diarySummary->show();
    }
}

void MainWindow::onDeletePublished(int index)
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认删除",
        "确定删除这条已发布日记吗？",
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) return;

    QString globalDir = QApplication::applicationDirPath() + "/data";
    if (DiaryManager::removePublished(globalDir, index)) {
        refreshPublishedList();
        m_diarySummary->setText("🗑️ 已发布日记已删除");
        m_diarySummary->show();
    }
}

void MainWindow::onViewTimeline(int activeIndex)
{
    QString globalDir = QApplication::applicationDirPath() + "/data";
    QVector<DiaryEntry> published = DiaryManager::loadPublished(globalDir);
    if (published.isEmpty()) {
        QMessageBox::information(this, "已发布日记", "暂无已发布的日记。");
        return;
    }

    // 使用 QDialog 作为时间线查看器（替代 HTML 的覆盖层）
    QDialog* timelineDialog = new QDialog(this);
    timelineDialog->setWindowTitle("📰 已发布日记 - 朋友圈时间线");
    timelineDialog->resize(600, 700);
    timelineDialog->setStyleSheet(
        "QDialog { background: #e9f0f5; }");

    QVBoxLayout* dialogLayout = new QVBoxLayout(timelineDialog);

    // 顶部栏
    QHBoxLayout* topBar = new QHBoxLayout();
    QPushButton* backBtn = new QPushButton("← 返回");
    backBtn->setStyleSheet(
        "QPushButton { background: white; border-radius: 20px;"
        "padding: 8px 20px; font-size: 13px; font-weight: 600;"
        "color: #0f5b7a; border: none; }"
        "QPushButton:hover { background: #f0f4f9; }");
    connect(backBtn, &QPushButton::clicked, timelineDialog, &QDialog::accept);
    topBar->addWidget(backBtn);

    QLabel* topTitle = new QLabel("📰 已发布日记");
    topTitle->setStyleSheet("font-weight:700; font-size:16px; color:#0f5b7a;");
    topBar->addWidget(topTitle);
    topBar->addStretch();
    dialogLayout->addLayout(topBar);

    // 滚动区域
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet("QScrollArea { border: none; background: transparent; }");

    QWidget* listWidget = new QWidget();
    listWidget->setStyleSheet("background: transparent;");
    QVBoxLayout* listLayout = new QVBoxLayout(listWidget);
    listLayout->setSpacing(16);

    for (int i = 0; i < published.size(); ++i) {
        const DiaryEntry& item = published[i];
        QString timeStr = item.publishedAt.isEmpty()
                              ? item.createdAt
                              : item.publishedAt;

        QWidget* card = new QWidget();
        card->setStyleSheet(
            "background: white; border-radius: 20px; padding: 18px;");
        QVBoxLayout* cardLayout = new QVBoxLayout(card);

        // 头部（作者 + 分类 + 时间 + 目的地）
        QHBoxLayout* cardHeader = new QHBoxLayout();
        QString authorStr = item.userNickname.isEmpty() ? item.userId : item.userNickname;
        QLabel* authorLabel = new QLabel(QString("👤 %1").arg(authorStr));
        authorLabel->setStyleSheet("font-size:11px; color:#1f6d49; font-weight:700;");
        cardHeader->addWidget(authorLabel);
        QLabel* catLabel = new QLabel(QString("🏷️ %1").arg(item.category));
        catLabel->setStyleSheet("font-size:11px; color:#7f9aaa; font-weight:600;");
        cardHeader->addWidget(catLabel);
        cardHeader->addStretch();

        QString destInfo;
        if (!item.trackPoints.isEmpty()) {
            QJsonObject last = item.trackPoints.last();
            QString addr = item.campusName.isEmpty() ? last["name"].toString()
                : item.campusName + ", " + last["name"].toString();
            destInfo = QString(" · 🏁 %1").arg(addr);
        }
        QLabel* timeLabel = new QLabel(
            QString("🕐 %1%2").arg(timeStr, destInfo));
        timeLabel->setStyleSheet("font-size:11px; color:#7f9aaa; font-weight:600;");
        cardHeader->addWidget(timeLabel);
        cardLayout->addLayout(cardHeader);

        // 热度 + 评分行
        QHBoxLayout* statsRow = new QHBoxLayout();
        statsRow->setSpacing(10);

        // 热度（浏览次数）
        QLabel* viewsLabel = new QLabel(
            QString("👁 %1 次浏览").arg(item.views));
        viewsLabel->setStyleSheet("font-size:11px; color:#7f9aaa; font-weight:500;");
        statsRow->addWidget(viewsLabel);

        // 分隔线
        QLabel* sepLabel = new QLabel("|");
        sepLabel->setStyleSheet("font-size:11px; color:#cbdde6;");
        statsRow->addWidget(sepLabel);

        // 平均评分显示
        double avg = item.avgRating();
        QLabel* ratingDisplay = new QLabel();
        ratingDisplay->setStyleSheet("font-size:12px; color:#f59e0b; font-weight:700;");
        {
            // 全部用 ★(U+2605)，颜色区分：前N颗金色，后面灰色
            QString starStr;
            for (int s = 1; s <= 5; ++s)
                starStr += QChar(0x2605);
            if (avg > 0)
                ratingDisplay->setText(QString("⭐ %1  (%2分 · %3人评)")
                    .arg(starStr).arg(avg, 0, 'f', 1).arg(item.ratingCount()));
            else
                ratingDisplay->setText("⭐ 暂无评分");
        }
        statsRow->addWidget(ratingDisplay);
        statsRow->addStretch();
        cardLayout->addLayout(statsRow);

        // 操作按钮行：仅"查看详情"
        QHBoxLayout* actionRow = new QHBoxLayout();
        int cardIndex = i;
        QPushButton* detailBtn = new QPushButton("📖 查看详情");
        detailBtn->setStyleSheet(
            "QPushButton { background:#e9f2f5; border:none; border-radius:16px;"
            "padding:6px 14px; font-size:11px; color:#1e4a6b; font-weight:600; }"
            "QPushButton:hover { background:#d9e5f0; }");
        detailBtn->setCursor(Qt::PointingHandCursor);
        connect(detailBtn, &QPushButton::clicked, this,
            [this, cardIndex]() { showDiaryDetail(cardIndex); });
        actionRow->addWidget(detailBtn);
        actionRow->addStretch();
        cardLayout->addLayout(actionRow);

        // 内容
        QLabel* content = new QLabel(item.content.isEmpty() ? "（无内容）" : item.content);
        content->setWordWrap(true);
        content->setStyleSheet(
            "font-size:13px; color:#1f2f38; line-height:1.6;"
            "white-space:pre-wrap; padding:8px 0;");
        cardLayout->addWidget(content);

        // 配图预览（可点击放大）
        if (!item.photos.isEmpty()) {
            QHBoxLayout* photoLayout = new QHBoxLayout();
            for (const auto& p : item.photos) {
                if (p.attachOrder > 3) break; // 最多显示3张
                QLabel* photoLabel = new QLabel();
                photoLabel->setCursor(Qt::PointingHandCursor);
                QPixmap pm(p.imageFilePath);
                QString photoPath = p.imageFilePath;  // 捕获路径
                if (!pm.isNull()) {
                    photoLabel->setPixmap(pm.scaled(120, 120,
                        Qt::KeepAspectRatio, Qt::SmoothTransformation));
                } else {
                    photoLabel->setText("🖼️");
                }
                photoLabel->setStyleSheet(
                    "border:2px solid #e2edf2; border-radius:12px; padding:4px;");
                photoLabel->setToolTip(QString("点击放大 · %1").arg(p.caption));

                // 点击图片打开大图预览
                photoLabel->installEventFilter(this);
                photoLabel->setProperty("photoPath", photoPath);
                photoLayout->addWidget(photoLabel);
            }
            photoLayout->addStretch();
            cardLayout->addLayout(photoLayout);
        }

        listLayout->addWidget(card);
    }

    listLayout->addStretch();
    scrollArea->setWidget(listWidget);
    dialogLayout->addWidget(scrollArea);

    // 滚动到指定位置
    if (activeIndex >= 0 && activeIndex < published.size()) {
        // 简单实现：布局完成后滚动
        QTimer::singleShot(100, timelineDialog, [scrollArea, activeIndex]() {
            // 估算滚动位置
            int approxY = activeIndex * 200;
            scrollArea->verticalScrollBar()->setValue(approxY);
        });
    }

    timelineDialog->exec();
    timelineDialog->deleteLater();
}

// ============================================================
// 单条日记详情（浏览计数 + 交互式评分）
// ============================================================

void MainWindow::showDiaryDetail(int index)
{
    QString globalDir = QApplication::applicationDirPath() + "/data";
    QVector<DiaryEntry> published = DiaryManager::loadPublished(globalDir);
    if (index < 0 || index >= published.size()) return;

    // 📖 打开详情即浏览 → 该日记热度 +1
    DiaryManager::incrementViews(globalDir, index);
    published = DiaryManager::loadPublished(globalDir);
    const DiaryEntry& item = published[index];

    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle(QString("📖 %1 的日记").arg(
        item.userNickname.isEmpty() ? item.userId : item.userNickname));
    dialog->resize(560, 650);
    dialog->setStyleSheet("QDialog { background: #e9f0f5; }");

    QVBoxLayout* dl = new QVBoxLayout(dialog);

    // 顶部操作栏
    QHBoxLayout* top = new QHBoxLayout();
    QPushButton* backBtn = new QPushButton("← 返回列表");
    backBtn->setStyleSheet(
        "QPushButton { background: white; border-radius: 20px;"
        "padding: 8px 20px; font-size: 13px; font-weight: 600;"
        "color: #0f5b7a; border: none; }"
        "QPushButton:hover { background: #f0f4f9; }");
    connect(backBtn, &QPushButton::clicked, dialog, &QDialog::accept);
    top->addWidget(backBtn);
    top->addStretch();
    dl->addLayout(top);

    // 卡片主体
    QScrollArea* sa = new QScrollArea();
    sa->setWidgetResizable(true);
    sa->setStyleSheet("QScrollArea { border: none; background: transparent; }");

    QWidget* card = new QWidget();
    card->setStyleSheet("background: white; border-radius: 20px; padding: 20px;");
    QVBoxLayout* cl = new QVBoxLayout(card);
    cl->setSpacing(14);

    // 头部信息
    QString authorStr = item.userNickname.isEmpty() ? item.userId : item.userNickname;
    QLabel* header = new QLabel(
        QString("👤 %1  ·  🏷️ %2\n🕐 %3  ·  👁 %4 次浏览")
            .arg(authorStr, item.category,
                 item.publishedAt.isEmpty() ? item.createdAt : item.publishedAt)
            .arg(item.views));
    header->setStyleSheet("font-size:12px; color:#7f9aaa; font-weight:500; line-height:1.8;");
    cl->addWidget(header);

    // 目的地信息
    if (!item.trackPoints.isEmpty()) {
        QJsonObject last = item.trackPoints.last();
        QString addr = item.campusName.isEmpty() ? last["name"].toString()
            : item.campusName + ", " + last["name"].toString();
        QLabel* dest = new QLabel(QString("🏁 %1").arg(addr));
        dest->setStyleSheet("font-size:12px; color:#1f6d49; font-weight:600;");
        cl->addWidget(dest);
    }

    // 正文
    QLabel* content = new QLabel(item.content.isEmpty() ? "（无内容）" : item.content);
    content->setWordWrap(true);
    content->setStyleSheet(
        "font-size:14px; color:#1f2f38; line-height:1.8; white-space:pre-wrap;"
        "padding:8px 0;");
    cl->addWidget(content);

    // 配图（可点击放大）
    if (!item.photos.isEmpty()) {
        QHBoxLayout* pl = new QHBoxLayout();
        pl->setSpacing(8);
        for (const auto& p : item.photos) {
            QLabel* plb = new QLabel();
            plb->setCursor(Qt::PointingHandCursor);
            QPixmap pm(p.imageFilePath);
            QString pp = p.imageFilePath;
            if (!pm.isNull())
                plb->setPixmap(pm.scaled(160, 160, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            else
                plb->setText("🖼️");
            plb->setStyleSheet("border:2px solid #e2edf2; border-radius:14px; padding:4px;");
            plb->setToolTip("点击放大");
            plb->installEventFilter(this);
            plb->setProperty("photoPath", pp);
            pl->addWidget(plb);
        }
        pl->addStretch();
        cl->addLayout(pl);
    }

    // 获取当前用户ID
    QString curUid = m_userManager.currentUserId();

    // ---- 评分区域 ----
    // 平均分显示
    QHBoxLayout* sr = new QHBoxLayout();
    QLabel* avgLabel = new QLabel();
    avgLabel->setStyleSheet("font-size:16px; color:#c4860b; font-weight:700;");

    auto buildAvgText = [](const DiaryEntry& e) -> QString {
        double a = e.avgRating();
        QString ss;
        for (int k = 1; k <= 5; ++k) ss += QChar(0x2605);
        if (a > 0)
            return QString("评分: %1  %2 分  (%3人评)")
                .arg(ss).arg(a, 0, 'f', 1).arg(e.ratingCount());
        return QString("评分: %1  暂无").arg(ss);
    };
    avgLabel->setText(buildAvgText(item));
    sr->addWidget(avgLabel);
    sr->addStretch();
    cl->addLayout(sr);

    // 5 颗可点击星星 — 只用 QChar(0x2605)，金/灰色区分
    QHBoxLayout* rr = new QHBoxLayout();
    rr->setSpacing(6);
    QLabel* rp = new QLabel("打分(点击后关闭):");
    rp->setStyleSheet("font-size:12px; color:#5e7a8c; font-weight:600;");
    rr->addWidget(rp);

    int di = index;
    QString dd = globalDir;
    int cr = item.getUserRating(curUid);
    QDialog* dlg = dialog;  // 原始指针，只用来 close

    auto makeStarStyle = [](bool filled) {
        return QString(
            "QPushButton { background:transparent;"
            " border:2px solid %1; border-radius:6px;"
            " font-size:26px; color:%1; min-width:40px; min-height:40px; }"
            "QPushButton:hover { background:#fff8e0;"
            " border-color:#f59e0b; color:#f59e0b; }"
        ).arg(filled ? "#f59e0b" : "#d0d0d0");
    };

    for (int star = 1; star <= 5; ++star) {
        QPushButton* sb = new QPushButton(QChar(0x2605));
        sb->setCursor(Qt::PointingHandCursor);
        sb->setStyleSheet(makeStarStyle(cr > 0 && star <= cr));
        sb->setToolTip(QString("%1 分").arg(star));

        int score = star;
        // ★ 唯一操作：存盘 + 关闭弹窗。绝不动 UI。
        connect(sb, &QPushButton::clicked, dlg,
            [dd, di, curUid, score]() {
                DiaryManager::setUserRating(dd, di, curUid, score);
            });
        connect(sb, &QPushButton::clicked, dlg, &QDialog::accept);

        rr->addWidget(sb);
    }
    rr->addStretch();
    cl->addLayout(rr);

    // 提示
    QLabel* tipLabel = new QLabel("评分后弹窗将自动关闭，重新打开即可看到更新");
    tipLabel->setStyleSheet("font-size:11px; color:#7f9aaa; padding:4px 0;");
    cl->addWidget(tipLabel);

    cl->addStretch();
    sa->setWidget(card);
    dl->addWidget(sa);

    dialog->exec();
    dialog->deleteLater();
}

// ============================================================
// 刷新草稿和已发布列表
// ============================================================

void MainWindow::refreshDraftList()
{
    // 清空旧列表项
    QLayoutItem* item;
    while ((item = m_draftListLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    QVector<DiaryEntry> drafts = m_diaryManager.loadDrafts();
    m_draftCountBadge->setText(QString::number(drafts.size()));

    if (drafts.isEmpty()) {
        m_draftListPanel->hide();
        return;
    }

    m_draftListPanel->show();

    for (int i = 0; i < drafts.size(); ++i) {
        const DiaryEntry& draft = drafts[i];

        QWidget* row = new QWidget();
        row->setStyleSheet(
            "background: white; border-radius: 14px; padding: 10px 14px;"
            "border: 1px solid #e2edf2;");
        QHBoxLayout* rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(0, 0, 0, 0);

        // 信息区
        QVBoxLayout* infoLayout = new QVBoxLayout();
        QLabel* titleLabel = new QLabel(
            QString("📄 %1").arg(draft.preview()));
        titleLabel->setStyleSheet(
            "font-weight:600; font-size:12px; color:#1e4a6b;");
        infoLayout->addWidget(titleLabel);

        QString meta = QString("🏷️ %1 · 🕐 %2 · 📝 %3 字 · 🖼️ %4 张图 · 🗺️ %5 轨迹点")
                           .arg(draft.category)
                           .arg(draft.createdAt)
                           .arg(draft.content.length())
                           .arg(draft.photos.size())
                           .arg(draft.trackPoints.size());
        QLabel* metaLabel = new QLabel(meta);
        metaLabel->setStyleSheet("font-size:10px; color:#7f9aaa;");
        infoLayout->addWidget(metaLabel);

        rowLayout->addLayout(infoLayout, 1);

        // 操作按钮
        QPushButton* loadBtn = new QPushButton("📖 加载");
        loadBtn->setStyleSheet(
            "QPushButton { background:#f0f4f9; border:none; border-radius:14px;"
            "padding:4px 10px; font-size:10px; color:#3a5b6e; }"
            "QPushButton:hover { background:#d9e5f0; }");
        int idx = i;
        connect(loadBtn, &QPushButton::clicked, this, [this, idx]() {
            onLoadDraft(idx);
        });
        rowLayout->addWidget(loadBtn);

        QPushButton* pubBtn = new QPushButton("📤 发布");
        pubBtn->setStyleSheet(
            "QPushButton { background:#1f6d49; color:white; border:none;"
            "border-radius:14px; padding:4px 10px; font-size:10px; }"
            "QPushButton:hover { background:#0e5437; }");
        connect(pubBtn, &QPushButton::clicked, this, [this, idx]() {
            onPublishDraft(idx);
        });
        rowLayout->addWidget(pubBtn);

        QPushButton* delBtn = new QPushButton("🗑️ 删除");
        delBtn->setStyleSheet(
            "QPushButton { background:#fff2e6; border:none; border-radius:14px;"
            "padding:4px 10px; font-size:10px; color:#c2410c; }"
            "QPushButton:hover { background:#ffe0cc; }");
        connect(delBtn, &QPushButton::clicked, this, [this, idx]() {
            onDeleteDraft(idx);
        });
        rowLayout->addWidget(delBtn);

        m_draftListLayout->addWidget(row);
    }
}

void MainWindow::refreshPublishedList()
{
    // 清空旧列表项
    QLayoutItem* item;
    while ((item = m_publishedListLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    QString globalDir = QApplication::applicationDirPath() + "/data";
    QVector<DiaryEntry> published = DiaryManager::loadPublished(globalDir);
    m_publishedCountBadge->setText(QString::number(published.size()));

    if (published.isEmpty()) {
        m_publishedPanel->hide();
        return;
    }

    m_publishedPanel->show();

    // 只显示最新一条（任何人都能看到）
    int showCount = qMin(1, published.size());
    for (int i = 0; i < showCount; ++i) {
        const DiaryEntry& entry = published[i];

        QWidget* row = new QWidget();
        row->setStyleSheet(
            "background: white; border-radius: 14px; padding: 10px 14px;"
            "border: 1px solid #d4e8dc;");
        QHBoxLayout* rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(0, 0, 0, 0);

        // 信息区
        QVBoxLayout* infoLayout = new QVBoxLayout();
        QLabel* titleLabel = new QLabel(
            QString("📰 %1").arg(entry.preview()));
        titleLabel->setStyleSheet(
            "font-weight:600; font-size:12px; color:#1e4a6b;");
        titleLabel->setCursor(Qt::PointingHandCursor);
        infoLayout->addWidget(titleLabel);

        QString timeStr = entry.publishedAt.isEmpty()
                              ? entry.createdAt : entry.publishedAt;

        QString destInfo;
        if (!entry.trackPoints.isEmpty()) {
            QJsonObject last = entry.trackPoints.last();
            QString addr = entry.campusName.isEmpty() ? last["name"].toString()
                : entry.campusName + ", " + last["name"].toString();
            destInfo = QString(" · 🏁 %1").arg(addr);
        }

        QString authorStr = entry.userNickname.isEmpty() ? entry.userId : entry.userNickname;
        QString meta = QString("👤 %1 · 🏷️ %2 · 🕐 %3 · 📝 %4 字 · 🖼️ %5 张图%6")
                           .arg(authorStr)
                           .arg(entry.category)
                           .arg(timeStr)
                           .arg(entry.content.length())
                           .arg(entry.photos.size())
                           .arg(destInfo);
        QLabel* metaLabel = new QLabel(meta);
        metaLabel->setStyleSheet("font-size:10px; color:#7f9aaa;");
        infoLayout->addWidget(metaLabel);

        rowLayout->addLayout(infoLayout, 1);

        // 点击查看
        int idx = i;
        // 让整行可点击
        row->setCursor(Qt::PointingHandCursor);
        row->installEventFilter(this);

        QPushButton* delBtn = new QPushButton("🗑️");
        delBtn->setStyleSheet(
            "QPushButton { background:#fff2e6; border:none; border-radius:14px;"
            "padding:4px 10px; font-size:10px; color:#c2410c; }"
            "QPushButton:hover { background:#ffe0cc; }");
        connect(delBtn, &QPushButton::clicked, this, [this, idx]() {
            onDeletePublished(idx);
        });
        rowLayout->addWidget(delBtn);

        // 点击整行查看时间线
        // 使用 eventFilter 或直接给子控件安装
        // 简化：给信息布局中的标题安装点击事件
        // （由于 QLabel 不支持 clicked 信号，用 mousePressEvent 太复杂）
        // 这里我们增加一个"查看"按钮
        QPushButton* viewBtn = new QPushButton("👁 查看");
        viewBtn->setStyleSheet(
            "QPushButton { background:#e9f2f5; border:none; border-radius:14px;"
            "padding:4px 10px; font-size:10px; color:#1e4a6b; }"
            "QPushButton:hover { background:#d9e5f0; }");
        connect(viewBtn, &QPushButton::clicked, this, [this, idx]() {
            onViewTimeline(idx);
        });
        rowLayout->addWidget(viewBtn);

        m_publishedListLayout->addWidget(row);
    }
}
