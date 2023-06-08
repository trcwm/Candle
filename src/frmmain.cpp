// This file is a part of "Candle" application.
// Copyright 2015-2016 Hayrullin Denis Ravilevich
// Copyright 2023 Niels Moseley

// #define INITTIME //QTime time; time.start();
// #define PRINTTIME(x) //qDebug() << "time elapse" << QString("%1:").arg(x) << time.elapsed(); time.start();

#define PROGRESSMINLINES 10000
#define PROGRESSSTEP 1000

#include <QFileDialog>
#include <QTextStream>
#include <QDebug>
#include <QStringList>
#include <QTextBlock>
#include <QTextCursor>
#include <QMessageBox>
#include <QComboBox>
#include <QScrollBar>
#include <QShortcut>
#include <QAction>
#include <QLayout>
#include <QMimeData>
#include <QElapsedTimer>

#include "widgets/positiondisplay.h"

#include "frmmain.h"
#include "ui_frmmain.h"

frmMain::frmMain(QWidget *parent) : QMainWindow(parent),
                                    ui(new Ui::frmMain)
{
    // Loading settings
    m_settingsFileName = qApp->applicationDirPath() + "/settings.ini";
    preloadSettings();

    m_settings = new frmSettings(this);
    ui->setupUi(this);

#ifdef WINDOWS
    if (QSysInfo::windowsVersion() >= QSysInfo::WV_WINDOWS7)
    {
        m_taskBarButton = NULL;
        m_taskBarProgress = NULL;
    }
#endif

#ifndef UNIX
    // FIXME: console
    // ui->cboCommand->setStyleSheet("QComboBox {padding: 2;} QComboBox::drop-down {width: 0; border-style: none;} QComboBox::down-arrow {image: url(noimg);	border-width: 0;}");
#endif
    //    ui->scrollArea->updateMinimumWidth();

    m_lastDrawnLineIndex = 0;
    m_fileProcessedCommandIndex = 0;
    m_cellChanged = false;
    m_programLoading = false;
    m_currentModel = &m_programModel;
    m_transferCompleted = true;

    ui->cmdFit->setParent(ui->glwVisualizer);
    ui->cmdIsometric->setParent(ui->glwVisualizer);
    ui->cmdTop->setParent(ui->glwVisualizer);
    ui->cmdFront->setParent(ui->glwVisualizer);
    ui->cmdLeft->setParent(ui->glwVisualizer);

#if 0
    connect(ui->cboJogStep, &ComboBoxKey::currentTextChanged, [=](QString)
            { updateJogTitle(); });
    connect(ui->cboJogFeed, &ComboBoxKey::currentTextChanged, [=](QString)
            { updateJogTitle(); });
#endif

    // Prepare "Send"-button
    ui->cmdFileSend->setMinimumWidth(qMax(ui->cmdFileSend->width(), ui->cmdFileOpen->width()));
    QMenu *menuSend = new QMenu();
    menuSend->addAction(tr("Send from current line"), this, SLOT(onActSendFromLineTriggered()));
    ui->cmdFileSend->setMenu(menuSend);

    foreach (StyledToolButton *button, this->findChildren<StyledToolButton *>(QRegularExpression("cmdUser\\d")))
    {
        connect(button, SIGNAL(clicked(bool)), this, SLOT(onCmdUserClicked(bool)));
    }

    m_spindleTab = new GUI::SpindleTab();
    connect(m_spindleTab, &GUI::SpindleTab::toggled, this, &frmMain::onCmdSpindleToggled);
    connect(m_spindleTab, &GUI::SpindleTab::clicked, this, &frmMain::onCmdSpindleClicked);

    // Setting up slider boxes
    m_overrideTab = new GUI::OverrideTab();
    m_overrideTab->feed()->setRatio(1);
    m_overrideTab->feed()->setMinimum(10);
    m_overrideTab->feed()->setMaximum(200);
    m_overrideTab->feed()->setCurrentValue(100);
    m_overrideTab->feed()->setTitle(tr("Feed rate:"));
    m_overrideTab->feed()->setSuffix("%");
    // connect(m_overrideTab->feed(), SIGNAL(toggled(bool)), this, SLOT(onOverridingToggled(bool)));
    connect(m_overrideTab->feed(), &SliderBox::toggled, [=]
            { updateProgramEstimatedTime(m_currentDrawer->viewParser()->getLineSegmentList()); });
    connect(m_overrideTab->feed(), &SliderBox::valueChanged, [=]
            { updateProgramEstimatedTime(m_currentDrawer->viewParser()->getLineSegmentList()); });

    m_overrideTab->rapid()->setRatio(50);
    m_overrideTab->rapid()->setMinimum(25);
    m_overrideTab->rapid()->setMaximum(100);
    m_overrideTab->rapid()->setCurrentValue(100);
    m_overrideTab->rapid()->setTitle(tr("Rapid speed:"));
    m_overrideTab->rapid()->setSuffix("%");
    // connect(m_overrideTab->rapid(), SIGNAL(toggled(bool)), this, SLOT(onOverridingToggled(bool)));

    connect(m_overrideTab->rapid(), &SliderBox::toggled, [=]
            { updateProgramEstimatedTime(m_currentDrawer->viewParser()->getLineSegmentList()); });
    connect(m_overrideTab->rapid(), &SliderBox::valueChanged, [=]
            { updateProgramEstimatedTime(m_currentDrawer->viewParser()->getLineSegmentList()); });

    m_overrideTab->spindle()->setRatio(1);
    m_overrideTab->spindle()->setMinimum(50);
    m_overrideTab->spindle()->setMaximum(200);
    m_overrideTab->spindle()->setCurrentValue(100);
    m_overrideTab->spindle()->setTitle(tr("Spindle speed:"));
    m_overrideTab->spindle()->setSuffix("%");
    // connect(m_overrideTab->spindle(), SIGNAL(toggled(bool)), this, SLOT(onOverridingToggled(bool)));

    m_originDrawer = new OriginDrawer();
    m_codeDrawer = new GcodeDrawer();
    m_codeDrawer->setViewParser(&m_viewParser);
    m_probeDrawer = new GcodeDrawer();
    m_probeDrawer->setViewParser(&m_probeParser);
    m_probeDrawer->setVisible(false);
    m_currentDrawer = m_codeDrawer;
    m_toolDrawer.setToolPosition(QVector3D(0, 0, 0));

    QShortcut *insertShortcut = new QShortcut(QKeySequence(Qt::Key_Insert), ui->tblProgram);
    QShortcut *deleteShortcut = new QShortcut(QKeySequence(Qt::Key_Delete), ui->tblProgram);

    connect(insertShortcut, SIGNAL(activated()), this, SLOT(onTableInsertLine()));
    connect(deleteShortcut, SIGNAL(activated()), this, SLOT(onTableDeleteLines()));

    m_tableMenu = new QMenu(this);
    m_tableMenu->addAction(tr("&Insert line"), this, SLOT(onTableInsertLine()), insertShortcut->key());
    m_tableMenu->addAction(tr("&Delete lines"), this, SLOT(onTableDeleteLines()), deleteShortcut->key());

    ui->glwVisualizer->addDrawable(m_originDrawer);
    ui->glwVisualizer->addDrawable(m_codeDrawer);
    ui->glwVisualizer->addDrawable(m_probeDrawer);
    ui->glwVisualizer->addDrawable(&m_toolDrawer);
    ui->glwVisualizer->addDrawable(&m_selectionDrawer);
    ui->glwVisualizer->fitDrawable();

    connect(ui->glwVisualizer, SIGNAL(rotationChanged()), this, SLOT(onVisualizatorRotationChanged()));
    connect(ui->glwVisualizer, SIGNAL(resized()), this, SLOT(placeVisualizerButtons()));
    connect(&m_programModel, SIGNAL(dataChanged(QModelIndex, QModelIndex)), this, SLOT(onTableCellChanged(QModelIndex, QModelIndex)));
    connect(&m_probeModel, SIGNAL(dataChanged(QModelIndex, QModelIndex)), this, SLOT(onTableCellChanged(QModelIndex, QModelIndex)));

    ui->tblProgram->setModel(&m_programModel);
    ui->tblProgram->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    connect(ui->tblProgram->verticalScrollBar(), SIGNAL(actionTriggered(int)), this, SLOT(onScroolBarAction(int)));
    connect(ui->tblProgram->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)), this, SLOT(onTableCurrentChanged(QModelIndex, QModelIndex)));
    clearTable();

    m_senderErrorBox = new QMessageBox(QMessageBox::Warning, qApp->applicationDisplayName(), QString(),
                                       QMessageBox::Ignore | QMessageBox::Abort, this);
    m_senderErrorBox->setCheckBox(new QCheckBox(tr("Don't show again")));

    // create the status widget before loadSettings
    // to avoid nullptr dereference..
    m_statusWidget = new GUI::StatusWidget();
    m_consoleTab = new GUI::ConsoleTab();
    connect(m_consoleTab, &GUI::ConsoleTab::returnPressed, this, &frmMain::onCboCommandReturnPressed);

    m_jogWidget = new GUI::JogWidget();
    connect(m_jogWidget, &GUI::JogWidget::jogVectorChanged, this, &frmMain::onJogVectorChanged);
    connect(m_jogWidget, &GUI::JogWidget::stopClicked, this, &frmMain::onJogStopClicked);

    m_buttonBar = new GUI::ButtonBar();
    addToolBar(m_buttonBar);
    connect(m_buttonBar, &GUI::ButtonBar::cmdHome, this, &frmMain::onCmdHome_clicked);
    connect(m_buttonBar, &GUI::ButtonBar::cmdRestart, this, &frmMain::onCmdReset_clicked);
    connect(m_buttonBar, &GUI::ButtonBar::cmdUnlock, this, &frmMain::onCmdUnlock_clicked);
    connect(m_buttonBar, &GUI::ButtonBar::cmdProbeZ, this, &frmMain::onCmdTouch_clicked);
    connect(m_buttonBar, &GUI::ButtonBar::cmdZeroXY, this, &frmMain::onCmdZeroXY_clicked);
    connect(m_buttonBar, &GUI::ButtonBar::cmdZeroZ, this, &frmMain::onCmdZeroZ_clicked);
    connect(m_buttonBar, &GUI::ButtonBar::cmdOrigin, this, &frmMain::onCmdRestoreOrigin_clicked);

    // Loading settings
    loadSettings();
    ui->tblProgram->hideColumn(4);
    ui->tblProgram->hideColumn(5);
    updateControlsState();

#if 0
    // FIXME: job
    // Prepare jog buttons
    foreach (StyledToolButton *button, ui->grpJog->findChildren<StyledToolButton *>(QRegularExpression("cmdJogFeed\\d")))
    {
        connect(button, SIGNAL(clicked(bool)), this, SLOT(onCmdJogFeedClicked()));
    }
#endif

    // Setting up spindle slider box
    connect(m_spindleTab->slider(), &SliderBox::valueUserChanged, [=]
            { m_updateSpindleSpeed = true; });

    // Setup serial port
    m_serialPort.setParity(QSerialPort::NoParity);
    m_serialPort.setDataBits(QSerialPort::Data8);
    m_serialPort.setFlowControl(QSerialPort::NoFlowControl);
    m_serialPort.setStopBits(QSerialPort::OneStop);

    if (m_settings->port() != "")
    {
        m_serialPort.setPortName(m_settings->port());
        m_serialPort.setBaudRate(m_settings->baud());
    }

    connect(&m_serialPort, SIGNAL(readyRead()), this, SLOT(onSerialPortReadyRead()), Qt::QueuedConnection);
    connect(&m_serialPort, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(onSerialPortError(QSerialPort::SerialPortError)));

    this->installEventFilter(this);
    ui->tblProgram->installEventFilter(this);

    // FIXME: jog
    // ui->cboJogStep->installEventFilter(this);
    // ui->cboJogFeed->installEventFilter(this);

    connect(&m_timerConnection, SIGNAL(timeout()), this, SLOT(onTimerConnection()));
    connect(&m_timerStateQuery, SIGNAL(timeout()), this, SLOT(onTimerStateQuery()));
    m_timerConnection.start(1000);
    m_timerStateQuery.start();

    // Handle file drop
    if (qApp->arguments().count() > 1 && isGCodeFile(qApp->arguments().last()))
    {
        loadFile(qApp->arguments().last());
    }

    auto *displayBoxLayout = new QVBoxLayout();
    m_machineDisplay = new GUI::PositionDisplay(tr("Machine"));
    m_workDisplay = new GUI::PositionDisplay(tr("Work"));

    displayBoxLayout->addWidget(m_jogWidget);
    displayBoxLayout->addWidget(m_workDisplay);
    displayBoxLayout->addWidget(m_machineDisplay);
    displayBoxLayout->addWidget(m_statusWidget);

    m_tabWidget = new QTabWidget();
    m_tabWidget->addTab(m_consoleTab, tr("Console"));
    m_tabWidget->addTab(m_overrideTab, tr("Override"));
    m_tabWidget->addTab(m_spindleTab, tr("Spindle"));

    displayBoxLayout->addWidget(m_tabWidget);

    ui->verticalLayout_2->addLayout(displayBoxLayout);
}

frmMain::~frmMain()
{
    saveSettings();

    delete m_senderErrorBox;
    delete ui;
}

bool frmMain::isGCodeFile(QString fileName)
{
    return fileName.endsWith(".txt", Qt::CaseInsensitive) || fileName.endsWith(".nc", Qt::CaseInsensitive) || fileName.endsWith(".ncc", Qt::CaseInsensitive) || fileName.endsWith(".ngc", Qt::CaseInsensitive) || fileName.endsWith(".tap", Qt::CaseInsensitive);
}

double frmMain::toolZPosition()
{
    return m_toolDrawer.toolPosition().z();
}

void frmMain::preloadSettings()
{
    QSettings set(m_settingsFileName, QSettings::IniFormat);
    set.setIniCodec("UTF-8");

    qApp->setStyleSheet(QString(qApp->styleSheet()).replace(QRegExp("font-size:\\s*\\d+"), "font-size: " + set.value("fontSize", "8").toString()));

    // Update v-sync in glformat
    QGLFormat fmt = QGLFormat::defaultFormat();
    fmt.setSwapInterval(set.value("vsync", false).toBool() ? 1 : 0);
    QGLFormat::setDefaultFormat(fmt);
}

