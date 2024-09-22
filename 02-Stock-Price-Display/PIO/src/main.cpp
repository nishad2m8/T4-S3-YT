#include <lvgl.h>
#include <LV_Helper.h>
#include <LilyGo_AMOLED.h>
#include <ui.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "credentials.h" // Include the credentials header

// Define display and touch hardware specifics
LilyGo_Class amoled;

// Arrays to store stock data for the candle chart
float openValues[10];
float highValues[10];
float lowValues[10];
float closeValues[10];
std::string dates[10]; // Array to store the dates

// Vector to store pointers to created objects for easy cleanup
std::vector<lv_obj_t *> candleObjects;

void fetchStockData() {
    Serial.println("Starting HTTP request");
    std::string symbol = "TSLA";
    std::string apiKey = API_KEY;
    std::string url = "https://www.alphavantage.co/query?function=TIME_SERIES_DAILY&symbol=" + symbol + "&apikey=" + apiKey;

    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(url.c_str()); // Specify the URL

        int httpCode = http.GET(); // Make the request

        if (httpCode > 0) {
            Serial.printf("HTTP request successful, code: %d\n", httpCode);
            String payload = http.getString();
            Serial.println("HTTP response received");

            // Increase JSON document size
            DynamicJsonDocument jsonData(32768); // Increased the size to handle larger payloads
            DeserializationError error = deserializeJson(jsonData, payload);

            if (error) {
                Serial.print(F("deserializeJson() failed: "));
                Serial.println(error.f_str());
                return;
            }

            // Extract Meta Data
            std::string symbol = jsonData["Meta Data"]["2. Symbol"].as<std::string>();
            std::string lastRefreshed = jsonData["Meta Data"]["3. Last Refreshed"].as<std::string>();

            Serial.printf("Symbol: %s\n", symbol.c_str());
            Serial.printf("Last Refreshed: %s\n", lastRefreshed.c_str());

            // Extract Time Series data
            JsonObject timeSeries = jsonData["Time Series (Daily)"].as<JsonObject>();
            std::vector<std::pair<std::string, JsonObject>> timeSeriesData;

            for (JsonPair kv : timeSeries) {
                timeSeriesData.push_back(std::make_pair(kv.key().c_str(), kv.value().as<JsonObject>()));
            }

            // Sort the data in descending order of time
            std::sort(timeSeriesData.begin(), timeSeriesData.end(), [](const std::pair<std::string, JsonObject>& a, const std::pair<std::string, JsonObject>& b) {
                return a.first > b.first;
            });

            // Get the most recent 10 data points
            for (int i = 0; i < 10 && i < timeSeriesData.size(); ++i) {
                auto& data = timeSeriesData[i];
                openValues[i] = data.second["1. open"].as<float>();
                highValues[i] = data.second["2. high"].as<float>();
                lowValues[i] = data.second["3. low"].as<float>();
                closeValues[i] = data.second["4. close"].as<float>();
                dates[i] = data.first; // Store the date

                // Debug output for each data point
                Serial.printf("Data point %d - Open: %.2f, High: %.2f, Low: %.2f, Close: %.2f\n", i, openValues[i], highValues[i], lowValues[i], closeValues[i]);
            }

            // Update LVGL labels with the most recent data point
            if (!timeSeriesData.empty()) {
                auto& latestData = timeSeriesData.front();
                std::string latestDate = latestData.first;
                char open[10], high[10], low[10], close[10];
                snprintf(open, sizeof(open), "%.2f", latestData.second["1. open"].as<float>());
                snprintf(high, sizeof(high), "%.2f", latestData.second["2. high"].as<float>());
                snprintf(low, sizeof(low), "%.2f", latestData.second["3. low"].as<float>());
                snprintf(close, sizeof(close), "%.2f", latestData.second["4. close"].as<float>());
                const char* volume = latestData.second["5. volume"].as<const char*>();

                Serial.printf("Date: %s\n", latestDate.c_str());
                Serial.printf("Open: %s\n", open);
                Serial.printf("High: %s\n", high);
                Serial.printf("Low: %s\n", low);
                Serial.printf("Close: %s\n", close);
                Serial.printf("Volume: %s\n", volume);

                // Update LVGL labels
                lv_label_set_text(ui_Label_Symbol, symbol.c_str());
                lv_label_set_text(ui_Label_updated, lastRefreshed.c_str());
                lv_label_set_text(ui_Label_date, latestDate.c_str());
                lv_label_set_text(ui_Label_close, close);
                lv_label_set_text(ui_Label_open, open);
                lv_label_set_text(ui_Label_high, high);
                lv_label_set_text(ui_Label_low, low);
                lv_label_set_text(ui_Label_volume, volume);
            } else {
                Serial.println("No data found in Time Series (Daily)");
            }
        } else {
            Serial.printf("GET request failed: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end(); // Free the resources
    } else {
        Serial.println("WiFi not connected");
    }
}

void drawCandleChart() {
    int chartWidth = 380;
    int chartHeight = 245;
    int chartX = 62;
    int chartY = 320;
    int candleWidth = chartWidth / 10 - 5;

    // Find the minimum and maximum values in the data to scale appropriately
    float minValue = *std::min_element(lowValues, lowValues + 10);
    float maxValue = *std::max_element(highValues, highValues + 10);
    float valueRange = maxValue - minValue;

    // Debug output for min and max values
    Serial.printf("Min Value: %f, Max Value: %f, Value Range: %f\n", minValue, maxValue, valueRange);

    for (int i = 0; i < 10; ++i) {
        int x = chartX + i * (candleWidth + 2);
        int openY = chartY + chartHeight - (chartHeight * (openValues[9 - i] - minValue) / valueRange);
        int closeY = chartY + chartHeight - (chartHeight * (closeValues[9 - i] - minValue) / valueRange);
        int highY = chartY + chartHeight - (chartHeight * (highValues[9 - i] - minValue) / valueRange);
        int lowY = chartY + chartHeight - (chartHeight * (lowValues[9 - i] - minValue) / valueRange);

        // Debug output for each candle's drawing positions
        Serial.printf("Candle %d - X: %d, OpenY: %d, CloseY: %d, HighY: %d, LowY: %d\n", i, x, openY, closeY, highY, lowY);

        lv_color_t candleColor;
        bool isBullish = closeValues[9 - i] > openValues[9 - i];

        if (isBullish) {
            candleColor = lv_color_hex(0x00FF00); // Green color
            lv_obj_set_style_bg_color(ui_Panel_Status, lv_color_hex(0x00FF00), LV_PART_MAIN | LV_STATE_DEFAULT);
        } else {
            candleColor = lv_color_hex(0xFF0000); // Red color
            lv_obj_set_style_bg_color(ui_Panel_Status, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);
        }

        // Draw the high-low thin box
        lv_obj_t *hl_box = lv_obj_create(lv_scr_act());
        lv_obj_set_style_bg_color(hl_box, candleColor, 0);
        lv_obj_set_pos(hl_box, x + candleWidth / 2 - 1, highY); // Adjust x to center the box
        lv_obj_set_size(hl_box, 5, lowY - highY); // Width of 5 pixels for thin box
        lv_obj_set_style_bg_opa(hl_box, LV_OPA_COVER, 0); // Ensure the box is fully opaque
        lv_obj_set_style_radius(hl_box, 0, 0); // No rounded edges
        candleObjects.push_back(hl_box); // Store the object pointer for cleanup

        // Draw the candle body
        lv_obj_t *candle = lv_obj_create(lv_scr_act());
        lv_obj_set_style_bg_color(candle, candleColor, 0);
        lv_obj_set_style_radius(candle, 0, 0); // No rounded edges

        if (isBullish) {
            lv_obj_set_pos(candle, x, closeY);
            lv_obj_set_size(candle, candleWidth, openY - closeY);
        } else {
            lv_obj_set_pos(candle, x, openY);
            lv_obj_set_size(candle, candleWidth, closeY - openY);
        }

        lv_obj_set_style_bg_opa(candle, LV_OPA_COVER, 0); // Ensure the candle is fully opaque
        candleObjects.push_back(candle); // Store the object pointer for cleanup

        // Add date label below each candle
        lv_obj_t *date_label = lv_label_create(lv_scr_act());
        std::string dateOnly = dates[9 - i].substr(8, 2); // Extract the day part of the date, reverse the order
        lv_label_set_text(date_label, dateOnly.c_str());
        lv_obj_set_style_text_color(date_label, lv_color_hex(0xFFFFFF), 0);
        lv_obj_align(date_label, LV_ALIGN_TOP_LEFT, x + candleWidth / 2 - 5, chartY + chartHeight + 5); // Center below the candle
        candleObjects.push_back(date_label); // Store the object pointer for cleanup
    }

    // Add y-axis labels for price values
    int labelCount = 5; // Number of labels
    for (int j = 0; j <= labelCount; ++j) {
        float value = minValue + j * (valueRange / labelCount);
        int y = chartY + chartHeight - (chartHeight * (value - minValue) / valueRange);

        // Debug output for y-axis labels
        Serial.printf("Y-axis Label %d - Value: %f, Y: %d\n", j, value, y);

        lv_obj_t *value_label = lv_label_create(lv_scr_act());
        char value_str[10];
        snprintf(value_str, sizeof(value_str), "%d", static_cast<int>(value)); // Format as integer
        lv_label_set_text(value_label, value_str);
        lv_obj_set_style_text_color(value_label, lv_color_hex(0xFFFFFF), 0);
        lv_obj_align(value_label, LV_ALIGN_TOP_LEFT, chartX - 40, y - 10); // Position to the left of the chart
        candleObjects.push_back(value_label); // Store the object pointer for cleanup
    }
}

void clearCandleChart()
{
    for (lv_obj_t *obj : candleObjects)
    {
        lv_obj_del(obj);
    }
    candleObjects.clear();
}

void setup(void) {
    Serial.begin(115200);
    Serial.println("Setup started");

    // Initialize AMOLED Display
    if (!amoled.begin()) {
        Serial.println("AMOLED init failed");
        while (1) delay(1000);
    }
    Serial.println("AMOLED initialized");

    // Set the orientation of the AMOLED display
    amoled.setRotation(3); // Portrait mode. USB port on the right bottom.

    // Initialize LVGL
    beginLvglHelper(amoled);
    Serial.println("LVGL initialized");

    // Initialize UI from LVGL helper
    ui_init();
    Serial.println("UI initialized");

    // Connect to WiFi
    Serial.println("Connecting to WiFi");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    fetchStockData();
    drawCandleChart(); // Draw the chart after fetching the data
}

void loop() {
    // Handle LVGL tasks
    lv_task_handler();
    delay(5);
}
