#include "vehicle_assist.h"

#include <stddef.h>

#define INITIAL_TARGET_SPEED_KPH 10.0f
#define MIN_TARGET_SPEED_KPH 10.0f
#define MAX_TARGET_SPEED_KPH 130.0f
#define TARGET_SPEED_STEP_KPH 5.0f
#define NORMAL_DECELERATION_MPS2 10.0f
#define MAX_INPUT_SPEED_KPH 300.0f
#define MAX_LEAD_DISTANCE_M 10000.0f

static bool inputs_are_valid(const VehicleAssistInputs *inputs)
{
    return inputs->current_speed_kph >= 0.0f
        && inputs->current_speed_kph <= MAX_INPUT_SPEED_KPH
        && inputs->lead_distance_m >= 0.0f
        && inputs->lead_distance_m <= MAX_LEAD_DISTANCE_M
        && inputs->rain_wetness_percent >= 0.0f
        && inputs->rain_wetness_percent <= 100.0f
        && inputs->target_speed_button >= VEHICLE_SPEED_BUTTON_NONE
        && inputs->target_speed_button <= VEHICLE_SPEED_BUTTON_DECREASE;
}

static void update_target_speed(
    VehicleAssistController *controller,
    VehicleSpeedButton button)
{
    if (button == VEHICLE_SPEED_BUTTON_INCREASE) {
        controller->target_speed_kph += TARGET_SPEED_STEP_KPH;
    } else if (button == VEHICLE_SPEED_BUTTON_DECREASE) {
        controller->target_speed_kph -= TARGET_SPEED_STEP_KPH;
    }

    if (controller->target_speed_kph < MIN_TARGET_SPEED_KPH) {
        controller->target_speed_kph = MIN_TARGET_SPEED_KPH;
    } else if (controller->target_speed_kph > MAX_TARGET_SPEED_KPH) {
        controller->target_speed_kph = MAX_TARGET_SPEED_KPH;
    }
}

static VehicleRainLevel update_rain_level(
    VehicleRainLevel current_level,
    float wetness_percent)
{
    switch (current_level) {
    case VEHICLE_RAIN_DRY:
        if (wetness_percent >= 70.0f) {
            return VEHICLE_RAIN_HEAVY;
        }
        return wetness_percent >= 25.0f
            ? VEHICLE_RAIN_LIGHT
            : VEHICLE_RAIN_DRY;
    case VEHICLE_RAIN_LIGHT:
        if (wetness_percent <= 15.0f) {
            return VEHICLE_RAIN_DRY;
        }
        return wetness_percent >= 70.0f
            ? VEHICLE_RAIN_HEAVY
            : VEHICLE_RAIN_LIGHT;
    case VEHICLE_RAIN_HEAVY:
        return wetness_percent <= 60.0f
            ? VEHICLE_RAIN_LIGHT
            : VEHICLE_RAIN_HEAVY;
    default:
        return VEHICLE_RAIN_DRY;
    }
}

bool vehicle_assist_init(VehicleAssistController *controller)
{
    if (controller == NULL) {
        return false;
    }

    controller->target_speed_kph = INITIAL_TARGET_SPEED_KPH;
    controller->rain_level = VEHICLE_RAIN_DRY;
    return true;
}

bool vehicle_assist_update(
    VehicleAssistController *controller,
    const VehicleAssistInputs *inputs,
    VehicleAssistOutputs *outputs)
{
    VehicleAssistOutputs next_outputs = {0};
    float current_speed_mps;

    if (controller == NULL || inputs == NULL || outputs == NULL
        || !inputs_are_valid(inputs)) {
        return false;
    }

    update_target_speed(controller, inputs->target_speed_button);
    controller->rain_level = update_rain_level(
        controller->rain_level,
        inputs->rain_wetness_percent);

    current_speed_mps = inputs->current_speed_kph / 3.6f;
    next_outputs.stopping_distance_m =
        (current_speed_mps * current_speed_mps)
        / (2.0f * NORMAL_DECELERATION_MPS2);
    next_outputs.emergency_brake =
        inputs->lead_distance_m > 0.0f
        && inputs->lead_distance_m < next_outputs.stopping_distance_m;

    next_outputs.cruise_enabled =
        inputs->cruise_enabled
        && !inputs->brake_pressed
        && !next_outputs.emergency_brake;
    next_outputs.brake =
        inputs->brake_pressed
        || next_outputs.emergency_brake
        || (next_outputs.cruise_enabled
            && !inputs->accelerator_pressed
            && inputs->current_speed_kph > controller->target_speed_kph);
    next_outputs.accelerate =
        !next_outputs.brake
        && (inputs->accelerator_pressed
            || (next_outputs.cruise_enabled
                && inputs->current_speed_kph < controller->target_speed_kph));
    next_outputs.target_speed_kph = controller->target_speed_kph;
    next_outputs.rain_level = controller->rain_level;
    next_outputs.wiper_level = (unsigned int)controller->rain_level;

    *outputs = next_outputs;
    return true;
}
