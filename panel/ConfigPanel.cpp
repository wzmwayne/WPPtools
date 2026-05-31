#include "ConfigPanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QCoreApplication>
#include <QApplication>
#include <QLocalSocket>
#include <QDialog>
#include <QTextBrowser>
#include <QDesktopServices>
#include <QMouseEvent>
#include <QUrl>
#include <windows.h>
#include <cstdio>

#define LOG(fmt, ...) do { \
    fprintf(stderr, "[WPStoolsPanel] " fmt "\n", ##__VA_ARGS__); \
    fflush(stderr); \
} while(0)

// ── ConfigPanel ─────────────────────────────────────────────────────────

ConfigPanel::ConfigPanel(QWidget *parent) : QWidget(parent) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    resize(480, 520);

    qApp->setStyleSheet(QString::fromUtf8(
        "QWidget { background: #111; color: #ddd; font-size: 13px; }"
        "QGroupBox {"
            "border: 1px solid #333; border-radius: 6px;"
            "margin-top: 14px; padding: 14px 10px 10px; font-weight: bold;"
        "}"
        "QGroupBox::title {"
            "subcontrol-origin: margin; left: 10px; padding: 0 4px;"
        "}"
        "QPushButton {"
            "background: #333; border: none; border-radius: 4px;"
            "padding: 7px 18px;"
        "}"
        "QPushButton:hover { background: #444; }"
        "QPushButton:pressed { background: #555; }"
        "QPushButton:disabled { background: #222; color: #555; }"
        "QSlider::groove:horizontal {"
            "background: #333; height: 6px; border-radius: 3px;"
        "}"
        "QSlider::handle:horizontal {"
            "background: #aaa; width: 14px; height: 14px;"
            "margin: -4px 0; border-radius: 7px;"
        "}"
        "QSpinBox { background: #222; border: 1px solid #333;"
            "border-radius: 4px; padding: 3px 6px; min-width: 56px; color: #fff; }"
        "QLabel { background: transparent; }"
    ));

    m_timer = new QTimer(this);
    m_timer->setInterval(1000);
    connect(m_timer, &QTimer::timeout, this, &ConfigPanel::updateStatus);
    m_timer->start();

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // title bar
    m_titleBar = new QWidget;
    m_titleBar->setFixedHeight(32);
    m_titleBar->setStyleSheet("QWidget { background: #1a1a1a; border-bottom: 1px solid #333; }");
    auto *tbLayout = new QHBoxLayout(m_titleBar);
    tbLayout->setContentsMargins(10, 0, 4, 0);
    auto *titleLbl = new QLabel(QString::fromUtf8("WPStools \u914d\u7f6e\u9762\u677f"));
    titleLbl->setStyleSheet("font-weight: bold; color: #eee; background: transparent;");
    auto *closeBtn = new QPushButton(QString::fromUtf8("\u00d7"));
    closeBtn->setFixedSize(28, 28);
    closeBtn->setStyleSheet(
        "QPushButton { background: transparent; color: #fff;"
        "font-size: 15px; border: none; border-radius: 4px; }"
        "QPushButton:hover { background: #c33; color: #fff; }"
    );
    tbLayout->addWidget(titleLbl);
    tbLayout->addStretch();
    tbLayout->addWidget(closeBtn);
    root->addWidget(m_titleBar);
    connect(closeBtn, &QPushButton::clicked, this, &QWidget::close);

    // content wrapper
    auto *content = new QVBoxLayout;
    content->setContentsMargins(16, 10, 16, 12);
    content->setSpacing(10);

    // header
    auto *header = new QHBoxLayout;
    m_statusIcon = new QLabel(QString::fromUtf8("\u25cf"));
    m_statusIcon->setStyleSheet("color: #6a6; font-size: 18px; background: transparent;");
    m_statusIcon->setFixedWidth(20);
    m_statusText = new QLabel(QString::fromUtf8("\u8fde\u63a5\u4e2d..."));
    m_startBtn = new QPushButton(QString::fromUtf8("\u25b6 \u542f\u52a8"));
    m_stopBtn  = new QPushButton(QString::fromUtf8("\u25a0 \u505c\u6b62"));
    m_stopBtn->setEnabled(false);
    header->addWidget(m_statusIcon);
    header->addWidget(m_statusText);
    header->addStretch();
    header->addWidget(m_startBtn);
    header->addWidget(m_stopBtn);
    content->addLayout(header);

    // toolbar group
    auto *tg = new QGroupBox(QString::fromUtf8("\u5de5\u5177\u680f\u8bbe\u7f6e"));
    auto *tl = new QVBoxLayout(tg);
    tl->addWidget(makeSlider(QString::fromUtf8("\u900f\u660e\u5ea6"),      "toolbar_opacity",     0, 255, 140));
    tl->addWidget(makeSlider(QString::fromUtf8("\u56fe\u6807\u5927\u5c0f"),  "toolbar_icon_size",   16, 64, 28));
    tl->addWidget(makeSlider(QString::fromUtf8("\u6309\u94ae\u5927\u5c0f"),  "toolbar_button_size", 28, 80, 44));
    content->addWidget(tg);

    // control group
    auto *cg = new QGroupBox(QString::fromUtf8("\u4fa7\u8fb9\u63a7\u5236\u8bbe\u7f6e"));
    auto *cl = new QVBoxLayout(cg);
    cl->addWidget(makeSlider(QString::fromUtf8("\u900f\u660e\u5ea6"),      "control_opacity",      0, 255, 160));
    cl->addWidget(makeSlider(QString::fromUtf8("\u5b57\u53f7"),            "control_font_size",    16, 64, 32));
    cl->addWidget(makeSlider(QString::fromUtf8("\u9762\u677f\u5bbd\u5ea6"), "control_width",        60, 200, 90));
    cl->addWidget(makeSlider(QString::fromUtf8("\u9762\u677f\u9ad8\u5ea6"), "control_height",      100, 400, 200));
    cl->addWidget(makeSlider(QString::fromUtf8("\u8fb9\u7f18\u8ddd\u79bb"), "control_edge_offset",   0, 50, 4));
    content->addWidget(cg);

    // auto-start
    m_autoStartCb = new QCheckBox(QString::fromUtf8("\u5f00\u673a\u81ea\u542f\u52a8 WPPTouchHelper"));
    m_autoStartCb->setStyleSheet(
        "QCheckBox { color: #ddd; font-size: 13px; spacing: 8px; }"
        "QCheckBox::indicator { width: 16px; height: 16px; }"
    );
    {
        QProcess proc;
        proc.start("powershell", {
            "-NoProfile", "-Command",
            "Get-ScheduledTask -TaskName 'touchtools' -ErrorAction SilentlyContinue"
        });
        proc.waitForFinished(3000);
        m_autoStartCb->setChecked(proc.exitCode() == 0
            && proc.readAllStandardOutput().contains("touchtools"));
    }
    }
    content->addWidget(m_autoStartCb);
    connect(m_autoStartCb, &QCheckBox::toggled, this, &ConfigPanel::onAutoStartToggled);

    // save
    m_saveBtn = new QPushButton(QString::fromUtf8("\u4fdd\u5b58\u914d\u7f6e"));
    m_saveBtn->setStyleSheet(
        "QPushButton { background: #555; color: #fff; font-weight: bold;"
        "padding: 9px 0; border-radius: 4px; }"
        "QPushButton:hover { background: #666; }"
        "QPushButton:pressed { background: #777; }"
    );
    content->addWidget(m_saveBtn);

    // about
    m_aboutBtn = new QPushButton(QString::fromUtf8("\u5173\u4e8e WPStools ..."));
    m_aboutBtn->setStyleSheet(
        "QPushButton { background: #2a2a2a; color: #999;"
        "padding: 5px 0; border-radius: 4px; }"
        "QPushButton:hover { color: #ddd; background: #3a3a3a; }"
    );
    content->addWidget(m_aboutBtn);

    root->addLayout(content);

    connect(m_startBtn, &QPushButton::clicked, this, &ConfigPanel::onStart);
    connect(m_stopBtn,  &QPushButton::clicked, this, &ConfigPanel::onStop);
    connect(m_saveBtn,  &QPushButton::clicked, this, &ConfigPanel::onSave);
    connect(m_aboutBtn, &QPushButton::clicked, this, &ConfigPanel::onAbout);

    loadCfg();
    updateStatus();
}

