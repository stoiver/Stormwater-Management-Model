//-----------------------------------------------------------------------------
//   toolkitAPI.c
//
//   Project: EPA SWMM5
//   Version: 5.1
//   Date:    08/30/2016
//   Author:  B. McDonnell (EmNet LLC)
//            K. Ratliff
//            Laurent Courty
//
//   Exportable Functions for Project Definition API.
//
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "headers.h"
#include "swmm5.h"                     // declaration of exportable functions
#include "hash.h"


// Function Declarations for API
int     massbal_getRoutingFlowTotal(SM_RoutingTotals *routingTot);
int     massbal_getRunoffTotal(SM_RunoffTotals *runoffTot);
double  massbal_getTotalArea(void);
int     massbal_getNodeTotalInflow(int index, double *value);

int  stats_getNodeStat(int index, SM_NodeStats *nodeStats);
int  stats_getStorageStat(int index, SM_StorageStats *storageStats);
int  stats_getOutfallStat(int index, SM_OutfallStats *outfallStats);
int  stats_getLinkStat(int index, SM_LinkStats *linkStats);
int  stats_getPumpStat(int index, SM_PumpStats *pumpStats);
int  stats_getSubcatchStat(int index, SM_SubcatchStats *subcatchStats);

//-----------------------------------------------------------------------------
//  Extended API Functions
//-----------------------------------------------------------------------------
void DLLEXPORT swmm_getAPIError(int errcode, char *s)
//
// Input:   errcode = error code
// Output:  errmessage String
// Return:  API Error
// Purpose: Get an error message
{
    char *errmsg = error_getMsg(errcode);
    strcpy(s, errmsg);
}

int DLLEXPORT swmm_getSimulationDateTime(int timetype, int *year, int *month, int *day,
                                         int *hours, int *minutes, int *seconds)
//
// Input:   timetype = time type to return
// Output:  year, month, day, hours, minutes, seconds = int
// Return:  API Error
// Purpose: Get the simulation start, end and report date times
{
    int errcode = 0;
    // Check if Open
    if (swmm_IsOpenFlag() == FALSE)
    {
        errcode = ERR_API_INPUTNOTOPEN;
    }
    else
    {
        DateTime _dtime;
        switch (timetype)
        {
        //StartDateTime (globals.h)
        case SM_STARTDATE: _dtime = StartDateTime; break;
        //EndDateTime (globals.h)
        case SM_ENDDATE: _dtime = EndDateTime;  break;
        //ReportStart (globals.h)
        case SM_REPORTDATE: _dtime = ReportStart;  break;
        default: return(ERR_API_OUTBOUNDS);
        }
        datetime_decodeDate(_dtime, year, month, day);
        datetime_decodeTime(_dtime, hours, minutes, seconds);
    }

    return (errcode);
}

int DLLEXPORT swmm_setSimulationDateTime(int timetype, char *dtimestr)
//
// Input:   timetype = time type to return
//          DateTime String
// Return:  API Error
// Purpose: Get the simulation start, end and report date times
{
    int errcode = 0;

    char theDate[10];
    char theTime[9];

    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        errcode = ERR_API_INPUTNOTOPEN;
    }
    // Check if Simulation is Running
    else if(swmm_IsStartedFlag() == TRUE)
    {
        errcode = ERR_API_SIM_NRUNNING;
    }
    else
    {
        strncpy(theDate, dtimestr, 10);
        strncpy(theTime, dtimestr+11, 9);

        switch(timetype)
        {
            //StartDateTime (globals.h)
            case SM_STARTDATE:
                project_readOption("START_DATE", theDate);
                project_readOption("START_TIME", theTime);
                StartDateTime = StartDate + StartTime;
                TotalDuration = floor((EndDateTime - StartDateTime) * SECperDAY);
                // --- convert total duration to milliseconds
                TotalDuration *= 1000.0;
                break;
            //EndDateTime (globals.h)
            case SM_ENDDATE:
                project_readOption("END_DATE", theDate);
                project_readOption("END_TIME", theTime);
                EndDateTime = EndDate + EndTime;
                TotalDuration = floor((EndDateTime - StartDateTime) * SECperDAY);
                // --- convert total duration to milliseconds
                TotalDuration *= 1000.0;
                break;
            //ReportStart (globals.h)
            case SM_REPORTDATE:
                project_readOption("REPORT_START_DATE", theDate);
                project_readOption("REPORT_START_TIME", theTime);
                ReportStart = ReportStartDate + ReportStartTime;
                break;
            default: errcode = ERR_API_OUTBOUNDS; break;
        }
    }

    return (errcode);
}

int DLLEXPORT  swmm_getSimulationUnit(int type, int *value)
//
// Input:   type = simulation unit type
// Output:  enum representation of units
// Returns: API Error
// Purpose: get simulation unit types
{
    int errcode = 0;
    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        errcode = ERR_API_INPUTNOTOPEN;
    }
    else
    {
        switch(type)
        {
            // System Unit (enum.h UnitsType)
            case SM_SYSTEMUNIT:  *value = UnitSystem; break;
            // Flow Unit (enum.h FlowUnitsType)
            case SM_FLOWUNIT:  *value = FlowUnits; break;
            // Concentration Unit
            //case 2:  *value = UnitSystem; break;
            // Type not available
            default: errcode = ERR_API_OUTBOUNDS; break;
        }
    }

    return (errcode);
}

int DLLEXPORT  swmm_getSimulationAnalysisSetting(int type, int *value)
//
// Input:   type = analysis type
// Output:  setting True or False
// Returns: API Error
// Purpose: get simulation analysis setting
{
    int errcode = 0;
    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        errcode = ERR_API_INPUTNOTOPEN;
    }
    else
    {
        switch(type)
        {
            // No ponding at nodes (True or False)
            case SM_ALLOWPOND:  *value = AllowPonding; break;
            // Do flow routing in steady state periods  (True or False)
            case SM_SKIPSTEADY:  *value = SkipSteadyState; break;
            // Analyze rainfall/runoff  (True or False)
            case SM_IGNORERAIN:  *value = IgnoreRainfall; break;
            // Analyze RDII (True or False)
            case SM_IGNORERDII:  *value = IgnoreRDII; break;
            // Analyze snowmelt (True or False)
            case SM_IGNORESNOW:  *value = IgnoreSnowmelt; break;
            // Analyze groundwater (True or False)
            case SM_IGNOREGW:  *value = IgnoreGwater; break;
            // Analyze flow routing (True or False)
            case SM_IGNOREROUTE:  *value = IgnoreRouting; break;
            // Analyze water quality (True or False)
            case SM_IGNORERQUAL:  *value = IgnoreQuality; break;
            // Type not available
            default: errcode = ERR_API_OUTBOUNDS; break;
        }
    }
    return (errcode);
}

