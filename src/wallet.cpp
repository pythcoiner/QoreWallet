#include "wallet.h"
#include "config.h"
#include "result.h"
#include "script/descriptor.h"
#include "script/signingprovider.h"
#include <QJsonObject>
#include <QJsonValue>
#include <qcontainerfwd.h>
#include <qjsonvalue.h>

auto toString(AccountKind kind) -> QString {
    switch (kind) {
    case AccountKind::P2PK:
        return "p2pk";
    case AccountKind::P2PKH:
        return "p2pkh";
    case AccountKind::SegwitV0:
        return "segwit";
    case AccountKind::Taproot:
        return "taproot";
    case AccountKind::SilentPayment:
        return "silent_payment";
    case AccountKind::Lightning:
        return "lightning";
    case AccountKind::Escrow:
        return "escrow";
    case AccountKind::Unknown:
        return "unknown";
    }
        return "unknown";
};

#define ACCOUNTKIND_FROM(Variant) \
if (value == toString(AccountKind::Variant)) return AccountKind::Variant

auto fromString(const QString &value) {
    ACCOUNTKIND_FROM(P2PK);
    ACCOUNTKIND_FROM(P2PKH);
    ACCOUNTKIND_FROM(SegwitV0);
    ACCOUNTKIND_FROM(Taproot);
    ACCOUNTKIND_FROM(Lightning);
    ACCOUNTKIND_FROM(Escrow);
    return AccountKind::Unknown;
}

Account::Account() {
    name = "";
    descriptor = nullptr;
    kind = AccountKind::Unknown;
}

auto Account::value() const -> QJsonValue {
    auto map = QJsonObject();
    map.insert("name", name);
    map.insert("descriptor", QString(descriptor->ToString().c_str()));
    map.insert("kind", toString(kind));
    return QJsonValue(map);
}

auto Account::tryFromJson(const QJsonValue &value) -> Result<Account, config::Error> {
    Account account{};
    if (!value.isObject()) return Err(config::Error::AccountIsNotObject);
    auto obj = value.toObject();
    if (!obj.contains("name")) return Err(config::Error::AccountName);
    if (!obj.contains("descriptor")) return Err(config::Error::AccountDescriptor);
    if (!obj.contains("kind")) return Err(config::Error::AccountKind);
    auto name = obj.value("name");
    if (!name.isString()) return Err(config::Error::AccountName);
    account.name = name.toString();
    auto descriptor = obj.value("descriptor");
    if (!descriptor.isString()) return Err(config::Error::AccountDescriptor);
    std::string error;
    FlatSigningProvider provider;
    std::string descrString = descriptor.toString().toStdString();
    auto descriptors = Parse(descrString, provider, error, false);
    if (descriptors.size() != 1) return Err(config::Error::AccountDescriptor);
    QSharedPointer<Descriptor> ptr(descriptors.at(0).release());
    account.descriptor = ptr;
    auto kind = obj.value("kind");
    if (!kind.isString()) return Err(config::Error::AccountKind);
    account.kind = fromString(kind.toString());
    return Ok(account);
}

auto toValue(const AccountMap &accounts) -> QJsonValue {
    auto array = QJsonArray();
    for (const auto& account : accounts) {
        array.push_front(account.value());
    }
    return QJsonValue(array);
}
