// ServiceInformJobGet.c

// SPDX-FileCopyrightText: 2022-2023, Yaskawa America, Inc.
// SPDX-FileCopyrightText: 2022-2023, Delft University of Technology
//
// SPDX-License-Identifier: Apache-2.0

#include "MotoROS.h"
#include "ServiceInformJobGet.h"
#include "ServiceInformJobShared.h"

rcl_service_t g_serviceInformJobGet;
ServiceInformJobGet_Messages g_messages_InformJobGet;

void Ros_ServiceInformJobGet_Initialize()
{
    MOTOROS2_MEM_TRACE_START(svc_inform_get_init);

    rcl_ret_t ret;

    const rosidl_service_type_support_t* type_support = ROSIDL_GET_SRV_TYPE_SUPPORT(motoros2_interfaces, srv, InformJobGet);

    ret = rclc_service_init_default(&g_serviceInformJobGet, &g_microRosNodeInfo.node, type_support, SERVICE_NAME_INFORM_JOB_GET);
    motoRosAssert_withMsg(ret == RCL_RET_OK, SUBCODE_FAIL_INIT_SERVICE_INFORM_GET, "Failed to init service (%d)", (int)ret);

    rosidl_runtime_c__String__init(&g_messages_InformJobGet.req.job_name);
    rosidl_runtime_c__String__init(&g_messages_InformJobGet.resp.message);
    rosidl_runtime_c__String__init(&g_messages_InformJobGet.resp.job_content);

    MOTOROS2_MEM_TRACE_REPORT(svc_inform_get_init);
}

void Ros_ServiceInformJobGet_Cleanup()
{
    MOTOROS2_MEM_TRACE_START(svc_inform_get_fini);

    rcl_ret_t ret;

    ret = rcl_service_fini(&g_serviceInformJobGet, &g_microRosNodeInfo.node);
    if (ret != RCL_RET_OK)
        Ros_Debug_BroadcastMsg("Failed cleaning up " SERVICE_NAME_INFORM_JOB_GET " service: %d", ret);

    rosidl_runtime_c__String__fini(&g_messages_InformJobGet.req.job_name);
    rosidl_runtime_c__String__fini(&g_messages_InformJobGet.resp.message);
    rosidl_runtime_c__String__fini(&g_messages_InformJobGet.resp.job_content);

    MOTOROS2_MEM_TRACE_REPORT(svc_inform_get_fini);
}

void Ros_ServiceInformJobGet_Trigger(const void* request_msg, void* response_msg)
{
    motoros2_interfaces__srv__InformJobGet_Request* request = (motoros2_interfaces__srv__InformJobGet_Request*)request_msg;
    motoros2_interfaces__srv__InformJobGet_Response* response = (motoros2_interfaces__srv__InformJobGet_Response*)response_msg;

    response->success = FALSE;

    // Validate job name
    if (strlen(request->job_name.data) == 0 || strlen(request->job_name.data) > 8)
    {
        response->result_code = INFORM_JOB_RESULT_INVALID_JOB_NAME;
        rosidl_runtime_c__String__assign(&response->message, Ros_ServiceInformJob_ResultCodeToString(response->result_code));
        return;
    }

    // Open file
    int fd = mpOpenFile(request->job_name.data, "r");
    if (fd < 0)
    {
        response->result_code = INFORM_JOB_RESULT_JOB_NOT_FOUND;
        rosidl_runtime_c__String__assign(&response->message, Ros_ServiceInformJob_ResultCodeToString(response->result_code));
        return;
    }

    // Read file content
    char buffer[1024]; // Assuming a max job size of 1024 bytes
    int bytes_read = mpRead(fd, buffer, sizeof(buffer));
    mpClose(fd);

    if (bytes_read < 0)
    {
        response->result_code = INFORM_JOB_RESULT_READ_ERROR;
        rosidl_runtime_c__String__assign(&response->message, Ros_ServiceInformJob_ResultCodeToString(response->result_code));
        return;
    }

    buffer[bytes_read] = '\0';

    // Set response
    response->success = TRUE;
    response->result_code = INFORM_JOB_RESULT_OK;
    rosidl_runtime_c__String__assign(&response->message, Ros_ServiceInformJob_ResultCodeToString(response->result_code));
    rosidl_runtime_c__String__assign(&response->job_content, buffer);
}
