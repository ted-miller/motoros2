// ServiceInformJobDelete.h

// SPDX-FileCopyrightText: 2022-2023, Yaskawa America, Inc.
// SPDX-FileCopyrightText: 2022-2023, Delft University of Technology
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MOTOROS2_SERVICE_INFORM_JOB_DELETE_H
#define MOTOROS2_SERVICE_INFORM_JOB_DELETE_H

extern rcl_service_t g_serviceInformJobDelete;

typedef struct
{
    motoros2_interfaces__srv__InformJobDelete_Request req;
    motoros2_interfaces__srv__InformJobDelete_Response resp;
} ServiceInformJobDelete_Messages;

extern ServiceInformJobDelete_Messages g_messages_InformJobDelete;

void Ros_ServiceInformJobDelete_Initialize();
void Ros_ServiceInformJobDelete_Cleanup();
void Ros_ServiceInformJobDelete_Trigger(const void* request_msg, void* response_msg);

#endif // MOTOROS2_SERVICE_INFORM_JOB_DELETE_H