void frmMain::loadSettings()
{
    QSettings set(m_settingsFileName, QSettings::IniFormat);
    set.setIniCodec("UTF-8");

    m_settingsLoading = true;

    m_settings->setFontSize(set.value("fontSize", 8).toInt());
    m_settings->setPort(set.value("port").toString());
    m_settings->setBaud(set.value("baud").toInt());
    m_settings->setIgnoreErrors(set.value("ignoreErrors", false).toBool());
    m_settings->setAutoLine(set.value("autoLine", true).toBool());
    m_settings->setToolDiameter(set.value("toolDiameter", 3).toDouble());
    m_settings->setToolLength(set.value("toolLength", 15).toDouble());
    m_settings->setAntialiasing(set.value("antialiasing", true).toBool());
    m_settings->setMsaa(set.value("msaa", true).toBool());
    m_settings->setVsync(set.value("vsync", false).toBool());
    m_settings->setZBuffer(set.value("zBuffer", false).toBool());
    m_settings->setSimplify(set.value("simplify", false).toBool());
    m_settings->setSimplifyPrecision(set.value("simplifyPrecision", 0).toDouble());
    m_settings->setGrayscaleSegments(set.value("grayscaleSegments", false).toBool());
    m_settings->setGrayscaleSCode(set.value("grayscaleSCode", true).toBool());
    m_settings->setDrawModeVectors(set.value("drawModeVectors", true).toBool());
    m_settings->setMoveOnRestore(set.value("moveOnRestore", false).toBool());
    m_settings->setRestoreMode(set.value("restoreMode", 0).toInt());
    m_settings->setLineWidth(set.value("lineWidth", 1).toDouble());
    m_settings->setArcLength(set.value("arcLength", 0).toDouble());
    m_settings->setArcDegree(set.value("arcDegree", 0).toDouble());
    m_settings->setArcDegreeMode(set.value("arcDegreeMode", true).toBool());
    m_settings->setShowProgramCommands(set.value("showProgramCommands", 0).toBool());
    m_settings->setShowUICommands(set.value("showUICommands", 0).toBool());
    m_settings->setSpindleSpeedMin(set.value("spindleSpeedMin", 0).toInt());
    m_settings->setSpindleSpeedMax(set.value("spindleSpeedMax", 100).toInt());
    m_settings->setLaserPowerMin(set.value("laserPowerMin", 0).toInt());
    m_settings->setLaserPowerMax(set.value("laserPowerMax", 100).toInt());
    m_settings->setRapidSpeed(set.value("rapidSpeed", 0).toInt());
    m_settings->setHeightmapProbingFeed(set.value("heightmapProbingFeed", 0).toInt());
    m_settings->setAcceleration(set.value("acceleration", 10).toInt());
    m_settings->setToolAngle(set.value("toolAngle", 0).toDouble());
    m_settings->setToolType(set.value("toolType", 0).toInt());
    m_settings->setFps(set.value("fps", 60).toInt());
    m_settings->setQueryStateTime(set.value("queryStateTime", 250).toInt());

    m_settings->setPanelUserCommands(set.value("panelUserCommandsVisible", true).toBool());
    m_settings->setPanelHeightmap(set.value("panelHeightmapVisible", true).toBool());
    m_settings->setPanelSpindle(set.value("panelSpindleVisible", true).toBool());
    m_settings->setPanelOverriding(set.value("panelOverridingVisible", true).toBool());
    m_settings->setPanelJog(set.value("panelJogVisible", true).toBool());

    // FIXME:
    m_consoleTab->setMinimumHeight(set.value("consoleMinHeight", 100).toInt());

    ui->chkAutoScroll->setChecked(set.value("autoScroll", false).toBool());

    m_spindleTab->setMinSpindleSpeed(m_settings->spindleSpeedMin());
    m_spindleTab->setMaxSpindleSpeed(m_settings->spindleSpeedMax());
    m_spindleTab->setValue(set.value("spindleSpeed", 100).toInt());

    m_overrideTab->feed()->setChecked(set.value("feedOverride", false).toBool());
    m_overrideTab->feed()->setValue(set.value("feedOverrideValue", 100).toInt());

    m_overrideTab->rapid()->setChecked(set.value("rapidOverride", false).toBool());
    m_overrideTab->rapid()->setValue(set.value("rapidOverrideValue", 100).toInt());

    m_overrideTab->spindle()->setChecked(set.value("spindleOverride", false).toBool());
    m_overrideTab->spindle()->setValue(set.value("spindleOverrideValue", 100).toInt());

    m_settings->setUnits(set.value("units", 0).toInt());
    m_storedX = set.value("storedX", 0).toDouble();
    m_storedY = set.value("storedY", 0).toDouble();
    m_storedZ = set.value("storedZ", 0).toDouble();

    m_buttonBar->setOriginTooltip(QString(tr("Restore origin:\n%1, %2, %3")).arg(m_storedX).arg(m_storedY).arg(m_storedZ));

    m_recentFiles = set.value("recentFiles", QStringList()).toStringList();
    m_recentHeightmaps = set.value("recentHeightmaps", QStringList()).toStringList();
    m_lastFolder = set.value("lastFolder", QDir::homePath()).toString();

    this->restoreGeometry(set.value("formGeometry", QByteArray()).toByteArray());
    m_settings->resize(set.value("formSettingsSize", m_settings->size()).toSize());
    QByteArray splitterState = set.value("splitter", QByteArray()).toByteArray();

    if (splitterState.length() == 0)
    {
        ui->splitter->setStretchFactor(0, 1);
        ui->splitter->setStretchFactor(1, 1);
    }
    else
        ui->splitter->restoreState(splitterState);

    ui->chkAutoScroll->setVisible(ui->splitter->sizes()[1]);
    resizeCheckBoxes();

    // FIXME: console
    // ui->cboCommand->setMinimumHeight(ui->cboCommand->height());
    // ui->cmdClearConsole->setFixedHeight(ui->cboCommand->height());
    // ui->cmdCommandSend->setFixedHeight(ui->cboCommand->height());

    m_storedKeyboardControl = set.value("keyboardControl", false).toBool();

    m_settings->setAutoCompletion(set.value("autoCompletion", true).toBool());
    m_settings->setTouchCommand(set.value("touchCommand").toString());
    m_settings->setSafePositionCommand(set.value("safePositionCommand").toString());

    foreach (StyledToolButton *button, this->findChildren<StyledToolButton *>(QRegularExpression("cmdUser\\d")))
    {
        int i = button->objectName().right(1).toInt();
        m_settings->setUserCommands(i, set.value(QString("userCommands%1").arg(i)).toString());
    }

    // FIXME:
#if 0
    ui->cboJogStep->setItems(set.value("jogSteps").toStringList());
    ui->cboJogStep->setCurrentIndex(ui->cboJogStep->findText(set.value("jogStep").toString()));
    ui->cboJogFeed->setItems(set.value("jogFeeds").toStringList());
    ui->cboJogFeed->setCurrentIndex(ui->cboJogFeed->findText(set.value("jogFeed").toString()));
#else
    m_jogWidget->setStepItems(set.value("jogSteps").toStringList());
    m_jogWidget->setFeedItems(set.value("jogFeeds").toStringList());
#endif

    foreach (ColorPicker *pick, m_settings->colors())
    {
        pick->setColor(QColor(set.value(pick->objectName().mid(3), "black").toString()));
    }

    updateRecentFilesMenu();

    ui->tblProgram->horizontalHeader()->restoreState(set.value("header", QByteArray()).toByteArray());

    // Update right panel width
    applySettings();
    show();

    // ui->scrollArea->updateMinimumWidth();

    // Restore last commands list
    // FIXME: console
    // ui->cboCommand->addItems(set.value("recentCommands", QStringList()).toStringList());
    // ui->cboCommand->setCurrentIndex(-1);

    m_settingsLoading = false;
}

void frmMain::saveSettings()
{
    QSettings set(m_settingsFileName, QSettings::IniFormat);
    set.setIniCodec("UTF-8");

    set.setValue("port", m_settings->port());
    set.setValue("baud", m_settings->baud());
    set.setValue("ignoreErrors", m_settings->ignoreErrors());
    set.setValue("autoLine", m_settings->autoLine());
    set.setValue("toolDiameter", m_settings->toolDiameter());
    set.setValue("toolLength", m_settings->toolLength());
    set.setValue("antialiasing", m_settings->antialiasing());
    set.setValue("msaa", m_settings->msaa());
    set.setValue("vsync", m_settings->vsync());
    set.setValue("zBuffer", m_settings->zBuffer());
    set.setValue("simplify", m_settings->simplify());
    set.setValue("simplifyPrecision", m_settings->simplifyPrecision());
    set.setValue("grayscaleSegments", m_settings->grayscaleSegments());
    set.setValue("grayscaleSCode", m_settings->grayscaleSCode());
    set.setValue("drawModeVectors", m_settings->drawModeVectors());

    set.setValue("spindleSpeed", m_spindleTab->value());
    set.setValue("lineWidth", m_settings->lineWidth());
    set.setValue("arcLength", m_settings->arcLength());
    set.setValue("arcDegree", m_settings->arcDegree());
    set.setValue("arcDegreeMode", m_settings->arcDegreeMode());
    set.setValue("showProgramCommands", m_settings->showProgramCommands());
    set.setValue("showUICommands", m_settings->showUICommands());
    set.setValue("spindleSpeedMin", m_settings->spindleSpeedMin());
    set.setValue("spindleSpeedMax", m_settings->spindleSpeedMax());
    set.setValue("laserPowerMin", m_settings->laserPowerMin());
    set.setValue("laserPowerMax", m_settings->laserPowerMax());
    set.setValue("moveOnRestore", m_settings->moveOnRestore());
    set.setValue("restoreMode", m_settings->restoreMode());
    set.setValue("rapidSpeed", m_settings->rapidSpeed());
    set.setValue("heightmapProbingFeed", m_settings->heightmapProbingFeed());
    set.setValue("acceleration", m_settings->acceleration());
    set.setValue("toolAngle", m_settings->toolAngle());
    set.setValue("toolType", m_settings->toolType());
    set.setValue("fps", m_settings->fps());
    set.setValue("queryStateTime", m_settings->queryStateTime());
    set.setValue("autoScroll", ui->chkAutoScroll->isChecked());
    set.setValue("header", ui->tblProgram->horizontalHeader()->saveState());
    set.setValue("splitter", ui->splitter->saveState());
    set.setValue("formGeometry", this->saveGeometry());
    set.setValue("formSettingsSize", m_settings->size());

    // FIXME: Jog
    // set.setValue("keyboardControl", ui->chkKeyboardControl->isChecked());
    set.setValue("autoCompletion", m_settings->autoCompletion());
    set.setValue("units", m_settings->units());
    set.setValue("storedX", m_storedX);
    set.setValue("storedY", m_storedY);
    set.setValue("storedZ", m_storedZ);
    set.setValue("recentFiles", m_recentFiles);
    set.setValue("recentHeightmaps", m_recentHeightmaps);
    set.setValue("lastFolder", m_lastFolder);
    set.setValue("touchCommand", m_settings->touchCommand());
    set.setValue("safePositionCommand", m_settings->safePositionCommand());
    set.setValue("panelUserCommandsVisible", m_settings->panelUserCommands());
    set.setValue("panelHeightmapVisible", m_settings->panelHeightmap());
    set.setValue("panelSpindleVisible", m_settings->panelSpindle());
    set.setValue("panelOverridingVisible", m_settings->panelOverriding());
    set.setValue("panelJogVisible", m_settings->panelJog());
    set.setValue("fontSize", m_settings->fontSize());
    set.setValue("consoleMinHeight", m_consoleTab->minimumHeight());

    set.setValue("feedOverride", m_overrideTab->feed()->isChecked());
    set.setValue("feedOverrideValue", m_overrideTab->feed()->value());
    set.setValue("rapidOverride", m_overrideTab->rapid()->isChecked());
    set.setValue("rapidOverrideValue", m_overrideTab->rapid()->value());
    set.setValue("spindleOverride", m_overrideTab->spindle()->isChecked());
    set.setValue("spindleOverrideValue", m_overrideTab->spindle()->value());

    foreach (StyledToolButton *button, findChildren<StyledToolButton *>(QRegularExpression("cmdUser\\d")))
    {
        int i = button->objectName().right(1).toInt();
        set.setValue(QString("userCommands%1").arg(i), m_settings->userCommands(i));
    }

    // FIXME:
    set.setValue("jogSteps", m_jogWidget->getStepItems());
    // set.setValue("jogStep", ui->cboJogStep->currentText());
    set.setValue("jogFeeds", m_jogWidget->getFeedItems());
    // set.setValue("jogFeed", ui->cboJogFeed->currentText());

    foreach (ColorPicker *pick, m_settings->colors())
    {
        set.setValue(pick->objectName().mid(3), pick->color().name());
    }

    QStringList list;

    // FIXME: console
    // for (int i = 0; i < ui->cboCommand->count(); i++)
    //    list.append(ui->cboCommand->itemText(i));
    // set.setValue("recentCommands", list);
}

