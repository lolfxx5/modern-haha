/* msi.hh = help context identifiers for MSI API automation help */

#define HELPID_MsiInstall_Object               9000
#define HELPID_MsiInstall_UILevel              9001
#define HELPID_MsiInstall_EnableLog            9002
#define HELPID_MsiInstall_CreateRecord         9003
#define HELPID_MsiInstall_OpenPackage          9004
#define HELPID_MsiInstall_OpenProduct          9005
#define HELPID_MsiInstall_OpenDatabase         9006
#define HELPID_MsiInstall_SummaryInformation   9007
#define HELPID_MsiInstall_InstallProduct       9008
#define HELPID_MsiInstall_Version              9009
#define HELPID_MsiInstall_LastErrorRecord      9010
#define HELPID_MsiInstall_RegistryValue        9011
#define HELPID_MsiInstall_Environment          9012
#define HELPID_MsiInstall_FileAttributes       9013
#define HELPID_MsiInstall_FileSize             9014
#define HELPID_MsiInstall_FileVersion          9015
/*efine HELPID_MsiInstall_ExternalUI           9016*/
#define HELPID_MsiInstall_ProductState         9017
#define HELPID_MsiInstall_ProductInfo          9018
#define HELPID_MsiInstall_ConfigureProduct     9019
#define HELPID_MsiInstall_ReinstallProduct     9020
#define HELPID_MsiInstall_CollectUserInfo      9021
#define HELPID_MsiInstall_ApplyPatch           9022
#define HELPID_MsiInstall_FeatureParent        9023
#define HELPID_MsiInstall_FeatureState         9024
#define HELPID_MsiInstall_UseFeature           9025
#define HELPID_MsiInstall_FeatureUsageCount    9026
#define HELPID_MsiInstall_FeatureUsageDate     9027
#define HELPID_MsiInstall_ConfigureFeature     9028
#define HELPID_MsiInstall_ReinstallFeature     9029
#define HELPID_MsiInstall_ProvideComponent     9030
#define HELPID_MsiInstall_ComponentPath        9031
#define HELPID_MsiInstall_ProvideQualifiedComponent 9032
#define HELPID_MsiInstall_QualifierDescription 9033
#define HELPID_MsiInstall_ComponentQualifiers  9034
#define HELPID_MsiInstall_Products             9035
#define HELPID_MsiInstall_Features             9036
#define HELPID_MsiInstall_Components           9037
#define HELPID_MsiInstall_ComponentClients     9038
#define HELPID_MsiInstall_Patches              9039
#define HELPID_MsiInstall_RelatedProducts      9040
#define HELPID_MsiInstall_PatchInfo            9041
#define HELPID_MsiInstall_PatchTransforms      9042
#define HELPID_MsiInstall_AddSource            9043
#define HELPID_MsiInstall_ClearSourceList      9044
#define HELPID_MsiInstall_ForceSourceListResolution  9045
#define HELPID_MsiInstall_GetShortcutTarget    9046
#define HELPID_MsiInstall_FileHash             9047
#define HELPID_MsiInstall_FileSignatureInfo    9048

#define HELPID_MsiFeatureInfo_Object           9100
#define HELPID_MsiFeatureInfo_Title            9101
#define HELPID_MsiFeatureInfo_Description      9102
#define HELPID_MsiFeatureInfo_Attributes       9103

#define HELPID_MsiCollection_Object            9200
#define HELPID_MsiCollection_Item              9201
#define HELPID_MsiCollection_Count             9202
#define HELPID_MsiRecordCollection_Object      9203

#define HELPID_MsiRecord_Object                9300
#define HELPID_MsiRecord_FieldCount            9301
#define HELPID_MsiRecord_StringData            9302
#define HELPID_MsiRecord_IntegerData           9303
#define HELPID_MsiRecord_DataSize              9304
#define HELPID_MsiRecord_IsNull                9305
#define HELPID_MsiRecord_SetStream             9306
#define HELPID_MsiRecord_ReadStream            9307
#define HELPID_MsiRecord_ClearData             9308
#define HELPID_MsiRecord_FormatText            9309

#define HELPID_MsiView_Object                  9400
#define HELPID_MsiView_Execute                 9401
#define HELPID_MsiView_Fetch                   9402
#define HELPID_MsiView_Modify                  9403
#define HELPID_MsiView_Close                   9404
#define HELPID_MsiView_ColumnInfo              9405
#define HELPID_MsiView_GetError                9406

#define HELPID_MsiDatabase_Object              9500
#define HELPID_MsiDatabase_DatabaseState       9501
#define HELPID_MsiDatabase_SummaryInformation  9502
#define HELPID_MsiDatabase_OpenView            9503
#define HELPID_MsiDatabase_Commit              9504
#define HELPID_MsiDatabase_PrimaryKeys         9505
#define HELPID_MsiDatabase_Import              9506
#define HELPID_MsiDatabase_Export              9507
#define HELPID_MsiDatabase_Merge               9508
#define HELPID_MsiDatabase_GenerateTransform   9509
#define HELPID_MsiDatabase_ApplyTransform      9510
#define HELPID_MsiDatabase_TablePersistent     9511
#define HELPID_MsiDatabase_EnableUIPreview     9512
#define HELPID_MsiDatabase_CreateTransformSummaryInfo 9513

#define HELPID_MsiSummaryInfo_Object           9600
#define HELPID_MsiSummaryInfo_Property         9601
#define HELPID_MsiSummaryInfo_PropertyCount    9602
#define HELPID_MsiSummaryInfo_Persist          9603

#define HELPID_MsiEngine_Object                9700
#define HELPID_MsiEngine_Application           9701
#define HELPID_MsiEngine_Property              9702
#define HELPID_MsiEngine_Language              9703
#define HELPID_MsiEngine_Mode                  9704
#define HELPID_MsiEngine_Database              9705
#define HELPID_MsiEngine_DoAction              9706
#define HELPID_MsiEngine_Sequence              9707
#define HELPID_MsiEngine_Message               9708
#define HELPID_MsiEngine_FormatRecord          9709
#define HELPID_MsiEngine_EvaluateCondition     9710
#define HELPID_MsiEngine_SourcePath            9711
#define HELPID_MsiEngine_TargetPath            9712
#define HELPID_MsiEngine_FeatureCurrentState   9713
#define HELPID_MsiEngine_FeatureRequestState   9714
#define HELPID_MsiEngine_FeatureValidStates    9715
#define HELPID_MsiEngine_FeatureCost           9716
#define HELPID_MsiEngine_ComponentCurrentState 9717
#define HELPID_MsiEngine_ComponentRequestState 9718
#define HELPID_MsiEngine_SetInstallLevel       9719
#define HELPID_MsiEngine_VerifyDiskSpace       9720
#define HELPID_MsiEngine_ProductProperty       9721
#define HELPID_MsiEngine_FeatureInfo           9722
#define HELPID_MsiEngine_ComponentCosts        9723

#define HELPID_MsiUIPreview_Object             9800
#define HELPID_MsiUIPreview_ViewDialog         9801
#define HELPID_MsiUIPreview_ViewBillboard      9802
/*efine HELPID_MsiUIPreview_Property    HELPID_MsiEngine_Property */

#define HELPID_MsiInstallState                 9901
#define HELPID_MsiReinstallMode                9902
#define HELPID_MsiInstallMode                  9903
