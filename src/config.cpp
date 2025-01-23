#include "config.h"
#include "result.h"
#include "wallet.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <qlogging.h>

#define ERROR_IF_NOT(condition, Variant) \
if (!condition) { \
    config->emitError(config::Error::Variant); \
    return; \
}

#define RETURN_IF_NOT(condition, Variant) \
if (!condition) { \
    return Err(config::Error::Variant); \
}

#define RETURN_IF_ERROR(result) \
if (!result.isErr()) { \
    return Err(result.unwrapErr()); \
}

Config::Config() {
    m_path = Config::path();
}

Config::Config(const QString &path) {
    m_path = path;
}

auto Config::init() -> Result<void, config::Error> {
    if (s_instance != nullptr) {
        return Err(config::Error::AlreadyInit);
    }
    s_instance = new Config;
    return Ok();
}

auto Config::init(const QString &path) -> Result<void, config::Error> {
    if (s_instance != nullptr) {
        return Err(config::Error::AlreadyInit);
    }
    s_instance = new Config(path);
    return Ok();
}

auto Config::path() -> Path {
#if defined(Q_OS_WIN)
    auto path = 
        QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)
        + "/CoreWallet";
    return path;
#elif defined(Q_OS_MACOS)
    auto path = 
        QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)
        + "/CoreWallet";
    return path;
#else 
    auto path = 
        QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
        + "/.core_wallet";
    return path;
#endif
}

auto Config::get() -> Config* {
    return s_instance;
}

void Config::write() {
    //TODO: at least sanity check there is an empty wallets section
    auto *config = Config::get();
    auto doc = QJsonDocument(Config::payload()).toJson();
    auto file = QFile(Config::path());
    ERROR_IF_NOT(file.exists(), FileNotExists)
    auto openSuccess = file.open(QIODevice::WriteOnly);
    ERROR_IF_NOT(openSuccess, FailOpenFile)
    auto written = file.write(doc);
    ERROR_IF_NOT((written > 0), FailWriteFile)
    file.close();
    emit config->written();
}

void Config::load() {
    auto *config = Config::get();
    auto file = QFile(Config::path());
    ERROR_IF_NOT(file.exists(), FileNotExists)
    auto openSuccess = file.open(QIODevice::ReadOnly);
    ERROR_IF_NOT(openSuccess, FailOpenFile)
    auto payload = file.readAll();
    auto doc =  QJsonDocument::fromJson(payload);
    ERROR_IF_NOT(!doc.isNull(), ParsingFail);
    ERROR_IF_NOT(doc.isObject(), ConfigNotObject);
    config->m_payload = doc.object();
    if (config->m_payload.contains("wallets")) {
        auto result = Config::payloadListWallets(config->m_wallets);
        if (result.isErr()) {
            config->emitError(result.unwrapErr());
        } else {
            config->broadcastWallets();
        }
    }
    config->emitLoaded();
}

auto Config::valueParseAccounts(const QJsonValue &value, AccountMap &accounts) -> Result<void, config::Error> {
    if (!value.isArray()) return Err(config::Error::WalletPayloadNotArray);
    auto rawAccounts = value.toArray();
    for (const auto& data : rawAccounts) {
        auto accountResult = Account::tryFromJson(data);
        if (accountResult.isErr()) return Err(accountResult.unwrapErr());
        auto account = accountResult.unwrap();
        accounts.insert(account.name, account);
    }
    return Ok();
}

auto Config::payloadListWallets(QList<QString> &wallets) -> Result<void, config::Error> {
    auto walletsResult = Config::payloadGetWalletSection();
    RETURN_IF_ERROR(walletsResult)
    wallets = walletsResult.unwrap().keys().toList();
    return Ok();
}

auto Config::payloadLoadWallet(const QString &name) -> Result<AccountMap, config::Error> {
    auto accounts = AccountMap();
    auto walletResult = Config::payloadGetWallet(name);
    RETURN_IF_ERROR(walletResult)
    auto parseResult = Config::valueParseAccounts(walletResult.unwrap(), accounts);
    RETURN_IF_ERROR(parseResult)
    return Ok(accounts);
}

auto Config::payloadContainsWalletSection() -> Result<void, config::Error> {
    auto payload = Config::get()->m_payload;
    RETURN_IF_NOT(payload.contains("wallets"), WalletsMissing)
    RETURN_IF_NOT(payload.value("wallets").isObject(), WalletsNotObject);
    return Ok();
}

auto Config::payloadGetWalletSection() -> Result<QJsonObject, config::Error> {
    RETURN_IF_ERROR(Config::payloadContainsWalletSection())
    return Ok(Config::get()->m_payload
        .value("wallets")
        .toObject()
              );
}

auto Config::payloadContainsWallet(const QString &name) -> Result<void, config::Error> {
    auto sectionResult = Config::payloadGetWalletSection();
    RETURN_IF_ERROR(sectionResult)
    auto section = sectionResult.unwrap();
    RETURN_IF_NOT(section.contains(name), WalletNotExists)
    return Ok();
}

auto Config::payloadGetWallet(const QString &name) -> Result<QJsonArray, config::Error> {
    RETURN_IF_ERROR(Config::payloadContainsWallet(name))
    auto walletValue = 
        Config::payloadGetWalletSection()
        .unwrap()
        .value(name);
    RETURN_IF_NOT(walletValue.isArray(), WalletPayloadNotArray)
    return Ok(walletValue.toArray());
}

auto Config::payload() -> QJsonObject {
    return Config::get()->m_payload;
}

auto Config::payloadUpdateWallet(
    const QString &wallet_name, 
    const AccountMap &accounts
) -> Result<void, config::Error> {
    auto wallet = toValue(accounts);
    auto sectionResult = Config::payloadGetWalletSection();
    RETURN_IF_ERROR(sectionResult)
    auto section = sectionResult.unwrap();
    section.insert(wallet_name, wallet);
    Config::get()->m_payload.insert("wallets", section);
    Config::write();
    return Ok();
}

auto Config::payloadDeleteWallet(
    const QString &wallet_name
) -> Result<void, config::Error> {
    RETURN_IF_ERROR(Config::payloadContainsWallet(wallet_name))
    auto sectionResult = Config::payloadGetWalletSection();
    RETURN_IF_ERROR(sectionResult)
    auto section = sectionResult.unwrap();
    section.remove(wallet_name);
    Config::get()->m_payload.insert("wallets", section);
    Config::write();
    return Ok();
}

void Config::broadcastWallets() {
    emit this->wallets(m_wallets);
}

void Config::getWallet(const QString &wallet_name) {
    auto accountsResult = Config::payloadLoadWallet(wallet_name);
    if (accountsResult.isOk()) {
        emit this->accounts(wallet_name, accountsResult.unwrap());
    } else {
        emit this->cannotGetWallet(wallet_name, accountsResult.unwrapErr());
    }
}

void Config::updateWallet(const QString &wallet_name, const AccountMap &accounts) {
    auto result = Config::payloadUpdateWallet(wallet_name, accounts);
    if (result.isOk()) {
        emit this->walletUpdated(wallet_name);
    } else {
        emit this->walletUpdateFail(wallet_name, result.unwrapErr());
    }
}

void Config::deleteWallet(const QString &wallet_name) {
    auto result = Config::payloadDeleteWallet(wallet_name);
    if (result.isOk()) {
        emit this->walletDeleted(wallet_name);
    } else {
        emit this->walletDeleteFail(wallet_name, result.unwrapErr());
    }
}

void Config::emitError(config::Error error) {
    emit this->error(error);
}

void Config::emitLoaded() {
    emit this->loaded();
}
