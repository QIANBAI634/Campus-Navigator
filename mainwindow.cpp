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

    // 构建 UI
    applyGlobalStylesheet();
    setupUI();
    populateSelectors();
    updateStats();
    updateWordCount();
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
    ));
}

// ============================================================
// UI 构建
// ============================================================

void MainWindow::setupUI()
{
    // 滚动内容容器（模拟 HTML 中的白色卡片）
    m_scrollContent = new QWidget();
    m_scrollContent->setObjectName("scrollContent");
    m_scrollContent->setFixedWidth(690);
    m_scrollContent->setStyleSheet(
        "#scrollContent {"
        "  background: rgba(255,255,255,0.96);"
        "  border-radius: 40px;"
        "}"
    );

    QVBoxLayout* mainLayout = new QVBoxLayout(m_scrollContent);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 添加各面板
    mainLayout->addWidget(createHeaderPanel());
    mainLayout->addWidget(createNavigationPanel());

    // 分隔线
    QFrame* sep = new QFrame();
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("background:#cbdde6; max-height:1px; margin: 12px 28px;");
    mainLayout->addWidget(sep);

    mainLayout->addWidget(createDiaryPanel());
    mainLayout->addWidget(createDraftListPanel());
    mainLayout->addWidget(createPublishedPanel());

    mainLayout->addSpacing(20);

    // 底部提示
    QLabel* footer = new QLabel(
        "⚡ 基于真实校园路网 | 路口节点已隐藏 | Dijkstra 最短路径算法");
    footer->setAlignment(Qt::AlignCenter);
    footer->setStyleSheet(
        "font-size:11px; color:#5e7a8c; padding:8px 0 24px 0;");
    mainLayout->addWidget(footer);

    // 滚动区域包裹内容
    m_scrollArea = new QScrollArea();
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setWidget(m_scrollContent);
    m_scrollArea->setFixedWidth(720);  // 690 + 预留滚动条宽度

    // 外层居中容器：横向居中，纵向铺满
    QWidget* centerContainer = new QWidget(this);
    centerContainer->setStyleSheet("background: transparent;");
    QVBoxLayout* outerVLayout = new QVBoxLayout(centerContainer);
    outerVLayout->setContentsMargins(0, 0, 0, 0);
    QHBoxLayout* centerHLayout = new QHBoxLayout();
    centerHLayout->addStretch();
    centerHLayout->addWidget(m_scrollArea);
    centerHLayout->addStretch();
    outerVLayout->addLayout(centerHLayout);
    setCentralWidget(centerContainer);
}

// ============================================================
// 头部面板
// ============================================================