bool frmMain::saveChanges(bool heightMapMode)
{
    if ((!heightMapMode && m_fileChanged))
    {
        int res = QMessageBox::warning(this, this->windowTitle(), tr("G-code program file was changed. Save?"),
                                       QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if (res == QMessageBox::Cancel)
            return false;
        else if (res == QMessageBox::Yes)
            on_actFileSave_triggered();
        m_fileChanged = false;
    }

    return true;
}

void frmMain::updateControlsState()
{
    bool portOpened = m_serialPort.isOpen();

    m_buttonBar->setEnabled(portOpened);
    m_buttonBar->enableUserButtons(portOpened && !m_processingFile);
    m_spindleTab->setEnabled(portOpened);

    m_jogWidget->setEnabled(portOpened && !m_processingFile);
    m_consoleTab->setEnabled(portOpened);
    // ui->cboCommand->setEnabled(portOpened && (!ui->chkKeyboardControl->isChecked()));
    // ui->cmdCommandSend->setEnabled(portOpened);
    //     ui->widgetFeed->setEnabled(!m_transferringFile);

    ui->chkTestMode->setEnabled(portOpened && !m_processingFile);
    m_buttonBar->enableControlButtons(!m_processingFile);

    m_spindleTab->enableButton(!m_processingFile);

    ui->actFileNew->setEnabled(!m_processingFile);
    ui->actFileOpen->setEnabled(!m_processingFile);
    ui->cmdFileOpen->setEnabled(!m_processingFile);
    ui->cmdFileReset->setEnabled(!m_processingFile && m_programModel.rowCount() > 1);
    ui->cmdFileSend->setEnabled(portOpened && !m_processingFile && m_programModel.rowCount() > 1);
    ui->cmdFilePause->setEnabled(m_processingFile && !ui->chkTestMode->isChecked());
    ui->cmdFileAbort->setEnabled(m_processingFile);
    ui->actFileOpen->setEnabled(!m_processingFile);
    ui->mnuRecent->setEnabled(!m_processingFile && (m_recentFiles.count() > 0));
    ui->actFileSave->setEnabled(m_programModel.rowCount() > 1);
    ui->actFileSaveAs->setEnabled(m_programModel.rowCount() > 1);

    ui->tblProgram->setEditTriggers(m_processingFile ? QAbstractItemView::NoEditTriggers : QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked | QAbstractItemView::EditKeyPressed | QAbstractItemView::AnyKeyPressed);

    if (!portOpened)
    {
        m_statusWidget->setStatus(StatusType::NOTCONNECTED);
    }

    setWindowTitle(m_programFileName.isEmpty() ? qApp->applicationDisplayName()
                                               : m_programFileName.mid(m_programFileName.lastIndexOf("/") + 1) + " - " + qApp->applicationDisplayName());

    // FIXME: jog
    // if (!m_processingFile)
    //    ui->chkKeyboardControl->setChecked(m_storedKeyboardControl);

#ifdef WINDOWS
    if (QSysInfo::windowsVersion() >= QSysInfo::WV_WINDOWS7)
    {
        if (!m_processingFile && m_taskBarProgress)
            m_taskBarProgress->hide();
    }
#endif

    style()->unpolish(ui->cmdFileOpen);
    style()->unpolish(ui->cmdFileReset);
    style()->unpolish(ui->cmdFileSend);
    style()->unpolish(ui->cmdFilePause);
    style()->unpolish(ui->cmdFileAbort);
    ui->cmdFileOpen->ensurePolished();
    ui->cmdFileReset->ensurePolished();
    ui->cmdFileSend->ensurePolished();
    ui->cmdFilePause->ensurePolished();
    ui->cmdFileAbort->ensurePolished();

    // ui->grpProgram->setTitle(m_heightMapMode ? tr("Heightmap") : tr("G-code program"));
    ui->grpProgram->setTitle(tr("G-code program"));

    style()->unpolish(ui->grpProgram);
    ui->grpProgram->ensurePolished();

    // FIXME: jog
    // ui->grpHeightMapSettings->setEnabled(!m_processingFile && !ui->chkKeyboardControl->isChecked());

    ui->chkTestMode->setVisible(true);
    ui->chkAutoScroll->setVisible(ui->splitter->sizes()[1]);
    ui->tblProgram->setVisible(true);

    ui->cmdFileSend->setText(tr("Send"));
    ui->actFileSaveTransformedAs->setVisible(false);
    // ui->cmdFileSend->menu()->actions().first()->setEnabled(!ui->cmdHeightMapMode->isChecked());

    m_selectionDrawer.setVisible(true);
}

void frmMain::openPort()
{
    if (m_serialPort.open(QIODevice::ReadWrite))
    {
        m_statusWidget->setStatus(StatusType::PORTOPEN);
        grblReset();
    }
}

void frmMain::sendCommand(QString command, int tableIndex, bool showInConsole)
{
    if (!m_serialPort.isOpen() || !m_resetCompleted)
        return;

    command = command.toUpper();

    // Commands queue
    if ((bufferLength() + command.length() + 1) > BUFFERLENGTH)
    {
        //        qDebug() << "queue:" << command;

        CommandQueue cq;

        cq.command = command;
        cq.tableIndex = tableIndex;
        cq.showInConsole = showInConsole;

        m_queue.append(cq);
        return;
    }

    CommandAttributes ca;

    //    if (!(command == "$G" && tableIndex < -1) && !(command == "$#" && tableIndex < -1)
    //            && (!m_transferringFile || (m_transferringFile && m_showAllCommands) || tableIndex < 0)) {
    if (showInConsole)
    {
        m_consoleTab->appendPlainText(command);
        ca.consoleIndex = m_consoleTab->blockCount() - 1;
    }
    else
    {
        ca.consoleIndex = -1;
    }

    ca.command = command;
    ca.length = command.length() + 1;
    ca.tableIndex = tableIndex;

    m_commands.append(ca);

    // Processing spindle speed only from g-code program
    QRegExp s("[Ss]0*(\\d+)");
    if (s.indexIn(command) != -1 && ca.tableIndex > -2)
    {
        int speed = s.cap(1).toInt();
        if (m_spindleTab->value() != speed)
        {
            m_spindleTab->setValue(speed);
        }
    }

    // Set M2 & M30 commands sent flag
    if (command.contains(QRegExp("M0*2|M30")))
    {
        m_fileEndSent = true;
    }

    m_serialPort.write((command + "\r").toLatin1());
}

void frmMain::grblReset()
{
    qDebug() << "grbl reset";

    m_serialPort.write(QByteArray(1, (char)24));
    //    m_serialPort.flush();

    m_processingFile = false;
    m_transferCompleted = true;
    m_fileCommandIndex = 0;

    m_reseting = true;
    m_homing = false;
    m_resetCompleted = false;
    m_updateSpindleSpeed = true;
    m_statusWidget->reset();
    m_statusReceived = true;

    // Drop all remaining commands in buffer
    m_commands.clear();
    m_queue.clear();

    // Prepare reset response catch
    CommandAttributes ca;
    ca.command = "[CTRL+X]";
    if (m_settings->showUICommands())
    {
        m_consoleTab->appendPlainText(ca.command);
    }

    ca.consoleIndex = m_settings->showUICommands() ? m_consoleTab->blockCount() - 1 : -1;
    ca.tableIndex = -1;
    ca.length = ca.command.length() + 1;
    m_commands.append(ca);

    updateControlsState();
}

int frmMain::bufferLength()
{
    int length = 0;

    foreach (CommandAttributes ca, m_commands)
    {
        length += ca.length;
    }

    return length;
}

void frmMain::onSerialPortReadyRead()
{
    while (m_serialPort.canReadLine())
    {
        QString data = m_serialPort.readLine().trimmed();

        // Filter prereset responses
        if (m_reseting)
        {
            qDebug() << "reseting filter:" << data;
            if (!dataIsReset(data))
                continue;
            else
            {
                m_reseting = false;
                m_timerStateQuery.setInterval(m_settings->queryStateTime());
            }
        }

        // Status response
        if (data[0] == '<')
        {
            StatusType status = StatusType::UNKNOWN;

            m_statusReceived = true;

            // Update machine coordinates
            static QRegExp mpx("MPos:([^,]*),([^,]*),([^,^>^|]*)");
            if (mpx.indexIn(data) != -1)
            {
                m_machineDisplay->setPosition(mpx.cap(1).toFloat(),
                                              mpx.cap(2).toFloat(),
                                              mpx.cap(3).toFloat());
            }

            // Status
            static QRegExp stx("<([^,^>^|]*)");
            if (stx.indexIn(data) != -1)
            {
                status = GUI::StatusWidget::statusFromString(stx.cap(1));
                auto prevStatus = m_statusWidget->getStatus();

                m_statusWidget->setStatus(status);

                // Update controls
                m_buttonBar->enablePositionButtons(status == StatusType::IDLE);
                ui->chkTestMode->setEnabled(status != StatusType::RUN && !m_processingFile);
                ui->chkTestMode->setChecked(status == StatusType::CHECK);
                ui->cmdFilePause->setChecked(status == StatusType::HOLD0 || status == StatusType::HOLD1 || status == StatusType::QUEUE);
                m_spindleTab->enableButton(!m_processingFile || status == StatusType::HOLD0);
#ifdef WINDOWS
                if (QSysInfo::windowsVersion() >= QSysInfo::WV_WINDOWS7)
                {
                    if (m_taskBarProgress)
                        m_taskBarProgress->setPaused(status == StatusType::HOLD0 || status == StatusType::HOLD1 || status == StatusType::QUEUE);
                }
#endif

                // Update "elapsed time" timer
                if (m_processingFile)
                {
                    QTime time(0, 0, 0);
                    int elapsed = m_startTimer.elapsed();
                    ui->glwVisualizer->setSpendTime(time.addMSecs(elapsed));
                }

                // Test for job complete
                if (m_processingFile && m_transferCompleted &&
                    ((status == StatusType::IDLE && prevStatus == StatusType::RUN) || status == StatusType::CHECK))
                {
                    qDebug() << "job completed:" << m_fileCommandIndex << m_currentModel->rowCount() - 1;

                    // Shadow last segment
                    GcodeViewParse *parser = m_currentDrawer->viewParser();
                    QList<LineSegment *> list = parser->getLineSegmentList();
                    if (m_lastDrawnLineIndex < list.count())
                    {
                        list[m_lastDrawnLineIndex]->setDrawn(true);
                        m_currentDrawer->update(QList<int>() << m_lastDrawnLineIndex);
                    }

                    // Update state
                    m_processingFile = false;
                    m_fileProcessedCommandIndex = 0;
                    m_lastDrawnLineIndex = 0;
                    m_storedParserStatus.clear();

                    updateControlsState();

                    qApp->beep();

                    m_timerStateQuery.stop();
                    m_timerConnection.stop();

                    QMessageBox::information(this, qApp->applicationDisplayName(), tr("Job done.\nTime elapsed: %1").arg(ui->glwVisualizer->spendTime().toString("hh:mm:ss")));

                    m_timerStateQuery.setInterval(m_settings->queryStateTime());
                    m_timerConnection.start();
                    m_timerStateQuery.start();
                }

                // Abort
                static double x = sNan;
                static double y = sNan;
                static double z = sNan;

                if (m_aborting)
                {
                    switch (status)
                    {
                    case StatusType::IDLE: // Idle
                        if (!m_processingFile && m_resetCompleted)
                        {
                            m_aborting = false;
                            restoreOffsets();
                            restoreParserState();
                            return;
                        }
                        break;
                    case StatusType::HOLD0: // Hold
                    case StatusType::HOLD1:
                    case StatusType::QUEUE:
                        if (!m_reseting && compareCoordinates(x, y, z))
                        {
                            x = sNan;
                            y = sNan;
                            z = sNan;
                            grblReset();
                        }
                        else
                        {
                            auto mpos = m_machineDisplay->getPosition();
                            x = mpos.x();
                            y = mpos.y();
                            z = mpos.z();
                        }
                        break;
                    default:
                        break;
                    }
                }
            }

            // Store work offset
            static QVector3D workOffset;
            static QRegExp wpx("WCO:([^,]*),([^,]*),([^,^>^|]*)");

            if (wpx.indexIn(data) != -1)
            {
                workOffset = QVector3D(wpx.cap(1).toDouble(), wpx.cap(2).toDouble(), wpx.cap(3).toDouble());
            }

            // Update work coordinates

            auto mpos = m_machineDisplay->getPosition();
            m_workDisplay->setPosition(mpos - workOffset);

            // Update tool position
            QVector3D toolPosition;
            if (!(status == StatusType::CHECK && m_fileProcessedCommandIndex < m_currentModel->rowCount() - 1))
            {
                toolPosition = toMetric(m_workDisplay->getPosition());
                m_toolDrawer.setToolPosition(m_codeDrawer->getIgnoreZ() ? QVector3D(toolPosition.x(), toolPosition.y(), 0) : toolPosition);
            }

            // toolpath shadowing
            if (m_processingFile && status != StatusType::CHECK)
            {
                GcodeViewParse *parser = m_currentDrawer->viewParser();

                bool toolOntoolpath = false;

                QList<int> drawnLines;
                QList<LineSegment *> list = parser->getLineSegmentList();

                for (int i = m_lastDrawnLineIndex; i < list.count() && list.at(i)->getLineNumber() <= (m_currentModel->data(m_currentModel->index(m_fileProcessedCommandIndex, 4)).toInt() + 1); i++)
                {
                    if (list.at(i)->contains(toolPosition))
                    {
                        toolOntoolpath = true;
                        m_lastDrawnLineIndex = i;
                        break;
                    }
                    drawnLines << i;
                }

                if (toolOntoolpath)
                {
                    foreach (int i, drawnLines)
                    {
                        list.at(i)->setDrawn(true);
                    }
                    if (!drawnLines.isEmpty())
                        m_currentDrawer->update(drawnLines);
                }
                else if (m_lastDrawnLineIndex < list.count())
                {
                    qDebug() << "tool missed:" << list.at(m_lastDrawnLineIndex)->getLineNumber()
                             << m_currentModel->data(m_currentModel->index(m_fileProcessedCommandIndex, 4)).toInt()
                             << m_fileProcessedCommandIndex;
                }
            }

            // Get overridings
            static QRegExp ov("Ov:([^,]*),([^,]*),([^,^>^|]*)");
            if (ov.indexIn(data) != -1)
            {
                updateOverride(m_overrideTab->feed(), ov.cap(1).toInt(), 0x91);
                updateOverride(m_overrideTab->spindle(), ov.cap(3).toInt(), 0x9a);

                int rapid = ov.cap(2).toInt();
                m_overrideTab->rapid()->setCurrentValue(rapid);

                int target = m_overrideTab->rapid()->isChecked() ? m_overrideTab->rapid()->value() : 100;

                if (rapid != target)
                    switch (target)
                    {
                    case 25:
                        m_serialPort.write(QByteArray(1, char(0x97)));
                        break;
                    case 50:
                        m_serialPort.write(QByteArray(1, char(0x96)));
                        break;
                    case 100:
                        m_serialPort.write(QByteArray(1, char(0x95)));
                        break;
                    }

                // Update pins state
                QString pinState;
                static QRegExp pn("Pn:([^|^>]*)");
                if (pn.indexIn(data) != -1)
                {
                    pinState.append(QString(tr("PS: %1")).arg(pn.cap(1)));
                }

                // Process spindle state
                static QRegExp as("A:([^,^>^|]+)");
                if (as.indexIn(data) != -1)
                {
                    QString state = as.cap(1);
                    m_spindleCW = state.contains("S");
                    if (state.contains("S") || state.contains("C"))
                    {
                        m_timerToolAnimation.start(25, this);
                        m_spindleTab->setChecked(true);
                    }
                    else
                    {
                        m_timerToolAnimation.stop();
                        m_spindleTab->setChecked(false);
                    }

                    if (!pinState.isEmpty())
                        pinState.append(" / ");
                    pinState.append(QString(tr("AS: %1")).arg(as.cap(1)));
                }
                else
                {
                    m_timerToolAnimation.stop();
                    m_spindleTab->setChecked(false);
                }
                ui->glwVisualizer->setPinState(pinState);
            }

            // Get feed/spindle values
            static QRegExp fs("FS:([^,]*),([^,^|^>]*)");
            if (fs.indexIn(data) != -1)
            {
                ui->glwVisualizer->setSpeedState((QString(tr("F/S: %1 / %2")).arg(fs.cap(1)).arg(fs.cap(2))));
            }
        }
        else if (data.length() > 0)
        {

            // Processed commands
            if (m_commands.length() > 0 && !dataIsFloating(data) && !(m_commands[0].command != "[CTRL+X]" && dataIsReset(data)))
            {

                static QString response; // Full response string

                if ((m_commands[0].command != "[CTRL+X]" && dataIsEnd(data)) || (m_commands[0].command == "[CTRL+X]" && dataIsReset(data)))
                {

                    response.append(data);

                    // Take command from buffer
                    CommandAttributes ca = m_commands.takeFirst();
                    QTextBlock tb = m_consoleTab->findBlockByNumber(ca.consoleIndex);
                    QTextCursor tc(tb);

                    // Restore absolute/relative coordinate system after jog
                    if (ca.command.toUpper() == "$G" && ca.tableIndex == -2)
                    {
                        // FIXME: jog
                        // if (ui->chkKeyboardControl->isChecked())
                        if (false)
                            m_absoluteCoordinates = response.contains("G90");
                        else if (response.contains("G90"))
                            sendCommand("G90", -1, m_settings->showUICommands());
                    }

                    // Jog
                    if (ca.command.toUpper().contains("$J=") && ca.tableIndex == -2)
                    {
                        // This gets called when a jog operation has completed.
                        // We might need another jog step...
                        jogStep();
                    }

                    // Process parser status
                    if (ca.command.toUpper() == "$G" && ca.tableIndex == -3)
                    {
                        // Update status in visualizer window
                        ui->glwVisualizer->setParserStatus(response.left(response.indexOf("; ")));

                        // Store parser status
                        if (m_processingFile)
                            storeParserState();

                        // Spindle speed
                        QRegExp rx(".*S([\\d\\.]+)");
                        if (rx.indexIn(response) != -1)
                        {
                            double speed = toMetric(rx.cap(1).toDouble()); // RPM in imperial?
                            m_spindleTab->slider()->setCurrentValue(speed);
                        }

                        m_updateParserStatus = true;
                    }

                    // Store origin
                    if (ca.command == "$#" && ca.tableIndex == -2)
                    {
                        qDebug() << "Received offsets:" << response;
                        QRegExp rx(".*G92:([^,]*),([^,]*),([^\\]]*)");

                        if (rx.indexIn(response) != -1)
                        {
                            if (m_settingZeroXY)
                            {
                                m_settingZeroXY = false;
                                m_storedX = toMetric(rx.cap(1).toDouble());
                                m_storedY = toMetric(rx.cap(2).toDouble());
                            }
                            else if (m_settingZeroZ)
                            {
                                m_settingZeroZ = false;
                                m_storedZ = toMetric(rx.cap(3).toDouble());
                            }
                            m_buttonBar->setOriginTooltip(QString(tr("Restore origin:\n%1, %2, %3")).arg(m_storedX).arg(m_storedY).arg(m_storedZ));
                        }
                    }

                    // Homing response
                    if ((ca.command.toUpper() == "$H" || ca.command.toUpper() == "$T") && m_homing)
                        m_homing = false;

                    // Reset complete
                    if (ca.command == "[CTRL+X]")
                    {
                        m_resetCompleted = true;
                        m_updateParserStatus = true;
                    }

                    // Clear command buffer on "M2" & "M30" command (old firmwares)
                    if ((ca.command.contains("M2") || ca.command.contains("M30")) && response.contains("ok") && !response.contains("[Pgm End]"))
                    {
                        m_commands.clear();
                        m_queue.clear();
                    }

#if 0
                    // Process probing on heightmap mode only from table commands
                    if (ca.command.contains("G38.2") && m_heightMapMode && ca.tableIndex > -1)
                    {
                        // Get probe Z coordinate
                        // "[PRB:0.000,0.000,0.000:0];ok"
                        QRegExp rx(".*PRB:([^,]*),([^,]*),([^]^:]*)");
                        double z = qQNaN();
                        if (rx.indexIn(response) != -1)
                        {
                            qDebug() << "probing coordinates:" << rx.cap(1) << rx.cap(2) << rx.cap(3);
                            z = toMetric(rx.cap(3).toDouble());
                        }

                        static double firstZ;
                        if (m_probeIndex == -1)
                        {
                            firstZ = z;
                            z = 0;
                        }
                        else
                        {
                            // Calculate delta Z
                            z -= firstZ;

                            // Calculate table indexes
                            int row = trunc(m_probeIndex / m_heightMapModel.columnCount());
                            int column = m_probeIndex - row * m_heightMapModel.columnCount();
                            if (row % 2)
                                column = m_heightMapModel.columnCount() - 1 - column;

                            // Store Z in table
                            m_heightMapModel.setData(m_heightMapModel.index(row, column), z, Qt::UserRole);
                            ui->tblHeightMap->update(m_heightMapModel.index(m_heightMapModel.rowCount() - 1 - row, column));
                            updateHeightMapInterpolationDrawer();
                        }

                        m_probeIndex++;
                    }
#endif

                    // Change state query time on check mode on
                    if (ca.command.contains(QRegExp("$[cC]")))
                    {
                        m_timerStateQuery.setInterval(response.contains("Enable") ? 1000 : m_settings->queryStateTime());
                    }

                    // Add response to console
                    if (tb.isValid() && tb.text() == ca.command)
                    {
                        // FIXME: console
                        // This code seems to make sure that if the console cursor is at the end of the
                        // text, the console auto scrolls to the end when adding new text.
                        // bool scrolledDown = ui->txtConsole->verticalScrollBar()->value() == ui->txtConsole->verticalScrollBar()->maximum();

                        // Update text block numbers
                        int blocksAdded = response.count("; ");

                        if (blocksAdded > 0)
                            for (int i = 0; i < m_commands.count(); i++)
                            {
                                if (m_commands[i].consoleIndex != -1)
                                    m_commands[i].consoleIndex += blocksAdded;
                            }

                        tc.beginEditBlock();
                        tc.movePosition(QTextCursor::EndOfBlock);

                        tc.insertText(" < " + QString(response).replace("; ", "\r\n"));
                        tc.endEditBlock();

                        // if (scrolledDown)
                        //     ui->txtConsole->verticalScrollBar()->setValue(ui->txtConsole->verticalScrollBar()->maximum());
                    }

                    // Check queue
                    if (m_queue.length() > 0)
                    {
                        CommandQueue cq = m_queue.takeFirst();
                        while ((bufferLength() + cq.command.length() + 1) <= BUFFERLENGTH)
                        {
                            sendCommand(cq.command, cq.tableIndex, cq.showInConsole);
                            if (m_queue.isEmpty())
                                break;
                            else
                                cq = m_queue.takeFirst();
                        }
                    }

                    // Add response to table, send next program commands
                    if (m_processingFile)
                    {

                        // Only if command from table
                        if (ca.tableIndex > -1)
                        {
                            m_currentModel->setData(m_currentModel->index(ca.tableIndex, 2), GCodeItem::Processed);
                            m_currentModel->setData(m_currentModel->index(ca.tableIndex, 3), response);

                            m_fileProcessedCommandIndex = ca.tableIndex;

                            if (ui->chkAutoScroll->isChecked() && ca.tableIndex != -1)
                            {
                                ui->tblProgram->scrollTo(m_currentModel->index(ca.tableIndex + 1, 0)); // TODO: Update by timer
                                ui->tblProgram->setCurrentIndex(m_currentModel->index(ca.tableIndex, 1));
                            }
                        }

                        // Update taskbar progress
#ifdef WINDOWS
                        if (QSysInfo::windowsVersion() >= QSysInfo::WV_WINDOWS7)
                        {
                            if (m_taskBarProgress)
                                m_taskBarProgress->setValue(m_fileProcessedCommandIndex);
                        }
#endif
                        // Process error messages
                        static bool holding = false;
                        static QString errors;

                        if (ca.tableIndex > -1 && response.toUpper().contains("ERROR") && !m_settings->ignoreErrors())
                        {
                            errors.append(QString::number(ca.tableIndex + 1) + ": " + ca.command + " < " + response + "\n");

                            m_senderErrorBox->setText(tr("Error message(s) received:\n") + errors);

                            if (!holding)
                            {
                                holding = true; // Hold transmit while messagebox is visible
                                response.clear();

                                m_serialPort.write("!");
                                m_senderErrorBox->checkBox()->setChecked(false);
                                qApp->beep();
                                int result = m_senderErrorBox->exec();

                                holding = false;
                                errors.clear();
                                if (m_senderErrorBox->checkBox()->isChecked())
                                    m_settings->setIgnoreErrors(true);
                                if (result == QMessageBox::Ignore)
                                    m_serialPort.write("~");
                                else
                                    on_cmdFileAbort_clicked();
                            }
                        }

                        // Check transfer complete (last row always blank, last command row = rowcount - 2)
                        if (m_fileProcessedCommandIndex == m_currentModel->rowCount() - 2 || ca.command.contains(QRegExp("M0*2|M30")))
                            m_transferCompleted = true;
                        // Send next program commands
                        else if (!m_fileEndSent && (m_fileCommandIndex < m_currentModel->rowCount()) && !holding)
                            sendNextFileCommands();
                    }

                    // Scroll to first line on "M30" command
                    if (ca.command.contains("M30"))
                        ui->tblProgram->setCurrentIndex(m_currentModel->index(0, 1));

                    // FIXME:
                    // Toolpath shadowing on check mode
                    if (m_statusWidget->getStatus() == StatusType::CHECK)
                    {
                        GcodeViewParse *parser = m_currentDrawer->viewParser();
                        QList<LineSegment *> list = parser->getLineSegmentList();

                        if (!m_transferCompleted && m_fileProcessedCommandIndex < m_currentModel->rowCount() - 1)
                        {
                            int i;
                            QList<int> drawnLines;

                            for (i = m_lastDrawnLineIndex; i < list.count() && list.at(i)->getLineNumber() <= (m_currentModel->data(m_currentModel->index(m_fileProcessedCommandIndex, 4)).toInt()); i++)
                            {
                                drawnLines << i;
                            }

                            if (!drawnLines.isEmpty() && (i < list.count()))
                            {
                                m_lastDrawnLineIndex = i;
                                QVector3D vec = list.at(i)->getEnd();
                                m_toolDrawer.setToolPosition(vec);
                            }

                            foreach (int i, drawnLines)
                            {
                                list.at(i)->setDrawn(true);
                            }
                            if (!drawnLines.isEmpty())
                                m_currentDrawer->update(drawnLines);
                        }
                        else
                        {
                            foreach (LineSegment *s, list)
                            {
                                if (!qIsNaN(s->getEnd().length()))
                                {
                                    m_toolDrawer.setToolPosition(s->getEnd());
                                    break;
                                }
                            }
                        }
                    }

                    response.clear();
                }
                else
                {
                    response.append(data + "; ");
                }
            }
            else
            {
                // Unprocessed responses
                qDebug() << "floating response:" << data;

                // Handle hardware reset
                if (dataIsReset(data))
                {
                    qDebug() << "hardware reset";

                    m_processingFile = false;
                    m_transferCompleted = true;
                    m_fileCommandIndex = 0;

                    m_reseting = false;
                    m_homing = false;
                    m_statusWidget->reset();

                    m_updateParserStatus = true;
                    m_statusReceived = true;

                    m_commands.clear();
                    m_queue.clear();

                    updateControlsState();
                }

                m_consoleTab->appendPlainText(data);
                // ui->txtConsole->appendPlainText(data);
            }
        }
        else
        {
            // Blank response
            //            ui->txtConsole->appendPlainText(data);
        }
    }
}

void frmMain::onSerialPortError(QSerialPort::SerialPortError error)
{
    static QSerialPort::SerialPortError previousError;

    if (error != QSerialPort::NoError && error != previousError)
    {
        previousError = error;
        // FIXME: console
        // m_consoleTab->appendPlainText(tr("Serial port error ") + QString::number(error) + ": " + m_serialPort.errorString());
        if (m_serialPort.isOpen())
        {
            m_serialPort.close();
            updateControlsState();
        }
    }
}

void frmMain::onTimerConnection()
{
    if (!m_serialPort.isOpen())
    {
        openPort();
    }
    else if (!m_homing /* && !m_reseting*/ && !ui->cmdFilePause->isChecked() && m_queue.length() == 0)
    {
        if (m_updateSpindleSpeed)
        {
            m_updateSpindleSpeed = false;
            sendCommand(QString("S%1").arg(m_spindleTab->value()), -2, m_settings->showUICommands());
        }
        if (m_updateParserStatus)
        {
            m_updateParserStatus = false;
            sendCommand("$G", -3, false);
        }
    }
}

void frmMain::onTimerStateQuery()
{
    if (m_serialPort.isOpen() && m_resetCompleted && m_statusReceived)
    {
        m_serialPort.write(QByteArray(1, '?'));
        m_statusReceived = false;
    }

    ui->glwVisualizer->setBufferState(QString(tr("Buffer: %1 / %2 / %3")).arg(bufferLength()).arg(m_commands.length()).arg(m_queue.length()));
}

void frmMain::onVisualizatorRotationChanged()
{
    ui->cmdIsometric->setChecked(false);
}

void frmMain::onScroolBarAction(int action)
{
    Q_UNUSED(action)

    if (m_processingFile)
        ui->chkAutoScroll->setChecked(false);
}

void frmMain::onJogTimer()
{
    m_jogBlock = false;
}

void frmMain::placeVisualizerButtons()
{
    ui->cmdIsometric->move(ui->glwVisualizer->width() - ui->cmdIsometric->width() - 8, 8);
    ui->cmdTop->move(ui->cmdIsometric->geometry().left() - ui->cmdTop->width() - 8, 8);
    ui->cmdLeft->move(ui->glwVisualizer->width() - ui->cmdLeft->width() - 8, ui->cmdIsometric->geometry().bottom() + 8);
    ui->cmdFront->move(ui->cmdLeft->geometry().left() - ui->cmdFront->width() - 8, ui->cmdIsometric->geometry().bottom() + 8);
    //    ui->cmdFit->move(ui->cmdTop->geometry().left() - ui->cmdFit->width() - 10, 10);
    ui->cmdFit->move(ui->glwVisualizer->width() - ui->cmdFit->width() - 8, ui->cmdLeft->geometry().bottom() + 8);
}

void frmMain::showEvent(QShowEvent *se)
{
    Q_UNUSED(se)

    placeVisualizerButtons();

#ifdef WINDOWS
    if (QSysInfo::windowsVersion() >= QSysInfo::WV_WINDOWS7)
    {
        if (m_taskBarButton == NULL)
        {
            m_taskBarButton = new QWinTaskbarButton(this);
            m_taskBarButton->setWindow(this->windowHandle());
            m_taskBarProgress = m_taskBarButton->progress();
        }
    }
#endif

    ui->glwVisualizer->setUpdatesEnabled(true);

    resizeCheckBoxes();
}

void frmMain::hideEvent(QHideEvent *he)
{
    Q_UNUSED(he)

    ui->glwVisualizer->setUpdatesEnabled(false);
}

void frmMain::resizeEvent(QResizeEvent *re)
{
    Q_UNUSED(re)

    placeVisualizerButtons();
    resizeCheckBoxes();
}

void frmMain::resizeCheckBoxes()
{
    static int widthCheckMode = ui->chkTestMode->sizeHint().width();
    static int widthAutoScroll = ui->chkAutoScroll->sizeHint().width();

    // Transform checkboxes

    this->setUpdatesEnabled(false);

    updateLayouts();

    if (ui->chkTestMode->sizeHint().width() > ui->chkTestMode->width())
    {
        widthCheckMode = ui->chkTestMode->sizeHint().width();
        ui->chkTestMode->setText(tr("Check"));
        ui->chkTestMode->setMinimumWidth(ui->chkTestMode->sizeHint().width());
        updateLayouts();
    }

    if (ui->chkAutoScroll->sizeHint().width() > ui->chkAutoScroll->width() && ui->chkTestMode->text() == tr("Check"))
    {
        widthAutoScroll = ui->chkAutoScroll->sizeHint().width();
        ui->chkAutoScroll->setText(tr("Scroll"));
        ui->chkAutoScroll->setMinimumWidth(ui->chkAutoScroll->sizeHint().width());
        updateLayouts();
    }

    if (ui->spacerBot->geometry().width() + ui->chkAutoScroll->sizeHint().width() - ui->spacerBot->sizeHint().width() > widthAutoScroll && ui->chkAutoScroll->text() == tr("Scroll"))
    {
        ui->chkAutoScroll->setText(tr("Autoscroll"));
        updateLayouts();
    }

    if (ui->spacerBot->geometry().width() + ui->chkTestMode->sizeHint().width() - ui->spacerBot->sizeHint().width() > widthCheckMode && ui->chkTestMode->text() == tr("Check"))
    {
        ui->chkTestMode->setText(tr("Check mode"));
        updateLayouts();
    }

    this->setUpdatesEnabled(true);
    this->repaint();
}

void frmMain::timerEvent(QTimerEvent *te)
{
    if (te->timerId() == m_timerToolAnimation.timerId())
    {
        auto slider = m_spindleTab->slider();
        m_toolDrawer.rotate((m_spindleCW ? -40 : 40) * (double)(slider->currentValue()) / (slider->maximum()));
    }
    else
    {
        QMainWindow::timerEvent(te);
    }
}

void frmMain::closeEvent(QCloseEvent *ce)
{
    if (!saveChanges(false))
    {
        ce->ignore();
        return;
    }

    if (m_processingFile && QMessageBox::warning(this, this->windowTitle(), tr("File sending in progress. Terminate and exit?"),
                                                 QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
    {
        ce->ignore();
        return;
    }

    if (m_serialPort.isOpen())
        m_serialPort.close();
    if (m_queue.length() > 0)
    {
        m_commands.clear();
        m_queue.clear();
    }
}

void frmMain::dragEnterEvent(QDragEnterEvent *dee)
{
    if (m_processingFile)
        return;

    if (dee->mimeData()->hasFormat("text/plain"))
    {
        dee->acceptProposedAction();
    }

#if 0    
    else if (dee->mimeData()->hasFormat("text/uri-list") && dee->mimeData()->urls().count() == 1)
    {
        QString fileName = dee->mimeData()->urls().at(0).toLocalFile();

        if ((!m_heightMapMode && isGCodeFile(fileName)) || (m_heightMapMode && isHeightmapFile(fileName)))
            dee->acceptProposedAction();
    }
#endif    
}

void frmMain::dropEvent(QDropEvent *de)
{
    QString fileName = de->mimeData()->urls().at(0).toLocalFile();

    if (!saveChanges(false))
        return;

    // Load dropped g-code file
    if (!fileName.isEmpty())
    {
        addRecentFile(fileName);
        updateRecentFilesMenu();
        loadFile(fileName);
        // Load dropped text
    }
    else
    {
        m_programFileName.clear();
        m_fileChanged = true;
        loadFile(de->mimeData()->text().split("\n"));
    }
}

void frmMain::on_actFileExit_triggered()
{
    close();
}

void frmMain::on_cmdFileOpen_clicked()
{
    if (!saveChanges(false))
        return;

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open"), m_lastFolder,
                                                    tr("G-Code files (*.nc *.ncc *.ngc *.tap *.gcode *.gc *.txt);;All files (*.*)"));

    if (!fileName.isEmpty())
        m_lastFolder = fileName.left(fileName.lastIndexOf(QRegExp("[/\\\\]+")));

    if (fileName != "")
    {
        addRecentFile(fileName);
        updateRecentFilesMenu();

        loadFile(fileName);
    }
}

void frmMain::loadFile(QList<QString> data)
{
    QElapsedTimer timer;
    timer.start();

    // Reset tables
    clearTable();
    m_probeModel.clear();
    m_currentModel = &m_programModel;

    // Reset parsers
    m_viewParser.reset();
    m_probeParser.reset();

    // Reset code drawer
    m_currentDrawer = m_codeDrawer;
    m_codeDrawer->update();
    ui->glwVisualizer->fitDrawable(m_codeDrawer);
    updateProgramEstimatedTime(QList<LineSegment *>());

    // Reset tableview
    QByteArray headerState = ui->tblProgram->horizontalHeader()->saveState();
    ui->tblProgram->setModel(NULL);

    // Prepare parser
    GcodeParser gp;
    gp.setTraverseSpeed(m_settings->rapidSpeed());
    if (m_codeDrawer->getIgnoreZ())
        gp.reset(QVector3D(qQNaN(), qQNaN(), 0));

    qDebug() << "Prepared to load:" << timer.elapsed();
    timer.start();

    // Block parser updates on table changes
    m_programLoading = true;

    QString command;
    QString stripped;
    QString trimmed;
    QList<QString> args;
    GCodeItem item;

    // Prepare model
    m_programModel.data().clear();
    m_programModel.data().reserve(data.count());

    QProgressDialog progress(tr("Opening file..."), tr("Abort"), 0, data.count(), this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setFixedSize(progress.sizeHint());
    if (data.count() > PROGRESSMINLINES)
    {
        progress.show();
        progress.setStyleSheet("QProgressBar {text-align: center; qproperty-format: \"\"}");
    }

    while (!data.isEmpty())
    {
        command = data.takeFirst();

        // Trim command
        trimmed = command.trimmed();

        if (!trimmed.isEmpty())
        {
            // Split command
            stripped = GcodePreprocessorUtils::removeComment(command);
            args = GcodePreprocessorUtils::splitCommand(stripped);

            //            PointSegment *ps = gp.addCommand(args);
            gp.addCommand(args);

            //        if (ps && (qIsNaN(ps->point()->x()) || qIsNaN(ps->point()->y()) || qIsNaN(ps->point()->z())))
            //                   qDebug() << "nan point segment added:" << *ps->point();

            item.command = trimmed;
            item.state = GCodeItem::InQueue;
            item.line = gp.getCommandNumber();
            item.args = args;

            m_programModel.data().append(item);
        }

        if (progress.isVisible() && (data.count() % PROGRESSSTEP == 0))
        {
            progress.setValue(progress.maximum() - data.count());
            qApp->processEvents();
            if (progress.wasCanceled())
                break;
        }
    }
    progress.close();

    m_programModel.insertRow(m_programModel.rowCount());

    qDebug() << "model filled:" << timer.elapsed();
    timer.start();

    updateProgramEstimatedTime(m_viewParser.getLinesFromParser(&gp, m_settings->arcPrecision(), m_settings->arcDegreeMode()));
    qDebug() << "view parser filled:" << timer.elapsed();

    m_programLoading = false;

    // Set table model
    ui->tblProgram->setModel(&m_programModel);
    ui->tblProgram->horizontalHeader()->restoreState(headerState);

    // Update tableview
    connect(ui->tblProgram->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)), this, SLOT(onTableCurrentChanged(QModelIndex, QModelIndex)));
    ui->tblProgram->selectRow(0);

    //  Update code drawer
    m_codeDrawer->update();
    ui->glwVisualizer->fitDrawable(m_codeDrawer);

    updateControlsState();
}

void frmMain::loadFile(QString fileName)
{
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(this, this->windowTitle(), tr("Can't open file:\n") + fileName);
        return;
    }

    // Set filename
    m_programFileName = fileName;

    // Prepare text stream
    QTextStream textStream(&file);

    // Read lines
    QList<QString> data;
    while (!textStream.atEnd())
        data.append(textStream.readLine());

    // Load lines
    loadFile(data);
}