int DLLEXPORT  swmm_getSimulationParam(int type, double *value)
//
// Input:   type = analysis type
// Output:  Simulation Parameter
// Returns: error code
// Purpose: Get simulation analysis parameter
{
    int errcode = 0;

    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        errcode = ERR_API_INPUTNOTOPEN;
    }
    // Output  setting
    else
    {
        switch(type)
        {
            // Routing time step (sec)
            case SM_ROUTESTEP: *value = RouteStep; break;
            // Minimum variable time step (sec)
            case SM_MINROUTESTEP: *value = MinRouteStep; break;
            // Time step for lengthening (sec)
            case SM_LENGTHSTEP: *value = LengtheningStep; break;
            // Antecedent dry days
            case SM_STARTDRYDAYS: *value = StartDryDays; break;
            // Courant time step factor
            case SM_COURANTFACTOR: *value = CourantFactor; break;
            // Minimum nodal surface area
            case SM_MINSURFAREA: *value = MinSurfArea; break;
            // Minimum conduit slope
            case SM_MINSLOPE: *value = MinSlope; break;
            // Runoff continuity error
            case SM_RUNOFFERROR: *value = RunoffError; break;
            // Groundwater continuity error
            case SM_GWERROR: *value = GwaterError; break;
            // Flow routing error
            case SM_FLOWERROR: *value = FlowError; break;
            // Quality routing error
            case SM_QUALERROR: *value = QualError; break;
            // DW routing head tolerance (ft)
            case SM_HEADTOL: *value = HeadTol; break;
            // Tolerance for steady system flow
            case SM_SYSFLOWTOL: *value = SysFlowTol; break;
            // Tolerance for steady nodal inflow
            case SM_LATFLOWTOL: *value = LatFlowTol; break;
            // Type not available
            default: errcode = ERR_API_OUTBOUNDS; break;
        }
    }
    return (errcode);
}

int DLLEXPORT  swmm_countObjects(int type, int *count)
//
// Input:   type = object type (Based on SM_ObjectType enum)
// Output:  count = pointer to integer
// Returns: API Error
// Purpose: uses Object Count table to find number of elements of an object
{
    if(type >= MAX_OBJ_TYPES)return ERR_API_OUTBOUNDS;
    *count = Nobjects[type];
    return (0);
}

int DLLEXPORT swmm_getObjectId(int type, int index, char *id)
//
// Input:   type = object type (Based on SM_ObjectType enum)
//          index = Index of desired ID
// Output:  id = pointer to id pass by reference
// Return:  API Error
// Purpose: Gets ID for any object
{
    int errcode = 0;
    //Provide Empty Character Array
    strcpy(id,"");

    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        errcode = ERR_API_INPUTNOTOPEN;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[type])
    {
        errcode = ERR_API_OBJECT_INDEX;
    }
    else
    {
        switch (type)
        {
            case SM_GAGE:
                strcpy(id,Gage[index].ID); break;
            case SM_SUBCATCH:
                strcpy(id,Subcatch[index].ID); break;
            case SM_NODE:
                strcpy(id,Node[index].ID); break;
            case SM_LINK:
                strcpy(id,Link[index].ID); break;
            case SM_POLLUT:
                strcpy(id,Pollut[index].ID); break;
            case SM_LANDUSE:
                strcpy(id,Landuse[index].ID); break;
            case SM_TIMEPATTERN:
                strcpy(id,Pattern[index].ID); break;
            case SM_CURVE:
                strcpy(id,Curve[index].ID); break;
            case SM_TSERIES:
                strcpy(id,Tseries[index].ID); break;
            //case SM_CONTROL:
                //strcpy(id,Rules[index].ID); break;
            case SM_TRANSECT:
                strcpy(id,Transect[index].ID); break;
            case SM_AQUIFER:
                strcpy(id,Aquifer[index].ID); break;
            case SM_UNITHYD:
                strcpy(id,UnitHyd[index].ID); break;
            case SM_SNOWMELT:
                strcpy(id,Snowmelt[index].ID); break;
            //case SM_SHAPE:
                //strcpy(id,Shape[index].ID); break;
            //case SM_LID:
                //strcpy(id,LidProcs[index].ID); break;
            default: errcode = ERR_API_OUTBOUNDS; break;
        }
   }
   return(errcode);
}

int DLLEXPORT swmm_getNodeType(int index, int *Ntype)
//
// Input:   index = Index of desired ID
//          Ntype = Node type (Based on enum SM_NodeType)
// Return:  API Error
// Purpose: Gets Node Type
{
    int errcode = 0;
    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        errcode = ERR_API_INPUTNOTOPEN;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[NODE])
    {
        errcode = ERR_API_OBJECT_INDEX;
    }
    else *Ntype = Node[index].type;

    return(errcode);
}

int DLLEXPORT swmm_getLinkType(int index, int *Ltype)
//
// Input:   index = Index of desired ID
//          Ltype = Link type (Based on enum SM_LinkType)
// Return:  API Error
// Purpose: Gets Link Type
{
    int errcode = 0;
    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        errcode = ERR_API_INPUTNOTOPEN;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[LINK])
    {
        errcode = ERR_API_OBJECT_INDEX;
    }
    else *Ltype = Link[index].type;

    return(errcode);
}

int DLLEXPORT swmm_getLinkConnections(int index, int *Node1, int *Node2)
//
// Input:   index = Index of desired ID
// Output:  Node1 and Node2 indeces
// Return:  API Error
// Purpose: Gets link Connection ID Indeces
{
    int errcode = 0;
    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        errcode = ERR_API_INPUTNOTOPEN;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[LINK])
    {
        errcode = ERR_API_OBJECT_INDEX;
    }
    else
    {
        *Node1 = Link[index].node1;
        *Node2 = Link[index].node2;
    }
    return(errcode);
}

int DLLEXPORT swmm_getLinkDirection(int index, signed char *value)
//
// Input:   index = Index of desired ID
// Output:  Link Direction (Only changes is slope < 0)
// Return:  API Error
// Purpose: Gets Link Direction
{
    int errcode = 0;
    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        errcode = ERR_API_INPUTNOTOPEN;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[LINK])
    {
        errcode = ERR_API_OBJECT_INDEX;
    }
    else
    {
        *value = Link[index].direction;
    }
    return(errcode);
}

int DLLEXPORT swmm_getNodeParam(int index, int Param, double *value)
//
// Input:   index = Index of desired ID
//          param = Parameter desired (Based on enum SM_NodeProperty)
// Output:  value = value to be output
// Return:  API Error
// Purpose: Gets Node Parameter
{
    int errcode = 0;
    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        errcode = ERR_API_INPUTNOTOPEN;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[NODE])
    {
        errcode = ERR_API_OBJECT_INDEX;
    }
    else
    {
        switch(Param)
        {
            case SM_INVERTEL:
                *value = Node[index].invertElev * UCF(LENGTH); break;
            case SM_FULLDEPTH:
                *value = Node[index].fullDepth * UCF(LENGTH); break;
            case SM_SURCHDEPTH:
                *value = Node[index].surDepth * UCF(LENGTH); break;
            case SM_PONDAREA:
                *value = Node[index].pondedArea * UCF(LENGTH) * UCF(LENGTH); break;
            case SM_INITDEPTH:
                *value = Node[index].initDepth * UCF(LENGTH); break;
            case SM_SURFAREA:
                *value = Node[index].surfaceArea * UCF(LENGTH) * UCF(LENGTH); break;
            case SM_COUPAREA:
                *value = Node[index].couplingArea * UCF(LENGTH) * UCF(LENGTH); break;
            case SM_OVERLANDDEPTH:
                *value = Node[index].overlandDepth * UCF(LENGTH); break;
            default: errcode = ERR_API_OUTBOUNDS; break;
        }
    }
    return(errcode);
}

