//UWI_ProcessInfo.h
//

#ifndef UWIPROCESSINFO_H
#define UWIPROCESSINFO_H

#define PACKED __attribute__ ((__packed__)) 

/// <summary>
/// The maximum number of entries available in the process table.
/// </summary>
#define PROCESS_TABLE_SIZE (16)

/// <summary>
/// THe maximum number of weld control parameters for any one process.
/// </summary>
#define NUM_WELDCONTROL_PARAMS 6

/// <summary>
/// The maximum number of available groups during filtering of the process database.
/// </summary>
#define PROCESS_FILTER_GROUPS (4)
/// <summary>
/// Denotes any filter with this ID value as invalid.
/// </summary>
#define PROCESS_FILTER_INVALID_ID (0)
/// <summary>
/// The maximum number of available filters for a single filter group.
/// </summary>
#define PROCESS_FILTER_MAX_PER_GROUP (40)

/// <summary>
/// Denotes any process with this ID as invalid or unconfigured.
/// </summary>
#define PROCESS_TABLE_INVALID_ID (0xFFFFFFFF)
/// <summary>
/// Bit mask used to determine if a process ID value indicates the process is using Job Mode.
/// </summary>
#define PROCESS_ID_JOB_FLAG	(0x80000000)
/// <summary>
/// The minimum ID number for a process. Values of zero and below are considered invalid.
/// </summary>
#define PROCESS_TABLE_MIN_ID (1)

/// <summary>
/// UWI_WELDPROC_INFO Length of label
/// </summary>
#define	UWI_WELD_PROC_LABEL_LEN			64

/// <summary>
/// Enumeration of all available types of units, for different types of measurement.
/// </summary>
enum UnitOfMeasure
{
    None = 0, Other, Meters, Kilograms, Seconds, Amps, Kelvin, Moles, Candela, Farads,
    Hertz, Joules, Newtons, Ohms, Pascals, Volts, Watts, Milliseconds, Minutes, Hours,
    Days, Months, Years, Centimeters, Kilometers, Inches, Feet, Yards, Miles, CentimetersPerSec,
    CentimetersPerMin, MetersPerSec, MetersPerMin, InchesPerSec, InchesPerMin, FtPerSec, FtPerMin, Bars, LbPerSqFt, Horsepower,
    Kilowatts, DegreesC, AmpsPerMs, VoltsPerMs, KwPerMs, AmpsMs, VoltsMs, KwMs, Percent, CmPerSecSq,
    LitersPerMin, Kilopascals, GramsPerLiter, LitersPerMole, NewtonMeters, Henries, Pounds, MicrosecPerTick, Millimeters, MillimetersPerSec,
    JoulesPerMm, Degrees, PoundsPerCuIn, Microseconds, MillimetersPerMin, MicroMeterPerSec,

    ENUM_MAX_UOM
};

/// <summary>
/// Represents a single filter value used to search through the process database.
/// </summary>
struct ProcessDisplayFilterEntry
{
    /// <summary>
    /// The zero-based index of the filter group in which the contents belong. The maximum index is (<see cref="PROCESS_FILTER_GROUPS"/> - 1). 
    /// </summary>
    INT32 filterGroupIndex; //0 - 3
    /// <summary>
    /// The non-zero identifier value of the filter group.
    /// </summary>
    INT32 filterId;
    /// <summary>
    /// The string representation of the filter that will be displayed to the user.
    /// </summary>
    char filterLabel[64];
} PACKED;

/// <summary>
/// Represents a single setting used to modify the control parameter of the welder.
/// </summary>
struct WeldControlSetting
{
    /// <summary>
    /// The minimum allowed value.
    /// </summary>
    INT32 min;
    /// <summary>
    /// The maximum allowed value.
    /// </summary>
    INT32 max;
    /// <summary>
    /// The unit of measure that the value represents.
    /// </summary>
    enum UnitOfMeasure unit;
    /// <summary>
    /// The number of available decimal places for the fixed-point precision value.
    /// </summary>
    INT16 numberOfDecimalPlaces;
    /// <summary>
    /// The default value used when initalizing weld settings before user input.
    /// </summary>
    INT32 defaultValue;
    /// <summary>
    /// The string representation of the control settings displayed to the user.
    /// </summary>
    char label[32];
} PACKED;

/// <summary>
/// Represents a set of weld control parameters used to modify the welder's output.
/// </summary>
struct ProcessInfo
{
    /// <summary>
    /// An identifier value, unique to each process.
    /// </summary>
    UINT32 id;

    /// <summary>
    /// The full description of the process, displayed to the user during process selection. Maximum size 
    /// of the label is <see cref="UWI_WELD_PROC_LABEL_LEN"/> bytes.
    /// </summary>
    char label[UWI_WELD_PROC_LABEL_LEN];

    /// <summary>
    /// Array of which filters can be applied to this process.
    /// </summary>
    INT32 displayFilters[PROCESS_FILTER_GROUPS];

    /// <summary>
    /// Bit mask that determines which <see cref="processControl"/> settings are used by the process.
    /// </summary>
    UINT16 processControlMask;

    /// <summary>
    /// Array of the available weld control parameters.
    /// </summary>
    struct WeldControlSetting processControl[NUM_WELDCONTROL_PARAMS];

    /// <summary>
    /// The current version of the process table. This value is used to determine the validity of the process table based on version number mismatches.
    /// </summary>
    UINT16 version;

    /// <summary>
    /// A computed value unique to each process and its contents.
    /// </summary>
    UINT16 checksum;

    /// <summary>
    /// Disable features in the Arc Start File editor for this specific process
    /// </summary>
    UINT16 asfPermissions;

    /// <summary>
    /// Disable features in the Arc End File editor for this specific process
    /// </summary>
    UINT16 aefPermissions;

    /// <summary>
    /// Reserved and unused bytes.
    /// </summary>
    UINT8 reserved[56];
} PACKED;

/// <summary>
/// Returns an array of [16] ProcessInfo structs
/// </summary>
extern struct ProcessInfo* UWI_GetProcessTable();

#undef PACKED

#endif // !PROCESSINFO_H
