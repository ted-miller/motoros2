// ServiceInformJobShared.c

// SPDX-FileCopyrightText: 2022-2023, Yaskawa America, Inc.
// SPDX-FileCopyrightText: 2022-2023, Delft University of Technology
//
// SPDX-License-Identifier: Apache-2.0

#include "MotoROS.h"
#include "ServiceInformJobShared.h"

const char* const Ros_ServiceInformJob_ResultCodeToString(InformJobResultCodes code)
{
    switch(code)
    {
        case INFORM_JOB_RESULT_OK:
            return "OK";
        case INFORM_JOB_RESULT_JOB_NOT_FOUND:
            return "Job not found";
        case INFORM_JOB_RESULT_JOB_ALREADY_EXISTS:
            return "Job already exists";
        case INFORM_JOB_RESULT_READ_ERROR:
            return "Error reading job file";
        case INFORM_JOB_RESULT_WRITE_ERROR:
            return "Error writing job file";
        case INFORM_JOB_RESULT_DELETE_ERROR:
            return "Error deleting job file";
        case INFORM_JOB_RESULT_INVALID_JOB_NAME:
            return "Invalid job name";
        case INFORM_JOB_RESULT_INVALID_JOB_CONTENT:
            return "Invalid job content";
        case INFORM_JOB_RESULT_API_ERROR:
            return "API error";
        default:
            return "Unknown error";
    }
}
