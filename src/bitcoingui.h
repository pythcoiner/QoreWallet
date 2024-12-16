#pragma once

#include <QMainWindow>
#include <QPushButton>
#include <qevent.h>
#include <vector>

class GuiController;

class BitcoinGUI : public QMainWindow
{
    Q_OBJECT

public:
    explicit BitcoinGUI(GuiController *controller);
    ~BitcoinGUI() override;

Q_SIGNALS:
    void quitRequested();
    void listCommands();

public Q_SLOT:
    void rcvCommands(std::vector<std::string> commands);
    void doClose();

protected:
    void closeEvent(QCloseEvent *ev) override;

private:
    GuiController *m_controller;
    QPushButton *m_button = nullptr;
    bool m_allow_close = false;
};