int DLLEXPORT swmm_setNodeParam(int index, int Param, double value)
//
// Input:   index = Index of desired ID
//          param = Parameter desired (Based on enum SM_NodeProperty)
//          value = value to be input
// Return:  API Error
// Purpose: Sets Node Parameter
{
    int errcode = 0;
    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        errcode = ERR_API_INPUTNOTOPEN;
    }
     // Check if Simulation is Running
    //~ else if(swmm_IsStartedFlag() == TRUE)
    //~ {
        //~ errcode = ERR_API_SIM_NRUNNING;
    //~ }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[NODE])
    {
        errcode = ERR_API_OBJECT_INDEX;
    }
    else
    {
        switch(Param)
        {
            case SM_INVERTEL:
                Node[index].invertElev = value / UCF(LENGTH); break;
            case SM_FULLDEPTH:
                Node[index].fullDepth = value / UCF(LENGTH); break;
            case SM_SURCHDEPTH:
                Node[index].surDepth = value / UCF(LENGTH); break;
            case SM_PONDAREA:
                Node[index].pondedArea = value / ( UCF(LENGTH) * UCF(LENGTH) ); break;
            case SM_INITDEPTH:
                Node[index].initDepth = value / UCF(LENGTH); break;
            case SM_SURFAREA:
                Node[index].surfaceArea = value / UCF(LENGTH) * UCF(LENGTH);
                Node[index].fullVolume = node_getVolume(index, Node[index].fullDepth);
                break;
            case SM_COUPAREA:
                Node[index].couplingArea = value / ( UCF(LENGTH) * UCF(LENGTH) ); break;
            case SM_OVERLANDDEPTH:
                Node[index].overlandDepth = value / UCF(LENGTH); break;
            default: errcode = ERR_API_OUTBOUNDS; break;
        }
    }
    // Re-validated a node BEM 1/20/2017 Probably need to re-validate connecting links
    //node_validate(index)
    return(errcode);
}

int DLLEXPORT swmm_getLinkParam(int index, int Param, double *value)
//
// Input:   index = Index of desired ID
//          param = Parameter desired (Based on enum SM_LinkProperty)
// Output:  value = value to be output
// Return:  API Error
// Purpose: Gets Link Parameter
{
    int errcode = 0;
    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        errcode = ERR_API_INPUTNOTOPEN;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[LINK])
    {
        errcode = ERR_API_OBJECT_INDEX;
    }
    else
    {
        switch(Param)
        {
            case SM_OFFSET1:
                *value = Link[index].offset1 * UCF(LENGTH); break;
            case SM_OFFSET2:
                *value = Link[index].offset2 * UCF(LENGTH); break;
            case SM_INITFLOW:
                *value = Link[index].q0 * UCF(FLOW); break;
            case SM_FLOWLIMIT:
                *value = Link[index].qLimit * UCF(FLOW); break;
            case SM_INLETLOSS:
                *value = Link[index].cLossInlet; break;
            case SM_OUTLETLOSS:
                *value = Link[index].cLossOutlet; break;
            case SM_AVELOSS:
                *value = Link[index].cLossAvg; break;
            default: errcode = ERR_API_OUTBOUNDS; break;
        }
    }
    return(errcode);
}

int DLLEXPORT swmm_setLinkParam(int index, int Param, double value)
//
// Input:   index = Index of desired ID
//          param = Parameter desired (Based on enum SM_LinkProperty)
//          value = value to be output
// Return:  API Error
// Purpose: Sets Link Parameter
{
    int errcode = 0;
    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        errcode = ERR_API_INPUTNOTOPEN;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[LINK])
    {
        errcode = ERR_API_OBJECT_INDEX;
    }
    else
    {
        switch(Param)
        {
            // offset1
            case SM_OFFSET1:
                // Check if Simulation is Running
                if(swmm_IsStartedFlag() == TRUE)
                {
                    errcode = ERR_API_SIM_NRUNNING; break;
                }
                Link[index].offset1 = value / UCF(LENGTH); break;
            case SM_OFFSET2:
                // Check if Simulation is Running
                if(swmm_IsStartedFlag() == TRUE)
                {
                    errcode = ERR_API_SIM_NRUNNING; break;
                }
                Link[index].offset2 = value / UCF(LENGTH); break;
            case SM_INITFLOW:
                Link[index].q0 = value / UCF(FLOW); break;
            case SM_FLOWLIMIT:
                Link[index].qLimit = value / UCF(FLOW); break;
            case SM_INLETLOSS:
                Link[index].cLossInlet; break;
            case SM_OUTLETLOSS:
                Link[index].cLossOutlet; break;
            case SM_AVELOSS:
                Link[index].cLossAvg; break;
            default: errcode = ERR_API_OUTBOUNDS; break;
        }
        // re-validated link
        //link_validate(index);
    }

    return(errcode);
}


int DLLEXPORT swmm_getSubcatchParam(int index, int Param, double *value)
//
// Input:   index = Index of desired ID
//          param = Parameter desired (Based on enum SM_SubcProperty)
// Output:  value = value to be output
// Return:  API Error
// Purpose: Gets Subcatchment Parameter
{
    int errcode = 0;
    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        errcode = ERR_API_INPUTNOTOPEN;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[SUBCATCH])
    {
        errcode = ERR_API_OBJECT_INDEX;
    }
    else
    {
        switch(Param)
        {
            case SM_WIDTH:
                *value = Subcatch[index].width * UCF(LENGTH); break;
            case SM_AREA:
                *value = Subcatch[index].area * UCF(LANDAREA); break;
            case SM_FRACIMPERV:
                *value = Subcatch[index].fracImperv; break;
            case SM_SLOPE:
                *value = Subcatch[index].slope; break;
            case SM_CURBLEN:
                *value = Subcatch[index].curbLength * UCF(LENGTH); break;
            default: errcode = ERR_API_OUTBOUNDS; break;
        }
    }
    return(errcode);
}

