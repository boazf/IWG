#ifndef AppConfig_h
#define AppConfig_h

#include <Arduino.h>
#include <WString.h>
#include <IPAddress.h>
#include <EEPROM.h>
#include <time.h>
#include <Observers.h>

#define MAX_SERVER_NAME 64
#define APP_CONFIG_EEPROM_START_ADDR 0

class AppConfigChangedParam
{
public:
    AppConfigChangedParam()
    {
    }
};

typedef struct AppConfigStore_
{
    int mDisconnect;
    int rDisconnect;
    int mReconnect;
    int rReconnect;
    bool limitCycles;
    int recoveryCycles;
    char server1[MAX_SERVER_NAME + 1];
    char server2[MAX_SERVER_NAME + 1];
    byte lanAddress[4];
    time_t connectionTestPeriod;
    bool autoRecovery;
    int maxHistory;
    bool DST;
    bool initialized;
    bool periodicallyRestartRouter;
    bool periodicallyRestartModem;
    time_t periodicRestartTime;
} AppConfigStore;

class AppConfig
{
public:
    static void init();
    static int getMDisconnect();
    static void setMDisconnect(int value);
    static int getRDisconnect();
    static void setRDisconnect(int value);
    static int getMReconnect();
    static void setMReconnect(int value);
    static int getRReconnect();
    static void setRReconnect(int value);
    static bool getLimitCycles();
    static void setLimitCycles(bool value);
    static int getRecoveryCycles();
    static void setRecoveryCycles(int value);
    static String getServer1();
    static void setServer1(const String &value);
    static String getServer2();
    static void setServer2(const String &value);
    static IPAddress getLANAddr();
    static void setLANAddr(const IPAddress &value);
    static time_t getConnectionTestPeriod();
    static void setConnectionTestPeriod(time_t value);
    static bool getAutoRecovery();
    static void setAutoRecovery(bool value);
    static int getMaxHistory();
    static void setMaxHistory(int value);
    static bool getDST();
    static void setDST(bool value);
    static bool getPeriodicallyRestartRouter();
    static void setPeriodicallyRestartRouter(bool value);
    static bool getPeriodicallyRestartModem();
    static void setPeriodicallyRestartModem(bool value);
    static time_t getPeriodicRestartTime();
    static void setPeriodicRestartTime(time_t value);
    static Observers<AppConfigChangedParam> &getAppConfigChanged()
    {
        return appConfigChanged;
    }
    static void commit();

private:
    static AppConfigStore store;
    static Observers<AppConfigChangedParam> appConfigChanged;

private:
    static bool isInitialized();
    static void setInitialized(bool isInitialized);
    static int internalGetMDisconnect();
    static void internalSetMDisconnect(int value);
    static int internalGetRDisconnect();
    static void internalSetRDisconnect(int value);
    static int internalGetMReconnect();
    static void internalSetMReconnect(int value);
    static int internalGetRReconnect();
    static void internalSetRReconnect(int value);
    static bool internalGetLimitCycles();
    static void internalSetLimitCycles(bool value);
    static int internalGetRecoveryCycles();
    static void internalSetRecoveryCycles(int value);
    static String internalGetServer1();
    static void internalSetServer1(const String &value);
    static String internalGetServer2();
    static void internalSetServer2(const String &value);
    static IPAddress internalGetLANAddr();
    static void internalSetLANAddr(const IPAddress &value);
    static time_t internalGetConnectionTestPeriod();
    static void internalSetConnectionTestPeriod(time_t value);
    static bool internalGetAutoRecovery();
    static void internalSetAutoRecovery(bool value);
    static int internalGetMaxHistory();
    static void internalSetMaxHistory(int value);
    static bool internalGetDST();
    static void internalSetDST(bool value);
    static bool internalGetPeriodicallyRestartRouter();
    static void internalSetPeriodicallyRestartRouter(bool value);
    static bool internalGetPeriodicallyRestartModem();
    static void internalSetPeriodicallyRestartModem(bool value);
    static time_t internalGetPeriodicRestartTime();
    static void internalSetPeriodicRestartTime(time_t value);
    template<typename T>
    static T &getField(int offset, T &value)
    {
        return EEPROM.get<T>(APP_CONFIG_EEPROM_START_ADDR + offset, value);
    }

    template<typename T>
    static void putField(int offset, T &value)
    {
        EEPROM.put<T>(APP_CONFIG_EEPROM_START_ADDR + offset, value);
    }
};

void InitAppConfig();

#endif // AppConfig_h