QTime frmMain::updateProgramEstimatedTime(QList<LineSegment *> lines)
{
    double time = 0;

    for (int i = 0; i < lines.count(); i++)
    {
        LineSegment *ls = lines[i];
        //    foreach (LineSegment *ls, lines) {
        double length = (ls->getEnd() - ls->getStart()).length();

        if (!qIsNaN(length) && !qIsNaN(ls->getSpeed()) && ls->getSpeed() != 0)
        {
            auto feed = m_overrideTab->feed();
            auto rapid = m_overrideTab->rapid();

            time +=
                length / ((feed->isChecked() && !ls->isFastTraverse())
                              ? (ls->getSpeed() * feed->value() / 100)
                          : (rapid->isChecked() && ls->isFastTraverse())
                              ? (ls->getSpeed() * rapid->value() / 100)
                              : ls->getSpeed()); // TODO: Update for rapid override
        }

        //        qDebug() << "length/time:" << length << ((ui->chkFeedOverride->isChecked() && !ls->isFastTraverse())
        //                                                 ? (ls->getSpeed() * ui->txtFeed->value() / 100) : ls->getSpeed())
        //                 << time;

        //        if (qIsNaN(length)) qDebug() << "length nan:" << i << ls->getLineNumber() << ls->getStart() << ls->getEnd();
        //        if (qIsNaN(ls->getSpeed())) qDebug() << "speed nan:" << ls->getSpeed();
    }

    time *= 60;

    QTime t;

    t.setHMS(0, 0, 0);
    t = t.addSecs(time);

    ui->glwVisualizer->setSpendTime(QTime(0, 0, 0));
    ui->glwVisualizer->setEstimatedTime(t);

    return t;
}

