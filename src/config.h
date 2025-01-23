#pragma once

#include "result.h"
#include "wallet.h"
#include <QObject>
#include <QSettings>
#include <cstdint>
#include <qcontainerfwd.h>
#include <qjsonarray.h>
#include <qjsonobject.h>
#include <qjsonvalue.h>

using Path = QString;

namespace config {
    enum class Error : uint8_t {
        AlreadyInit,
        ConfigFileNotFound,
        AccountIsNotObject,
        AccountName,
        AccountDescriptor,
        AccountKind,
        FileNotExists, 
        FailOpenFile,
        FailWriteFile,
        ParsingFail,
        ConfigNotObject,
        WalletsMissing,
        WalletsNotObject,
        WalletNotExists,
        WalletPayloadNotArray,
    };
}

struct WalletConfig {
    QString name;
    AccountMap accounts;
    Path db_path;
};

class Config: public QObject {
Q_OBJECT

public:
    Config();
    Config(const QString &path);
    Config(const Config&) = delete;
    auto operator=(const Config&) -> Config& = delete;
    ~Config() override = default;

    static auto init() -> Result<void, config::Error>;
    static auto init(const QString &path) -> Result<void, config::Error>;
    static auto path() -> Path;
    static auto get() -> Config*;

    static void load();
    static void write();

    static auto payload() -> QJsonObject;

    // parse accounts from a wallet entry
    static auto valueParseAccounts(const QJsonValue &value, AccountMap &accounts) -> Result<void, config::Error>;

    // parse a specific wallet from a config payload
    static auto payloadLoadWallet(const QString &name) -> Result<AccountMap, config::Error>;
    // list wallet from a config payload
    static auto payloadListWallets(QList<QString> &wallets) -> Result<void, config::Error>;
    // error if config payload do no contain "wallets" section
    static auto payloadContainsWalletSection() -> Result<void, config::Error>;
    // return a copy of the "wallet" section from config payload
    static auto payloadGetWalletSection() -> Result<QJsonObject, config::Error>;
    // error if there is no "wallets" section or the specified wallet do not exists in the config payload
    static auto payloadContainsWallet(const QString &name) -> Result<void, config::Error>;
    // get a copy of the wallet entry from the config payload for the specified wallet name
    static auto payloadGetWallet(const QString &name) -> Result<QJsonArray, config::Error>;
    // insert or update the wallet entry in the config paylod for the specified wallet name
    static auto payloadUpdateWallet(const QString &wallet_name, const AccountMap &accounts) -> Result<void, config::Error>;
    // delete the wallet entry  in the config payload for the specified wallet name
    static auto payloadDeleteWallet(const QString &wallet_name) -> Result<void, config::Error>;

    void broadcastWallets();
    void emitError(config::Error);
    void emitLoaded();

public slots:
    void getWallet(const QString &wallet_name);
    void updateWallet(const QString &wallet_name, const AccountMap &accounts);
    void deleteWallet(const QString &wallet_name);

signals:
    // Config::load()
    void error(config::Error);
    void wallets(QList<QString>);
    void loaded();

    // Config::write()
    void written();

    // Config::getWallet()
    void cannotGetWallet(QString name, config::Error);
    void accounts(QString name, AccountMap);

    // Config::updateWallet()
    void walletUpdated(QString name);
    void walletUpdateFail(QString name, config::Error);

    // Config::deleteWallet()
    void walletDeleted(QString name);
    void walletDeleteFail(QString name, config::Error);

private:
    inline static Config *s_instance = nullptr; //NOLINT(readability-identifier-naming)
    QList<QString> m_wallets;
    QJsonObject m_payload;
    Path m_path;
    bool m_loaded = false;
};
