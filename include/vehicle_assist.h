#ifndef VEHICLE_ASSIST_H
#define VEHICLE_ASSIST_H

#include <stdbool.h>

typedef enum {
    VEHICLE_SPEED_BUTTON_NONE = 0,
    VEHICLE_SPEED_BUTTON_INCREASE,
    VEHICLE_SPEED_BUTTON_DECREASE
} VehicleSpeedButton;

typedef enum {
    VEHICLE_RAIN_DRY = 0,
    VEHICLE_RAIN_LIGHT,
    VEHICLE_RAIN_HEAVY
} VehicleRainLevel;

typedef struct {
    float current_speed_kph;
    float lead_distance_m;
    float rain_wetness_percent;
    bool cruise_enabled;
    bool accelerator_pressed;
    bool brake_pressed;
    VehicleSpeedButton target_speed_button;
} VehicleAssistInputs;

typedef struct {
    bool cruise_enabled;
    bool accelerate;
    bool brake;
    bool emergency_brake;
    float target_speed_kph;
    float stopping_distance_m;
    VehicleRainLevel rain_level;
    unsigned int wiper_level;
} VehicleAssistOutputs;

typedef struct {
    float target_speed_kph;
    VehicleRainLevel rain_level;
} VehicleAssistController;

bool vehicle_assist_init(VehicleAssistController *controller);
bool vehicle_assist_update(
    VehicleAssistController *controller,
    const VehicleAssistInputs *inputs,
    VehicleAssistOutputs *outputs);

#endif