int DLLEXPORT swmm_setSubcatchParam(int index, int Param, double value)
//
// Input:   index = Index of desired ID
//          param = Parameter desired (Based on enum SM_SubcProperty)
//          value = value to be output
// Return:  API Error
// Purpose: Sets Subcatchment Parameter
{
    int errcode = 0;
    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        errcode = ERR_API_INPUTNOTOPEN;
    }
     // Check if Simulation is Running
    else if(swmm_IsStartedFlag() == TRUE)
    {
        errcode = ERR_API_SIM_NRUNNING;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[SUBCATCH])
    {
        errcode = ERR_API_OBJECT_INDEX;
    }
    else
    {
        switch(Param)
        {
            case SM_WIDTH:
                Subcatch[index].width = value / UCF(LENGTH); break;
            case SM_AREA:
                Subcatch[index].area = value / UCF(LANDAREA); break;
            case SM_FRACIMPERV:
                Subcatch[index].fracImperv; break;
            case SM_SLOPE:
                Subcatch[index].slope; break;
            case SM_CURBLEN:
                Subcatch[index].curbLength = value / UCF(LENGTH); break;
            default: errcode = ERR_API_OUTBOUNDS; break;
        }
    }
    //re-validate subcatchment
    subcatch_validate(index); // incorprate callback here

    return(errcode);
}

int DLLEXPORT swmm_getSubcatchOutConnection(int index, int *type, int *ObjIndex )
//
// Input:   index = Index of desired ID
//         (Subcatchments can load to Node or another Subcatchment)
// Output:  Type of Object
//          Index of Object
// Return:  API Error
// Purpose: Gets Subcatchment Connection ID Indeces for either Node or Subcatchment
{
    int errcode = 0;
    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
        errcode = ERR_API_INPUTNOTOPEN;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[SUBCATCH])
    {
        errcode = ERR_API_OBJECT_INDEX;
    }
    else
    {
        if (Subcatch[index].outNode == -1 && Subcatch[index].outSubcatch == -1)
        {
            *ObjIndex = index; // Case of self Loading subcatchment
            *type = SUBCATCH;
        }
        if (Subcatch[index].outNode >= 0)
        {
            *ObjIndex = Subcatch[index].outNode;
            *type = NODE;
        }
        if (Subcatch[index].outSubcatch >= 0)
        {
            *ObjIndex = Subcatch[index].outSubcatch;
            *type = SUBCATCH;
        }
    }
    return(errcode);
}


//-------------------------------
// Active Simulation Results API
//-------------------------------

int DLLEXPORT swmm_getCurrentDateTimeStr(char *dtimestr)
//
// Output:  DateTime String
// Return:  API Error
// Purpose: Get the current simulation time
{
    //Provide Empty Character Array
    char     theDate[12];
    char     theTime[9];
    char     _DTimeStr[22];
    DateTime currentTime;

    // Check if Simulation is Running
    if(swmm_IsStartedFlag() == FALSE) return(ERR_API_SIM_NRUNNING);

    // Fetch Current Time
    currentTime = getDateTime(NewRoutingTime);

    // Convert To Char
    datetime_dateToStr(currentTime, theDate);
    datetime_timeToStr(currentTime, theTime);

    strcpy(_DTimeStr, theDate);
    strcat(_DTimeStr, " ");
    strcat(_DTimeStr, theTime);

    strcpy(dtimestr, _DTimeStr);
    return(0);
}


int DLLEXPORT swmm_getNodeResult(int index, int type, double *result)
//
// Input:   index = Index of desired ID
//          type = Result Type (SM_NodeResult)
// Output:  result = result data desired (byref)
// Return:  API Error
// Purpose: Gets Node Simulated Value at Current Time
{
    int errcode = 0;
    // Check if Simulation is Running
    if(swmm_IsStartedFlag() == FALSE)
    {
        errcode = ERR_API_SIM_NRUNNING;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[NODE])
    {
        errcode = ERR_API_OBJECT_INDEX;
    }
    else
    {
        switch (type)
        {
            case SM_TOTALINFLOW:
                *result = Node[index].inflow * UCF(FLOW); break;
            case SM_TOTALOUTFLOW:
                *result = Node[index].outflow * UCF(FLOW); break;
            case SM_LOSSES:
                *result = Node[index].losses * UCF(FLOW); break;
            case SM_NODEVOL:
                *result = Node[index].newVolume * UCF(VOLUME); break;
            case SM_NODEFLOOD:
                *result = Node[index].overflow * UCF(FLOW); break;
            case SM_NODEDEPTH:
                *result = Node[index].newDepth * UCF(LENGTH); break;
            case SM_NODEHEAD:
                *result = (Node[index].newDepth
                            + Node[index].invertElev) * UCF(LENGTH); break;
            case SM_LATINFLOW:
                *result = Node[index].newLatFlow * UCF(FLOW); break;
            case SM_COUPINFLOW:
                *result = Node[index].couplingInflow * UCF(FLOW); break;
            default: errcode = ERR_API_OUTBOUNDS; break;
        }
    }
    return(errcode);
}


int DLLEXPORT swmm_getLinkResult(int index, int type, double *result)
//
// Input:   index = Index of desired ID
//          type = Result Type (SM_LinkResult)
// Output:  result = result data desired (byref)
// Return:  API Error
// Purpose: Gets Link Simulated Value at Current Time
{
    int errcode = 0;
    // Check if Simulation is Running
    if(swmm_IsStartedFlag() == FALSE)
    {
        errcode = ERR_API_SIM_NRUNNING;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[LINK])
    {
        errcode = ERR_API_OBJECT_INDEX;
    }
    else
    {
        switch (type)
        {
            case SM_LINKFLOW:
                *result = Link[index].newFlow * UCF(FLOW) ; break;
            case SM_LINKDEPTH:
                *result = Link[index].newDepth * UCF(LENGTH); break;
            case SM_LINKVOL:
                *result = Link[index].newVolume * UCF(VOLUME); break;
            case SM_USSURFAREA:
                *result = Link[index].surfArea1 * UCF(LENGTH) * UCF(LENGTH); break;
            case SM_DSSURFAREA:
                *result = Link[index].surfArea2 * UCF(LENGTH) * UCF(LENGTH); break;
            case SM_SETTING:
                *result = Link[index].setting; break;
            case SM_TARGETSETTING:
                *result = Link[index].targetSetting; break;
            case SM_FROUDE:
                *result = Link[index].froude; break;
            default: errcode = ERR_API_OUTBOUNDS; break;
        }
    }
    return(errcode);
}


int DLLEXPORT swmm_getSubcatchResult(int index, int type, double *result)
//
// Input:   index = Index of desired ID
//          type = Result Type (SM_SubcResult)
// Output:  result = result data desired (byref)
// Return:  API Error
// Purpose: Gets Subcatchment Simulated Value at Current Time
{
    int errcode = 0;
    // Check if Simulation is Running
    if(swmm_IsStartedFlag() == FALSE)
    {
        errcode = ERR_API_SIM_NRUNNING;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[SUBCATCH])
    {
        errcode = ERR_API_OBJECT_INDEX;
    }
    else
    {
        switch (type)
        {
            case SM_SUBCRAIN:
                *result = Subcatch[index].rainfall * UCF(RAINFALL); break;
            case SM_SUBCEVAP:
                *result = Subcatch[index].evapLoss * UCF(EVAPRATE); break;
            case SM_SUBCINFIL:
                *result = Subcatch[index].infilLoss * UCF(RAINFALL); break;
            case SM_SUBCRUNON:
                *result = Subcatch[index].runon * UCF(FLOW); break;
            case SM_SUBCRUNOFF:
                *result = Subcatch[index].newRunoff * UCF(FLOW); break;
            case SM_SUBCSNOW:
                *result = Subcatch[index].newSnowDepth * UCF(RAINDEPTH); break;
            default: errcode = ERR_API_OUTBOUNDS; break;
        }
    }
    return(errcode);
}


