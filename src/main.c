//mpMain.c

// SPDX-FileCopyrightText: 2022-2023, Yaskawa America, Inc.
// SPDX-FileCopyrightText: 2022-2023, Delft University of Technology
//
// SPDX-License-Identifier: Apache-2.0

#include "MotoROS.h"

#if !(defined (DX200) || defined (YRC1000) || defined (YRC1000u))
#error MotoROS2 is only supported on DX2 and YRC1 generation controllers
#endif

void RosInitTask();

//Main entrypoint, called by M+ runtime
// cppcheck-suppress unusedFunction
void mpUsrRoot(int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10)
{
    //Creates and starts a new task in a seperate thread of execution.
    //All arguments will be passed to the new task if the function
    //prototype will accept them.
    int RosInitTaskID = mpCreateTask(MP_PRI_TIME_NORMAL, MP_STACK_SIZE, (FUNCPTR)RosInitTask,
        arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10);
    if (RosInitTaskID == ERROR)
        mpSetAlarm(ALARM_TASK_CREATE_FAIL, APPLICATION_NAME " FAILED TO CREATE TASK", SUBCODE_INITIALIZATION);

    //Ends the initialization task.
    mpExitUsrRoot;
}

void Ros_Sleep(float milliseconds)
{
    mpTaskDelay(milliseconds / mpGetRtc()); //Tick length varies between controller models
}

//Report version info to display on pendant
void Ros_ReportVersionInfoToController()
{
    MP_APPINFO_SEND_DATA appInfoSendData;
    MP_STD_RSP_DATA stdResponseData;

    snprintf(appInfoSendData.AppName, MP_MAX_APP_NAME,
        "%s (%s)", APPLICATION_NAME, MOTOPLUS_LIBMICROROS_ROS2_CODENAME);
    snprintf(appInfoSendData.Version, MP_MAX_APP_VERSION,
        "%s", APPLICATION_VERSION);
    snprintf(appInfoSendData.Comment, MP_MAX_APP_COMMENT,
        "%s", "micro-ROS based ROS 2 interface");
    mpApplicationInfoNotify(&appInfoSendData, &stdResponseData); //don't care about return value
}

