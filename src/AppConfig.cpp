/*
 * Copyright 2020-2025 Boaz Feldboim
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// SPDX-License-Identifier: Apache-2.0

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
#define DEFAULT_DST false
#define DEFAULT_PERIODICALLY_RESTART_ROUTER false
#define DEFAULT_PERIODICALLY_RESTART_MODEM false
#define DEFAULT_PERIODIC_RESTART_TIME ((time_t)((4 * 60 + 30) * 60))
void AppConfig::init()
{
    EEPROM.begin(1024);
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
        setDST(internalGetDST());
        setPeriodicallyRestartRouter(internalGetPeriodicallyRestartRouter());
        setPeriodicallyRestartModem(internalGetPeriodicallyRestartModem());
        setPeriodicRestartTime(internalGetPeriodicRestartTime());
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
        setDST(DEFAULT_DST);
        setInitialized(true);
        setPeriodicallyRestartRouter(DEFAULT_PERIODICALLY_RESTART_ROUTER);
        setPeriodicallyRestartModem(DEFAULT_PERIODICALLY_RESTART_MODEM);
        setPeriodicRestartTime(DEFAULT_PERIODIC_RESTART_TIME);
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
    strncpy(store.server1, value.c_str(), MAX_SERVER_NAME);
    store.server1[MAX_SERVER_NAME] = '\0';
}

String AppConfig::getServer2()
{
    return store.server2;
}

void AppConfig::setServer2(const String &value)
{
    strncpy(store.server2, value.c_str(), MAX_SERVER_NAME);
    store.server2[MAX_SERVER_NAME] = '\0';
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

bool AppConfig::getDST()
{
    return store.DST;
}

void AppConfig::setDST(bool value)
{
    store.DST = value;
}

bool AppConfig::getPeriodicallyRestartRouter()
{
    return store.periodicallyRestartRouter;
}

void AppConfig::setPeriodicallyRestartRouter(bool value)
{
    store.periodicallyRestartRouter = value;
}

bool AppConfig::getPeriodicallyRestartModem()
{
    return store.periodicallyRestartModem;
}

void AppConfig::setPeriodicallyRestartModem(bool value)
{
    store.periodicallyRestartModem = value;
}

time_t AppConfig::getPeriodicRestartTime()
{
    return store.periodicRestartTime;
}

void AppConfig::setPeriodicRestartTime(time_t value)
{
    store.periodicRestartTime = value;
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

    if (store.DST != internalGetDST())
    {
        internalSetDST(store.DST);
        dirty = true;
    }

    if (store.periodicallyRestartRouter != internalGetPeriodicallyRestartRouter())
    {
        internalSetPeriodicallyRestartRouter(store.periodicallyRestartRouter);
        dirty = true;
    }

    if (store.periodicallyRestartModem != internalGetPeriodicallyRestartModem())
    {
        internalSetPeriodicallyRestartModem(store.periodicallyRestartModem);
        dirty = true;
    }

    if (store.periodicRestartTime != internalGetPeriodicRestartTime())
    {
        internalSetPeriodicRestartTime(store.periodicRestartTime);
        dirty = true;
    }

    if (dirty)
    {
        EEPROM.commit();
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
        getField<char>(offsetof(AppConfigStore, server1) + i, buff[i]);
    }

    return String(buff);
}

void AppConfig::internalSetServer1(const String &value)
{
    for(size_t i = 0; i < value.length(); i++)
    {
        putField<char>(offsetof(AppConfigStore, server1) + i, value[i]);
    }
    putField<char>(offsetof(AppConfigStore, server1) + value.length(), 0);
}

String AppConfig::internalGetServer2()
{
    char buff[sizeof(AppConfigStore::server2)];

    for(size_t i = 0; i < sizeof(AppConfigStore::server2) - 1; i++)
    {
        getField<char>(offsetof(AppConfigStore, server2) + i, buff[i]);
    }

    return String(buff);
}

void AppConfig::internalSetServer2(const String &value)
{
    for(size_t i = 0; i < value.length(); i++)
    {
        putField<char>(offsetof(AppConfigStore, server2) + i, value[i]);
    }
    putField<char>(offsetof(AppConfigStore, server2) + value.length(), 0);
}

IPAddress AppConfig::internalGetLANAddr()
{
    uint8_t buff[sizeof(AppConfigStore::lanAddress)];

    for(size_t i = 0; i < sizeof(AppConfigStore::lanAddress); i++)
    {
        getField<uint8_t>(offsetof(AppConfigStore, lanAddress) + i, buff[i]);
    }

    return IPAddress(buff);
}

void AppConfig::internalSetLANAddr(const IPAddress &value)
{
    for(size_t i = 0; i < sizeof(AppConfigStore::lanAddress); i++)
    {
        putField<uint8_t>(offsetof(AppConfigStore, lanAddress) + i, value[i]);
    }
}

bool AppConfig::isInitialized()
{
    byte value = 0;
    getField<byte>(offsetof(AppConfigStore, initialized), value);
    return value != 255;
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

bool AppConfig::internalGetDST()
{
    bool value = false;
    return getField<bool>(offsetof(AppConfigStore, DST), value);
}

void AppConfig::internalSetDST(bool value)
{
    putField<bool>(offsetof(AppConfigStore, DST), value);
}

bool AppConfig::internalGetPeriodicallyRestartRouter()
{
    bool value = false;
    return getField<bool>(offsetof(AppConfigStore, periodicallyRestartRouter), value);
}

void AppConfig::internalSetPeriodicallyRestartRouter(bool value)
{
    putField<bool>(offsetof(AppConfigStore, periodicallyRestartRouter), value);
}

bool AppConfig::internalGetPeriodicallyRestartModem()
{
    bool value = false;
    return getField<bool>(offsetof(AppConfigStore, periodicallyRestartModem), value);
}

void AppConfig::internalSetPeriodicallyRestartModem(bool value)
{
    putField<bool>(offsetof(AppConfigStore, periodicallyRestartModem), value);
}

time_t AppConfig::internalGetPeriodicRestartTime()
{
    time_t value;
    return getField<time_t>(offsetof(AppConfigStore, periodicRestartTime), value);
}

void AppConfig::internalSetPeriodicRestartTime(time_t value)
{
    putField<time_t>(offsetof(AppConfigStore, periodicRestartTime), value);
}

void InitAppConfig()
{
    AppConfig::init();
}