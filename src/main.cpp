#include "common/args.h"
#include "config.h"
#include "controller.h"
#include "wallet.h"
#include <QApplication>

// bitcoin/src/common/system.cpp
void SetupEnvironment();

// needed by bitcoin `bitcoin_node` library
const std::function<std::string(const char*)> G_TRANSLATION_FUN = nullptr;

int main(int argc, char *argv[]) {
    auto initStatus = Config::init();
    if (initStatus.isErr()) {
        std::cout << "Fail to init ConfigManager!";
        return 1;
    }
    std::cout << "ConfigManager initialized.\n";

    SetupEnvironment();

    QApplication app(argc, argv);

    // Parse command-line options
    SetupServerArgs(gArgs, false);
    std::string error;
    if (!gArgs.ParseParameters(argc, argv, error)) {
        qCritical() << "fail to parse args: " << error;
        return EXIT_FAILURE;
    }

        auto controller = GuiController(&app);
        return QApplication::exec();

}