void RosInitTask()
{
    g_Ros_Controller.tidIncMoveThread = INVALID_TASK;

    Ros_ConfigFile_SetAllDefaultValues();

    //Check to see if another version of MotoROS2.out is running on this controller.
    motoRosAssert_withMsg(!Ros_IsOtherInstanceRunning(), SUBCODE_MULTIPLE_INSTANCES_DETECTED, "MotoROS2 - Multiple Instances");

    // init debug broadcast
    Ros_Debug_BroadcastMsg("---");
    Ros_Debug_BroadcastMsg(APPLICATION_NAME " %s - boot", APPLICATION_VERSION);
    Ros_Debug_BroadcastMsg("M+ libmicroros version: '%s'", MOTOPLUS_LIBMICROROS_VERSION_STR);
    Ros_Debug_BroadcastMsg("PlatformLib version: %u.%u.%u", MOTOROS_PLATFORM_LIB_MAJOR,
        MOTOROS_PLATFORM_LIB_MINOR, MOTOROS_PLATFORM_LIB_PATCH);

    //useful to see on debug-monitor
    Ros_Debug_LogToConsole("%s - boot", APPLICATION_VERSION);

#ifdef MOTOROS2_TESTING_ENABLE
    Ros_Debug_BroadcastMsg("===");
    MOTOROS2_MEM_TRACE_START(testing)
    Ros_Debug_BroadcastMsg("Performing unit tests");
    BOOL bTestResult = TRUE;
    bTestResult &= Ros_Testing_CtrlGroup();
    bTestResult &= Ros_Testing_RosMotoPlusConversionUtils();
    bTestResult &= Ros_Testing_ControllerStatusIO();
    bTestResult &= Ros_Testing_ActionServer_FJT();
    bTestResult &= Ros_Testing_TimeConversionUtils();
    bTestResult ? Ros_Debug_BroadcastMsg("Testing SUCCESSFUL") : Ros_Debug_BroadcastMsg("!!! Testing FAILED !!!");
    MOTOROS2_MEM_TRACE_REPORT(testing)
    Ros_Debug_BroadcastMsg("===");
#endif

    Ros_ConfigFile_Parse();

    Ros_ReportVersionInfoToController();

    //==================================
    Ros_Controller_SetIOState(IO_FEEDBACK_WAITING_MP_INCMOVE, FALSE);
    Ros_Controller_SetIOState(IO_FEEDBACK_MP_INCMOVE_DONE, FALSE);
    Ros_Controller_SetIOState(IO_FEEDBACK_INITIALIZATION_DONE, FALSE);
    Ros_Controller_SetIOState(IO_FEEDBACK_CONTROLLERRUNNING, TRUE);
    Ros_Controller_SetIOState(IO_FEEDBACK_AGENTCONNECTED, FALSE);

    Ros_Controller_SetIOState(IO_FEEDBACK_FAILURE, FALSE);

    Ros_Controller_SetIOState(IO_FEEDBACK_RESERVED_1, FALSE);
    Ros_Controller_SetIOState(IO_FEEDBACK_RESERVED_2, FALSE);
    Ros_Controller_SetIOState(IO_FEEDBACK_RESERVED_3, FALSE);
    Ros_Controller_SetIOState(IO_FEEDBACK_RESERVED_4, FALSE);
    Ros_Controller_SetIOState(IO_FEEDBACK_RESERVED_5, FALSE);
    Ros_Controller_SetIOState(IO_FEEDBACK_RESERVED_6, FALSE);
    Ros_Controller_SetIOState(IO_FEEDBACK_RESERVED_7, FALSE);
    Ros_Controller_SetIOState(IO_FEEDBACK_RESERVED_8, FALSE);

    //==================================
    FOREVER
    {
        MOTOROS2_MEM_TRACE_START(full_connection_cycle);
        Ros_Controller_StatusInit();

        Ros_Allocation_Initialize(&g_motoros2_Allocator);

        Ros_mpGetRobotCalibrationData_Initialize(); //must occur before Ros_Controller_Initialize

        Ros_Communication_ConnectToAgent();

        Ros_Controller_SetIOState(IO_FEEDBACK_AGENTCONNECTED, TRUE);

        Ros_Communication_Initialize();

        // non-recoverable if this fails
        motoRosAssert(Ros_Controller_Initialize(), SUBCODE_FAIL_ROS_CONTROLLER_INIT);

        Ros_InformChecker_ValidateJob();

        Ros_PositionMonitor_Initialize();
        Ros_ActionServer_FJT_Initialize(); //initialize action server - FollowJointTrajectory

        Ros_ServiceQueueTrajPoint_Initialize();
        Ros_ServiceReadWriteIO_Initialize();
        Ros_ServiceResetError_Initialize();
        Ros_ServiceStartTrajMode_Initialize();
        Ros_ServiceStartPointQueueMode_Initialize();
        Ros_ServiceStartRtMode_Initialize();
        Ros_ServiceStopTrajMode_Initialize();
        Ros_ServiceSelectMotionTool_Initialize();

        // Start executor that performs all communication
        // (This task deletes itself when the agent disconnects.)
        SEM_ID semCommunicationExecutorStatus = mpSemBCreate(SEM_Q_FIFO, SEM_FULL);
        int tid = mpCreateTask(MP_PRI_TIME_NORMAL, MP_STACK_SIZE,
                                (FUNCPTR)Ros_Communication_StartExecutors,
                                (int)semCommunicationExecutorStatus, 0, 0, 0, 0, 0, 0, 0, 0, 0);

        if (tid == ERROR)
            mpSetAlarm(ALARM_TASK_CREATE_FAIL, APPLICATION_NAME " FAILED TO CREATE TASK", SUBCODE_EXECUTOR);

        Ros_Debug_BroadcastMsg("Initialization complete.");

        //==================================
        ULONG tickBefore = 0;

        while(g_Ros_Communication_AgentIsConnected)
        {
            //figure out how long to sleep to achieve the user-configured rate
            ULONG tickNow = tickGet();

            ULONG tickDiff = 0;
            if (tickNow > tickBefore)
                tickDiff = tickNow - tickBefore;
            else //unsigned rollover
                tickDiff = (ULONG_MAX - tickBefore) + tickNow;

            float elapsedMs = tickDiff * mpGetRtc(); //time it took to read and publish data

            if (elapsedMs < g_nodeConfigSettings.controller_status_monitor_period)
                Ros_Sleep(g_nodeConfigSettings.controller_status_monitor_period - elapsedMs);
            else
                Ros_Sleep(1); //don't hog the CPU from other tasks

            tickBefore = tickGet();

            //Check controller status.
            //This is being done on an independent thread (as opposed to being refreshed on-demand)
            //so that the motion thread can react as needed.
            if (!Ros_Controller_IoStatusUpdate())
            {
                Ros_Debug_BroadcastMsg("main: IoStatusUpdate failed, forcing disconnect/shutdown");
                // force a 'disconnection', so tasks can start shutting down
                g_Ros_Communication_AgentIsConnected = FALSE;
                // now assert
                motoRosAssert(FALSE, SUBCODE_FAIL_IO_STATUS_UPDATE);
            }

            //Update robot's feedback position and publish the topics
            Ros_PositionMonitor_UpdateLocation();
        }

        //==================================
        Ros_Controller_SetIOState(IO_FEEDBACK_AGENTCONNECTED, FALSE);
        Ros_Debug_BroadcastMsg("Micro-ROS PC Agent disconnected");
        //Also print to console, for easier debugging (but only if not logging to stdout already)
        if (!g_nodeConfigSettings.log_to_stdout)
        {
            Ros_Debug_LogToConsole("Micro-ROS PC Agent disconnected");
        }

        Ros_Debug_BroadcastMsg("Waiting for motion to stop before releasing memory");
        do
        {
            Ros_Sleep(1000);
        } while (Ros_Controller_IsInMotion()); //wait for motion to complete before terminating tasks

        //wait for Ros_Communication_StartExecutors to finish
        mpSemTake(semCommunicationExecutorStatus, WAIT_FOREVER);
        mpSemDelete(semCommunicationExecutorStatus);

        Ros_ServiceSelectMotionTool_Cleanup();
        Ros_ServiceStopTrajMode_Cleanup();
        Ros_ServiceStartTrajMode_Cleanup();
        Ros_ServiceStartPointQueueMode_Cleanup();
        Ros_ServiceStartRtMode_Cleanup();
        Ros_ServiceResetError_Cleanup();
        Ros_ServiceReadWriteIO_Cleanup();
        Ros_ServiceQueueTrajPoint_Cleanup();

        Ros_ActionServer_FJT_Cleanup();
        Ros_PositionMonitor_Cleanup();
        Ros_Controller_Cleanup();
        Ros_Communication_Cleanup(); 
        Ros_mpGetRobotCalibrationData_Cleanup();

        //--------------------------------
        Ros_Controller_SetIOState(IO_FEEDBACK_INITIALIZATION_DONE, FALSE);

        Ros_Sleep(2500);

        Ros_Debug_BroadcastMsg("Shutdown complete. Available memory: (%d) bytes", mpNumBytesFree());
        MOTOROS2_MEM_TRACE_REPORT(full_connection_cycle);
    }
}