QWidget* MainWindow::createHeaderPanel()
{
    QWidget* header = new QWidget();
    header->setStyleSheet(
        "background: #0a2b3e;"
        "border-top-left-radius: 40px;"
        "border-top-right-radius: 40px;"
        "padding: 20px 24px;"
    );

    QVBoxLayout* layout = new QVBoxLayout(header);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(8);

    QLabel* title = new QLabel("📍 校园导航系统");
    title->setObjectName("heroTitle");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    QLabel* subtitle = new QLabel("基于 Dijkstra 的最短路径规划 | 200+ 景区/校园");
    subtitle->setObjectName("heroSubtitle");
    subtitle->setAlignment(Qt::AlignCenter);
    layout->addWidget(subtitle);

    // ---- 景区选择 + 用户切换 行 ----
    QHBoxLayout* topRow = new QHBoxLayout();
    topRow->setSpacing(12);

    // 景区/校园选择器
    QLabel* campusLabel = new QLabel("🏛️ 目的地");
    campusLabel->setStyleSheet(
        "color: rgba(255,255,255,0.9); font-size: 12px; font-weight: 600;");
    topRow->addWidget(campusLabel);

    m_campusSelect = new QComboBox();
    m_campusSelect->setMinimumWidth(180);
    m_campusSelect->setStyleSheet(
        "QComboBox { background: rgba(255,255,255,0.9); border-radius: 12px;"
        "padding: 6px 12px; font-size: 12px; color: #0a2b3e; }"
        "QComboBox:hover { background: white; }"
        "QComboBox QAbstractItemView { max-height: 300px; }");
    // 填充200个景区
    const auto& campuses = getAllCampuses();
    for (int i = 0; i < campuses.size(); ++i) {
        QString display = QString("%1  ★%2  %3")
            .arg(campuses[i].name)
            .arg(campuses[i].heat, 0, 'f', 1)
            .arg(campuses[i].city);
        m_campusSelect->addItem(display, i);
    }
    connect(m_campusSelect, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onCampusChanged);
    topRow->addWidget(m_campusSelect, 1);

    // 用户切换
    QLabel* userLabel = new QLabel("👤");
    userLabel->setStyleSheet("color: rgba(255,255,255,0.9); font-size: 12px;");
    topRow->addWidget(userLabel);

    m_userSelect = new QComboBox();
    m_userSelect->setMaximumWidth(130);
    m_userSelect->setStyleSheet(
        "QComboBox { background: rgba(255,255,255,0.9); border-radius: 12px;"
        "padding: 6px 10px; font-size: 12px; color: #0a2b3e; }"
        "QComboBox:hover { background: white; }");
    for (const auto& u : m_userManager.allUsers()) {
        m_userSelect->addItem(u.avatar + " " + u.nickname, u.userId);
    }
    connect(m_userSelect, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onUserChanged);
    topRow->addWidget(m_userSelect);

    layout->addLayout(topRow);

    // 统计信息
    m_statsLabel = new QLabel();
    m_statsLabel->setAlignment(Qt::AlignCenter);
    m_statsLabel->setStyleSheet(
        "color: rgba(255,255,255,0.9);"
        "font-size: 11px;"
        "font-weight: 600;"
        "background: rgba(255,255,255,0.15);"
        "border-radius: 20px;"
        "padding: 4px 16px;"
        "margin-top: 8px;"
    );
    layout->addWidget(m_statsLabel, 0, Qt::AlignCenter);

    return header;
}

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

    // ---- 按钮行：规划 + 完成导航 ----
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
    m_finishBtn->setEnabled(false);  // 初始没有导航路径，禁用
    connect(m_finishBtn, &QPushButton::clicked, this, &MainWindow::onFinishNavigation);
    btnLayout->addWidget(m_finishBtn, 1);

    layout->addLayout(btnLayout);

    // ---- 地图组件 ----
    m_mapWidget = new MapWidget();
    m_mapWidget->setGraph(&m_graph);
    // 依次尝试多个路径加载地图图片
    QStringList mapPaths = {
        QApplication::applicationDirPath() + "/北邮校园地图.jpg",
        QApplication::applicationDirPath() + "/../北邮校园地图.jpg",
        "C:/Users/37915/Desktop/CampusNav/北邮校园地图.jpg"
    };
    for (const QString& path : mapPaths) {
        if (m_mapWidget->loadMap(path)) break;
    }
    m_mapWidget->placeLandmarkDots();
    layout->addWidget(m_mapWidget);

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

    // 执行 Dijkstra 算法
    auto [dist, prev] = m_graph.dijkstra(startIdx, endIdx);
    double totalDist = dist[endIdx];

    if (!std::isfinite(totalDist)) {
        m_distanceLabel->setText("📏 路径距离: 不可达");
        m_pathDisplay->setStyleSheet(
            "background:#fff2e6; color:#c2410c;"
            "border-radius:14px; padding:12px 16px; font-size:13px;");
        m_pathDisplay->setText("🚫 路径不可达");
        return;
    }

    // 重构路径
    QVector<int> fullPath = m_graph.reconstructPath(prev, startIdx, endIdx);
    if (fullPath.isEmpty()) {
        m_distanceLabel->setText("📏 路径距离: 不可达");
        m_pathDisplay->setText("路径重建失败");
        return;
    }

    // 过滤只显示地标
    QVector<int> landmarkPath = m_graph.filterLandmarkPath(fullPath);
    QString displayStr = m_graph.formatPathDisplay(landmarkPath);

    m_distanceLabel->setText(
        QString("📏 最短距离: %1 米 (基于真实路网)")
            .arg(static_cast<int>(totalDist)));

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

        // 头部
        QHBoxLayout* cardHeader = new QHBoxLayout();
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
