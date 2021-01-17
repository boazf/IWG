#include <Arduino.h>
#include <AppConfig.h>

AppConfigStore AppConfig::store;

#define DEFAUL_AUTO_RECOVERY true
#define DEFAULT_CONNECTION_TEST_PERIOD 300
#define DEFAULT_LAN_ADDRESS IPAddress(0,0,0,0)
#define DEFAULT_LIMIT_CYCLES true
#define DEFAULT_MAX_HISTORY 10
#define DEFAULT_MODEM_DISCONNECT 5
#define DEFAULT_MODEM_RECONNECT 210
#define DEFAULT_ROUTER_DISCONNECT 5
#define DEFAULT_RECOVERY_CYCLES 5
#define DEFAULT_ROUTER_RECONNECT 180
#define DEFAULT_SERVER1 "google.com"
#define DEFAULT_SERVER2 "yahoo.com"

void AppConfig::init()
{
#ifdef ESP32
    EEPROM.begin(1024);
#endif
    if (isInitialized())
    {
        setAutoRecovery(internalGetAutoRecovery());
        setConnectionTestPeriod(internalGetConnectionTestPeriod());
        setLANAddr(internalGetLANAddr());
        setLimitCycles(internalGetLimitCycles());
        setMaxHistory(internalGetMaxHistory());
        setMDisconnect(internalGetMDisconnect());
        setMReconnect(internalGetMReconnect());
        setRDisconnect(internalGetRDisconnect());
        setRecoveryCycles(internalGetRecoveryCycles());
        setRReconnect(internalGetRReconnect());
        setServer1(internalGetServer1());
        setServer2(internalGetServer2());
    }
    else
    {
        setAutoRecovery(DEFAUL_AUTO_RECOVERY);
        setConnectionTestPeriod(DEFAULT_CONNECTION_TEST_PERIOD);
        setLANAddr(DEFAULT_LAN_ADDRESS);
        setLimitCycles(DEFAULT_LIMIT_CYCLES);
        setMaxHistory(DEFAULT_MAX_HISTORY);
        setMDisconnect(DEFAULT_MODEM_DISCONNECT);
        setMReconnect(DEFAULT_MODEM_RECONNECT);
        setRDisconnect(DEFAULT_ROUTER_DISCONNECT);
        setRecoveryCycles(DEFAULT_RECOVERY_CYCLES);
        setRReconnect(DEFAULT_ROUTER_RECONNECT);
        setServer1(DEFAULT_SERVER1);
        setServer2(DEFAULT_SERVER2);
        setInitialized(true);
        commit();
    }
}

int AppConfig::getMDisconnect()
{
    return store.mDisconnect;
}

void AppConfig::setMDisconnect(int value)
{
    store.mDisconnect = value;
}

int AppConfig::getRDisconnect()
{
    return store.rDisconnect;
}

void AppConfig::setRDisconnect(int value)
{
    store.rDisconnect = value;
}

int AppConfig::getMReconnect()
{
    return store.mReconnect;
}

void AppConfig::setMReconnect(int value)
{
    store.mReconnect = value;
}

int AppConfig::getRReconnect()
{
    return store.rReconnect;
}

void AppConfig::setRReconnect(int value)
{
    store.rReconnect = value;
}

bool AppConfig::getLimitCycles()
{
    return store.limitCycles;
}

void AppConfig::setLimitCycles(bool value)
{
    store.limitCycles = value;
}

int AppConfig::getRecoveryCycles()
{
    return store.recoveryCycles;
}

void AppConfig::setRecoveryCycles(int value)
{
    store.recoveryCycles = value;
}

String AppConfig::getServer1()
{
    return store.server1;
}

void AppConfig::setServer1(const String &value)
{
    strcpy(store.server1, value.c_str());
}

String AppConfig::getServer2()
{
    return store.server2;
}

void AppConfig::setServer2(const String &value)
{
    strcpy(store.server2, value.c_str());
}

IPAddress AppConfig::getLANAddr()
{
    return store.lanAddress;
}

void AppConfig::setLANAddr(const IPAddress &value)
{
    store.lanAddress[0] = value[0];
    store.lanAddress[1] = value[1];
    store.lanAddress[2] = value[2];
    store.lanAddress[3] = value[3];
}