void ConfigPanel::loadCfg() {
    QString path = QCoreApplication::applicationDirPath() + "/WPStools.json";
    QFile f(path);
    if (f.open(QIODevice::ReadOnly)) {
        m_cfg = QJsonDocument::fromJson(f.readAll()).object();
        f.close();
    }
    for (auto &e : m_entries) {
        int v = m_cfg.contains(e.key) ? m_cfg[e.key].toInt() : e.sl->value();
        e.sl->setValue(v);
        e.sp->setValue(v);
    }
}

void ConfigPanel::saveCfg() {
    for (auto &e : m_entries)
        m_cfg[e.key] = e.sl->value();
    QString path = QCoreApplication::applicationDirPath() + "/WPStools.json";
    QFile f(path);
    if (f.open(QIODevice::WriteOnly)) {
        f.write(QJsonDocument(m_cfg).toJson());
        f.close();
    }
}

static bool isPipeRunning() {
    QLocalSocket sock;
    sock.connectToServer("WPStoolsPipe");
    if (sock.waitForConnected(200)) {
        sock.disconnectFromServer();
        return true;
    }
    return false;
}

void ConfigPanel::updateStatus() {
    bool running = isPipeRunning();

    if (running) {
        m_statusIcon->setStyleSheet("color: #6a6; font-size: 18px; background: transparent;");
        m_statusText->setText(QString::fromUtf8("\u8fd0\u884c\u4e2d"));
        m_startBtn->setEnabled(false);
        m_stopBtn->setEnabled(true);
    } else {
        m_statusIcon->setStyleSheet("color: #866; font-size: 18px; background: transparent;");
        m_statusText->setText(QString::fromUtf8("\u5df2\u505c\u6b62"));
        m_startBtn->setEnabled(true);
        m_stopBtn->setEnabled(false);
    }
}