int DLLEXPORT swmm_getNodeStats(int index, SM_NodeStats *nodeStats)
//
// Output:  Node Stats Structure (SM_NodeStats)
// Return:  API Error
// Purpose: Gets Node Stats and Converts Units
{
    int errorcode = stats_getNodeStat(index, nodeStats);

    if (errorcode == 0)
    {
        // Current Average Depth
        nodeStats->avgDepth *= (UCF(LENGTH) / (double)StepCount);
        // Current Maximum Depth
        nodeStats->maxDepth *= UCF(LENGTH);
        // Current Maximum Lateral Inflow
        nodeStats->maxLatFlow *= UCF(FLOW);
        // Current Maximum Inflow
        nodeStats->maxInflow *= UCF(FLOW);
        // Cumulative Lateral Inflow
        nodeStats->totLatFlow *= UCF(VOLUME);
        // Time Courant Critical (hrs)
        nodeStats->timeCourantCritical /= 3600.0;
        // Cumulative Flooded Volume
        nodeStats->volFlooded *= UCF(VOLUME);
        // Time Flooded (hrs)
        nodeStats->timeFlooded /= 3600.0;
        // Current Maximum Overflow
        nodeStats->maxOverflow *= UCF(FLOW);
        // Current Maximum Ponding Volume
        nodeStats->maxPondedVol *= UCF(VOLUME);
        // Time Surcharged
        nodeStats->timeSurcharged /= 3600.0;
    }
    return (errorcode);
}


int DLLEXPORT swmm_getNodeTotalInflow(int index, double *value)
//
// Input:   Node Index
// Output:  Node Total inflow Volume.
// Return:  API Error
// Purpose: Get Node Total Inflow Volume.
{

    int errorcode = massbal_getNodeTotalInflow(index, value);

    if (errorcode == 0)
    {
        *value *= UCF(VOLUME);
    }

    return(errorcode);
}

int DLLEXPORT swmm_getStorageStats(int index, SM_StorageStats *storageStats)
//
// Output:  Storage Node Stats Structure (SM_StorageStats)
// Return:  API Error
// Purpose: Gets Storage Node Stats and Converts Units
{
    int errorcode = stats_getStorageStat(index, storageStats);

    if (errorcode == 0)
    {
        // Initial Volume
        storageStats->initVol *= UCF(VOLUME);
        // Current Average Volume
        storageStats->avgVol *= (UCF(VOLUME) / (double)StepCount);
        // Current Maximum Volume
        storageStats->maxVol *= UCF(VOLUME);
        // Current Maximum Flow
        storageStats->maxFlow *= UCF(FLOW);
        // Current Evaporation Volume
        storageStats->evapLosses *= UCF(VOLUME);
        // Current Exfiltration Volume
        storageStats->exfilLosses *= UCF(VOLUME);
    }

    return (errorcode);
}

int DLLEXPORT swmm_getOutfallStats(int index, SM_OutfallStats *outfallStats)
//
// Output:  Outfall Stats Structure (SM_OutfallStats)
// Return:  API Error
// Purpose: Gets Outfall Node Stats and Converts Units
// Note:    Caller is responsible for calling swmm_freeOutfallStats
//          to free the pollutants array.
{
    int p;
    int errorcode = stats_getOutfallStat(index, outfallStats);

    if (errorcode == 0)
    {
        // Current Average Flow
        if ( outfallStats->totalPeriods > 0 )
        {
            outfallStats->avgFlow *= (UCF(FLOW) / (double)outfallStats->totalPeriods);
        }
        else
        {
            outfallStats->avgFlow *= 0.0;
        }
        // Current Maximum Flow
        outfallStats->maxFlow *= UCF(FLOW);
        // Convert Mass Units
        if (Nobjects[POLLUT] > 0)
        {
            for (p = 0; p < Nobjects[POLLUT]; p++)
                outfallStats->totalLoad[p] *= (LperFT3 * Pollut[p].mcf);
                if (Pollut[p].units == COUNT)
                {
                    outfallStats->totalLoad[p] = LOG10(outfallStats->totalLoad[p]);
                }
        }
    }

    return (errorcode);
}

void DLLEXPORT swmm_freeOutfallStats(SM_OutfallStats *outfallStats)
//
// Return:  API Error
// Purpose: Frees Outfall Node Stats and Converts Units
// Note:    API user is responsible for calling swmm_freeOutfallStats
//          since this function performs a memory allocation.
{
    FREE(outfallStats->totalLoad);
}



int DLLEXPORT swmm_getLinkStats(int index, SM_LinkStats *linkStats)
//
// Output:  Link Stats Structure (SM_LinkStats)
// Return:  API Error
// Purpose: Gets Link Stats and Converts Units
{
    int errorcode = stats_getLinkStat(index, linkStats);

    if (errorcode == 0)
    {
        // Cumulative Maximum Flowrate
        linkStats->maxFlow *= UCF(FLOW);
        // Cumulative Maximum Velocity
        linkStats->maxVeloc *= UCF(LENGTH);
        // Cumulative Maximum Depth
        linkStats->maxDepth *= UCF(LENGTH);
        // Cumulative Time Normal Flow
        linkStats->timeNormalFlow /= 3600.0;
        // Cumulative Time Inlet Control
        linkStats->timeInletControl /= 3600.0;
        // Cumulative Time Surcharged
        linkStats->timeSurcharged /= 3600.0;
        // Cumulative Time Upstream Full
        linkStats->timeFullUpstream /= 3600.0;
        // Cumulative Time Downstream Full
        linkStats->timeFullDnstream /= 3600.0;
        // Cumulative Time Full Flow
        linkStats->timeFullFlow /= 3600.0;
        // Cumulative Time Capacity limited
        linkStats->timeCapacityLimited /= 3600.0;
        // Cumulative Time Courant Critical Flow
        linkStats->timeCourantCritical /= 3600.0;
    }

    return (errorcode);
}


int DLLEXPORT swmm_getPumpStats(int index, SM_PumpStats *pumpStats)
//
// Output:  Pump Link Stats Structure (SM_PumpStats)
// Return:  API Error
// Purpose: Gets Pump Link Stats and Converts Units
{
    int errorcode = stats_getPumpStat(index, pumpStats);

    if (errorcode == 0)
    {
        // Cumulative Minimum Flow
        pumpStats->minFlow *= UCF(FLOW);
        // Cumulative Average Flow
        if (pumpStats->totalPeriods > 0)
        {
            pumpStats->avgFlow *= (UCF(FLOW) / (double)pumpStats->totalPeriods);
        }
        else
        {
            pumpStats->avgFlow *= 0.0;
        }
        // Cumulative Maximum Flow
        pumpStats->maxFlow *= UCF(FLOW);
        // Cumulative Pumping Volume
        pumpStats->volume *= UCF(VOLUME);
    }

    return (errorcode);
}