/*-
 * Copyright (c) 2014 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. [rescinded 22 July 1999]
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
INT64 strtoll(const char* nptr, char** endptr, int base)
{
    register const char* s = nptr;
    register UINT64 acc;
    register int c;
    register UINT64 cutoff;
    register int neg = 0, any, cutlim;
    /*
     * Skip white space and pick up leading +/- sign if any.
     * If base is 0, allow 0x for hex and 0 for octal, else
     * assume decimal; if base is already 16, allow 0x.
     */
    do {
        c = *s++;
    } while (isspace(c));
    if (c == '-') {
        neg = 1;
        c = *s++;
    }
    else if (c == '+')
        c = *s++;
    if ((base == 0 || base == 16) &&
        c == '0' && (*s == 'x' || *s == 'X')) {
        c = s[1];
        s += 2;
        base = 16;
    }
    if (base == 0)
        base = c == '0' ? 8 : 10;
    /*
     * Compute the cutoff value between legal numbers and illegal
     * numbers.  That is the largest legal value, divided by the
     * base.  An input number that is greater than this value, if
     * followed by a legal input character, is too big.  One that
     * is equal to this value may be valid or not; the limit
     * between valid and invalid numbers is then based on the last
     * digit.  For instance, if the range for longs is
     * [-2147483648..2147483647] and the input base is 10,
     * cutoff will be set to 214748364 and cutlim to either
     * 7 (neg==0) or 8 (neg==1), meaning that if we have accumulated
     * a value > 214748364, or equal but the next digit is > 7 (or 8),
     * the number is too big, and we will return a range error.
     *
     * Set any if any `digits' consumed; make it negative to indicate
     * overflow.
     */
    cutoff = neg ? -(UINT64)LONG_LONG_MIN : LONG_LONG_MAX;
    cutlim = cutoff % (UINT64)base;
    cutoff /= (UINT64)base;
    for (acc = 0, any = 0;; c = *s++) {
        if (isdigit(c))
            c -= '0';
        else if (isalpha(c))
            c -= isupper(c) ? 'A' - 10 : 'a' - 10;
        else
            break;
        if (c >= base)
            break;
        if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
            any = -1;
        else {
            any = 1;
            acc *= base;
            acc += c;
        }
    }
    if (any < 0) {
        acc = neg ? LONG_LONG_MIN : LONG_LONG_MAX;
        errno = ERANGE;
    }
    else if (neg)
        acc = -acc;
    if (endptr != 0)
        *endptr = (char*)(any ? s - 1 : nptr);
    return (acc);
}
