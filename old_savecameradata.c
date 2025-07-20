/*
 * Copyright (c) 2024, BlackBerry Limited.
 * Licensed under the Apache License, Version 2.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <termios.h>
#include <camera/camera_api.h>

// for making folder "frames" to store images
#include <sys/stat.h>
#include <sys/types.h>


#define NUM_CHANNELS (3)

const camera_frametype_t cSupportedFrametypes[] = {
    CAMERA_FRAMETYPE_YCBYCR,
    CAMERA_FRAMETYPE_CBYCRY,
    CAMERA_FRAMETYPE_RGB8888,
    CAMERA_FRAMETYPE_BGR8888,
};
#define NUM_SUPPORTED_FRAMETYPES (sizeof(cSupportedFrametypes) / sizeof(cSupportedFrametypes[0]))

static void listAvailableCameras(void);
static void processCameraData(camera_handle_t handle, camera_buffer_t* buffer, void* arg);
static void blockOnKeyPress(void);

// Global frame counter
static int gFrameCounter = 0;

// Save RGB8888 frame as PPM
void saveFrameAsPPM_RGB8888(const char* filename, uint8_t* buffer, uint32_t width, uint32_t height, uint32_t stride) {
    FILE* f = fopen(filename, "wb");
    if (!f) {
        perror("Failed to open file for writing");
        return;
    }

    fprintf(f, "P6\n%d %d\n255\n", width, height);

    for (uint32_t y = 0; y < height; y++) {
        uint8_t* row = buffer + y * stride;
        for (uint32_t x = 0; x < width; x++) {
            fwrite(&row[x * 4], 1, 3, f);  // Write R, G, B (ignore alpha)
        }
    }

    fclose(f);
}

///
// Save BGR8888 frame as PPM
void saveFrameAsPPM_BGR8888(const char* filename, uint8_t* buffer, uint32_t width, uint32_t height, uint32_t stride) {
    FILE* f = fopen(filename, "wb");
    if (!f) {
        perror("Failed to open file for writing");
        return;
    }

    fprintf(f, "P6\n%d %d\n255\n", width, height);

    for (uint32_t y = 0; y < height; y++) {
        uint8_t* row = buffer + y * stride;
        for (uint32_t x = 0; x < width; x++) {
            fwrite(&row[x * 4], 1, 3, f);  // Write R, G, B (ignore alpha)
        }
    }

    fclose(f);
}

/// 
// Save YCBYCR frame as PPM
void saveFrameAsPPM_YCBYCR(const char* filename, uint8_t* buffer, uint32_t width, uint32_t height, uint32_t stride) {
    FILE* f = fopen(filename, "wb");
    if (!f) {
        perror("Failed to open file for writing");
        return;
    }

    fprintf(f, "P6\n%d %d\n255\n", width, height);

    for (uint32_t y = 0; y < height; y++) {
        uint8_t* row = buffer + y * stride;
        for (uint32_t x = 0; x < width; x++) {
            fwrite(&row[x * 4], 1, 3, f);  // Write R, G, B (ignore alpha)
        }
    }

    fclose(f);
}

///
// Save CBYCRY frame as PPM
void saveFrameAsPPM_CBYCRY(const char* filename, uint8_t* buffer, uint32_t width, uint32_t height, uint32_t stride) {
    FILE* f = fopen(filename, "wb");
    if (!f) {
        perror("Failed to open file for writing");
        return;
    }

    fprintf(f, "P6\n%d %d\n255\n", width, height);

    for (uint32_t y = 0; y < height; y++) {
        uint8_t* row = buffer + y * stride;
        for (uint32_t x = 0; x < width; x++) {
            fwrite(&row[x * 4], 1, 3, f);  // Write R, G, B (ignore alpha)
        }
    }

    fclose(f);
}



// For making a folder to store image frames
void ensureFramesFolderExists(void) {
    struct stat st = {0};
    if (stat("frames", &st) == -1) {
        mkdir("frames", 0777);
    }
}


int main(int argc, char* argv[])
{
    int err;
    int opt;
    camera_unit_t unit = CAMERA_UNIT_NONE;
    camera_handle_t handle = CAMERA_HANDLE_INVALID;
    camera_frametype_t frametype = CAMERA_FRAMETYPE_UNSPECIFIED;

    ensureFramesFolderExists();

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

    if ((unit == CAMERA_UNIT_NONE) || (unit >= CAMERA_UNIT_NUM_UNITS)) {
        listAvailableCameras();
        printf("Please provide camera unit with -u option\n");
        exit(EXIT_SUCCESS);
    }

    err = camera_open(unit, CAMERA_MODE_RO, &handle);
    if ((err != CAMERA_EOK) || (handle == CAMERA_HANDLE_INVALID)) {
        printf("Failed to open CAMERA_UNIT_%d: err = %d\n", (int)unit, err);
        exit(EXIT_FAILURE);
    }

    err = camera_get_vf_property(handle, CAMERA_IMGPROP_FORMAT, &frametype);
    if (err != CAMERA_EOK) {
        printf("Failed to get frametype: err = %d\n", err);
        (void)camera_close(handle);
        exit(EXIT_FAILURE);
    }

    bool unsupported = true;
    for (uint i = 0; i < NUM_SUPPORTED_FRAMETYPES; i++) {
        if (frametype == cSupportedFrametypes[i]) {
            unsupported = false;
            break;
        }
    }

    if (unsupported) {
        printf("Unsupported frametype: %d\n", frametype);
        (void)camera_close(handle);
        exit(EXIT_FAILURE);
    }

    err = camera_start_viewfinder(handle, processCameraData, NULL, NULL);
    if (err != CAMERA_EOK) {
        printf("Failed to start viewfinder: err = %d\n", err);
        (void)camera_close(handle);
        exit(EXIT_FAILURE);
    }

    printf("Capturing frames... Press any key to stop.\n");
    blockOnKeyPress();

    err = camera_stop_viewfinder(handle);
    if (err != CAMERA_EOK) {
        printf("Failed to stop viewfinder: err = %d\n", err);
        (void)camera_close(handle);
        exit(EXIT_FAILURE);
    }

    err = camera_close(handle);
    if (err != CAMERA_EOK) {
        printf("Failed to close camera: err = %d\n", err);
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}

static void listAvailableCameras(void)
{
    int err;
    uint numSupported;
    camera_unit_t* supportedCameras;

    err = camera_get_supported_cameras(0, &numSupported, NULL);
    if (err != CAMERA_EOK) {
        printf("Failed to get camera list: err = %d\n", err);
        return;
    }

    if (numSupported == 0) {
        printf("No supported cameras detected!\n");
        return;
    }

    supportedCameras = (camera_unit_t*)calloc(numSupported, sizeof(camera_unit_t));
    if (!supportedCameras) {
        printf("Memory allocation failed\n");
        return;
    }

    err = camera_get_supported_cameras(numSupported, &numSupported, supportedCameras);
    if (err != CAMERA_EOK) {
        printf("Failed to get camera list: err = %d\n", err);
    } else {
        printf("Available camera units:\n");
        for (uint i = 0; i < numSupported; i++) {
            printf("\tCAMERA_UNIT_%d (use -u %d)\n", supportedCameras[i], supportedCameras[i]);
        }
    }

    free(supportedCameras);
}

static void processCameraData(camera_handle_t handle, camera_buffer_t* buffer, void* arg)
{
    (void)handle;
    (void)arg;

    if (buffer->frametype != CAMERA_FRAMETYPE_RGB8888) {
        printf("\rUnsupported frame type %d\n", buffer->frametype);
        return;
    }

    uint32_t width = buffer->framedesc.rgb8888.width;
    uint32_t height = buffer->framedesc.rgb8888.height;
    uint32_t stride = buffer->framedesc.rgb8888.stride;

    // Generate filename
    char filename[64];
    snprintf(filename, sizeof(filename), "frames/frame_%04d.ppm", gFrameCounter++);

    // Save frame
    saveFrameAsPPM_RGB8888(filename, buffer->framebuf, width, height, stride);
    saveFrameAsPPM_BGR8888(filename, buffer->framebuf, width, height, stride);
    saveFrameAsPPM_YCBYCR(filename, buffer->framebuf, width, height, stride);
    saveFrameAsPPM_CBYCRY(filename, buffer->framebuf, width, height, stride);

    printf("\rSaved %s", filename);
    fflush(stdout);
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
    (void)read(STDIN_FILENO, &key, 1);
    (void)tcsetattr(STDIN_FILENO, TCSANOW, &oldterm);
}
