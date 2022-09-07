//////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2002 Microsoft Corporation
//
//  Module Name:
//      BaseClusterGuids.h
//
//  Description:
//      This file contains the guids used by BaseCluster.
//
//  Maintained By:
//      John Franco (JFranco) 01-JUL-2002
//
//////////////////////////////////////////////////////////////////////////////


// Make sure that this file is included only once per compile path.
#pragma once

//////////////////////////////////////////////////////////////////////////////
// Constant Declarations
//////////////////////////////////////////////////////////////////////////////

#include <initguid.h>

//
//  Minor Task IDs
//

// {DBC51A1A-5099-42be-A806-D161AC4A3878}
DEFINE_GUID( TASKID_Minor_Commit_Forming_Node,
0xdbc51a1a, 0x5099, 0x42be, 0xa8, 0x6, 0xd1, 0x61, 0xac, 0x4a, 0x38, 0x78 );

// {11ABF069-6495-49ac-81ED-F27A1E4C5F3F}
DEFINE_GUID( TASKID_Minor_Commit_Joining_Node,
0x11abf069, 0x6495, 0x49ac, 0x81, 0xed, 0xf2, 0x7a, 0x1e, 0x4c, 0x5f, 0x3f );

// {3ABE1492-7E05-402c-81AA-1C3F1D782031}
DEFINE_GUID( TASKID_Minor_Configuring_Cluster_Node,
0x3abe1492, 0x7e05, 0x402c, 0x81, 0xaa, 0x1c, 0x3f, 0x1d, 0x78, 0x20, 0x31 );

// {3ABE1494-7E05-402c-81AA-1C3F1D782031}
DEFINE_GUID( TASKID_Minor_Rolling_Back_Cluster_Configuration,
0x3abe1494, 0x7e05, 0x402c, 0x81, 0xaa, 0x1c, 0x3f, 0x1d, 0x78, 0x20, 0x31 );

// {4DE4D086-3414-4621-BA1E-43277F972D12}
DEFINE_GUID( TASKID_Minor_Commit_Already_Complete,
0x4de4d086, 0x3414, 0x4621, 0xba, 0x1e, 0x43, 0x27, 0x7f, 0x97, 0x2d, 0x12 );

// {FFC1A75D-B6C0-4bb4-8625-02AD99ABF60E}
DEFINE_GUID( TASKID_Minor_Rollback_Failed_Incomplete_Commit,
0xffc1a75d, 0xb6c0, 0x4bb4, 0x86, 0x25, 0x2, 0xad, 0x99, 0xab, 0xf6, 0xe );

// {8B3DFF03-4DFC-41d5-B508-FC425CC1E5C4}
DEFINE_GUID( TASKID_Minor_Rollback_Not_Possible,
0x8b3dff03, 0x4dfc, 0x41d5, 0xb5, 0x8, 0xfc, 0x42, 0x5c, 0xc1, 0xe5, 0xc4 );

// {3ABE14A0-7E05-402c-81AA-1C3F1D782031}
DEFINE_GUID( TASKID_Minor_Cleaning_Up_Cluster_Database,
0x3abe14A0, 0x7e05, 0x402c, 0x81, 0xaa, 0x1c, 0x3f, 0x1d, 0x78, 0x20, 0x31 );

// {3ABE14A4-7E05-402c-81AA-1C3F1D782031}
DEFINE_GUID( TASKID_Minor_Form_Creating_Cluster_Database,
0x3abe14A4, 0x7e05, 0x402c, 0x81, 0xaa, 0x1c, 0x3f, 0x1d, 0x78, 0x20, 0x31 );

// {3ABE14A8-7E05-402c-81AA-1C3F1D782031}
DEFINE_GUID( TASKID_Minor_Form_Customizing_Cluster_Database,
0x3abe14A8, 0x7e05, 0x402c, 0x81, 0xaa, 0x1c, 0x3f, 0x1d, 0x78, 0x20, 0x31 );

// {3ABE14B0-7E05-402c-81AA-1C3F1D782031}
DEFINE_GUID( TASKID_Minor_Creating_ClusNet_Service,
0x3abe14B0, 0x7e05, 0x402c, 0x81, 0xaa, 0x1c, 0x3f, 0x1d, 0x78, 0x20, 0x31 );

// {3ABE14B4-7E05-402c-81AA-1C3F1D782031}
DEFINE_GUID( TASKID_Minor_Starting_ClusNet_Service,
0x3abe14B4, 0x7e05, 0x402c, 0x81, 0xaa, 0x1c, 0x3f, 0x1d, 0x78, 0x20, 0x31 );

// {3ABE14C0-7E05-402c-81AA-1C3F1D782031}
DEFINE_GUID( TASKID_Minor_Configuring_ClusDisk_Service,
0x3abe14C0, 0x7e05, 0x402c, 0x81, 0xaa, 0x1c, 0x3f, 0x1d, 0x78, 0x20, 0x31 );

// {3ABE14C4-7E05-402c-81AA-1C3F1D782031}
DEFINE_GUID( TASKID_Minor_Starting_ClusDisk_Service,
0x3abe14C4, 0x7e05, 0x402c, 0x81, 0xaa, 0x1c, 0x3f, 0x1d, 0x78, 0x20, 0x31 );

// {3ABE14D0-7E05-402c-81AA-1C3F1D782031}
DEFINE_GUID( TASKID_Minor_Configuring_Cluster_Service_Account,
0x3abe14D0, 0x7e05, 0x402c, 0x81, 0xaa, 0x1c, 0x3f, 0x1d, 0x78, 0x20, 0x31 );

// {4AA76355-C2B3-4f5b-87D6-5A11957E4280}
DEFINE_GUID( TASKID_Minor_Make_Cluster_Service_Account_Admin,
0x4aa76355, 0xc2b3, 0x4f5b, 0x87, 0xd6, 0x5a, 0x11, 0x95, 0x7e, 0x42, 0x80);

// {3ABE14E0-7E05-402c-81AA-1C3F1D782031}
DEFINE_GUID( TASKID_Minor_Creating_Cluster_Service,
0x3abe14E0, 0x7e05, 0x402c, 0x81, 0xaa, 0x1c, 0x3f, 0x1d, 0x78, 0x20, 0x31 );

// {3ABE14E4-7E05-402c-81AA-1C3F1D782031}
DEFINE_GUID( TASKID_Minor_Starting_Cluster_Service,
0x3abe14E4, 0x7e05, 0x402c, 0x81, 0xaa, 0x1c, 0x3f, 0x1d, 0x78, 0x20, 0x31 );

// {3ABE1500-7E05-402c-81AA-1C3F1D782031}
DEFINE_GUID( TASKID_Minor_Initializing_Cluster_Join,
0x3abe1500, 0x7e05, 0x402c, 0x81, 0xaa, 0x1c, 0x3f, 0x1d, 0x78, 0x20, 0x31 );

// {1D905EE7-3118-4c2c-BD61-2E4B9C109F71}
DEFINE_GUID( TASKID_Minor_Initializing_Cluster_Form,
0x1d905ee7, 0x3118, 0x4c2c, 0xbd, 0x61, 0x2e, 0x4b, 0x9c, 0x10, 0x9f, 0x71);

// {3ABE1518-7E05-402c-81AA-1C3F1D782031}
DEFINE_GUID( TASKID_Minor_Join_Sync_Cluster_Database,
0x3abe1518, 0x7e05, 0x402c, 0x81, 0xaa, 0x1c, 0x3f, 0x1d, 0x78, 0x20, 0x31 );
