#include <lvgl.h>
#include <LV_Helper.h>
#include <TFT_eSPI.h>
#include <LilyGo_AMOLED.h>
#include <ui.h>

// https://www.youtube.com/nishad2m8
// https://buymeacoffee.com/nishad2m8

// Define display and touch hardware specifics
LilyGo_Class amoled;
TFT_eSPI tft = TFT_eSPI();

void setup(void) {
    Serial.begin(115200);

    // Initialize AMOLED Display
    if (!amoled.begin()) {
        Serial.println("AMOLED init failed");
        while(1) delay(1000);
    }

    amoled.setRotation(3); // Portrait mode. USB port on the right bottom.

    // Initialize LVGL
    beginLvglHelper(amoled);

    // Initialize UI from LVGL helper
    ui_init();
}

void loop() {
    // Handle LVGL tasks
    lv_task_handler();
    delay(5);
}
