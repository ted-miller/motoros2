// ServiceInformJobPut.c

// SPDX-FileCopyrightText: 2022-2023, Yaskawa America, Inc.
// SPDX-FileCopyrightText: 2022-2023, Delft University of Technology
//
// SPDX-License-Identifier: Apache-2.0

#include "MotoROS.h"
#include "ServiceInformJobPut.h"
#include "ServiceInformJobShared.h"

rcl_service_t g_serviceInformJobPut;
ServiceInformJobPut_Messages g_messages_InformJobPut;

void Ros_ServiceInformJobPut_Initialize()
{
    MOTOROS2_MEM_TRACE_START(svc_inform_put_init);

    rcl_ret_t ret;

    const rosidl_service_type_support_t* type_support = ROSIDL_GET_SRV_TYPE_SUPPORT(motoros2_interfaces, srv, InformJobPut);

    ret = rclc_service_init_default(&g_serviceInformJobPut, &g_microRosNodeInfo.node, type_support, SERVICE_NAME_INFORM_JOB_PUT);
    motoRosAssert_withMsg(ret == RCL_RET_OK, SUBCODE_FAIL_INIT_SERVICE_INFORM_PUT, "Failed to init service (%d)", (int)ret);

    rosidl_runtime_c__String__init(&g_messages_InformJobPut.req.job_name);
    rosidl_runtime_c__String__init(&g_messages_InformJobPut.req.job_content);
    rosidl_runtime_c__String__init(&g_messages_InformJobPut.resp.message);

    MOTOROS2_MEM_TRACE_REPORT(svc_inform_put_init);
}

void Ros_ServiceInformJobPut_Cleanup()
{
    MOTOROS2_MEM_TRACE_START(svc_inform_put_fini);

    rcl_ret_t ret;

    ret = rcl_service_fini(&g_serviceInformJobPut, &g_microRosNodeInfo.node);
    if (ret != RCL_RET_OK)
        Ros_Debug_BroadcastMsg("Failed cleaning up " SERVICE_NAME_INFORM_JOB_PUT " service: %d", ret);

    rosidl_runtime_c__String__fini(&g_messages_InformJobPut.req.job_name);
    rosidl_runtime_c__String__fini(&g_messages_InformJobPut.req.job_content);
    rosidl_runtime_c__String__fini(&g_messages_InformJobPut.resp.message);

    MOTOROS2_MEM_TRACE_REPORT(svc_inform_put_fini);
}

void Ros_ServiceInformJobPut_Trigger(const void* request_msg, void* response_msg)
{
    motoros2_interfaces__srv__InformJobPut_Request* request = (motoros2_interfaces__srv__InformJobPut_Request*)request_msg;
    motoros2_interfaces__srv__InformJobPut_Response* response = (motoros2_interfaces__srv__InformJobPut_Response*)response_msg;

    response->success = FALSE;

    // Validate job name
    if (strlen(request->job_name.data) == 0 || strlen(request->job_name.data) > 8)
    {
        response->result_code = INFORM_JOB_RESULT_INVALID_JOB_NAME;
        rosidl_runtime_c__String__assign(&response->message, Ros_ServiceInformJob_ResultCodeToString(response->result_code));
        return;
    }

    // Open file for writing (creates if not exists, overwrites if it does)
    int fd = mpOpenFile(request->job_name.data, "w");
    if (fd < 0)
    {
        response->result_code = INFORM_JOB_RESULT_WRITE_ERROR;
        rosidl_runtime_c__String__assign(&response->message, Ros_ServiceInformJob_ResultCodeToString(response->result_code));
        return;
    }

    // Write content to file
    int bytes_written = mpWrite(fd, request->job_content.data, request->job_content.size);
    mpClose(fd);

    if (bytes_written != request->job_content.size)
    {
        response->result_code = INFORM_JOB_RESULT_WRITE_ERROR;
        rosidl_runtime_c__String__assign(&response->message, Ros_ServiceInformJob_ResultCodeToString(response->result_code));
        // try to delete the partial file
        mpDeleteFile(request->job_name.data);
        return;
    }

    // Set response
    response->success = TRUE;
    response->result_code = INFORM_JOB_RESULT_OK;
    rosidl_runtime_c__String__assign(&response->message, Ros_ServiceInformJob_ResultCodeToString(response->result_code));
}
