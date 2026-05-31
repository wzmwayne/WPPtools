#include "ConfigPanel.h"
#include "WppConfigPage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QApplication>
#include <QMouseEvent>

ConfigPanel::ConfigPanel(QWidget *parent) : QWidget(parent) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    resize(620, 520);

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

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // title bar
    m_titleBar = new QWidget;
    m_titleBar->setFixedHeight(32);
    m_titleBar->setStyleSheet("QWidget { background: #1a1a1a; border-bottom: 1px solid #333; }");
    auto *tbLayout = new QHBoxLayout(m_titleBar);
    tbLayout->setContentsMargins(10, 0, 4, 0);
    auto *titleLbl = new QLabel("Touchtools");
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

    // body: nav + content
    auto *body = new QHBoxLayout;
    body->setContentsMargins(0, 0, 0, 0);
    body->setSpacing(0);

    // nav list
    m_navList = new QListWidget;
    m_navList->setFixedWidth(150);
    m_navList->setFrameShape(QFrame::NoFrame);
    m_navList->setStyleSheet(
        "QListWidget { background: #181818; border-right: 1px solid #333; color: #999; }"
        "QListWidget::item { padding: 12px 14px; }"
        "QListWidget::item:selected { background: #333; color: #fff; }"
        "QListWidget::item:hover { background: #282828; color: #ddd; }"
    );
    m_navList->addItem("WPPTouchHelper");
    m_navList->setCurrentRow(0);

    // stacked content
    m_stack = new QStackedWidget;
    m_stack->addWidget(new WppConfigPage);

    body->addWidget(m_navList);
    body->addWidget(m_stack, 1);
    root->addLayout(body, 1);

    connect(m_navList, &QListWidget::currentRowChanged,
            m_stack, &QStackedWidget::setCurrentIndex);
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