time_t AppConfig::getConnectionTestPeriod()
{
    return store.connectionTestPeriod;
}

void AppConfig::setConnectionTestPeriod(time_t value)
{
    store.connectionTestPeriod = value;
}
bool AppConfig::getAutoRecovery()
{
    return store.autoRecovery;
}

void AppConfig::setAutoRecovery(bool value)
{
    store.autoRecovery = value;
}

int AppConfig::getMaxHistory()
{
    return store.maxHistory;
}

void AppConfig::setMaxHistory(int value)
{
    store.maxHistory = value;
}

Observers<AppConfigChangedParam> AppConfig::appConfigChanged;

void AppConfig::commit()
{
    bool dirty = false;

    if (store.autoRecovery != internalGetAutoRecovery())
    {
        internalSetAutoRecovery(store.autoRecovery);
        dirty = true;
    }

    if (store.connectionTestPeriod != internalGetConnectionTestPeriod())
    {
        internalSetConnectionTestPeriod(store.connectionTestPeriod);
        dirty = true;
    }

    if (IPAddress(store.lanAddress) != internalGetLANAddr())
    {
        internalSetLANAddr(store.lanAddress);
        dirty = true;
    }

    if (store.limitCycles != internalGetLimitCycles())
    {
        internalSetLimitCycles(store.limitCycles);
        dirty = true;
    }

    if (store.maxHistory != internalGetMaxHistory())
    {
        internalSetMaxHistory(store.maxHistory);
        dirty = true;
    }

    if (store.mDisconnect != internalGetMDisconnect())
    {
        internalSetMDisconnect(store.mDisconnect);
        dirty = true;
    }

    if (store.mReconnect != internalGetMReconnect())
    {
        internalSetMReconnect(store.mReconnect);
        dirty = true;
    }

    if (store.rDisconnect != internalGetRDisconnect())
    {
        internalSetRDisconnect(store.rDisconnect);
        dirty = true;
    }

    if (store.recoveryCycles != internalGetRecoveryCycles())
    {
        internalSetRecoveryCycles(store.recoveryCycles);
        dirty = true;
    }

    if (store.rReconnect != internalGetRReconnect())
    {
        internalSetRReconnect(store.rReconnect);
        dirty = true;
    }

    if (String(store.server1) != internalGetServer1())
    {
        internalSetServer1(store.server1);
        dirty = true;
    }

    if (String(store.server2) != internalGetServer2())
    {
        internalSetServer2(store.server2);
        dirty = true;
    }

    if (dirty)
    {
#ifdef ESP32
        EEPROM.commit();
#endif
        appConfigChanged.callObservers(AppConfigChangedParam());
    }
}

int AppConfig::internalGetMDisconnect()
{
    int value;
    return getField<int>(offsetof(AppConfigStore, mDisconnect), value);
}

void AppConfig::internalSetMDisconnect(int value)
{
    putField<int>(offsetof(AppConfigStore, mDisconnect), value);
}

int AppConfig::internalGetRDisconnect()
{
    int value;
    return getField<int>(offsetof(AppConfigStore, rDisconnect), value);
}

void AppConfig::internalSetRDisconnect(int value)
{
    putField<int>(offsetof(AppConfigStore, rDisconnect), value);
}

int AppConfig::internalGetMReconnect()
{
    int value;
    return getField<int>(offsetof(AppConfigStore, mReconnect), value);
}

void AppConfig::internalSetMReconnect(int value)
{
    putField<int>(offsetof(AppConfigStore, mReconnect), value);
}

int AppConfig::internalGetRReconnect()
{
    int value;
    return getField<int>(offsetof(AppConfigStore, rReconnect), value);
}

void AppConfig::internalSetRReconnect(int value)
{
    putField<int>(offsetof(AppConfigStore, rReconnect), value);
}

bool AppConfig::internalGetLimitCycles()
{
    bool value = false;
    return getField<bool>(offsetof(AppConfigStore, limitCycles), value);
}

void AppConfig::internalSetLimitCycles(bool value)
{
    putField<bool>(offsetof(AppConfigStore, limitCycles), value);
}

