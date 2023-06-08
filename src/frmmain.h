// This file is a part of "CandleM" application.
// Copyright 2015-2016 Hayrullin Denis Ravilevich
// Copyright 2023 Niels Moseley

#ifndef FRMMAIN_H
#define FRMMAIN_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include <QSettings>
#include <QTimer>
#include <QBasicTimer>
#include <QStringList>
#include <QList>
#include <QElapsedTimer>
#include <QMenu>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QProgressDialog>
#include <exception>

#include "parser/gcodeviewparse.h"

#include "drawers/origindrawer.h"
#include "drawers/gcodedrawer.h"
#include "drawers/tooldrawer.h"
#include "drawers/shaderdrawable.h"
#include "drawers/selectiondrawer.h"

#include "tables/gcodetablemodel.h"

#include "utils/interpolation.h"

#include "widgets/styledtoolbutton.h"
#include "widgets/sliderbox.h"
#include "widgets/positiondisplay.h"
#include "widgets/statuswidget.h"
#include "widgets/consoletab.h"
#include "widgets/overridetab.h"
#include "widgets/spindletab.h"
#include "widgets/jogwidget.h"
#include "widgets/buttonbar.h"

#include "frmsettings.h"
#include "frmabout.h"

#ifdef WINDOWS
    #include <QtWinExtras/QtWinExtras>
    #include "shobjidl.h"
#endif

namespace Ui {
class frmMain;
}

struct CommandAttributes {
    int length;
    int consoleIndex;
    int tableIndex;
    QString command;
};

struct CommandQueue {
    QString command;
    int tableIndex;
    bool showInConsole;
};

class CancelException : public std::exception {
public:
#ifdef Q_OS_MAC
#undef _GLIBCXX_USE_NOEXCEPT
#define _GLIBCXX_USE_NOEXCEPT _NOEXCEPT
#endif

    const char* what() const noexcept override
    {
        return "Operation was cancelled by user";
    }
};

class frmMain : public QMainWindow
{
    Q_OBJECT

public:
    explicit frmMain(QWidget *parent = 0);
    ~frmMain();

    double toolZPosition();

private slots:
    void placeVisualizerButtons();

    void onSerialPortReadyRead();
    void onSerialPortError(QSerialPort::SerialPortError);
    void onTimerConnection();
    void onTimerStateQuery();
    void onVisualizatorRotationChanged();
    void onScroolBarAction(int action);
    void onJogTimer();
    void onTableInsertLine();
    void onTableDeleteLines();
    void onActRecentFileTriggered();
    void onCboCommandReturnPressed();
    void onTableCurrentChanged(QModelIndex idx1, QModelIndex idx2);
    void onConsoleResized(QSize size);
    void onCmdUserClicked(bool checked);
    void onActSendFromLineTriggered();

    void on_actFileExit_triggered();
    void on_cmdFileOpen_clicked();
    void on_cmdFit_clicked();
    void on_cmdFileSend_clicked();
    void onTableCellChanged(QModelIndex i1, QModelIndex i2);
    void on_actServiceSettings_triggered();
    void on_actFileOpen_triggered();
    void on_cmdCommandSend_clicked();

    // Button bar messages
    void onCmdHome_clicked();
    void onCmdTouch_clicked();
    void onCmdZeroXY_clicked();
    void onCmdZeroZ_clicked();
    void onCmdRestoreOrigin_clicked();
    void onCmdReset_clicked();
    void onCmdUnlock_clicked();
    void onCmdSafePosition_clicked();
    
    void onCmdSpindleToggled(bool checked);
    void on_chkTestMode_clicked(bool checked);
    void on_cmdFilePause_clicked(bool checked);
    void on_cmdFileReset_clicked();
    void on_actFileNew_triggered();
    void on_cmdClearConsole_clicked();
    void on_actFileSaveAs_triggered();
    void on_actFileSave_triggered();
    void on_actFileSaveTransformedAs_triggered();
    void on_cmdTop_clicked();
    void on_cmdFront_clicked();
    void on_cmdLeft_clicked();
    void on_cmdIsometric_clicked();
    void on_actAbout_triggered();
    void on_chkKeyboardControl_toggled(bool checked);
    void on_tblProgram_customContextMenuRequested(const QPoint &pos);
    void on_splitter_splitterMoved(int pos, int index);
    void on_actRecentClear_triggered();

    void on_cmdFileAbort_clicked();
    void onCmdSpindleClicked(bool checked);

    void onJogVectorChanged();
    void onJogStopClicked();

protected:
    void showEvent(QShowEvent *se);
    void hideEvent(QHideEvent *he);
    void resizeEvent(QResizeEvent *re);
    void timerEvent(QTimerEvent *);
    void closeEvent(QCloseEvent *ce);
    void dragEnterEvent(QDragEnterEvent *dee);
    void dropEvent(QDropEvent *de);

private:
    const int BUFFERLENGTH = 127;

