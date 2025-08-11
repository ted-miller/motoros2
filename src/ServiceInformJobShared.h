// ServiceInformJobShared.h

// SPDX-FileCopyrightText: 2022-2023, Yaskawa America, Inc.
// SPDX-FileCopyrightText: 2022-2023, Delft University of Technology
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MOTOROS2_SERVICE_INFORM_JOB_SHARED_H
#define MOTOROS2_SERVICE_INFORM_JOB_SHARED_H

typedef enum
{
    INFORM_JOB_RESULT_OK,
    INFORM_JOB_RESULT_JOB_NOT_FOUND,
    INFORM_JOB_RESULT_JOB_ALREADY_EXISTS,
    INFORM_JOB_RESULT_READ_ERROR,
    INFORM_JOB_RESULT_WRITE_ERROR,
    INFORM_JOB_RESULT_DELETE_ERROR,
    INFORM_JOB_RESULT_INVALID_JOB_NAME,
    INFORM_JOB_RESULT_INVALID_JOB_CONTENT,
    INFORM_JOB_RESULT_API_ERROR
} InformJobResultCodes;

const char* const Ros_ServiceInformJob_ResultCodeToString(InformJobResultCodes code);

#endif // MOTOROS2_SERVICE_INFORM_JOB_SHARED_H
