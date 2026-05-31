#ifndef IENROLLMENTCONTROLLER_H
#define IENROLLMENTCONTROLLER_H

#include <StandardDefines.h>
#include "IFleetProvisioningService.h"

DefineStandardPointers(IEnrollmentController);
class IEnrollmentController {    

    Public Virtual EnrollmentStatus GetEnrollmentStatus() = 0;

    Public Virtual EnrollmentStatus EnrollDevice() = 0;
};

#endif // IENROLLMENTCONTROLLER_H