void frmMain::clearTable()
{
    m_programModel.clear();
    m_programModel.insertRow(0);
}

void frmMain::on_cmdFit_clicked()
{
    ui->glwVisualizer->fitDrawable(m_currentDrawer);
}

void frmMain::on_cmdFileSend_clicked()
{
    if (m_currentModel->rowCount() == 1)
        return;

    on_cmdFileReset_clicked();

    m_startTimer.start();

    m_transferCompleted = false;
    m_processingFile = true;
    m_fileEndSent = false;

    // FIXME: jog
    // m_storedKeyboardControl = ui->chkKeyboardControl->isChecked();
    // ui->chkKeyboardControl->setChecked(false);

    if (!ui->chkTestMode->isChecked())
        storeOffsets(); // Allready stored on check
    storeParserState();

#ifdef WINDOWS
    if (QSysInfo::windowsVersion() >= QSysInfo::WV_WINDOWS7)
    {
        if (m_taskBarProgress)
        {
            m_taskBarProgress->setMaximum(m_currentModel->rowCount() - 2);
            m_taskBarProgress->setValue(0);
            m_taskBarProgress->show();
        }
    }
#endif

    updateControlsState();
    ui->cmdFilePause->setFocus();

    sendNextFileCommands();
}

void frmMain::onActSendFromLineTriggered()
{
    if (m_currentModel->rowCount() == 1)
        return;

    // Line to start from
    int commandIndex = ui->tblProgram->currentIndex().row();

    // Set parser state
    if (m_settings->autoLine())
    {
        GcodeViewParse *parser = m_currentDrawer->viewParser();
        QList<LineSegment *> list = parser->getLineSegmentList();
        QVector<QList<int>> lineIndexes = parser->getLinesIndexes();

        int lineNumber = m_currentModel->data(m_currentModel->index(commandIndex, 4)).toInt();
        LineSegment *firstSegment = list.at(lineIndexes.at(lineNumber).first());
        LineSegment *lastSegment = list.at(lineIndexes.at(lineNumber).last());
        LineSegment *feedSegment = lastSegment;
        int segmentIndex = list.indexOf(feedSegment);
        while (feedSegment->isFastTraverse() && segmentIndex > 0)
            feedSegment = list.at(--segmentIndex);

        QStringList commands;

        commands.append(QString("M3 S%1").arg(qMax<double>(lastSegment->getSpindleSpeed(), m_spindleTab->value())));

        commands.append(QString("G21 G90 G0 X%1 Y%2")
                            .arg(firstSegment->getStart().x())
                            .arg(firstSegment->getStart().y()));
        commands.append(QString("G1 Z%1 F%2")
                            .arg(firstSegment->getStart().z())
                            .arg(feedSegment->getSpeed()));

        commands.append(QString("%1 %2 %3 F%4")
                            .arg(lastSegment->isMetric() ? "G21" : "G20")
                            .arg(lastSegment->isAbsolute() ? "G90" : "G91")
                            .arg(lastSegment->isFastTraverse() ? "G0" : "G1")
                            .arg(lastSegment->isMetric() ? feedSegment->getSpeed() : feedSegment->getSpeed() / 25.4));

        if (lastSegment->isArc())
        {
            commands.append(lastSegment->plane() == PointSegment::XY   ? "G17"
                            : lastSegment->plane() == PointSegment::ZX ? "G18"
                                                                       : "G19");
        }

        QMessageBox box(this);
        box.setIcon(QMessageBox::Information);
        box.setText(tr("Following commands will be sent before selected line:\n") + commands.join('\n'));
        box.setWindowTitle(qApp->applicationDisplayName());
        box.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        box.addButton(tr("Skip"), QMessageBox::DestructiveRole);

        int res = box.exec();
        if (res == QMessageBox::Cancel)
            return;
        else if (res == QMessageBox::Ok)
        {
            foreach (QString command, commands)
            {
                sendCommand(command, -1, m_settings->showUICommands());
            }
        }
    }

    m_fileCommandIndex = commandIndex;
    m_fileProcessedCommandIndex = commandIndex;
    m_lastDrawnLineIndex = 0;
    m_probeIndex = -1;

    QList<LineSegment *> list = m_viewParser.getLineSegmentList();

    QList<int> indexes;
    for (int i = 0; i < list.count(); i++)
    {
        list[i]->setDrawn(list.at(i)->getLineNumber() < m_currentModel->data().at(commandIndex).line);
        indexes.append(i);
    }
    m_codeDrawer->update(indexes);

    ui->tblProgram->setUpdatesEnabled(false);

    for (int i = 0; i < m_currentModel->data().count() - 1; i++)
    {
        m_currentModel->data()[i].state = i < commandIndex ? GCodeItem::Skipped : GCodeItem::InQueue;
        m_currentModel->data()[i].response = QString();
    }
    ui->tblProgram->setUpdatesEnabled(true);
    ui->glwVisualizer->setSpendTime(QTime(0, 0, 0));

    m_startTimer.start();

    m_transferCompleted = false;
    m_processingFile = true;
    m_fileEndSent = false;

    // FIXME: jog
    // m_storedKeyboardControl = ui->chkKeyboardControl->isChecked();
    // ui->chkKeyboardControl->setChecked(false);

    if (!ui->chkTestMode->isChecked())
        storeOffsets(); // Allready stored on check
    storeParserState();

#ifdef WINDOWS
    if (QSysInfo::windowsVersion() >= QSysInfo::WV_WINDOWS7)
    {
        if (m_taskBarProgress)
        {
            m_taskBarProgress->setMaximum(m_currentModel->rowCount() - 2);
            m_taskBarProgress->setValue(commandIndex);
            m_taskBarProgress->show();
        }
    }
#endif

    updateControlsState();
    ui->cmdFilePause->setFocus();

    m_fileCommandIndex = commandIndex;
    m_fileProcessedCommandIndex = commandIndex;
    sendNextFileCommands();
}