int DLLEXPORT swmm_getSubcatchStats(int index, SM_SubcatchStats *subcatchStats)
//
// Output:  Subcatchment Stats Structure (SM_SubcatchStats)
// Return:  API Error
// Purpose: Gets Subcatchment Stats and Converts Units
// Note: Caller is responsible for calling swmm_freeSubcatchStats
//       to free the pollutants array.
{
    int p;
    int errorcode = stats_getSubcatchStat(index, subcatchStats);

    if (errorcode == 0)
    {
        double a = Subcatch[index].area;

        // Cumulative Runon Volume
        subcatchStats->runon *= (UCF(RAINDEPTH) / a);
        // Cumulative Infiltration Volume
        subcatchStats->infil *= (UCF(RAINDEPTH) / a);
        // Cumulative Runoff Volume
        subcatchStats->runoff *= (UCF(RAINDEPTH) / a);
        // Maximum Runoff Rate
        subcatchStats->maxFlow *= UCF(FLOW);
        // Cumulative Rainfall Depth
        subcatchStats->precip *= (UCF(RAINDEPTH) / a);
        // Cumulative Evaporation Volume
        subcatchStats->evap *= (UCF(RAINDEPTH) / a);

        if (Nobjects[POLLUT] > 0)
        {
            for (p = 0; p < Nobjects[POLLUT]; p++)
                subcatchStats->surfaceBuildup[p] /= (a * UCF(LANDAREA));
                if (Pollut[p].units == COUNT)
                {
                    subcatchStats->surfaceBuildup[p] =
                        LOG10(subcatchStats->surfaceBuildup[p]);
                }
        }
    }

    return (errorcode);
}


void DLLEXPORT swmm_freeSubcatchStats(SM_SubcatchStats *subcatchStats)
//
// Return:  API Error
// Purpose: Frees Subcatchment Stats
// Note:    API user is responsible for calling swmm_freeSubcatchStats
//          since this function performs a memory allocation.
{
    FREE(subcatchStats->surfaceBuildup);
}

int DLLEXPORT swmm_getSystemRoutingStats(SM_RoutingTotals *routingTot)
//
// Output:  System Routing Totals Structure (SM_RoutingTotals)
// Return:  API Error
// Purpose: Gets System Flow Routing Totals and Converts Units
{
    int errorcode = massbal_getRoutingFlowTotal(routingTot);

    if (errorcode == 0)
    {
        // Cumulative Dry Weather Inflow Volume
        routingTot->dwInflow *= UCF(VOLUME);
        // Cumulative Wet Weather Inflow Volume
        routingTot->wwInflow *= UCF(VOLUME);
        // Cumulative Groundwater Inflow Volume
        routingTot->gwInflow *= UCF(VOLUME);
        // Cumulative I&I Inflow Volume
        routingTot->iiInflow *= UCF(VOLUME);
        // Cumulative External Inflow Volume
        routingTot->exInflow *= UCF(VOLUME);
        // Cumulative Flooding Volume
        routingTot->flooding *= UCF(VOLUME);
        // Cumulative Outflow Volume
        routingTot->outflow  *= UCF(VOLUME);
        // Cumulative Evaporation Loss
        routingTot->evapLoss *= UCF(VOLUME);
        // Cumulative Seepage Loss
        routingTot->seepLoss *= UCF(VOLUME);
        // Continuity Error
        routingTot->pctError *= 100;
    }

    return(errorcode);
}

int DLLEXPORT swmm_getSystemRunoffStats(SM_RunoffTotals *runoffTot)
//
// Output:  System Runoff Totals Structure (SM_RunoffTotals)
// Return:  API Error
// Purpose: Gets System Runoff Totals and Converts Units
{
    int errorcode =  massbal_getRunoffTotal(runoffTot);

    if (errorcode == 0)
    {
        double TotalArea = massbal_getTotalArea();
        // Cumulative Rainfall Volume
        runoffTot->rainfall *= (UCF(RAINDEPTH) / TotalArea);
        // Cumulative Evaporation Volume
        runoffTot->evap *= (UCF(RAINDEPTH) / TotalArea);
        // Cumulative Infiltration Volume
        runoffTot->infil *= (UCF(RAINDEPTH) / TotalArea);
        // Cumulative Runoff Volume
        runoffTot->runoff *= (UCF(RAINDEPTH) / TotalArea);
        // Cumulative Runon Volume
        runoffTot->runon *= (UCF(RAINDEPTH) / TotalArea);
        // Cumulative Drain Volume
        runoffTot->drains *= (UCF(RAINDEPTH) / TotalArea);
        // Cumulative Snow Removed Volume
        runoffTot->snowRemoved *= (UCF(RAINDEPTH) / TotalArea);
        // Initial Storage Volume
        runoffTot->initStorage *= (UCF(RAINDEPTH) / TotalArea);
        // Initial Snow Cover Volume
        runoffTot->initSnowCover *= (UCF(RAINDEPTH) / TotalArea);
        // Continuity Error
        runoffTot->pctError *= 100;
    }

    return(errorcode);
}

int DLLEXPORT swmm_getGagePrecip(int index, double *rainfall, double *snowfall, double *total)
//
// Input:   index = Index of desired ID
// Output:  Rainfall intensity and snow for the gage
// Return:  API Error
// Purpose: Gets the precipitaion value in the gage. 
{
    int errcode = 0;
    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
	    errcode = ERR_API_INPUTNOTOPEN;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[GAGE])
    {
	    errcode = ERR_API_OBJECT_INDEX;
    }
    // Read the rainfall value
    else
    {
        *total = gage_getPrecip(index, rainfall, snowfall);
    }
    return(errcode);
}

int DLLEXPORT swmm_setGagePrecip(int index, double value)
//
// Input:   index = Index of desired ID
//          value = rainfall intensity to be set
// Return:  API Error
// Purpose: Sets the precipitation in from the external database
{
    int errcode = 0;
    // Check if Open
    if(swmm_IsOpenFlag() == FALSE)
    {
	    errcode = ERR_API_INPUTNOTOPEN;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[GAGE])
    {
	    errcode = ERR_API_OBJECT_INDEX;
    }
    // Read the rainfall value
    else
    {
        if (Gage[index].dataSource != RAIN_API)
        {
            Gage[index].dataSource = RAIN_API;
        }
	    Gage[index].externalRain = value * UCF(RAINFALL);
    }
    return(errcode);
}


//-------------------------------
// Setters API
//-------------------------------

