#include <lvgl.h>
#include <LV_Helper.h>
#include <TFT_eSPI.h>
#include <LilyGo_AMOLED.h>
#include <ui.h>

// Define display and touch hardware specifics
LilyGo_Class amoled;
TFT_eSPI tft = TFT_eSPI();

// Timer function to update the vacuum cleaner's battery slider
void vacuumBatterySlider(lv_timer_t *timer)
{
    lv_obj_t *slider = (lv_obj_t *)timer->user_data;
    int value = lv_slider_get_value(slider);
    bool switch_on = lv_obj_has_state(ui_Switch_vacuum, LV_STATE_CHECKED);

    if (switch_on)
    {
        // Decrement battery value
        if (value > 0)
        {
            lv_slider_set_value(slider, value - 1, LV_ANIM_OFF);
        }
    }
    else
    {
        // Increment battery value
        if (value < 100)
        {
            lv_slider_set_value(slider, value + 1, LV_ANIM_OFF);
        }
    }

    // Update the label with the new slider value
    char buf[4]; // buffer to hold the string representation of the value
    sprintf(buf, "%d", value);
    lv_label_set_text(ui_Label_battery, buf);
}

// Timer function to update the EV battery bar
void evBatterySlider(lv_timer_t *timer)
{
    static bool increasing = true;
    static int value = 0;
    bool switch_on = lv_obj_has_state(ui_Switch_ev, LV_STATE_CHECKED);

    if (!switch_on)
    {
        // If the switch is off, set the value to 0
        value = 0;
        increasing = true; // Reset increasing flag
        lv_bar_set_value(ui_Bar_ev, value, LV_ANIM_OFF);

        // Update the label with the new bar value
        char buf[4]; // buffer to hold the string representation of the value
        sprintf(buf, "%d", value);
        lv_label_set_text(ui_Label_ev_battery, buf);

        return; // Do not proceed with incrementing
    }

    // Update the bar value if it's still increasing
    if (increasing)
    {
        value++;
        if (value >= 100)
        {
            value = 100;
            increasing = false;
        }

        lv_bar_set_value(ui_Bar_ev, value, LV_ANIM_OFF);

        // Update the label with the new bar value
        char buf[4]; // buffer to hold the string representation of the value
        sprintf(buf, "%d", value);
        lv_label_set_text(ui_Label_ev_battery, buf);
    }
}

// Timer function to manage the washing machine timer
void wmTimer(lv_timer_t *timer)
{
    static int countdown_time = 0;
    static bool is_counting_down = false;
    static lv_obj_t *active_button = NULL;

    // Check button statuses
    bool button_smart = lv_obj_has_state(ui_Button_Smart, LV_STATE_CHECKED);
    bool button_quick = lv_obj_has_state(ui_Button_Quick, LV_STATE_CHECKED);
    bool button_linen = lv_obj_has_state(ui_Button_Linen, LV_STATE_CHECKED);
    bool button_saver = lv_obj_has_state(ui_Button_Saver, LV_STATE_CHECKED);

    // Start countdown based on button press
    if (button_smart)
    {
        if (active_button != ui_Button_Smart)
        {
            active_button = ui_Button_Smart;
            countdown_time = 3600; // 60 minutes
            is_counting_down = true;
        }
    }
    else if (button_quick)
    {
        if (active_button != ui_Button_Quick)
        {
            active_button = ui_Button_Quick;
            countdown_time = 1800; // 30 minutes
            is_counting_down = true;
        }
    }
    else if (button_linen)
    {
        if (active_button != ui_Button_Linen)
        {
            active_button = ui_Button_Linen;
            countdown_time = 2700; // 45 minutes
            is_counting_down = true;
        }
    }
    else if (button_saver)
    {
        if (active_button != ui_Button_Saver)
        {
            active_button = ui_Button_Saver;
            countdown_time = 2400; // 40 minutes
            is_counting_down = true;
        }
    }
    else
    {
        // If no button is pressed or active button is turned off, reset the countdown
        active_button = NULL;
        is_counting_down = false;
        countdown_time = 0;
    }

    // Update the countdown timer
    if (is_counting_down && countdown_time > 0)
    {
        countdown_time--;

        // Update the label with the new countdown value
        int minutes = countdown_time / 60;
        int seconds = countdown_time % 60;
        char buf[6]; // buffer to hold the string representation of the value
        sprintf(buf, "%02d:%02d", minutes, seconds);
        lv_label_set_text(ui_Label_wm_timer, buf);

        if (countdown_time == 0)
        {
            is_counting_down = false; // Stop counting down when it reaches zero
        }
    }
    else
    {
        // If countdown time is 0, update the label to show 00:00
        lv_label_set_text(ui_Label_wm_timer, "00:00");
    }
}

// Timer function to manage the smart plug watt value
void smartPlug(lv_timer_t *timer)
{
    static bool increasing = true;
    static int watt_value = 155;
    bool switch_on = lv_obj_has_state(ui_Switch_SP, LV_STATE_CHECKED);

    if (!switch_on)
    {
        return; // If switch is off, do not update
    }

    // Update the watt value based on the current direction
    if (increasing)
    {
        watt_value++;
        if (watt_value >= 199)
        {
            watt_value = 199;
            increasing = false;
        }
    }
    else
    {
        watt_value--;
        if (watt_value <= 155)
        {
            watt_value = 155;
            increasing = true;
        }
    }

    // Update the label with the new watt value
    char buf[4]; // buffer to hold the string representation of the value
    sprintf(buf, "%d", watt_value);
    lv_label_set_text(ui_Label_watt, buf);
}

// Setup function to initialize hardware and create timers
void setup(void)
{
    Serial.begin(115200);

    // Initialize AMOLED Display
    if (!amoled.begin())
    {
        Serial.println("AMOLED init failed");
        while (1)
            delay(1000);
    }

    amoled.setRotation(3); // Portrait mode. USB port on the right bottom.

    // Initialize LVGL
    beginLvglHelper(amoled);

    // Initialize UI from LVGL helper
    ui_init();

    // Create a timer to update the vacuum slider value periodically
    lv_timer_create(vacuumBatterySlider, 100, ui_Slider_vacuum_batt);

    // Create a timer to update the EV battery bar value periodically
    lv_timer_create(evBatterySlider, 100, NULL); // Assuming no need to pass ui_Bar_ev as it's accessed globally

    // Create a timer to update the watt value periodically
    lv_timer_create(smartPlug, 100, NULL); // Assuming no need to pass any user data

    // Create a timer to update the countdown timer periodically
    lv_timer_create(wmTimer, 1000, NULL); // 1000 ms = 1 second
}

// Loop function to handle LVGL tasks
void loop()
{
    lv_task_handler(); // Handle LVGL tasks
    delay(5);
}
