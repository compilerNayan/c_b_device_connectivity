#ifndef ENROLLMENTCONTROLLER_H
#define ENROLLMENTCONTROLLER_H

#include "IEnrollmentController.h"

/* @RestController */
/* @RequestMapping("/enrollment") */
class EnrollmentController final : public IEnrollmentController {
    /* @Autowired */
    Private IFleetProvisioningServicePtr fleetProvisioningService;

    /* @GetMapping("/status") */
    Public Virtual EnrollmentStatus GetEnrollmentStatus() override {
        return fleetProvisioningService->GetEnrollmentStatus();
    }

    /* @PostMapping("/enroll") */
    Public Virtual EnrollmentStatus EnrollDevice() override {
        fleetProvisioningService->EnrollDevice();
        return fleetProvisioningService->GetEnrollmentStatus();
    }
};

#endif // ENROLLMENTCONTROLLER_H