void frmMain::on_cmdFileAbort_clicked()
{
    m_aborting = true;
    if (!ui->chkTestMode->isChecked())
    {
        m_serialPort.write("!");
    }
    else
    {
        grblReset();
    }
}

void frmMain::storeParserState()
{
    m_storedParserStatus = ui->glwVisualizer->parserStatus().remove(
        QRegExp("GC:|\\[|\\]|G[01234]\\s|M[0345]+\\s|\\sF[\\d\\.]+|\\sS[\\d\\.]+"));
}

void frmMain::restoreParserState()
{
    if (!m_storedParserStatus.isEmpty())
        sendCommand(m_storedParserStatus, -1, m_settings->showUICommands());
}

void frmMain::storeOffsets()
{
    //    sendCommand("$#", -2, m_settings->showUICommands());
}

void frmMain::restoreOffsets()
{
    // Still have pre-reset working position
    auto mpos = toMetric(m_machineDisplay->getPosition());
    auto wpos = toMetric(m_workDisplay->getPosition());

    sendCommand(QString("G21G53G90X%1Y%2Z%3").arg(mpos.x()).arg(mpos.y()).arg(mpos.z()), -1, m_settings->showUICommands());
    sendCommand(QString("G21G92X%1Y%2Z%3").arg(wpos.x()).arg(wpos.y()).arg(wpos.z()), -1, m_settings->showUICommands());
}

void frmMain::sendNextFileCommands()
{
    if (m_queue.length() > 0)
        return;

    QString command = feedOverride(m_currentModel->data(m_currentModel->index(m_fileCommandIndex, 1)).toString());

    while ((bufferLength() + command.length() + 1) <= BUFFERLENGTH && m_fileCommandIndex < m_currentModel->rowCount() - 1 && !(!m_commands.isEmpty() && m_commands.last().command.contains(QRegExp("M0*2|M30"))))
    {
        m_currentModel->setData(m_currentModel->index(m_fileCommandIndex, 2), GCodeItem::Sent);
        sendCommand(command, m_fileCommandIndex, m_settings->showProgramCommands());
        m_fileCommandIndex++;
        command = feedOverride(m_currentModel->data(m_currentModel->index(m_fileCommandIndex, 1)).toString());
    }
}

void frmMain::onTableCellChanged(QModelIndex i1, QModelIndex i2)
{
    Q_UNUSED(i2)

    GCodeTableModel *model = (GCodeTableModel *)sender();

    if (i1.column() != 1)
        return;
    // Inserting new line at end
    if (i1.row() == (model->rowCount() - 1) && model->data(model->index(i1.row(), 1)).toString() != "")
    {
        model->setData(model->index(model->rowCount() - 1, 2), GCodeItem::InQueue);
        model->insertRow(model->rowCount());
        if (!m_programLoading)
            ui->tblProgram->setCurrentIndex(model->index(i1.row() + 1, 1));
        // Remove last line
    } /*else if (i1.row() != (model->rowCount() - 1) && model->data(model->index(i1.row(), 1)).toString() == "") {
        ui->tblProgram->setCurrentIndex(model->index(i1.row() + 1, 1));
        m_tableModel.removeRow(i1.row());
    }*/

    if (!m_programLoading)
    {

        // Clear cached args
        model->setData(model->index(i1.row(), 5), QVariant());

        // Update visualizer
        updateParser();

        // Hightlight w/o current cell changed event (double hightlight on current cell changed)
        QList<LineSegment *> list = m_viewParser.getLineSegmentList();
        for (int i = 0; i < list.count() && list[i]->getLineNumber() <= m_currentModel->data(m_currentModel->index(i1.row(), 4)).toInt(); i++)
        {
            list[i]->setIsHightlight(true);
        }
    }
}

void frmMain::onTableCurrentChanged(QModelIndex idx1, QModelIndex idx2)
{
    // Update toolpath hightlighting
    if (idx1.row() > m_currentModel->rowCount() - 2)
        idx1 = m_currentModel->index(m_currentModel->rowCount() - 2, 0);
    if (idx2.row() > m_currentModel->rowCount() - 2)
        idx2 = m_currentModel->index(m_currentModel->rowCount() - 2, 0);

    GcodeViewParse *parser = m_currentDrawer->viewParser();
    QList<LineSegment *> list = parser->getLineSegmentList();
    QVector<QList<int>> lineIndexes = parser->getLinesIndexes();

    // Update linesegments on cell changed
    if (!m_currentDrawer->geometryUpdated())
    {
        for (int i = 0; i < list.count(); i++)
        {
            list.at(i)->setIsHightlight(list.at(i)->getLineNumber() <= m_currentModel->data(m_currentModel->index(idx1.row(), 4)).toInt());
        }
        // Update vertices on current cell changed
    }
    else
    {

        int lineFirst = m_currentModel->data(m_currentModel->index(idx1.row(), 4)).toInt();
        int lineLast = m_currentModel->data(m_currentModel->index(idx2.row(), 4)).toInt();
        if (lineLast < lineFirst)
            qSwap(lineLast, lineFirst);
        //        qDebug() << "table current changed" << idx1.row() << idx2.row() << lineFirst << lineLast;

        QList<int> indexes;
        for (int i = lineFirst + 1; i <= lineLast; i++)
        {
            foreach (int l, lineIndexes.at(i))
            {
                list.at(l)->setIsHightlight(idx1.row() > idx2.row());
                indexes.append(l);
            }
        }

        m_selectionDrawer.setEndPosition(indexes.isEmpty() ? QVector3D(sNan, sNan, sNan) : (m_codeDrawer->getIgnoreZ() ? QVector3D(list.at(indexes.last())->getEnd().x(), list.at(indexes.last())->getEnd().y(), 0) : list.at(indexes.last())->getEnd()));
        m_selectionDrawer.update();

        if (!indexes.isEmpty())
            m_currentDrawer->update(indexes);
    }

    // Update selection marker
    int line = m_currentModel->data(m_currentModel->index(idx1.row(), 4)).toInt();
    if (line > 0 && !lineIndexes.at(line).isEmpty())
    {
        QVector3D pos = list.at(lineIndexes.at(line).last())->getEnd();
        m_selectionDrawer.setEndPosition(m_codeDrawer->getIgnoreZ() ? QVector3D(pos.x(), pos.y(), 0) : pos);
    }
    else
    {
        m_selectionDrawer.setEndPosition(QVector3D(sNan, sNan, sNan));
    }
    m_selectionDrawer.update();
}

void frmMain::onTableInsertLine()
{
    if (ui->tblProgram->selectionModel()->selectedRows().count() == 0 || m_processingFile)
        return;

    int row = ui->tblProgram->selectionModel()->selectedRows()[0].row();

    m_currentModel->insertRow(row);
    m_currentModel->setData(m_currentModel->index(row, 2), GCodeItem::InQueue);

    updateParser();
    m_cellChanged = true;
    ui->tblProgram->selectRow(row);
}

