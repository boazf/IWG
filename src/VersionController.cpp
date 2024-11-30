#include <VersionController.h>

VersionController versionController;

#define X(a) {VersionController::NotificationType::a, #a},
const std::map<VersionController::NotificationType, String> VersionController::notificationTypesStrings =
{
    NOTIFICATION_TYPES
};
#undef X
