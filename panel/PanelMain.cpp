#define _WIN32_IE 0x0A00
#include <windows.h>
#include <shellapi.h>

#include <QApplication>
#include <QIcon>
#include <QImage>
#include <QPixmap>
#include "ConfigPanel.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    {
        SHSTOCKICONINFO sii = { sizeof(sii) };
        if (SUCCEEDED(SHGetStockIconInfo(SIID_SETTINGS, SHGSI_ICON | SHGSI_LARGEICON, &sii))) {
            app.setWindowIcon(QIcon(QPixmap::fromImage(QImage::fromHICON(sii.hIcon))));
            DestroyIcon(sii.hIcon);
        }
    }
    ConfigPanel panel;
    panel.show();
    return app.exec();
}