void frmMain::onTableDeleteLines()
{
    if (ui->tblProgram->selectionModel()->selectedRows().count() == 0 || m_processingFile ||
        QMessageBox::warning(this, this->windowTitle(), tr("Delete lines?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
        return;

    QModelIndex firstRow = ui->tblProgram->selectionModel()->selectedRows()[0];
    int rowsCount = ui->tblProgram->selectionModel()->selectedRows().count();
    if (ui->tblProgram->selectionModel()->selectedRows().last().row() == m_currentModel->rowCount() - 1)
        rowsCount--;

    qDebug() << "deleting lines" << firstRow.row() << rowsCount;

    if (firstRow.row() != m_currentModel->rowCount() - 1)
    {
        m_currentModel->removeRows(firstRow.row(), rowsCount);
    }
    else
        return;

    updateParser();
    m_cellChanged = true;
    ui->tblProgram->selectRow(firstRow.row());
}

void frmMain::on_actServiceSettings_triggered()
{
    if (m_settings->exec())
    {
        qDebug() << "Applying settings";
        qDebug() << "Port:" << m_settings->port() << "Baud:" << m_settings->baud();

        if (m_settings->port() != "" && (m_settings->port() != m_serialPort.portName() ||
                                         m_settings->baud() != m_serialPort.baudRate()))
        {
            if (m_serialPort.isOpen())
                m_serialPort.close();
            m_serialPort.setPortName(m_settings->port());
            m_serialPort.setBaudRate(m_settings->baud());
            openPort();
        }

        updateControlsState();
        applySettings();
    }
    else
    {
        m_settings->undo();
    }
}

bool buttonLessThan(StyledToolButton *b1, StyledToolButton *b2)
{
    //    return b1->text().toDouble() < b2->text().toDouble();
    return b1->objectName().right(1).toDouble() < b2->objectName().right(1).toDouble();
}

void frmMain::applySettings()
{
    m_originDrawer->setLineWidth(m_settings->lineWidth());
    m_toolDrawer.setToolDiameter(m_settings->toolDiameter());
    m_toolDrawer.setToolLength(m_settings->toolLength());
    m_toolDrawer.setLineWidth(m_settings->lineWidth());
    m_codeDrawer->setLineWidth(m_settings->lineWidth());

    ui->glwVisualizer->setLineWidth(m_settings->lineWidth());
    m_timerStateQuery.setInterval(m_settings->queryStateTime());

    m_toolDrawer.setToolAngle(m_settings->toolType() == 0 ? 180 : m_settings->toolAngle());
    m_toolDrawer.setColor(m_settings->colors("Tool"));
    m_toolDrawer.update();

    ui->glwVisualizer->setAntialiasing(m_settings->antialiasing());
    ui->glwVisualizer->setMsaa(m_settings->msaa());
    ui->glwVisualizer->setZBuffer(m_settings->zBuffer());
    ui->glwVisualizer->setVsync(m_settings->vsync());
    ui->glwVisualizer->setFps(m_settings->fps());
    ui->glwVisualizer->setColorBackground(m_settings->colors("VisualizerBackground"));
    ui->glwVisualizer->setColorText(m_settings->colors("VisualizerText"));

    m_spindleTab->setMaxSpindleSpeed(m_settings->spindleSpeedMax());
    m_spindleTab->setMinSpindleSpeed(m_settings->spindleSpeedMin());

    // FIXME: console
    // ui->cboCommand->setAutoCompletion(m_settings->autoCompletion());
    // ui->cboCommand->setCompleter(m_settings->autoCompletion());

    m_codeDrawer->setSimplify(m_settings->simplify());
    m_codeDrawer->setSimplifyPrecision(m_settings->simplifyPrecision());
    m_codeDrawer->setColorNormal(m_settings->colors("ToolpathNormal"));
    m_codeDrawer->setColorDrawn(m_settings->colors("ToolpathDrawn"));
    m_codeDrawer->setColorHighlight(m_settings->colors("ToolpathHighlight"));
    m_codeDrawer->setColorZMovement(m_settings->colors("ToolpathZMovement"));
    m_codeDrawer->setColorStart(m_settings->colors("ToolpathStart"));
    m_codeDrawer->setColorEnd(m_settings->colors("ToolpathEnd"));
    m_codeDrawer->setIgnoreZ(m_settings->grayscaleSegments() || !m_settings->drawModeVectors());
    m_codeDrawer->setGrayscaleSegments(m_settings->grayscaleSegments());
    m_codeDrawer->setGrayscaleCode(m_settings->grayscaleSCode() ? GcodeDrawer::S : GcodeDrawer::Z);
    m_codeDrawer->setDrawMode(m_settings->drawModeVectors() ? GcodeDrawer::Vectors : GcodeDrawer::Raster);
    m_codeDrawer->setGrayscaleMin(m_settings->laserPowerMin());
    m_codeDrawer->setGrayscaleMax(m_settings->laserPowerMax());
    m_codeDrawer->update();

    m_selectionDrawer.setColor(m_settings->colors("ToolpathHighlight"));

    // Adapt visualizer buttons colors
    const int LIGHTBOUND = 127;
    const int NORMALSHIFT = 40;
    const int HIGHLIGHTSHIFT = 80;

    QColor base = m_settings->colors("VisualizerBackground");
    bool light = base.value() > LIGHTBOUND;
    QColor normal, highlight;

    normal.setHsv(base.hue(), base.saturation(), base.value() + (light ? -NORMALSHIFT : NORMALSHIFT));
    highlight.setHsv(base.hue(), base.saturation(), base.value() + (light ? -HIGHLIGHTSHIFT : HIGHLIGHTSHIFT));

    ui->glwVisualizer->setStyleSheet(QString("QToolButton {border: 1px solid %1; \
                background-color: %3} QToolButton:hover {border: 1px solid %2;}")
                                         .arg(normal.name())
                                         .arg(highlight.name())
                                         .arg(base.name()));

    ui->cmdFit->setIcon(QIcon(":/images/fit_1.png"));
    ui->cmdIsometric->setIcon(QIcon(":/images/cube.png"));
    ui->cmdFront->setIcon(QIcon(":/images/cubeFront.png"));
    ui->cmdLeft->setIcon(QIcon(":/images/cubeLeft.png"));
    ui->cmdTop->setIcon(QIcon(":/images/cubeTop.png"));

    if (!light)
    {
        Util::invertButtonIconColors(ui->cmdFit);
        Util::invertButtonIconColors(ui->cmdIsometric);
        Util::invertButtonIconColors(ui->cmdFront);
        Util::invertButtonIconColors(ui->cmdLeft);
        Util::invertButtonIconColors(ui->cmdTop);
    }

    // FIXME: console
    // ui->cboCommand->setMinimumHeight(ui->cboCommand->height());
    // ui->cmdClearConsole->setFixedHeight(ui->cboCommand->height());
    // ui->cmdCommandSend->setFixedHeight(ui->cboCommand->height());

    // foreach (StyledToolButton *button, this->findChildren<StyledToolButton *>(QRegExp("cmdUser\\d")))
    foreach (StyledToolButton *button, findChildren<StyledToolButton *>(QRegularExpression("cmdUser\\d")))
    {
        button->setToolTip(m_settings->userCommands(button->objectName().right(1).toInt()));
        button->setEnabled(!button->toolTip().isEmpty());
    }
}

void frmMain::updateParser()
{
    QElapsedTimer timer;

    qDebug() << "updating parser:" << m_currentModel << m_currentDrawer;
    timer.start();

    GcodeViewParse *parser = m_currentDrawer->viewParser();

    GcodeParser gp;
    gp.setTraverseSpeed(m_settings->rapidSpeed());
    if (m_codeDrawer->getIgnoreZ())
        gp.reset(QVector3D(qQNaN(), qQNaN(), 0));

    ui->tblProgram->setUpdatesEnabled(false);

    QString stripped;
    QList<QString> args;

    QProgressDialog progress(tr("Updating..."), tr("Abort"), 0, m_currentModel->rowCount() - 2, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setFixedSize(progress.sizeHint());

    if (m_currentModel->rowCount() > PROGRESSMINLINES)
    {
        progress.show();
        progress.setStyleSheet("QProgressBar {text-align: center; qproperty-format: \"\"}");
    }

    for (int i = 0; i < m_currentModel->rowCount() - 1; i++)
    {
        // Get stored args
        args = m_currentModel->data().at(i).args;

        // Store args if none
        if (args.isEmpty())
        {
            stripped = GcodePreprocessorUtils::removeComment(m_currentModel->data().at(i).command);
            args = GcodePreprocessorUtils::splitCommand(stripped);
            m_currentModel->data()[i].args = args;
        }

        // Add command to parser
        gp.addCommand(args);

        // Update table model
        m_currentModel->data()[i].state = GCodeItem::InQueue;
        m_currentModel->data()[i].response = QString();
        m_currentModel->data()[i].line = gp.getCommandNumber();

        if (progress.isVisible() && (i % PROGRESSSTEP == 0))
        {
            progress.setValue(i);
            qApp->processEvents();
            if (progress.wasCanceled())
                break;
        }
    }
    progress.close();

    ui->tblProgram->setUpdatesEnabled(true);

    parser->reset();

    updateProgramEstimatedTime(parser->getLinesFromParser(&gp, m_settings->arcPrecision(), m_settings->arcDegreeMode()));
    m_currentDrawer->update();
    ui->glwVisualizer->updateExtremes(m_currentDrawer);
    updateControlsState();

    if (m_currentModel == &m_programModel)
        m_fileChanged = true;

    qDebug() << "Update parser time: " << timer.elapsed();
}

void frmMain::on_cmdCommandSend_clicked()
{
    // FIXME: console
#if 0    
    QString command = ui->cboCommand->currentText();
    if (command.isEmpty())
        return;

    ui->cboCommand->storeText();
    ui->cboCommand->setCurrentText("");
    sendCommand(command, -1);
#endif
}

void frmMain::on_actFileOpen_triggered()
{
    on_cmdFileOpen_clicked();
}

void frmMain::onCmdHome_clicked()
{
    m_homing = true;
    m_updateSpindleSpeed = true;
    sendCommand("$H", -1, m_settings->showUICommands());
}

void frmMain::onCmdTouch_clicked()
{
    //    m_homing = true;

    QStringList list = m_settings->touchCommand().split(";");

    foreach (QString cmd, list)
    {
        sendCommand(cmd.trimmed(), -1, m_settings->showUICommands());
    }
}

void frmMain::onCmdZeroXY_clicked()
{
    m_settingZeroXY = true;
    sendCommand("G92X0Y0", -1, m_settings->showUICommands());
    sendCommand("$#", -2, m_settings->showUICommands());
}

void frmMain::onCmdZeroZ_clicked()
{
    m_settingZeroZ = true;
    sendCommand("G92Z0", -1, m_settings->showUICommands());
    sendCommand("$#", -2, m_settings->showUICommands());
}

void frmMain::onCmdRestoreOrigin_clicked()
{
    // Restore offset
    sendCommand(QString("G21"), -1, m_settings->showUICommands());

    auto mpos = toMetric(m_machineDisplay->getPosition());
    auto wpos = toMetric(m_workDisplay->getPosition());
    sendCommand(QString("G53G90G0X%1Y%2Z%3").arg(mpos.x()).arg(mpos.y()).arg(mpos.z()), -1, m_settings->showUICommands());
    sendCommand(QString("G92X%1Y%2Z%3").arg(wpos.x() - m_storedX).arg(wpos.y() - m_storedY).arg(wpos.z() - m_storedZ), -1, m_settings->showUICommands());

    // Move tool
    if (m_settings->moveOnRestore())
        switch (m_settings->restoreMode())
        {
        case 0:
            sendCommand("G0X0Y0", -1, m_settings->showUICommands());
            break;
        case 1:
            sendCommand("G0X0Y0Z0", -1, m_settings->showUICommands());
            break;
        }
}

void frmMain::onCmdReset_clicked()
{
    grblReset();
}

void frmMain::onCmdUnlock_clicked()
{
    m_updateSpindleSpeed = true;
    sendCommand("$X", -1, m_settings->showUICommands());
}

void frmMain::onCmdSafePosition_clicked()
{
    QStringList list = m_settings->safePositionCommand().split(";");

    foreach (QString cmd, list)
    {
        sendCommand(cmd.trimmed(), -1, m_settings->showUICommands());
    }
}

void frmMain::onCmdSpindleToggled(bool checked)
{
    m_spindleTab->setProperty("overrided", checked);
    // style()->unpolish(m_spindleTab);
    // m_spindleTab->ensurePolished();

#if 0
    if (checked)
    {
        if (!ui->grpSpindle->isChecked())
            ui->grpSpindle->setTitle(tr("Spindle") + QString(tr(" (%1)")).arg(ui->slbSpindle->value()));
    }
    else
    {
        ui->grpSpindle->setTitle(tr("Spindle"));
    }
#endif
}

void frmMain::onCmdSpindleClicked(bool checked)
{
    if (ui->cmdFilePause->isChecked())
    {
        m_serialPort.write(QByteArray(1, char(0x9e)));
    }
    else
    {
        sendCommand(checked ? QString("M3 S%1").arg(m_spindleTab->value()) : "M5", -1, m_settings->showUICommands());
    }
}

void frmMain::on_chkTestMode_clicked(bool checked)
{
    if (checked)
    {
        storeOffsets();
        storeParserState();
        sendCommand("$C", -1, m_settings->showUICommands());
    }
    else
    {
        m_aborting = true;
        grblReset();
    };
}

void frmMain::on_cmdFilePause_clicked(bool checked)
{
    m_serialPort.write(checked ? "!" : "~");
}

void frmMain::on_cmdFileReset_clicked()
{
    m_fileCommandIndex = 0;
    m_fileProcessedCommandIndex = 0;
    m_lastDrawnLineIndex = 0;
    m_probeIndex = -1;


    QElapsedTimer timer;

    timer.start();

    QList<LineSegment *> list = m_viewParser.getLineSegmentList();

    QList<int> indexes;
    for (int i = 0; i < list.count(); i++)
    {
        list[i]->setDrawn(false);
        indexes.append(i);
    }
    m_codeDrawer->update(indexes);

    qDebug() << "drawn false:" << timer.elapsed();

    timer.start();

    ui->tblProgram->setUpdatesEnabled(false);

    for (int i = 0; i < m_currentModel->data().count() - 1; i++)
    {
        m_currentModel->data()[i].state = GCodeItem::InQueue;
        m_currentModel->data()[i].response = QString();
    }
    ui->tblProgram->setUpdatesEnabled(true);

    qDebug() << "table updated:" << timer.elapsed();

    ui->tblProgram->scrollTo(m_currentModel->index(0, 0));
    ui->tblProgram->clearSelection();
    ui->tblProgram->selectRow(0);

    ui->glwVisualizer->setSpendTime(QTime(0, 0, 0));

}

void frmMain::on_actFileNew_triggered()
{
    qDebug() << "changes:" << m_fileChanged;

    if (!saveChanges(false))
        return;

    // Reset tables
    clearTable();
    m_probeModel.clear();
    m_currentModel = &m_programModel;

    // Reset parsers
    m_viewParser.reset();
    m_probeParser.reset();

    // Reset code drawer
    m_codeDrawer->update();
    m_currentDrawer = m_codeDrawer;
    ui->glwVisualizer->fitDrawable();
    updateProgramEstimatedTime(QList<LineSegment *>());

    m_programFileName = "";

    // Reset tableview
    QByteArray headerState = ui->tblProgram->horizontalHeader()->saveState();
    ui->tblProgram->setModel(NULL);

    // Set table model
    ui->tblProgram->setModel(&m_programModel);
    ui->tblProgram->horizontalHeader()->restoreState(headerState);

    // Update tableview
    connect(ui->tblProgram->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)), this, SLOT(onTableCurrentChanged(QModelIndex, QModelIndex)));
    ui->tblProgram->selectRow(0);

    // Clear selection marker
    m_selectionDrawer.setEndPosition(QVector3D(sNan, sNan, sNan));
    m_selectionDrawer.update();


    updateControlsState();
}

void frmMain::on_cmdClearConsole_clicked()
{
    m_consoleTab->clear();
}

bool frmMain::saveProgramToFile(QString fileName, GCodeTableModel *model)
{
    QFile file(fileName);
    QDir dir;

    qDebug() << "Saving program";

    if (file.exists())
        dir.remove(file.fileName());
    if (!file.open(QIODevice::WriteOnly))
        return false;

    QTextStream textStream(&file);

    for (int i = 0; i < model->rowCount() - 1; i++)
    {
        textStream << model->data(model->index(i, 1)).toString() << "\r\n";
    }

    file.close();

    return true;
}

void frmMain::on_actFileSaveTransformedAs_triggered()
{
    QString fileName = (QFileDialog::getSaveFileName(this, tr("Save file as"), m_lastFolder, tr("G-Code files (*.nc *.ncc *.ngc *.tap  *.gcode *.gc *.txt)")));

    if (!fileName.isEmpty())
    {
        //saveProgramToFile(fileName, &m_programHeightmapModel);
    }
}

void frmMain::on_actFileSaveAs_triggered()
{
    QString fileName = (QFileDialog::getSaveFileName(this, tr("Save file as"), m_lastFolder, tr("G-Code files (*.nc *.ncc *.ngc *.tap  *.gcode *.gc *.txt)")));

    if (!fileName.isEmpty())
    {
        if (saveProgramToFile(fileName, &m_programModel))
        {
            m_programFileName = fileName;
            m_fileChanged = false;

            addRecentFile(fileName);
            updateRecentFilesMenu();

            updateControlsState();
        }
    }
}

void frmMain::on_actFileSave_triggered()
{
    // G-code saving
    if (m_programFileName.isEmpty())
        on_actFileSaveAs_triggered();
    else
    {
        saveProgramToFile(m_programFileName, &m_programModel);
        m_fileChanged = false;
    }
}

void frmMain::on_cmdTop_clicked()
{
    ui->glwVisualizer->setTopView();
}

void frmMain::on_cmdFront_clicked()
{
    ui->glwVisualizer->setFrontView();
}

void frmMain::on_cmdLeft_clicked()
{
    ui->glwVisualizer->setLeftView();
}

void frmMain::on_cmdIsometric_clicked()
{
    ui->glwVisualizer->setIsometricView();
}

void frmMain::on_actAbout_triggered()
{
    m_frmAbout.exec();
}

bool frmMain::dataIsEnd(QString data)
{
    QStringList ends;

    ends << "ok";
    ends << "error";
    //    ends << "Reset to continue";
    //    ends << "'$' for help";
    //    ends << "'$H'|'$X' to unlock";
    //    ends << "Caution: Unlocked";
    //    ends << "Enabled";
    //    ends << "Disabled";
    //    ends << "Check Door";
    //    ends << "Pgm End";

    foreach (QString str, ends)
    {
        if (data.contains(str))
            return true;
    }

    return false;
}

bool frmMain::dataIsFloating(QString data)
{
    QStringList ends;

    ends << "Reset to continue";
    ends << "'$H'|'$X' to unlock";
    ends << "ALARM: Soft limit";
    ends << "ALARM: Hard limit";
    ends << "Check Door";

    foreach (QString str, ends)
    {
        if (data.contains(str))
            return true;
    }

    return false;
}

bool frmMain::dataIsReset(QString data)
{
    return QRegExp("^GRBL|GCARVIN\\s\\d\\.\\d.").indexIn(data.toUpper()) != -1;
}

QString frmMain::feedOverride(QString command)
{
    // Feed override if not in heightmap probing mode
    //    if (!ui->cmdHeightMapMode->isChecked()) command = GcodePreprocessorUtils::overrideSpeed(command, ui->chkFeedOverride->isChecked() ?
    //        ui->txtFeed->value() : 100, &m_originalFeed);

    return command;
}

bool frmMain::eventFilter(QObject *obj, QEvent *event)
{
    // FIXME: add jog widget..
    // Main form events
    // if (obj == this || obj == ui->tblProgram || obj == ui->cboJogStep || obj == ui->cboJogFeed)
    if (obj == this || obj == ui->tblProgram)
    {

#if 0
        // FIXME:
        // Jog on keyboard control
        if (!m_processingFile && ui->chkKeyboardControl->isChecked() &&
            (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) && !static_cast<QKeyEvent *>(event)->isAutoRepeat())
        {

            switch (static_cast<QKeyEvent *>(event)->key())
            {
            case Qt::Key_4:
                if (event->type() == QEvent::KeyPress)
                    emit ui->cmdXMinus->pressed();
                else
                    emit ui->cmdXMinus->released();
                break;
            case Qt::Key_6:
                if (event->type() == QEvent::KeyPress)
                    emit ui->cmdXPlus->pressed();
                else
                    emit ui->cmdXPlus->released();
                break;
            case Qt::Key_8:
                if (event->type() == QEvent::KeyPress)
                    emit ui->cmdYPlus->pressed();
                else
                    emit ui->cmdYPlus->released();
                break;
            case Qt::Key_2:
                if (event->type() == QEvent::KeyPress)
                    emit ui->cmdYMinus->pressed();
                else
                    emit ui->cmdYMinus->released();
                break;
            case Qt::Key_9:
                if (event->type() == QEvent::KeyPress)
                    emit ui->cmdZPlus->pressed();
                else
                    emit ui->cmdZPlus->released();
                break;
            case Qt::Key_3:
                if (event->type() == QEvent::KeyPress)
                    emit ui->cmdZMinus->pressed();
                else
                    emit ui->cmdZMinus->released();
                break;
            }
        }
#endif

        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

            if (!m_processingFile && keyEvent->key() == Qt::Key_ScrollLock && obj == this)
            {
                // FIXME:
                // ui->chkKeyboardControl->toggle();
                // if (!ui->chkKeyboardControl->isChecked())
                {
                    // FIXME: console
                    // ui->cboCommand->setFocus();
                }
            }

            // FIXME: jog
            // if (!m_processingFile && ui->chkKeyboardControl->isChecked())
            if (!m_processingFile)
            {
                if (keyEvent->key() == Qt::Key_7)
                {
                    // ui->cboJogStep->setCurrentPrevious();
                }
                else if (keyEvent->key() == Qt::Key_1)
                {
                    // ui->cboJogStep->setCurrentNext();
                }
                else if (keyEvent->key() == Qt::Key_Minus)
                {
                    // ui->cboJogFeed->setCurrentPrevious();
                }
                else if (keyEvent->key() == Qt::Key_Plus)
                {
                    // ui->cboJogFeed->setCurrentNext();
                }
                else if (keyEvent->key() == Qt::Key_5)
                {
                    onJogStopClicked();
                }
                else if (keyEvent->key() == Qt::Key_0)
                {
                    onCmdSpindleClicked(!m_spindleTab->isChecked());
                }
                else if (keyEvent->key() == Qt::Key_Asterisk)
                {
                    m_spindleTab->incrementSlider();
                }
                else if (keyEvent->key() == Qt::Key_Slash)
                {
                    m_spindleTab->decrementSlider();
                }
            }

            if (obj == ui->tblProgram && m_processingFile)
            {
                if (keyEvent->key() == Qt::Key_PageDown || keyEvent->key() == Qt::Key_PageUp || keyEvent->key() == Qt::Key_Down || keyEvent->key() == Qt::Key_Up)
                {
                    ui->chkAutoScroll->setChecked(false);
                }
            }
        }
    }

    return QMainWindow::eventFilter(obj, event);
}

int frmMain::getConsoleMinHeight()
{
    // FIXME: console
    return m_consoleTab->minimumHeight();
}

void frmMain::onConsoleResized(QSize size)
{
    Q_UNUSED(size)

    int minHeight = getConsoleMinHeight();
    bool visible = m_consoleTab->height() > minHeight;
    if (m_consoleTab->isVisible() != visible)
    {
        // m_consoleTab->setVisible(visible);
    }

    m_consoleTab->setVisible(true);
}

bool frmMain::keyIsMovement(int key)
{
    return key == Qt::Key_4 || key == Qt::Key_6 || key == Qt::Key_8 || key == Qt::Key_2 || key == Qt::Key_9 || key == Qt::Key_3;
}

void frmMain::on_chkKeyboardControl_toggled(bool checked)
{
#if 0
    ui->grpJog->setProperty("overrided", checked);
    style()->unpolish(ui->grpJog);
    ui->grpJog->ensurePolished();
#endif

    // Store/restore coordinate system
    if (checked)
    {
        sendCommand("$G", -2, m_settings->showUICommands());
    }
    else
    {
        if (m_absoluteCoordinates)
            sendCommand("G90", -1, m_settings->showUICommands());
    }

    if (!m_processingFile)
        m_storedKeyboardControl = checked;

    updateControlsState();
}

void frmMain::on_tblProgram_customContextMenuRequested(const QPoint &pos)
{
    if (m_processingFile)
        return;

    if (ui->tblProgram->selectionModel()->selectedRows().count() > 0)
    {
        m_tableMenu->actions().at(0)->setEnabled(true);
        m_tableMenu->actions().at(1)->setEnabled(ui->tblProgram->selectionModel()->selectedRows()[0].row() != m_currentModel->rowCount() - 1);
    }
    else
    {
        m_tableMenu->actions().at(0)->setEnabled(false);
        m_tableMenu->actions().at(1)->setEnabled(false);
    }
    m_tableMenu->popup(ui->tblProgram->viewport()->mapToGlobal(pos));
}

void frmMain::on_splitter_splitterMoved(int pos, int index)
{
    Q_UNUSED(pos)
    Q_UNUSED(index)

    static bool tableCollapsed = ui->splitter->sizes()[1] == 0;

    if ((ui->splitter->sizes()[1] == 0) != tableCollapsed)
    {
        this->setUpdatesEnabled(false);
        ui->chkAutoScroll->setVisible(ui->splitter->sizes()[1]);
        updateLayouts();
        resizeCheckBoxes();

        this->setUpdatesEnabled(true);
        ui->chkAutoScroll->repaint();

        // Store collapsed state
        tableCollapsed = ui->splitter->sizes()[1] == 0;
    }
}

void frmMain::updateLayouts()
{
    this->update();
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
}

void frmMain::addRecentFile(QString fileName)
{
    m_recentFiles.removeAll(fileName);
    m_recentFiles.append(fileName);
    if (m_recentFiles.count() > 5)
        m_recentFiles.takeFirst();
}


void frmMain::onActRecentFileTriggered()
{
    QAction *action = static_cast<QAction *>(sender());
    QString fileName = action->text();

    if (action != NULL)
    {
        if (!saveChanges(false))
            return;

        loadFile(fileName);
    }
}

void frmMain::onCboCommandReturnPressed()
{
    // FIXME: console
    QString command = m_consoleTab->getCommand();
    if (command.isEmpty())
        return;

    m_consoleTab->clearCommand();
    sendCommand(command, -1);
}

void frmMain::updateRecentFilesMenu()
{
    foreach (QAction *action, ui->mnuRecent->actions())
    {
        if (action->text() == "")
            break;
        else
        {
            ui->mnuRecent->removeAction(action);
            delete action;
        }
    }

    foreach (QString file, m_recentFiles)
    {
        QAction *action = new QAction(file, this);
        connect(action, SIGNAL(triggered()), this, SLOT(onActRecentFileTriggered()));
        ui->mnuRecent->insertAction(ui->mnuRecent->actions()[0], action);
    }

    updateControlsState();
}

void frmMain::on_actRecentClear_triggered()
{
    m_recentFiles.clear();
    updateRecentFilesMenu();
}

double frmMain::toMetric(double value) const
{
    return m_settings->units() == 0 ? value : value * 25.4;
}

QVector3D frmMain::toMetric(const QVector3D &value) const
{
    return QVector3D(toMetric(value.x()),
                     toMetric(value.y()),
                     toMetric(value.z()));
}

QRectF frmMain::borderRectFromExtremes()
{
    QRectF rect;

    rect.setX(m_codeDrawer->getMinimumExtremes().x());
    rect.setY(m_codeDrawer->getMinimumExtremes().y());
    rect.setWidth(m_codeDrawer->getSizes().x());
    rect.setHeight(m_codeDrawer->getSizes().y());

    return rect;
}

bool frmMain::compareCoordinates(double x, double y, double z)
{
    return m_machineDisplay->getPosition() == QVector3D(x, y, z);
}

void frmMain::onCmdUserClicked(bool checked)
{
    Q_UNUSED(checked);

    int i = sender()->objectName().right(1).toInt();

    QStringList list = m_settings->userCommands(i).split(";");

    foreach (QString cmd, list)
    {
        sendCommand(cmd.trimmed(), -1, m_settings->showUICommands());
    }
}

void frmMain::updateOverride(SliderBox *slider, int value, char command)
{
    slider->setCurrentValue(value);

    int target = slider->isChecked() ? slider->value() : 100;
    bool smallStep = abs(target - slider->currentValue()) < 10 || m_settings->queryStateTime() < 100;

    if (slider->currentValue() < target)
    {
        m_serialPort.write(QByteArray(1, char(smallStep ? command + 2 : command)));
    }
    else if (slider->currentValue() > target)
    {
        m_serialPort.write(QByteArray(1, char(smallStep ? command + 3 : command + 1)));
    }
}

void frmMain::jogStep()
{
    QVector3D jogVector = m_jogWidget->getJogVector();

    if (jogVector.length() == 0)
        return;

    if (m_jogWidget->getStep() == 0)
    {
        const double acc = m_settings->acceleration();         // Acceleration mm/sec^2
        int speed = static_cast<int>(m_jogWidget->getFeed());  // Speed mm/min
        double v = (double)speed / 60;                         // Rapid speed mm/sec
        int N = 15;                                            // Planner blocks
        double dt = qMax(0.01, sqrt(v) / (2 * acc * (N - 1))); // Single jog command time
        double s = v * dt;                                     // Jog distance

        QVector3D vec = jogVector.normalized() * s;

        //    qDebug() << "jog" << speed << v << acc << dt <<s;

        sendCommand(QString("$J=G21G91X%1Y%2Z%3F%4")
                        .arg(vec.x(), 0, 'g', 4)
                        .arg(vec.y(), 0, 'g', 4)
                        .arg(vec.z(), 0, 'g', 4)
                        .arg(speed),
                    -2, m_settings->showUICommands());
    }
    else
    {
        int speed = static_cast<int>(m_jogWidget->getFeed()); // Speed mm/min
        QVector3D vec = jogVector * m_jogWidget->getStep();

        sendCommand(QString("$J=G21G91X%1Y%2Z%3F%4")
                        .arg(vec.x(), 0, 'g', 4)
                        .arg(vec.y(), 0, 'g', 4)
                        .arg(vec.z(), 0, 'g', 4)
                        .arg(speed),
                    -3, m_settings->showUICommands());
    }
}

void frmMain::onJogVectorChanged()
{
    jogStep();
}

void frmMain::onJogStopClicked()
{
    m_queue.clear();
    m_serialPort.write(QByteArray(1, char(0x85)));
    qDebug() << "Jog: stop";
}