    Ui::frmMain *ui;
    GcodeViewParse m_viewParser;
    GcodeViewParse m_probeParser;

    OriginDrawer *m_originDrawer;

    GcodeDrawer *m_codeDrawer;    
    GcodeDrawer *m_probeDrawer;
    GcodeDrawer *m_currentDrawer;

    ToolDrawer m_toolDrawer;
    SelectionDrawer m_selectionDrawer;

    GCodeTableModel m_programModel;
    GCodeTableModel m_probeModel;

    bool m_programLoading;
    bool m_settingsLoading;

    QSerialPort m_serialPort;

    frmSettings *m_settings;
    frmAbout m_frmAbout;

    GUI::PositionDisplay *m_workDisplay    = nullptr;
    GUI::PositionDisplay *m_machineDisplay = nullptr;
    GUI::StatusWidget    *m_statusWidget   = nullptr;
    GUI::ConsoleTab      *m_consoleTab     = nullptr;
    GUI::OverrideTab     *m_overrideTab    = nullptr;
    GUI::SpindleTab      *m_spindleTab     = nullptr;
    GUI::JogWidget       *m_jogWidget      = nullptr;
    GUI::ButtonBar       *m_buttonBar      = nullptr;
    
    QTabWidget           *m_tabWidget      = nullptr;
    
    QString m_settingsFileName;
    QString m_programFileName;
    QString m_lastFolder;

    bool m_fileChanged = false;

    QTimer m_timerConnection;
    QTimer m_timerStateQuery;
    QBasicTimer m_timerToolAnimation;

#ifdef WINDOWS
    QWinTaskbarButton *m_taskBarButton;
    QWinTaskbarProgress *m_taskBarProgress;
#endif

    QMenu *m_tableMenu;
    QList<CommandAttributes> m_commands;
    QList<CommandQueue> m_queue;
    QElapsedTimer m_startTimer;

    QMessageBox* m_senderErrorBox;

    // Stored origin
    double m_storedX = 0;
    double m_storedY = 0;
    double m_storedZ = 0;
    QString m_storedParserStatus;

    // Console window
    int m_storedConsoleMinimumHeight;
    int m_storedConsoleHeight;
    int m_consolePureHeight;

    // Flags
    bool m_settingZeroXY = false;
    bool m_settingZeroZ = false;
    bool m_homing = false;
    bool m_updateSpindleSpeed = false;
    bool m_updateParserStatus = false;
    bool m_updateFeed = false;

    bool m_reseting = false;
    bool m_resetCompleted = true;
    bool m_aborting = false;
    bool m_statusReceived = false;

    bool m_processingFile = false;
    bool m_transferCompleted = false;
    bool m_fileEndSent = false;

    bool m_cellChanged;

    // Indices
    int m_fileCommandIndex;
    int m_fileProcessedCommandIndex;
    int m_probeIndex;

    using StatusType = typename GUI::StatusWidget::StatusType;

    // Current values
    int m_lastDrawnLineIndex;
    double m_originalFeed;

    // Keyboard
    bool m_keyPressed = false;
    bool m_jogBlock = false;
    bool m_absoluteCoordinates;
    bool m_storedKeyboardControl;

    // Spindle
    bool m_spindleCW = true;
    bool m_spindleCommandSpeed = false;

    // Jog
    QStringList m_recentFiles;
    QStringList m_recentHeightmaps;

    void loadFile(QString fileName);
    void loadFile(QList<QString> data);
    void clearTable();
    void preloadSettings();
    void loadSettings();
    void saveSettings();
    bool saveChanges(bool heightMapMode);
    void updateControlsState();
    void openPort();
    void sendCommand(QString command, int tableIndex = -1, bool showInConsole = true);
    void grblReset();
    int  bufferLength();
    void sendNextFileCommands();
    void applySettings();
    void updateParser();
    bool dataIsFloating(QString data);
    bool dataIsEnd(QString data);
    bool dataIsReset(QString data);

    QTime updateProgramEstimatedTime(QList<LineSegment *> lines);
    bool saveProgramToFile(QString fileName, GCodeTableModel *model);
    QString feedOverride(QString command);

    bool eventFilter(QObject *obj, QEvent *event);
    bool keyIsMovement(int key);
    void resizeCheckBoxes();
    void updateLayouts();
    void updateRecentFilesMenu();
    void addRecentFile(QString fileName);

    // Convert inches to metric depending on
    // m_settings->units()
    double toMetric(double value) const;
    QVector3D toMetric(const QVector3D &value) const;

    QRectF borderRectFromTextboxes();
    QRectF borderRectFromExtremes();

    GCodeTableModel *m_currentModel;

    void storeParserState();
    void restoreParserState();
    void storeOffsets();
    void restoreOffsets();
    bool isGCodeFile(QString fileName);
    bool compareCoordinates(double x, double y, double z);
    int  getConsoleMinHeight();
    void updateOverride(SliderBox *slider, int value, char command);
    void jogStep();
};

#endif // FRMMAIN_H