void ConfigPanel::onStart() {
    if (isPipeRunning()) return;
    QString exe = QCoreApplication::applicationDirPath() + "/WPPTouchHelper.exe";
    QProcess::startDetached(exe);
    updateStatus();
}

void ConfigPanel::onStop() {
    QLocalSocket sock;
    sock.connectToServer("WPStoolsPipe");
    if (sock.waitForConnected(500)) {
        sock.write("exit");
        sock.waitForBytesWritten(500);
        sock.disconnectFromServer();
        sock.waitForDisconnected(1000);
    }
    QTimer::singleShot(500, this, &ConfigPanel::updateStatus);
}

void ConfigPanel::onSave() {
    saveCfg();

    QLocalSocket sock;
    sock.connectToServer("WPStoolsPipe");
    if (sock.waitForConnected(500)) {
        sock.write("show");
        sock.waitForBytesWritten(500);
        sock.disconnectFromServer();
    }
}

void ConfigPanel::onAbout() {
    auto *dlg = new QDialog(this);
    dlg->setWindowTitle(QString::fromUtf8("\u5173\u4e8e WPStools"));
    dlg->resize(520, 400);
    auto *vl = new QVBoxLayout(dlg);
    auto *tb = new QTextBrowser(dlg);
    tb->setOpenExternalLinks(true);
    tb->setHtml(QString::fromUtf8(
        "<h2>WPStools</h2>"
        "<p>WPS PPT \u89e6\u5c4f\u8f85\u52a9\u5de5\u5177</p>"
        "<hr>"
        "<p>\u7248\u6743 (C) 2026 Wayne</p>"
        "<p>\u672c\u7a0b\u5e8f\u662f\u81ea\u7531\u8f6f\u4ef6\uff1a"
        "\u60a8\u53ef\u4ee5\u91cd\u65b0\u53d1\u5e03\u5b83\u6216\u4fee\u6539\u5b83\uff0c"
        "\u524d\u63d0\u662f\u9075\u5b88 "
        "<a href=\"https://www.gnu.org/licenses/gpl-3.0.html\">GNU General Public License v3</a>\u3002</p>"
        "<p>\u672c\u7a0b\u5e8f\u53d1\u5e03\u65f6\u5e0c\u671b\u5b83\u6709\u7528\uff0c"
        "\u4f46\u4e0d\u63d0\u4f9b\u4efb\u4f55\u4fdd\u8bc1\uff1b"
        "\u65e0\u4e8b\u5148\u7684\u5546\u4e1a\u6027\u6216\u7279\u5b9a\u76ee\u7684\u9002\u7528\u6027\u4fdd\u8bc1\u3002</p>"
        "<hr>"
        "<p>\u6b64\u7a0b\u5e8f\u4f7f\u7528 Qt \u6846\u67b6\u5f00\u53d1\u3002"
        "Qt \u662f The Qt Company \u7684\u5546\u6807\u3002</p>"
        "<p>\u60a8\u53ef\u4ee5\u4ece\u4ee5\u4e0b\u94fe\u63a5\u83b7\u53d6 Qt \u6e90\u4ee3\u7801\uff1a"
        "<br><a href=\"https://download.qt.io/\">https://download.qt.io/</a></p>"
        "<p>\u6216\u4f7f\u7528\u672c\u5730 Qt \u5b89\u88c5\u76ee\u5f55\u4e2d\u7684\u6e90\u4ee3\u7801\uff1a"
        "<br>E:\\Qt\\6.11.1\\Src</p>"
    ));
    vl->addWidget(tb);
    auto *closeBtn = new QPushButton(QString::fromUtf8("\u5173\u95ed"));
    closeBtn->setStyleSheet(
        "QPushButton { background: #555; color: #fff; padding: 6px 20px; border-radius: 4px; }"
        "QPushButton:hover { background: #666; }"
    );
    vl->addWidget(closeBtn, 0, Qt::AlignRight);
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);
    dlg->setStyleSheet(
        "QWidget { background: #1a1a1a; color: #ccc; font-size: 13px; }"
        "QTextBrowser { background: transparent; border: none; }"
        "a { color: #6af; }"
    );
    dlg->exec();
    dlg->deleteLater();
}

