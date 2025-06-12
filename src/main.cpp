#include "ComputerController.h"
#include "Globals.h"
#include <SimpleTimer.h>

ComputerController *controller = nullptr;
SimpleTimer<> loopTimer(1);  // 1ms delay between loops

void setup() {
    controller = new ComputerController();
    controller->setup();
}

void loop() {
    if (controller != nullptr) {
        controller->loop();
    }
    
    // Use SimpleTimer for loop delay
    if (loopTimer.isReady()) {
        loopTimer.reset();
        vTaskDelay(pdMS_TO_TICKS(1));
    }
} 