// ServiceInformJobPut.h

// SPDX-FileCopyrightText: 2022-2023, Yaskawa America, Inc.
// SPDX-FileCopyrightText: 2022-2023, Delft University of Technology
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MOTOROS2_SERVICE_INFORM_JOB_PUT_H
#define MOTOROS2_SERVICE_INFORM_JOB_PUT_H

extern rcl_service_t g_serviceInformJobPut;

typedef struct
{
    motoros2_interfaces__srv__InformJobPut_Request req;
    motoros2_interfaces__srv__InformJobPut_Response resp;
} ServiceInformJobPut_Messages;

extern ServiceInformJobPut_Messages g_messages_InformJobPut;

void Ros_ServiceInformJobPut_Initialize();
void Ros_ServiceInformJobPut_Cleanup();
void Ros_ServiceInformJobPut_Trigger(const void* request_msg, void* response_msg);

#endif // MOTOROS2_SERVICE_INFORM_JOB_PUT_H
