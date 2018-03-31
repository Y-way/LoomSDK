/*
 * ===========================================================================
 * Loom SDK
 * Copyright 2011, 2012, 2013
 * The Game Engine Company, LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ===========================================================================
 */

#ifndef _LOOM_COMMON_PLATFORM_PLATFORMMOBILE_H_
#define _LOOM_COMMON_PLATFORM_PLATFORMMOBILE_H_

/**
 * Loom Mobile API
 *
 * For mobile specific functionality, Loom includes a cross-platform native API. This
 * abstraction handles various functionality on mobile devices, such as Vibration, etc..
 *
 */

///Callback for sensor changed API events.
typedef void (*SensorTripleChangedCallback)(int sensor, float x, float y, float z);

///Callback for custom URL opening event
typedef void (*OpenedViaCustomURLCallback)();

///Callback for remote notification opening event
typedef void (*OpenedViaRemoteNotificationCallback)();

///initializes the data for the Mobile class for this platform
void platform_mobileInitialize(SensorTripleChangedCallback sensorTripleChangedCB, 
                                OpenedViaCustomURLCallback customURLCB,
                                OpenedViaRemoteNotificationCallback remoteNotificationCB);

///tells the device to do a short vibration, if supported by the hardware
void platform_vibrate();

///sets whether or not to use the system screen sleep timeout
void platform_allowScreenSleep(bool sleep);

///enables location tracking for this device
void platform_startLocationTracking(int minDist, int minTime);

///disables location tracking for this device
void platform_stopLocationTracking();

///returns the device's location using GPS and/or NETWORK signals
const char *platform_getLocation();

///shares the specfied text via other applications on the device (ie. Twitter, Facebook)
bool platform_shareText(const char *subject, const char *text);

///returns if the application was launched via a Custom URL Scheme
bool platform_wasOpenedViaCustomURL();

///returns if the application was launched via a Remote Notification
bool platform_wasOpenedViaRemoteNotification();

///gets the the specified query key data from any custom scheme URL path that the application was launched with, or "" if not found
const char *platform_getOpenURLQueryData(const char *queryKey);

///sets the received open URL query and parses it for later usage
void platform_setOpenURLQueryData(const char *queryStr);

///gets the the data associated with the specified key from any potential custom payload attached to a
///Remote Notification that the application was launched with, or "" if not found
const char *platform_getRemoteNotificationData(const char *key);

///checks if a given sensor is supported on this hardware
bool platform_isSensorSupported(int sensor);

///checks if a given sensor is currently enabled
bool platform_isSensorEnabled(int sensor);

///checks if a given sensor has received any data yet
bool platform_hasSensorReceivedData(int sensor);

///enables the given sensor
bool platform_enableSensor(int sensor);

///disables the given sensor
void platform_disableSensor(int sensor);

///retrieves the current device rotation angles for the device based on an enabled Rotation sensor
void platform_getDeviceRotationAngles(float *rot);

///checks if Dolby Audio is supported on this platform
bool platform_isDolbyAudioSupported();

///sets the Dolby Audio processing state
void platform_setDolbyAudioProcessingEnabled(bool enabled);

///checks if Dolby Audio processing is currently enabled
bool platform_isDolbyAudioProcessingEnabled();

///checks if the specified Dolby Audio processing profile is supported on this hardware
bool platform_isDolbyAudioProcessingProfileSupported(const char *profile);

///sets the Dolby Audio processing profile to use
bool platform_setDolbyAudioProcessingProfile(const char *profile);

///gets the currently in use Dolby Audio processing profile
const char *platform_getSelectedDolbyAudioProfile();

#endif