int DLLEXPORT swmm_setLinkSetting(int index, double targetSetting)
//
// Input:   index = Index of desired ID
//          value = New Target Setting
// Output:  returns API Error
// Purpose: Sets Link open fraction (Weir, Orifice, Pump, and Outlet)
{
    DateTime currentTime;
    int errcode = 0;
    char _rule_[11] = "ToolkitAPI";

    // Check if Open
    if (swmm_IsOpenFlag() == FALSE)
    {
        errcode = ERR_API_INPUTNOTOPEN;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[LINK])
    {
        errcode = ERR_API_OBJECT_INDEX;
    }
    else
    {
        // --- check that new setting lies within feasible limits
        if (targetSetting < 0.0) targetSetting = 0.0;
        if (Link[index].type != PUMP && targetSetting > 1.0) targetSetting = 1.0;

        Link[index].targetSetting = targetSetting;

        // Use internal function to apply the new setting
        link_setSetting(index, 0.0);

        // Add control action to RPT file if desired flagged
        if (RptFlags.controls)
        {
            currentTime = getDateTime(NewRoutingTime);
            report_writeControlAction(currentTime, Link[index].ID, targetSetting, _rule_);
        }
    }
    return(errcode);
}


int DLLEXPORT swmm_setNodeInflow(int index, double flowrate)
//
// Input:   index = Index of desired ID
//          value = New Inflow Rate
// Output:  returns API Error
// Purpose: Sets new node inflow rate and holds until set again
{
    int errcode = 0;

    // Check if Open
    if (swmm_IsOpenFlag() == FALSE)
    {
        errcode = ERR_API_INPUTNOTOPEN;
    }
    // Check if object index is within bounds
    else if (index < 0 || index >= Nobjects[NODE])
    {
        errcode = ERR_API_OBJECT_INDEX;
    }
    else
    {
        // Check to see if node has an assigned inflow object
        TExtInflow* inflow;

        // --- check if an external inflow object for this constituent already exists
        inflow = Node[index].extInflow;
        while (inflow)
        {
            if (inflow->param == -1) break;
            inflow = inflow->next;
        }

        if (!inflow)
        {
            int param = -1;        // FLOW (-1) or Pollutant Index
            int type = FLOW_INFLOW;// Type of inflow (FLOW)
            int tSeries = -1;      // No Time Series
            int basePat = -1;      // No Base Pattern
            double cf = 1.0;       // Unit Convert (Converted during validation)
            double sf = 1.0;       // Scaling Factor
            double baseline = 0.0; // Baseline Inflow Rate

            // Initializes Inflow Object
            errcode = inflow_setExtInflow(index, param, type, tSeries,
                basePat, cf, baseline, sf);

            // Get The Inflow Object
            if ( errcode == 0 )
            {
                inflow = Node[index].extInflow;
            }
        }
        // Assign new flow rate
        if ( errcode == 0 )
        {
            inflow -> extIfaceInflow = flowrate;
        }
    }
    return(errcode);
}

int DLLEXPORT swmm_setOutfallStage(int index, double stage)
//
// Input:   index = Index of desired outfall
//          stage = New outfall stage (head)
// Output:  returns API Error
// Purpose: Sets new outfall stage and holds until set again.
{
    int errcode = 0;
    // Check if Open
    if (swmm_IsOpenFlag() == FALSE)
    {
        errcode = ERR_API_INPUTNOTOPEN;
    }
    // Check if object index is within bounds
    else if ( index < 0 || index >= Nobjects[NODE] )
    {
        errcode = ERR_API_OBJECT_INDEX;
    }
    else if ( Node[index].type != OUTFALL )
    {
        errcode = ERR_API_WRONG_TYPE;
    }
    else
    {
        int k = Node[index].subIndex;
        if ( Outfall[k].type != STAGED_OUTFALL )
        {
            // Change Boundary Conditions Setting Type
            Outfall[k].type = STAGED_OUTFALL;
        }
        Outfall[k].outfallStage = stage / UCF(LENGTH);
    }
    return(errcode);
}
//-----------------------------------------------------------------------------
// COUPLING FUNCTIONS
//-----------------------------------------------------------------------------


int DLLEXPORT swmm_setNodeOpening(int nodeID, int idx, int oType, double A,
                                  double l, double Co, double Cfw, double Csw)
//
// Input:   nodeID = Index of desired node
//          idx    = opening's index
//          otype  = type of opening (grate, etc). Not used yet
//          A      = area of the opening
//          l      = length of the opening (~circumference)
//          Co     = orifice coefficient
//          Cfw    = free weir coefficient
//          Csw    = submerged weir coefficient

// Return:  API Error
// Purpose: Sets Node opening parameters.
{
    int errcode;
    double u_A, u_l;

    
    // Check if Open
    if(swmm_IsOpenFlag() == FALSE) return(ERR_API_INPUTNOTOPEN);
    // Check if Simulation is Running
    if(swmm_IsStartedFlag() == TRUE) return(ERR_API_SIM_NRUNNING);
    // Check if object index is within bounds
    if (nodeID < 0 || nodeID >= Nobjects[NODE]) return(ERR_API_OBJECT_INDEX);

    u_A = A / ( UCF(LENGTH) * UCF(LENGTH) );
    u_l = l / UCF(LENGTH);

    printf("NodeID %d\n",nodeID);    
    errcode = coupling_setOpening(nodeID, idx, oType, u_A, u_l, Co, Cfw, Csw);
    return(errcode);
}

int DLLEXPORT swmm_deleteNodeOpening(int nodeID, int idx)
//
// Input:   nodeID = Index of desired node
//          idx    = opening's index
// Return:  Error code
// Purpose: Remove an opening from a node.
{
    // Check if Open
    if(swmm_IsOpenFlag() == FALSE) return(ERR_API_INPUTNOTOPEN);
    // Check if Simulation is Running
    if(swmm_IsStartedFlag() == TRUE) return(ERR_API_SIM_NRUNNING);
    // Check if object index is within bounds
    if (nodeID < 0 || nodeID >= Nobjects[NODE]) return(ERR_API_OBJECT_INDEX);

    coupling_deleteOpening(nodeID, idx);
    return(0);
}

int DLLEXPORT swmm_getNodeOpeningParam(int nodeID, int idx, int Param, double *value)
//
// Input:   nodeID = Index of desired node
//          idx    = opening's index
//          Param  = parameter to get (enum OpeningParams, type excluded)
// Output   value  = value to be output
// Return:  API Error
// Purpose: Get Node opening parameters.
{
    // Check if Open
    if(swmm_IsOpenFlag() == FALSE) return(ERR_API_INPUTNOTOPEN);
    // Check if object index is within bounds
    if (nodeID < 0 || nodeID >= Nobjects[NODE]) return(ERR_API_OBJECT_INDEX);

    TCoverOpening* opening;

    // --- check if an opening with this index exists
    opening = Node[nodeID].coverOpening;
    while ( opening )
    {
        if ( opening->ID == idx ) break;
        opening = opening->next;
    }
     // --- if it doesn't, return an error
    if ( opening == NULL ) return(ERR_API_OBJECT_INDEX);

    switch(Param)
    {
        // area
        case OPENING_AREA: *value = opening->area * UCF(LENGTH) * UCF(LENGTH); break;
        // length
        case OPENING_LENGTH: *value = opening->length * UCF(LENGTH); break;
        // coeffOrifice
        case ORIFICE_COEFF: *value = opening->coeffOrifice; break;
        // coeffFreeWeir
        case FREE_WEIR_COEFF: *value = opening->coeffFreeWeir; break;
        // coeffSubWeir
        case SUBMERGED_WEIR_COEFF: *value = opening->coeffSubWeir; break;
        // Type not available
        default: return(ERR_API_OUTBOUNDS);
    }
    return(0);
}

