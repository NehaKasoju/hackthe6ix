/*
 * Copyright (c) 2024, BlackBerry Limited. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <termios.h>

#include <camera/camera_api.h>

/**
 * @brief Number of channels for supported frametypes
 */
#define NUM_CHANNELS (3)

/**
 * @brief List of frametypes that @c processCameraData can operate on
 */
const camera_frametype_t cSupportedFrametypes[] = {
    CAMERA_FRAMETYPE_YCBYCR,
    CAMERA_FRAMETYPE_CBYCRY,
    CAMERA_FRAMETYPE_RGB8888,
    CAMERA_FRAMETYPE_BGR8888,
};
#define NUM_SUPPORTED_FRAMETYPES (sizeof(cSupportedFrametypes) / sizeof(cSupportedFrametypes[0]))

/**
 * @brief Prints a list of available cameras
 */
static void listAvailableCameras(void);

/**
 * @brief Callback function to be called when camera data is available
 *
 * @param handle Handle to the camera providing the data
 * @param buffer Buffer of camera data
 * @param arg Argument provided when starting streaming
 */
static void processCameraData(camera_handle_t handle, camera_buffer_t* buffer, void* arg);

/**
 * @brief Blocks until the user presses any key
 */
static void blockOnKeyPress(void);

int main(int argc, char* argv[])
{
    int err;
    int opt;
    camera_unit_t unit = CAMERA_UNIT_NONE;
    camera_handle_t handle = CAMERA_HANDLE_INVALID;
    camera_frametype_t frametype = CAMERA_FRAMETYPE_UNSPECIFIED;

    // Read command line options
    while ((opt = getopt(argc, argv, "u:")) != -1 || (optind < argc)) {
        switch (opt) {
        case 'u':
            unit = (camera_unit_t)strtol(optarg, NULL, 10);
            break;
        default:
            printf("Ignoring unrecognized option: %s\n", optarg);
            break;
        }
    }

    // If no camera unit has been specified, list the options and exit
    if ((unit == CAMERA_UNIT_NONE) || (unit >= CAMERA_UNIT_NUM_UNITS)) {
        listAvailableCameras();
        printf("Please provide camera unit with -u option\n");
        exit(EXIT_SUCCESS);
    }

    // Open a read-only handle for the specified camera unit.
    // CAMERA_MODE_RO doesn't give us access to change camera configuration
    // and we can't modify the memory in a provided buffer.
    err = camera_open(unit, CAMERA_MODE_RO, &handle);
    if ((err != CAMERA_EOK) || (handle == CAMERA_HANDLE_INVALID)) {
        printf("Failed to open CAMERA_UNIT_%d: err = %d\n", (int)unit, err);
        exit(EXIT_FAILURE);
    }

    // Make sure that this camera defaults to a supported frametype
    err = camera_get_vf_property(handle, CAMERA_IMGPROP_FORMAT, &frametype);
    if (err != CAMERA_EOK) {
        printf("Failed to get frametype for CAMERA_UNIT_%d: err = %d\n", (int)unit, err);
        (void)camera_close(handle);
        exit(EXIT_FAILURE);
    }
    bool unsupportedFrametype = true;
    for (uint i = 0; i < NUM_SUPPORTED_FRAMETYPES; i++) {
        if (frametype == cSupportedFrametypes[i]) {
            unsupportedFrametype = false;
            break;
        }
    }
    if (unsupportedFrametype) {
        printf("Camera frametype %d is not supported\n", (int)frametype);
        (void)camera_close(handle);
        exit(EXIT_FAILURE);
    }
    printf("\n");

    // Start the camera streaming: callbacks will start being received
    err = camera_start_viewfinder(handle, processCameraData, NULL, NULL);
    if (err != CAMERA_EOK) {
        printf("Failed to start CAMERA_UNIT_%d: err = %d\n", (int)unit, err);
        (void)camera_close(handle);
        exit(EXIT_FAILURE);
    }

    blockOnKeyPress();

    // Stop the camera streaming: no more callbacks will be received
    err = camera_stop_viewfinder(handle);
    printf("\r\n");
    if (err != CAMERA_EOK) {
        printf("Failed to stop CAMERA_UNIT_%d: err = %d\n", (int)unit, err);
        (void)camera_close(handle);
        exit(EXIT_FAILURE);
    }

    // Close the camera handle
    err = camera_close(handle);
    if (err != CAMERA_EOK) {
        printf("Failed to close CAMERA_UNIT_%d: err = %d\n", (int)unit, err);
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}

static void listAvailableCameras(void)
{
    int err;
    uint numSupported;
    camera_unit_t* supportedCameras;

    // Determine how many cameras are supported
    err = camera_get_supported_cameras(0, &numSupported, NULL);
    if (err != CAMERA_EOK) {
        printf("Failed to get number of supported cameras: err = %d\n", err);
        return;
    }

    if (numSupported == 0) {
        printf("No supported cameras detected!\n");
        return;
    }

    // Allocate an array big enough to hold all camera units
    supportedCameras = (camera_unit_t*)calloc(numSupported, sizeof(camera_unit_t));
    if (supportedCameras == NULL) {
        printf("Failed to allocate memory for supported cameras\n");
        return;
    }

    // Get the list of supported cameras
    err = camera_get_supported_cameras(numSupported, &numSupported, supportedCameras);
    if (err != CAMERA_EOK) {
        printf("Failed to get list of supported cameras: err = %d\n", err);
    } else {
        printf("Available camera units:\n");
        for (uint i = 0; i < numSupported; i++) {
            printf("\tCAMERA_UNIT_%d", supportedCameras[i]);
            printf(" (specify -u %d)\n", supportedCameras[i]);
        }
    }

    free(supportedCameras);
    return;
}

static void processCameraData(camera_handle_t handle, camera_buffer_t* buffer, void* arg)
{
    clock_t begin;
    clock_t end;
    double channelAverage[NUM_CHANNELS];

    // No need for handle or argument data
    (void)handle;
    (void)arg;

    // Camera data is buffer->framebuf and described by buffer->framedesc.
    // As an example, let's compute channel averages by iterating over the
    // bytes in each line and determining which channel the byte belongs to.
    begin = clock();
    memset(channelAverage, 0x0, sizeof(channelAverage));
    switch (buffer->frametype) {
    case CAMERA_FRAMETYPE_RGB8888:
    {
        // Channel averages ordering: R, G, B
        uint32_t width = buffer->framedesc.rgb8888.width;
        uint32_t height = buffer->framedesc.rgb8888.height;
        uint32_t stride = buffer->framedesc.rgb8888.stride;
        for (uint y = 0; y < height; y++) {
            uint8_t* linePointer = buffer->framebuf + y * stride;
            for (uint i = 0; i < 4 * width; i++) {
                uint chan = i % 4;
                if (chan == 3) {
                    continue;
                }
                channelAverage[chan] += (double)*(linePointer + i);
            }
        }
        for (uint chan = 0; chan < NUM_CHANNELS; chan++) {
            channelAverage[chan] /= (double)(width * height);
        }
        break;
    }
    case CAMERA_FRAMETYPE_BGR8888:
    {
        // Channel averages ordering: R, G, B
        uint32_t width = buffer->framedesc.bgr8888.width;
        uint32_t height = buffer->framedesc.bgr8888.height;
        uint32_t stride = buffer->framedesc.bgr8888.stride;
        for (uint y = 0; y < height; y++) {
            uint8_t* linePointer = buffer->framebuf + y * stride;
            for (uint i = 0; i < 4 * width; i++) {
                if (i % 4 == 3) {
                    continue;
                }
                uint chan = (4 * width - i + 3) % 4 - 1;
                channelAverage[chan] += (double)*(linePointer + i);
            }
        }
        for (uint chan = 0; chan < NUM_CHANNELS; chan++) {
            channelAverage[chan] /= (double)(width * height);
        }
        break;
    }
    case CAMERA_FRAMETYPE_YCBYCR:
    {
        // Channel averages ordering: Y, Cb, Cr
        uint32_t width = buffer->framedesc.ycbycr.width;
        uint32_t height = buffer->framedesc.ycbycr.height;
        uint32_t stride = buffer->framedesc.ycbycr.stride;
        for (uint y = 0; y < height; y++) {
            uint8_t* linePointer = buffer->framebuf + y * stride;
            for (uint i = 0; i < 2 * width; i++) {
                uint chan = i % 2;
                if (chan == 1) {
                    chan = (i - 1) % 4 / 2 + 1;
                }
                channelAverage[chan] += (double)*(linePointer + i);
            }
        }
        channelAverage[0] /= (double)(width * height);
        channelAverage[1] /= (double)(width / 2 * height);
        channelAverage[2] /= (double)(width / 2 * height);
        break;
    }
    case CAMERA_FRAMETYPE_CBYCRY:
    {
        // Channel averages ordering: Y, Cb, Cr
        uint32_t width = buffer->framedesc.cbycry.width;
        uint32_t height = buffer->framedesc.cbycry.height;
        uint32_t stride = buffer->framedesc.cbycry.stride;
        for (uint y = 0; y < height; y++) {
            uint8_t* linePointer = buffer->framebuf + y * stride;
            for (uint i = 0; i < 2 * width; i++) {
                uint chan = (i + 1) % 2;
                if (chan == 1) {
                    chan = ((i + 1) % 4 + 1) / 2;
                }
                channelAverage[chan] += (double)*(linePointer + i);
            }
        }
        channelAverage[0] /= (double)(width * height);
        channelAverage[1] /= (double)(width / 2 * height);
        channelAverage[2] /= (double)(width / 2 * height);
        break;
    }
    default:
        printf("\r");
        printf("Frametype %d is not suppported!", (int)buffer->frametype);
        printf(" (press any key to stop example)");
        fflush(stdout);
        return;
    }
    end = clock();

    printf("\r");
    printf("Channel averages: ");
    printf("%.3f, %.3f, %.3f", channelAverage[0], channelAverage[1], channelAverage[2]);
    printf(" took %.3f ms", (double)(end - begin) / CLOCKS_PER_SEC * 1000);
    printf(" (press any key to stop example)     ");
    fflush(stdout);

    return;
}

static void blockOnKeyPress(void)
{
    struct termios oldterm;
    struct termios newterm;
    char key;

    (void)tcgetattr(STDIN_FILENO, &oldterm);
    newterm = oldterm;
    newterm.c_lflag &= ~(ECHO | ICANON);
    (void)tcsetattr(STDIN_FILENO, TCSANOW, &newterm);
    // Blocking call: wait for 1 byte of data to become available
    (void)read(STDIN_FILENO, &key, 1);
    (void)tcsetattr(STDIN_FILENO, TCSANOW, &oldterm);

    return;
}
