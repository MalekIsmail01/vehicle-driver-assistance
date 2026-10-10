#include "vehicle_assist.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>

static bool nearly_equal(float left, float right)
{
    return fabsf(left - right) < 0.001f;
}

static VehicleAssistInputs default_inputs(void)
{
    VehicleAssistInputs inputs = {0};
    inputs.target_speed_button = VEHICLE_SPEED_BUTTON_NONE;
    return inputs;
}

static VehicleAssistOutputs update(
    VehicleAssistController *controller,
    VehicleAssistInputs *inputs)
{
    VehicleAssistOutputs outputs = {0};
    assert(vehicle_assist_update(controller, inputs, &outputs));
    return outputs;
}

static void test_initialization_and_validation(void)
{
    VehicleAssistController controller;
    VehicleAssistInputs inputs = default_inputs();
    VehicleAssistOutputs outputs = {0};

    assert(!vehicle_assist_init(NULL));
    assert(vehicle_assist_init(&controller));
    assert(nearly_equal(controller.target_speed_kph, 10.0f));
    assert(controller.rain_level == VEHICLE_RAIN_DRY);

    outputs.target_speed_kph = 42.0f;
    inputs.current_speed_kph = -1.0f;
    assert(!vehicle_assist_update(&controller, &inputs, &outputs));
    assert(nearly_equal(outputs.target_speed_kph, 42.0f));

    inputs.current_speed_kph = 0.0f;
    inputs.rain_wetness_percent = NAN;
    assert(!vehicle_assist_update(&controller, &inputs, &outputs));
}

static void test_cruise_control_and_brake_priority(void)
{
    VehicleAssistController controller;
    VehicleAssistInputs inputs = default_inputs();
    VehicleAssistOutputs outputs;

    assert(vehicle_assist_init(&controller));
    inputs.cruise_enabled = true;
    inputs.current_speed_kph = 5.0f;
    outputs = update(&controller, &inputs);
    assert(outputs.cruise_enabled);
    assert(outputs.accelerate);
    assert(!outputs.brake);

    inputs.current_speed_kph = 15.0f;
    outputs = update(&controller, &inputs);
    assert(outputs.brake);
    assert(!outputs.accelerate);

    inputs.accelerator_pressed = true;
    outputs = update(&controller, &inputs);
    assert(outputs.cruise_enabled);
    assert(!outputs.brake);
    assert(outputs.accelerate);

    inputs.accelerator_pressed = false;
    outputs = update(&controller, &inputs);
    assert(outputs.cruise_enabled);
    assert(outputs.brake);
    assert(!outputs.accelerate);

    inputs.accelerator_pressed = true;
    inputs.brake_pressed = true;
    outputs = update(&controller, &inputs);
    assert(!outputs.cruise_enabled);
    assert(outputs.brake);
    assert(!outputs.accelerate);
}

static void test_target_speed_limits(void)
{
    VehicleAssistController controller;
    VehicleAssistInputs inputs = default_inputs();
    VehicleAssistOutputs outputs;
    int step;

    assert(vehicle_assist_init(&controller));
    inputs.target_speed_button = VEHICLE_SPEED_BUTTON_DECREASE;
    outputs = update(&controller, &inputs);
    assert(nearly_equal(outputs.target_speed_kph, 10.0f));

    inputs.target_speed_button = VEHICLE_SPEED_BUTTON_INCREASE;
    for (step = 0; step < 30; ++step) {
        outputs = update(&controller, &inputs);
    }
    assert(nearly_equal(outputs.target_speed_kph, 130.0f));
    outputs = update(&controller, &inputs);
    assert(nearly_equal(outputs.target_speed_kph, 130.0f));
}

static void test_emergency_braking_distance(void)
{
    VehicleAssistController controller;
    VehicleAssistInputs inputs = default_inputs();
    VehicleAssistOutputs outputs;

    assert(vehicle_assist_init(&controller));
    inputs.current_speed_kph = 36.0f;
    inputs.lead_distance_m = 4.0f;
    inputs.cruise_enabled = true;
    inputs.accelerator_pressed = true;
    outputs = update(&controller, &inputs);
    assert(nearly_equal(outputs.stopping_distance_m, 5.0f));
    assert(outputs.emergency_brake);
    assert(outputs.brake);
    assert(!outputs.cruise_enabled);
    assert(!outputs.accelerate);

    inputs.lead_distance_m = 5.0f;
    outputs = update(&controller, &inputs);
    assert(!outputs.emergency_brake);

    inputs.lead_distance_m = 0.0f;
    outputs = update(&controller, &inputs);
    assert(!outputs.emergency_brake);
}

static void test_rain_hysteresis(void)
{
    VehicleAssistController controller;
    VehicleAssistInputs inputs = default_inputs();
    VehicleAssistOutputs outputs;

    assert(vehicle_assist_init(&controller));
    inputs.rain_wetness_percent = 25.0f;
    outputs = update(&controller, &inputs);
    assert(outputs.rain_level == VEHICLE_RAIN_LIGHT);
    assert(outputs.wiper_level == 1U);

    inputs.rain_wetness_percent = 20.0f;
    outputs = update(&controller, &inputs);
    assert(outputs.rain_level == VEHICLE_RAIN_LIGHT);

    inputs.rain_wetness_percent = 15.0f;
    outputs = update(&controller, &inputs);
    assert(outputs.rain_level == VEHICLE_RAIN_DRY);

    inputs.rain_wetness_percent = 70.0f;
    outputs = update(&controller, &inputs);
    assert(outputs.rain_level == VEHICLE_RAIN_HEAVY);
    assert(outputs.wiper_level == 2U);

    inputs.rain_wetness_percent = 65.0f;
    outputs = update(&controller, &inputs);
    assert(outputs.rain_level == VEHICLE_RAIN_HEAVY);

    inputs.rain_wetness_percent = 60.0f;
    outputs = update(&controller, &inputs);
    assert(outputs.rain_level == VEHICLE_RAIN_LIGHT);
}

int main(void)
{
    test_initialization_and_validation();
    test_cruise_control_and_brake_priority();
    test_target_speed_limits();
    test_emergency_braking_distance();
    test_rain_hysteresis();
    puts("All vehicle-assist tests passed.");
    return 0;
}
