#include "RCSwitchReceiver.h"

RCSwitchReceiver::RCSwitchReceiver(uint8_t interruptPin)
    : interruptPin(interruptPin) {
}

void RCSwitchReceiver::begin() {
    // The RCSwitch library expects Interrupt numbers rather than raw GPIOs
    rcSwitch.enableReceive(digitalPinToInterrupt(interruptPin));
}

bool RCSwitchReceiver::read() {
    if (rcSwitch.available()) {
        unsigned long received = rcSwitch.getReceivedValue();
        rcSwitch.resetAvailable();
        if (received == 0) {
            // 0 usually indicates an error in reception – ignore
            return false;
        }

        // Debounce identical codes that arrive in rapid succession
        unsigned long now = millis();
        if (received != lastCode || (now - lastReportTime) > RF_REPEAT_DELAY) {
            lastCode = received;
            lastReportTime = now;
            newCodeAvailable = true;
            return true;
        }
    }
    return false;
}

uint32_t RCSwitchReceiver::getButtonCode() {
    newCodeAvailable = false;
    return lastCode;
} 