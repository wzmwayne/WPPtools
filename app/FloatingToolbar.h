#pragma once
#include <QWidget>
#include <QPushButton>
#include <QTimer>

class FloatingToolbar : public QWidget {
    Q_OBJECT
public:
    explicit FloatingToolbar(QWidget *parent = nullptr);
    void applyConfig();
    void setKeepAlive(bool on);

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void onMouse();
    void onPen();
    void onEraser();
    void onExit();
    void refreshLoop();

private:
    void reposition();
    void setActive(QPushButton *btn);

    QPushButton *m_mouseBtn;
    QPushButton *m_penBtn;
    QPushButton *m_eraserBtn;
    QPushButton *m_exitBtn;
    QTimer m_timer;
    bool m_keepAlive = false;
};