int AppConfig::internalGetRecoveryCycles()
{
    int value;
    return getField<int>(offsetof(AppConfigStore, recoveryCycles), value);
}

void AppConfig::internalSetRecoveryCycles(int value)
{
    putField<int>(offsetof(AppConfigStore, recoveryCycles), value);
}

String AppConfig::internalGetServer1()
{
    char buff[sizeof(AppConfigStore::server1)];

    for(size_t i = 0; i < sizeof(AppConfigStore::server1) - 1; i++)
    {
        buff[i] = EEPROM.read(APP_CONFIG_EEPROM_START_ADDR + offsetof(AppConfigStore, server1) + i);
    }

    return String(buff);
}

void AppConfig::internalSetServer1(const String &value)
{
    for(size_t i = 0; i < value.length(); i++)
    {
        EEPROM.write(APP_CONFIG_EEPROM_START_ADDR + offsetof(AppConfigStore, server1) + i, value[i]);
    }
    EEPROM.write(APP_CONFIG_EEPROM_START_ADDR + offsetof(AppConfigStore, server1) + value.length(), 0);
}

String AppConfig::internalGetServer2()
{
    char buff[sizeof(AppConfigStore::server2)];

    for(size_t i = 0; i < sizeof(AppConfigStore::server2) - 1; i++)
    {
        buff[i] = EEPROM.read(APP_CONFIG_EEPROM_START_ADDR + offsetof(AppConfigStore, server2) + i);
    }

    return String(buff);
}

void AppConfig::internalSetServer2(const String &value)
{
    for(size_t i = 0; i < value.length(); i++)
    {
        EEPROM.write(APP_CONFIG_EEPROM_START_ADDR + offsetof(AppConfigStore, server2) + i, value[i]);
    }
    EEPROM.write(APP_CONFIG_EEPROM_START_ADDR + offsetof(AppConfigStore, server2) + value.length(), 0);
}

IPAddress AppConfig::internalGetLANAddr()
{
    uint8_t buff[sizeof(AppConfigStore::lanAddress)];

    for(size_t i = 0; i < sizeof(AppConfigStore::lanAddress); i++)
    {
        buff[i] = EEPROM.read(APP_CONFIG_EEPROM_START_ADDR + offsetof(AppConfigStore, lanAddress) + i);
    }

    return IPAddress(buff);
}

void AppConfig::internalSetLANAddr(const IPAddress &value)
{
    for(size_t i = 0; i < sizeof(AppConfigStore::lanAddress); i++)
    {
        EEPROM.write(APP_CONFIG_EEPROM_START_ADDR + offsetof(AppConfigStore, lanAddress) + i, value[i]);
    }
}

bool AppConfig::isInitialized()
{
#ifndef ESP32
    bool value = false;
    getField<bool>(offsetof(AppConfigStore, initialized), value);
    return value;
#else
    byte value = 0;
    getField<byte>(offsetof(AppConfigStore, initialized), value);
    return value != 255;
#endif
}

void AppConfig::setInitialized(bool isInitialized)
{
    putField<bool>(offsetof(AppConfigStore, initialized), isInitialized);
}

time_t AppConfig::internalGetConnectionTestPeriod()
{
    time_t value;
    return getField<time_t>(offsetof(AppConfigStore, connectionTestPeriod), value);
}

void AppConfig::internalSetConnectionTestPeriod(time_t value)
{
    putField<time_t>(offsetof(AppConfigStore, connectionTestPeriod), value);
}

bool AppConfig::internalGetAutoRecovery()
{
    bool value = false;
    return getField<bool>(offsetof(AppConfigStore, autoRecovery), value);
}

void AppConfig::internalSetAutoRecovery(bool value)
{
    putField<bool>(offsetof(AppConfigStore, autoRecovery), value);
}

int AppConfig::internalGetMaxHistory()
{
    int value;
    return getField<int>(offsetof(AppConfigStore, maxHistory), value);
}

void AppConfig::internalSetMaxHistory(int value)
{
    putField<int>(offsetof(AppConfigStore, maxHistory), value);
}

void InitAppConfig()
{
    AppConfig::init();
}