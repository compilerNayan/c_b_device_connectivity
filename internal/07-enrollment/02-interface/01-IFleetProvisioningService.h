#ifndef IFLEETPROVISIONINGSERVICE_H
#define IFLEETPROVISIONINGSERVICE_H

#include <StandardDefines.h>
#include "../01-type/01-EnrollmentStatus.h"

DefineStandardPointers(IFleetProvisioningService);
class IFleetProvisioningService {
    Public Virtual Bool IsEnrolled() = 0;
    Public Virtual Void EnrollDevice() = 0;
    Public Virtual EnrollmentStatus GetEnrollmentStatus() = 0;
    Public Virtual EnrollmentStatus WaitForEnrollment(Int timeoutMs) = 0;
    Public Virtual Void CloseConnection() = 0;
};

#endif // IFLEETPROVISIONINGSERVICE_H