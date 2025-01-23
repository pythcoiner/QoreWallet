#pragma once

#include "result.h"
#include "coins.h"
#include "primitives/transaction.h"
#include "script/descriptor.h"
#include "QHash"
#include <QList>
#include <QPair>
#include <QString>
#include <QJsonValue>
#include <cstdint>
#include <qjsonarray.h>
#include <qjsonobject.h>
#include <qjsonvalue.h>
#include <qobject.h>

namespace config {
    enum class Error : uint8_t;
}

namespace wallet {
    enum class Error : uint8_t {
        AccountAlreadyExists,
    };
}

enum class AccountKind: uint8_t {
    P2PK,
    P2PKH,
    SegwitV0,
    Taproot,
    SilentPayment,
    Lightning,
    Escrow,
    Unknown,
};

auto toString(AccountKind kind) -> QString;
auto fromString(const QString &value);

struct Account {
    QString name;
    QSharedPointer<Descriptor> descriptor;
    AccountKind kind = AccountKind::Unknown;

    Account();
    [[nodiscard]] auto value() const -> QJsonValue;
    static auto tryFromJson(const QJsonValue &value) -> Result<Account, config::Error>;
};

using AccountMap = QHash<QString /*account*/, Account>;
auto toValue(const AccountMap &accounts) -> QJsonValue;

class Wallet {

public:
    [[nodiscard]] auto listAccounts() const -> const AccountMap*;
    [[nodiscard]] auto getDescriptor(const QString &account) const -> std::optional<Descriptor>;
    auto createAccount(const QString &account, const Descriptor &descriptor) -> Result<void, wallet::Error>;

private:
    AccountMap m_accounts;
};
