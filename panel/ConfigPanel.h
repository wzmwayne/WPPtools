#pragma once
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QSlider>
#include <QSpinBox>
#include <QTimer>
#include <QJsonObject>

class ConfigPanel : public QWidget {
    Q_OBJECT
public:
    explicit ConfigPanel(QWidget *parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void onStart();
    void onStop();
    void onSave();
    void onAbout();
    void onAutoStartToggled(bool on);

private:
    void loadCfg();
    void saveCfg();
    void updateStatus();
    QWidget* makeSlider(const QString &label, const QString &key, int min, int max, int val);

    QWidget *m_titleBar;
    QLabel *m_statusIcon;
    QLabel *m_statusText;
    QPushButton *m_startBtn;
    QPushButton *m_stopBtn;
    QPushButton *m_saveBtn;
    QPushButton *m_aboutBtn;
    QCheckBox *m_autoStartCb;
    QTimer *m_timer;
    QJsonObject m_cfg;
    QPoint m_dragPos;
    bool m_dragging = false;

    struct Entry { QString key; QSlider *sl; QSpinBox *sp; };
    QList<Entry> m_entries;
};
