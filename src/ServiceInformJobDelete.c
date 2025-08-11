// ServiceInformJobDelete.c

// SPDX-FileCopyrightText: 2022-2023, Yaskawa America, Inc.
// SPDX-FileCopyrightText: 2022-2023, Delft University of Technology
//
// SPDX-License-Identifier: Apache-2.0

#include "MotoROS.h"
#include "ServiceInformJobDelete.h"
#include "ServiceInformJobShared.h"

rcl_service_t g_serviceInformJobDelete;
ServiceInformJobDelete_Messages g_messages_InformJobDelete;

void Ros_ServiceInformJobDelete_Initialize()
{
    MOTOROS2_MEM_TRACE_START(svc_inform_delete_init);

    rcl_ret_t ret;

    const rosidl_service_type_support_t* type_support = ROSIDL_GET_SRV_TYPE_SUPPORT(motoros2_interfaces, srv, InformJobDelete);

    ret = rclc_service_init_default(&g_serviceInformJobDelete, &g_microRosNodeInfo.node, type_support, SERVICE_NAME_INFORM_JOB_DELETE);
    motoRosAssert_withMsg(ret == RCL_RET_OK, SUBCODE_FAIL_INIT_SERVICE_INFORM_DELETE, "Failed to init service (%d)", (int)ret);

    rosidl_runtime_c__String__init(&g_messages_InformJobDelete.req.job_name);
    rosidl_runtime_c__String__init(&g_messages_InformJobDelete.resp.message);

    MOTOROS2_MEM_TRACE_REPORT(svc_inform_delete_init);
}

void Ros_ServiceInformJobDelete_Cleanup()
{
    MOTOROS2_MEM_TRACE_START(svc_inform_delete_fini);

    rcl_ret_t ret;

    ret = rcl_service_fini(&g_serviceInformJobDelete, &g_microRosNodeInfo.node);
    if (ret != RCL_RET_OK)
        Ros_Debug_BroadcastMsg("Failed cleaning up " SERVICE_NAME_INFORM_JOB_DELETE " service: %d", ret);

    rosidl_runtime_c__String__fini(&g_messages_InformJobDelete.req.job_name);
    rosidl_runtime_c__String__fini(&g_messages_InformJobDelete.resp.message);

    MOTOROS2_MEM_TRACE_REPORT(svc_inform_delete_fini);
}

void Ros_ServiceInformJobDelete_Trigger(const void* request_msg, void* response_msg)
{
    motoros2_interfaces__srv__InformJobDelete_Request* request = (motoros2_interfaces__srv__InformJobDelete_Request*)request_msg;
    motoros2_interfaces__srv__InformJobDelete_Response* response = (motoros2_interfaces__srv__InformJobDelete_Response*)response_msg;

    response->success = FALSE;

    // Validate job name
    if (strlen(request->job_name.data) == 0 || strlen(request->job_name.data) > 8)
    {
        response->result_code = INFORM_JOB_RESULT_INVALID_JOB_NAME;
        rosidl_runtime_c__String__assign(&response->message, Ros_ServiceInformJob_ResultCodeToString(response->result_code));
        return;
    }

    // Check if file exists before deleting
    int fd = mpOpenFile(request->job_name.data, "r");
    if (fd < 0)
    {
        response->result_code = INFORM_JOB_RESULT_JOB_NOT_FOUND;
        rosidl_runtime_c__String__assign(&response->message, Ros_ServiceInformJob_ResultCodeToString(response->result_code));
        return;
    }
    mpClose(fd);

    // Delete file
    if (mpDeleteFile(request->job_name.data) != 0)
    {
        response->result_code = INFORM_JOB_RESULT_DELETE_ERROR;
        rosidl_runtime_c__String__assign(&response->message, Ros_ServiceInformJob_ResultCodeToString(response->result_code));
        return;
    }

    // Set response
    response->success = TRUE;
    response->result_code = INFORM_JOB_RESULT_OK;
    rosidl_runtime_c__String__assign(&response->message, Ros_ServiceInformJob_ResultCodeToString(response->result_code));
}
