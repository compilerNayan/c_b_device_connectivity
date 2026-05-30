#ifndef ENROLLMENTSTATUS_H
#define ENROLLMENTSTATUS_H

#include "StandardDefines.h"

DefineStandardTypes(EnrollmentStatus)
enum class EnrollmentStatus {
    NotStarted,
    InProgress,
    Success,
    AlreadyEnrolled,
    Failed_TimeSync,
    Failed_MqttConnect,
    Failed_Subscription,
    Failed_CreateKeys,
    Failed_Provision,
    Failed_Unknown
};

#endif