// ServiceInformJobGet.h

// SPDX-FileCopyrightText: 2022-2023, Yaskawa America, Inc.
// SPDX-FileCopyrightText: 2022-2023, Delft University of Technology
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MOTOROS2_SERVICE_INFORM_JOB_GET_H
#define MOTOROS2_SERVICE_INFORM_JOB_GET_H

extern rcl_service_t g_serviceInformJobGet;

typedef struct
{
    motoros2_interfaces__srv__InformJobGet_Request req;
    motoros2_interfaces__srv__InformJobGet_Response resp;
} ServiceInformJobGet_Messages;

extern ServiceInformJobGet_Messages g_messages_InformJobGet;

void Ros_ServiceInformJobGet_Initialize();
void Ros_ServiceInformJobGet_Cleanup();
void Ros_ServiceInformJobGet_Trigger(const void* request_msg, void* response_msg);

#endif // MOTOROS2_SERVICE_INFORM_JOB_GET_H