int DLLEXPORT swmm_getNodeOpeningFlow(int nodeID, int idx, double *inflow)
//
// Input:   nodeID = Index of desired node
//          idx    = opening's index
// Output   inflow  = inflow to be output
// Return:  API Error
// Purpose: Get opening inflow.
{
    // Check if Open
    if(swmm_IsOpenFlag() == FALSE) return(ERR_API_INPUTNOTOPEN);
    // Check if Simulation is Running
    if(swmm_IsStartedFlag() == FALSE) return(ERR_API_SIM_NRUNNING);
    // Check if object index is within bounds
    if (nodeID < 0 || nodeID >= Nobjects[NODE]) return(ERR_API_OBJECT_INDEX);

    TCoverOpening* opening;

    // --- check if an opening with this index exists
    opening = Node[nodeID].coverOpening;
    while ( opening )
    {
        if ( opening->ID == idx ) break;
        opening = opening->next;
    }
     // --- if it doesn't, return an error
    if ( opening == NULL ) return(ERR_API_OBJECT_INDEX);

    *inflow = opening->newInflow * UCF(FLOW);
    return(0);
}

int DLLEXPORT swmm_getNodeOpeningType(int nodeID, int idx, int *type)
//
// Input:   nodeID = Index of desired node
//          idx    = opening's index
// Output   type  = opening type to be output
// Return:  API Error
// Purpose: Get opening type.
{
    // Check if Open
    if(swmm_IsOpenFlag() == FALSE) return(ERR_API_INPUTNOTOPEN);
    // Check if object index is within bounds
    if (nodeID < 0 || nodeID >= Nobjects[NODE]) return(ERR_API_OBJECT_INDEX);

    TCoverOpening* opening;

    // --- check if an opening with this index exists
    opening = Node[nodeID].coverOpening;
    while ( opening )
    {
        if ( opening->ID == idx ) break;
        opening = opening->next;
    }
     // --- if it doesn't, return an error
    if ( opening == NULL ) return(ERR_API_OBJECT_INDEX);

    *type = opening->type;
    return(0);
}

int DLLEXPORT swmm_getOpeningCouplingType(int nodeID, int idx, int *coupling)
//
// Input:   nodeID   = Index of desired node
//          idx      = opening's index
// Output   coupling = coupling type to be output
// Return:  API Error
// Purpose: Get opening coupling type.
{
    // Check if Open
    if(swmm_IsOpenFlag() == FALSE) return(ERR_API_INPUTNOTOPEN);
    // Check if object index is within bounds
    if (nodeID < 0 || nodeID >= Nobjects[NODE]) return(ERR_API_OBJECT_INDEX);

    TCoverOpening* opening;

    // --- check if an opening with this index exists
    opening = Node[nodeID].coverOpening;
    while ( opening )
    {
        if ( opening->ID == idx ) break;
        opening = opening->next;
    }
     // --- if it doesn't, return an error
    if ( opening == NULL ) return(ERR_API_OBJECT_INDEX);

    *coupling = opening->couplingType;
    return(0);
}

int DLLEXPORT swmm_getOpeningsNum(int nodeID, int *num)
//
// Input:   nodeID   = Index of desired node
//          idx      = opening's index
// Output   coupling = coupling type to be output
// Return:  API Error
// Purpose: Get the number of openings of a node.
{
    // Check if Open
    if(swmm_IsOpenFlag() == FALSE) return(ERR_API_INPUTNOTOPEN);
    // Check if object index is within bounds
    if (nodeID < 0 || nodeID >= Nobjects[NODE]) return(ERR_API_OBJECT_INDEX);

    *num = coupling_countOpenings(nodeID);

    return(0);
}

int DLLEXPORT swmm_getOpeningsIndices(int nodeID, int arr_size, int *arr)
//
// Input:   nodeID   = Index of desired node
//          arr_size = number of openings
// Output   arr      = an array of size arr_size
// Return:  API Error
// Purpose: Get the indices of all the openings in the node.
{
    // Check if Open
    if(swmm_IsOpenFlag() == FALSE) return(ERR_API_INPUTNOTOPEN);
    // Check if object index is within bounds
    if (nodeID < 0 || nodeID >= Nobjects[NODE]) return(ERR_API_OBJECT_INDEX);
    // Check if array size is correct
    int num;
    swmm_getOpeningsNum(nodeID, &num);
    if (num != arr_size) return(ERR_API_OBJECT_INDEX);

    int i = 0;
    TCoverOpening* opening;

    opening = Node[nodeID].coverOpening;
    while ( opening )
    {
        if ( opening )
        {
            arr[i] = opening->ID;
            i++;
        }
        opening = opening->next;
    }

    return(0);
}

int DLLEXPORT swmm_getNodeIsCoupled(int nodeID, int *iscoupled)
//
// Input:   nodeID = Index of desired node
// Return:  API error
// Purpose: Get the coupling status of a node.
{
    // Check if Open
    if(swmm_IsOpenFlag() == FALSE) return(ERR_API_INPUTNOTOPEN);
    // Check if object index is within bounds
    if (nodeID < 0 || nodeID >= Nobjects[NODE]) return(ERR_API_OBJECT_INDEX);

    *iscoupled = coupling_isNodeCoupled(nodeID);

    return(0);
}


int DLLEXPORT swmm_closeOpening(int nodeID, int idx)
//
// Input:   nodeID = Index of desired node
// Return:  API error
// Purpose: Close an opening.
{
    int errcode = 0;

    // Check if Open
    if (swmm_IsOpenFlag() == FALSE)
    {
        errcode = ERR_API_INPUTNOTOPEN;
    }
    // Check if object index is within bounds
    else if (nodeID < 0 || nodeID >= Nobjects[NODE])
    {
        errcode = ERR_API_OBJECT_INDEX;
    }

    // Close the opening
    errcode = coupling_closeOpening(nodeID, idx);
    return errcode;
}


int DLLEXPORT swmm_openOpening(int nodeID, int idx)
//
// Input:   nodeID = Index of desired node
// Return:  API error
// Purpose: Open an opening.
{
    int errcode = 0;

    // Check if Open
    if (swmm_IsOpenFlag() == FALSE)
    {
        errcode = ERR_API_INPUTNOTOPEN;
    }
    // Check if object index is within bounds
    else if (nodeID < 0 || nodeID >= Nobjects[NODE])
    {
        errcode = ERR_API_OBJECT_INDEX;
    }

    // Close the opening
    errcode = coupling_openOpening(nodeID, idx);
    return errcode;
}

