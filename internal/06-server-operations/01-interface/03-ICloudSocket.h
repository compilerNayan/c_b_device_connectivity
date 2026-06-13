#ifndef ICLOUDSOCKET_INTERNAL_H
#define ICLOUDSOCKET_INTERNAL_H

#include <StandardDefines.h>

DefineStandardPointers(ICloudSocket)
class ICloudSocket {
    Public Virtual ~ICloudSocket() = default;

    /** Opens a TCP connection to the remote host. */
    Public Virtual Bool OpenSocket(CStdString host, Int port) = 0;

    /** Closes the active connection, if any. */
    Public Virtual Bool CloseSocket() = 0;

    /** Returns true when the socket is connected and ready to send. */
    Public Virtual Bool IsSocketOpen() const = 0;

    /** Enqueues a payload to be sent on the next SendData call. */
    Public Virtual Bool QueueDataToSend(CStdString data) = 0;

    /** Sends one queued payload if the socket is open and the queue is not empty. */
    Public Virtual Void SendData() = 0;

    /** Returns the number of payloads waiting in the send queue. */
    Public Virtual Size GetPendingSendCount() const = 0;

    /** Closes and reopens the last configured host/port. */
    Public Virtual Bool Reconnect() = 0;

    /** Returns one complete received line (without trailing newline), if available. */
    Public Virtual Optional<StdString> ReceiveData() = 0;
};

#endif // ICLOUDSOCKET_INTERNAL_H
