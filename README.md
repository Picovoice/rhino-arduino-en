# Rhino SDK for Arduino boards - English language

Made in Vancouver, Canada by [Picovoice](https://picovoice.ai)

Rhino is Picovoice's Speech-to-Intent engine. It directly infers intent from spoken commands within a given context of
interest, in real-time. For example, given a spoken command:

> Can I have a small double-shot espresso?

Rhino infers what the user wants and emits the following inference result:

```json
{
  "isUnderstood": "true",
  "intent": "orderBeverage",
  "slots": {
    "beverage": "espresso",
    "size": "small",
    "numberOfShots": "2"
  }
}
```

Rhino is:

- using deep neural networks trained in real-world environments.
- compact and computationally-efficient. It is perfect for IoT.
- self-service. Developers can train custom contexts using [Picovoice Console](https://console.picovoice.ai/).

## Compatibility

- [Arduino Nano 33 BLE Sense](https://docs.arduino.cc/hardware/nano-33-ble)

## Dependency

- LibPrintf

## AccessKey

The Rhino SDK requires a valid `AccessKey` at initialization. `AccessKey`s act as your credentials when using Rhino SDKs.
You can create your `AccessKey` for free. Make sure to keep your `AccessKey` secret.
Signup or Login to [Picovoice Console](https://console.picovoice.ai/) to get your `AccessKey`.

## Installation

1. Install the [Arduino IDE](https://www.arduino.cc/en/software/) for your platform.
2. With the IDE open, go to `Tools`->`Manage Libraries...`
3. Search for `Rhino_EN`, then click `INSTALL`.

This package is intended to be used via the Arduino Library Manager.

## Example

The library comes with an example that can be used as a starting point to quickly get started with the library.

1. Open `File`->`Examples`->`Rhino_EN`->`RhinoExample`.
2. Replace `ACCESS_KEY` in the source with the `AccessKey` obtained from Picovoice Console.
3. Press `Upload` and check the `Serial Monitor` for outputs.

Additional information can be found in the [Picovoice Docs](https://picovoice.ai/docs/quick-start/rhino-arduino/).

## Integration

1. define all the necessary variables before `setup()`:

```c
#include <Rhino_EN.h>

#define MEMORY_BUFFER_SIZE ...
static uint8_t memory_buffer[MEMORY_BUFFER_SIZE] __attribute__((aligned(16));

static const char* ACCESS_KEY = ...; //AccessKey string obtained from [Picovoice Console](https://picovoice.ai/console/)

const uint8_t CONTEXT_ARRAY[] = {...};
static const float SENSITIVITY = 0.75f;
static const float ENDPOINT_DURATION_SEC = 1.0f;
static const bool REQUIRE_ENDPOINT = true;

pv_rhino_t *handle = NULL;
```

Sensitivity is the parameter that enables developers to trade miss rate for false alarm. A higher sensitivity value results in fewer misses at the cost of (potentially) increasing the erroneous inference rate.

Endpoint duration is a chunk of silence at the end of an utterance that marks the end of spoken command. A lower endpoint duration reduces delay and improves responsiveness. A higher endpoint duration assures Rhino doesn't return inference pre-emptively in case the user pauses before finishing the request.

Require endpoint is a parameter when set to `true`, Rhino requires an endpoint (a chunk of silence) after the spoken command. If set to `false`, Rhino tries to detect silence, but if it cannot, it still will provide inference regardless. Set to `false` only if operating in an environment with overlapping speech (e.g. people talking in the background).

`handle` is an instance of Rhino runtime engine.

2. put the following code block inside `setup()` in order to initialize the Rhino engine:

```c
const pv_status_t status = pv_rhino_init(
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
    // error handling logic
}
```

Rhino accepts single channel, 16-bit PCM audio. The sample rate can be retrieved using `pv_sample_rate()`. Rhino accepts input audio in consecutive chunks (aka frames); the length of each frame can be retrieved using `pv_rhino_frame_length()`. Inside the `loop()` function in the sketch, pass the recorded audio to the Rhino engine:

```cpp
const int16_t *pcm = picovoice::rhino::pv_audio_rec_get_new_buffer()
bool is_finalized = false;
pv_status_t status = pv_rhino_process(handle, pcm, &is_finalized);
if (status != PV_STATUS_SUCCESS) {
    // error handling logic
}
if (is_finalized) {
    // inference event logic/callback
}
```

## Create Custom Context

1. Compile and upload the `Rhino_EN/GetUUID` sketch from the `File -> Examples` menu. Copy the UUID of the board printed at the beginning of the session to the serial monitor.
2. Go to [Picovoice Console](https://console.picovoice.ai/) to create a context for [Rhino speech to intent engine](https://picovoice.ai/docs/quick-start/console-rhino/).
3. Select `Arm Cortex M` as the platform when training the model.
4. Select your board type (`Arduino Nano 33 BLE Sense`) and provide the UUID of the chipset on the board.

## Import the Custom Context

1. Download your custom voice model(s) from [Picovoice Console](https://console.picovoice.ai/).
2. Decompress the zip file. The model for Rhino speech to intent is located in two files: A binary `.rhn` file, and as a `.h` header file containing a `C` array version of the binary model.
3. Copy the contents of the array inside the `.h` header file and update the `CONTEXT_ARRAY` values in `params.h`.
