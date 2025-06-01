#ifndef CommonFsmDefs_h
#define CommonFsmDefs_h
#include <tinyfsm.hpp>
#include <RecoveryControl.h>

struct RecoveryStateChanged : tinyfsm::Event, RecoveryStateChangedParams
{
    RecoveryStateChanged(RecoveryTypes recoveryType, RecoverySource recoverySource) : RecoveryStateChangedParams(recoveryType, recoverySource) {}
};

#endif // CommonFsmDefs_h