#include "controller.h"
#include "bitcoingui.h"
#include <qlogging.h>
#include <vector>
#include "column.h"

BitcoinGUI::BitcoinGUI(
    GuiController *controller
) : m_controller(controller)
{
    m_button = new QPushButton("yay");
    auto *col = (new Column())
        ->push(m_button);

    connect(m_button, &QPushButton::clicked, this, &BitcoinGUI::listCommands);

    this->setCentralWidget(col);
}

BitcoinGUI::~BitcoinGUI(){}

void BitcoinGUI::closeEvent(QCloseEvent *ev) {
    Q_EMIT this->quitRequested();
    if (m_allow_close) {
        ev->accept();
    } else {
        ev->ignore();
    }
}

void BitcoinGUI::doClose() {
    m_allow_close = true;
    QApplication::exit(0);
}

void BitcoinGUI::rcvCommands(std::vector<std::string> commands) {
    for (auto cmd : commands) {
        qDebug() << cmd;
    }

}
