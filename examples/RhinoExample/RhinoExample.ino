/*
    Copyright 2025 Picovoice Inc.

    You may not use this file except in compliance with the license. A copy of the license is located in the "LICENSE"
    file accompanying this source.

    Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
    an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
    specific language governing permissions and limitations under the License.
*/

#include <Rhino_EN.h>

#include "params.h"

#define MEMORY_BUFFER_SIZE (70 * 1024)

static const char *ACCESS_KEY = "${ACCESS_KEY}"; // AccessKey string obtained from Picovoice Console (https://picovoice.ai/console/)

static int8_t memory_buffer[MEMORY_BUFFER_SIZE] __attribute__((aligned(16)));

static pv_rhino_t *handle = NULL;

static const float SENSITIVITY = 0.75f;
static const float ENDPOINT_DURATION_SEC = 1.0f;
static const bool REQUIRE_ENDPOINT = true;

static void inference_callback(bool is_understood, const char *intent, int32_t num_slots, const char **slots, const char **values) {
    Serial.println("{");
    Serial.print("    is_understood : '"); Serial.print(is_understood ? "true" : "false"); Serial.println("',");
    if (is_understood) {
        Serial.print("    intent : '"); Serial.print(intent); Serial.println("',");
        if (num_slots > 0) {
            Serial.println("    slots : {");
            for (int32_t i = 0; i < num_slots; i++) {
                Serial.print("        '"); Serial.print(slots[i]); Serial.print("' : '"); Serial.print(values[i]); Serial.println("',");
            }
            Serial.println("    }");
        }
    }
    Serial.println("}");
    Serial.println("");
}

static void print_error_message(char **message_stack, int32_t message_stack_depth) {
    for (int32_t i = 0; i < message_stack_depth; i++) {
        Serial.println(message_stack[i]);
    }
}

static void error_handler() {
    char **message_stack = NULL;
    int32_t message_stack_depth = 0;
    pv_status_t error_status = pv_get_error_stack(&message_stack, &message_stack_depth);
    if (error_status != PV_STATUS_SUCCESS) {
        Serial.println("Unable to get Rhino error state");
        while (1);
    }
    print_error_message(message_stack, message_stack_depth);
    pv_free_error_stack(message_stack);
    while (1);
}

void setup() {

    Serial.begin(9600);
    while (!Serial);

    pv_status_t status = picovoice::rhino::pv_audio_rec_init();
    if (status != PV_STATUS_SUCCESS) {
        Serial.print("Audio init failed with ");
        Serial.println(pv_status_to_string(status));
        while (1);
    }

    char **message_stack = NULL;
    int32_t message_stack_depth = 0;
    pv_status_t error_status;

    status = pv_rhino_init(
        ACCESS_KEY,
        memory_buffer,
        MEMORY_BUFFER_SIZE,
        CONTEXT_ARRAY,
        sizeof(CONTEXT_ARRAY),
        SENSITIVITY,
        ENDPOINT_DURATION_SEC,
        REQUIRE_ENDPOINT,
        &handle);
    if (status != PV_STATUS_SUCCESS) {
        Serial.print("Rhino init failed with ");
        Serial.println(pv_status_to_string(status));

        error_status = pv_get_error_stack(&message_stack, &message_stack_depth);
        if (error_status != PV_STATUS_SUCCESS) {
            Serial.println("Unable to get Rhino error state");
            while (1);
        }
        print_error_message(message_stack, message_stack_depth);
        pv_free_error_stack(message_stack);

        while (1);
    }
    Serial.println("The board is listening for utterances...");
}

void loop() {
    const int16_t *buffer = picovoice::rhino::pv_audio_rec_get_new_buffer();
    if (buffer) {
        bool is_finalized = false;
        pv_status_t status = pv_rhino_process(handle, buffer, &is_finalized);
        if (status != PV_STATUS_SUCCESS) {
            Serial.print("Rhino process failed with ");
            Serial.println(pv_status_to_string(status));
            error_handler();
        }
        if (is_finalized) {
            bool is_understood = false;
            status = pv_rhino_is_understood(handle, &is_understood);
            if (status != PV_STATUS_SUCCESS) {
                Serial.print("Rhino is_understood failed with ");
                Serial.println(pv_status_to_string(status));
                error_handler();
            }

            if (is_understood) {
                const char *intent = NULL;
                int32_t num_slots = 0;
                const char **slots = NULL;
                const char **values = NULL;

                status = pv_rhino_get_intent(
                        handle,
                        &intent,
                        &num_slots,
                        &slots,
                        &values);
                if (status != PV_STATUS_SUCCESS) {
                    Serial.print("Rhino get_intent failed with ");
                    Serial.println(pv_status_to_string(status));
                    error_handler();
                }

                inference_callback(is_understood, intent, num_slots, slots, values);

                status = pv_rhino_free_slots_and_values(handle, slots, values);
                if (status != PV_STATUS_SUCCESS) {
                    Serial.print("Rhino free_slots_and_values failed with ");
                    Serial.println(pv_status_to_string(status));
                    error_handler();
                }
            } else {
                inference_callback(is_understood, NULL, 0, NULL, NULL);
            }

            status = pv_rhino_reset(handle);
            if (status != PV_STATUS_SUCCESS) {
                Serial.print("Rhino reset failed with ");
                Serial.println(pv_status_to_string(status));
                error_handler();
            }
        }
    }
}
