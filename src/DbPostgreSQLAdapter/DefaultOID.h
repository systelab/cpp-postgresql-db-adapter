#pragma once

namespace systelab::db::postgresql {
    enum class PostgresqlOID {
        boolOID = 16,
        bytearrayOID = 17,
        charOID = 18,
        nameOID = 19,
        smallIntIOD = 21,
        intOID = 23,
        textOID = 25,
        floatOID = 700,
        doubleOID = 701,
        varcharOID = 1043,
        datetimeOID = 1184,
        pidOID=2206
    };
}