void ConfigPanel::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && event->position().y() <= m_titleBar->height()) {
        m_dragging = true;
        m_dragPos = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}

void ConfigPanel::mouseMoveEvent(QMouseEvent *event) {
    if (m_dragging && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPosition().toPoint() - m_dragPos);
        event->accept();
    }
}

void ConfigPanel::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_dragging = false;
        event->accept();
    }
}

void ConfigPanel::onAutoStartToggled(bool on) {
    // Clean up old registry Run entry regardless
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER,
            L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
            0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        RegDeleteValueW(hKey, L"WPStools");
        RegCloseKey(hKey);
    }

    if (on) {
        QDir dir(QCoreApplication::applicationDirPath());
        QString exe = QDir::toNativeSeparators(
            dir.absoluteFilePath("WPPTouchHelper.exe"));
        if (!QFile::exists(exe)) {
            LOG("auto-start FAILED: not found at %s", qPrintable(exe));
            return;
        }

        QString ps = QString(
            "$a=New-ScheduledTaskAction -Execute '%1' -WorkingDirectory '%2'; "
            "$t=New-ScheduledTaskTrigger -AtLogOn; "
            "$s=New-ScheduledTaskSettingsSet -AllowStartIfOnBatteries -DontStopIfGoingOnBatteries; "
            "Register-ScheduledTask -TaskName 'WPStools' -Action $a -Trigger $t -Settings $s -Force")
            .arg(exe, QDir::toNativeSeparators(dir.absolutePath()));
        QProcess::startDetached("powershell", { "-NoProfile", "-Command", ps });
        LOG("auto-start enabled via scheduled task: %s", qPrintable(exe));
    } else {
        QProcess::startDetached("powershell", {
            "-NoProfile", "-Command",
            "Unregister-ScheduledTask -TaskName 'WPStools' -Confirm:$false" });
        LOG("auto-start disabled (scheduled task removed)");
    }
}

QWidget* ConfigPanel::makeSlider(const QString &label, const QString &key,
                                 int min, int max, int val) {
    auto *row = new QWidget;
    auto *hl = new QHBoxLayout(row);
    hl->setContentsMargins(0, 2, 0, 2);

    auto *lb = new QLabel(label);
    lb->setFixedWidth(64);
    auto *sl = new QSlider(Qt::Horizontal);
    sl->setRange(min, max);
    sl->setValue(val);
    auto *sp = new QSpinBox;
    sp->setRange(min, max);
    sp->setValue(val);

    hl->addWidget(lb);
    hl->addWidget(sl, 1);
    hl->addWidget(sp);

    m_entries.append({key, sl, sp});

    connect(sl, &QSlider::valueChanged, sp, &QSpinBox::setValue);
    connect(sp, QOverload<int>::of(&QSpinBox::valueChanged), sl, &QSlider::setValue);

    return row;
}
