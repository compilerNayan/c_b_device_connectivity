#ifndef ENROLLMENTCONTROLLER_H
#define ENROLLMENTCONTROLLER_H

#include "IEnrollmentController.h"
#include "pubsub/ICommandBus.h"

/* @RestController */
/* @RequestMapping("/enrollment") */
class EnrollmentController final : public IEnrollmentController {
    /* @Autowired */
    Private IFleetProvisioningServicePtr fleetProvisioningService;

    /* @Autowired */
    Private ICommandBusPtr commandBus;

    /* @GetMapping("/status") */
    Public Virtual EnrollmentStatus GetEnrollmentStatus() override {
        return fleetProvisioningService->GetEnrollmentStatus();
    }

    /* @PostMapping("/enroll") */
    Public Virtual EnrollmentStatus EnrollDevice() override {
        logger->Info(Tag::Untagged, "Sending command to enroll device");
        commandBus->Publish(TOPIC_MQTT, Command(COMMAND_ENROLL_DEVICE, SENDER_ENROLLMENT_CONTROLLER, "Enrolling device"));
        Thread::Sleep(1000);
        return fleetProvisioningService->GetEnrollmentStatus();
    }
};

#endif // ENROLLMENTCONTROLLER_H