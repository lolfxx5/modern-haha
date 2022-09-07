<?xml version="1.0" encoding="UTF-16"?>
<!DOCTYPE DCARRIER SYSTEM "mantis.dtd">
<DCARRIER CarrierRevision="1">
	<TOOLINFO ToolName="iCat"><![CDATA[<?xml version="1.0"?>
<!DOCTYPE TOOL SYSTEM "tool.dtd">
<TOOL>
	<CREATED><NAME>Component Designer</NAME><VERSION>1.0.0.438</VERSION><BUILD>438</BUILD><DATE>12/18/2000</DATE></CREATED><LASTSAVED><NAME>iCat</NAME><VSGUID>{97b86ee0-259c-479f-bc46-6cea7ef4be4d}</VSGUID><VERSION>1.0.0.452</VERSION><BUILD>452</BUILD><DATE>6/19/2001</DATE></LASTSAVED></TOOL>
]]></TOOLINFO><COMPONENT Revision="7" Visibility="1000" MultiInstance="0" Released="1" Editable="1" HTMLFinal="0" ComponentVSGUID="{B0106027-9388-462F-9E09-5E75215F1730}" ComponentVIGUID="{37AE8550-B062-42F3-9580-3565190510CF}" PlatformGUID="{B784E719-C196-4DDB-B358-D9254426C38D}" RepositoryVSGUID="{8E0BE9ED-7649-47F3-810B-232D36C430B4}"><DISPLAYNAME>Event Log</DISPLAYNAME><VERSION>1.0</VERSION><DESCRIPTION>Logs event messages issued by programs and Windows.</DESCRIPTION><COPYRIGHT>2000 Microsoft Corp.</COPYRIGHT><VENDOR>Microsoft Corp.</VENDOR><OWNERS>drbeck</OWNERS><AUTHORS>drbeck; shbrown</AUTHORS><DATECREATED>12/18/2000</DATECREATED><DATEREVISED>6/19/2001</DATEREVISED><RESOURCE ResTypeVSGUID="{90D8E195-E710-4AF6-B667-B1805FFC9B8F}" BuildTypeMask="273" Name="RawDep(819):&quot;File&quot;,&quot;USER32.DLL&quot;" Localize="0"><PROPERTY Name="ComponentVSGUID" Format="GUID">{00000000-0000-0000-0000-000000000000}</PROPERTY><PROPERTY Name="RawType" Format="String">File</PROPERTY><PROPERTY Name="Value" Format="String">USER32.DLL</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{90D8E195-E710-4AF6-B667-B1805FFC9B8F}" BuildTypeMask="273" Name="RawDep(819):&quot;File&quot;,&quot;RPCRT4.DLL&quot;" Localize="0"><PROPERTY Name="ComponentVSGUID" Format="GUID">{00000000-0000-0000-0000-000000000000}</PROPERTY><PROPERTY Name="RawType" Format="String">File</PROPERTY><PROPERTY Name="Value" Format="String">RPCRT4.DLL</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{E66B49F6-4A35-4246-87E8-5C1A468315B5}" BuildTypeMask="819" Name="File(819):&quot;%11%&quot;,&quot;eventlog.dll&quot;"><PROPERTY Name="DstPath" Format="String">%11%</PROPERTY><PROPERTY Name="DstName" Format="String">eventlog.dll</PROPERTY><PROPERTY Name="NoExpand" Format="Boolean">0</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{90D8E195-E710-4AF6-B667-B1805FFC9B8F}" BuildTypeMask="819" Name="RawDep(819):&quot;File&quot;,&quot;NTDLL.DLL&quot;"><PROPERTY Name="RawType" Format="String">File</PROPERTY><PROPERTY Name="Value" Format="String">NTDLL.DLL</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{90D8E195-E710-4AF6-B667-B1805FFC9B8F}" BuildTypeMask="819" Name="RawDep(819):&quot;File&quot;,&quot;KERNEL32.DLL&quot;"><PROPERTY Name="RawType" Format="String">File</PROPERTY><PROPERTY Name="Value" Format="String">KERNEL32.DLL</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{90D8E195-E710-4AF6-B667-B1805FFC9B8F}" BuildTypeMask="819" Name="RawDep(819):&quot;File&quot;,&quot;MSVCRT.DLL&quot;"><PROPERTY Name="RawType" Format="String">File</PROPERTY><PROPERTY Name="Value" Format="String">MSVCRT.DLL</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{2C10DB69-39AB-48A4-A83F-9AB3ACBA7C45}" BuildTypeMask="819" Name="RegKey(819):&quot;HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog&quot;,&quot;DisplayName&quot;" Localize="0"><PROPERTY Name="KeyPath" Format="String">HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog</PROPERTY><PROPERTY Name="ValueName" Format="String">DisplayName</PROPERTY><PROPERTY Name="RegValue" Format="String">Event Log</PROPERTY><PROPERTY Name="RegType" Format="Integer">1</PROPERTY><PROPERTY Name="RegOp" Format="Integer">1</PROPERTY><PROPERTY Name="RegCond" Format="Integer">1</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{2C10DB69-39AB-48A4-A83F-9AB3ACBA7C45}" BuildTypeMask="819" Name="RegKey(819):&quot;HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog&quot;,&quot;Description&quot;" Localize="0"><PROPERTY Name="KeyPath" Format="String">HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog</PROPERTY><PROPERTY Name="ValueName" Format="String">Description</PROPERTY><PROPERTY Name="RegValue" Format="String">Logs event messages issued by programs and Windows.  Event Log reports contain information that can be useful in diagnosing problems.  Reports are viewed in Event Viewer.</PROPERTY><PROPERTY Name="RegType" Format="Integer">1</PROPERTY><PROPERTY Name="RegOp" Format="Integer">1</PROPERTY><PROPERTY Name="RegCond" Format="Integer">1</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{2C10DB69-39AB-48A4-A83F-9AB3ACBA7C45}" BuildTypeMask="819" Name="RegKey(819):&quot;HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog&quot;" Localize="0"><PROPERTY Name="KeyPath" Format="String">HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog</PROPERTY><PROPERTY Name="RegType" Format="Integer">1</PROPERTY><PROPERTY Name="RegOp" Format="Integer">1</PROPERTY><PROPERTY Name="RegCond" Format="Integer">1</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{2C10DB69-39AB-48A4-A83F-9AB3ACBA7C45}" BuildTypeMask="819" Name="RegKey(819):&quot;HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog&quot;,&quot;Type&quot;" Localize="0"><PROPERTY Name="KeyPath" Format="String">HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog</PROPERTY><PROPERTY Name="ValueName" Format="String">Type</PROPERTY><PROPERTY Name="RegValue" Format="Integer">32</PROPERTY><PROPERTY Name="RegType" Format="Integer">4</PROPERTY><PROPERTY Name="RegOp" Format="Integer">1</PROPERTY><PROPERTY Name="RegCond" Format="Integer">1</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{2C10DB69-39AB-48A4-A83F-9AB3ACBA7C45}" BuildTypeMask="819" Name="RegKey(819):&quot;HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog&quot;,&quot;Start&quot;" Localize="0"><PROPERTY Name="KeyPath" Format="String">HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog</PROPERTY><PROPERTY Name="ValueName" Format="String">Start</PROPERTY><PROPERTY Name="RegValue" Format="Integer">2</PROPERTY><PROPERTY Name="RegType" Format="Integer">4</PROPERTY><PROPERTY Name="RegOp" Format="Integer">1</PROPERTY><PROPERTY Name="RegCond" Format="Integer">1</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{2C10DB69-39AB-48A4-A83F-9AB3ACBA7C45}" BuildTypeMask="819" Name="RegKey(819):&quot;HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog&quot;,&quot;PlugPlayServiceType&quot;" Localize="0"><PROPERTY Name="KeyPath" Format="String">HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog</PROPERTY><PROPERTY Name="ValueName" Format="String">PlugPlayServiceType</PROPERTY><PROPERTY Name="RegValue" Format="Integer">3</PROPERTY><PROPERTY Name="RegType" Format="Integer">4</PROPERTY><PROPERTY Name="RegOp" Format="Integer">1</PROPERTY><PROPERTY Name="RegCond" Format="Integer">1</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{2C10DB69-39AB-48A4-A83F-9AB3ACBA7C45}" BuildTypeMask="819" Name="RegKey(819):&quot;HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog&quot;,&quot;ObjectName&quot;" Localize="0"><PROPERTY Name="KeyPath" Format="String">HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog</PROPERTY><PROPERTY Name="ValueName" Format="String">ObjectName</PROPERTY><PROPERTY Name="RegValue" Format="String">LocalSystem</PROPERTY><PROPERTY Name="RegType" Format="Integer">1</PROPERTY><PROPERTY Name="RegOp" Format="Integer">1</PROPERTY><PROPERTY Name="RegCond" Format="Integer">1</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{2C10DB69-39AB-48A4-A83F-9AB3ACBA7C45}" BuildTypeMask="819" Name="RegKey(819):&quot;HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog&quot;,&quot;ImagePath&quot;" Localize="0"><PROPERTY Name="KeyPath" Format="String">HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog</PROPERTY><PROPERTY Name="ValueName" Format="String">ImagePath</PROPERTY><PROPERTY Name="RegValue" Format="String">%SystemRoot%\system32\services.exe</PROPERTY><PROPERTY Name="RegType" Format="Integer">2</PROPERTY><PROPERTY Name="RegOp" Format="Integer">1</PROPERTY><PROPERTY Name="RegCond" Format="Integer">1</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{2C10DB69-39AB-48A4-A83F-9AB3ACBA7C45}" BuildTypeMask="819" Name="RegKey(819):&quot;HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog&quot;,&quot;Group&quot;" Localize="0"><PROPERTY Name="KeyPath" Format="String">HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog</PROPERTY><PROPERTY Name="ValueName" Format="String">Group</PROPERTY><PROPERTY Name="RegValue" Format="String">Event log</PROPERTY><PROPERTY Name="RegType" Format="Integer">1</PROPERTY><PROPERTY Name="RegOp" Format="Integer">1</PROPERTY><PROPERTY Name="RegCond" Format="Integer">1</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{2C10DB69-39AB-48A4-A83F-9AB3ACBA7C45}" BuildTypeMask="819" Name="RegKey(819):&quot;HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog&quot;,&quot;ErrorControl&quot;" Localize="0"><PROPERTY Name="KeyPath" Format="String">HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog</PROPERTY><PROPERTY Name="ValueName" Format="String">ErrorControl</PROPERTY><PROPERTY Name="RegValue" Format="Integer">1</PROPERTY><PROPERTY Name="RegType" Format="Integer">4</PROPERTY><PROPERTY Name="RegOp" Format="Integer">1</PROPERTY><PROPERTY Name="RegCond" Format="Integer">1</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{90D8E195-E710-4AF6-B667-B1805FFC9B8F}" BuildTypeMask="819" Name="RawDep(819):&quot;File&quot;,&quot;netevent.dll&quot;"><PROPERTY Name="RawType" Format="String">File</PROPERTY><PROPERTY Name="Value" Format="String">netevent.dll</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{2C10DB69-39AB-48A4-A83F-9AB3ACBA7C45}" BuildTypeMask="819" Name="RegKey(819):&quot;HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog\Application&quot;,&quot;DisplayNameFile&quot;" Localize="0"><PROPERTY Name="KeyPath" Format="String">HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog\Application</PROPERTY><PROPERTY Name="ValueName" Format="String">DisplayNameFile</PROPERTY><PROPERTY Name="RegValue" Format="String">%SystemRoot%\system32\els.dll</PROPERTY><PROPERTY Name="RegType" Format="Integer">2</PROPERTY><PROPERTY Name="RegOp" Format="Integer">1</PROPERTY><PROPERTY Name="RegCond" Format="Integer">1</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{2C10DB69-39AB-48A4-A83F-9AB3ACBA7C45}" BuildTypeMask="819" Name="RegKey(819):&quot;HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog\Application&quot;,&quot;DisplayNameID&quot;" Localize="0"><PROPERTY Name="KeyPath" Format="String">HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog\Application</PROPERTY><PROPERTY Name="ValueName" Format="String">DisplayNameID</PROPERTY><PROPERTY Name="RegValue" Format="Integer">256</PROPERTY><PROPERTY Name="RegType" Format="Integer">4</PROPERTY><PROPERTY Name="RegOp" Format="Integer">1</PROPERTY><PROPERTY Name="RegCond" Format="Integer">1</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{2C10DB69-39AB-48A4-A83F-9AB3ACBA7C45}" BuildTypeMask="819" Name="RegKey(819):&quot;HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog\Application&quot;,&quot;File&quot;" Localize="0"><PROPERTY Name="KeyPath" Format="String">HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog\Application</PROPERTY><PROPERTY Name="ValueName" Format="String">File</PROPERTY><PROPERTY Name="RegValue" Format="String">%SystemRoot%\system32\config\AppEvent.Evt</PROPERTY><PROPERTY Name="RegType" Format="Integer">2</PROPERTY><PROPERTY Name="RegOp" Format="Integer">1</PROPERTY><PROPERTY Name="RegCond" Format="Integer">1</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{2C10DB69-39AB-48A4-A83F-9AB3ACBA7C45}" BuildTypeMask="819" Name="RegKey(819):&quot;HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog\Application&quot;,&quot;MaxSize&quot;" Localize="0"><PROPERTY Name="KeyPath" Format="String">HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog\Application</PROPERTY><PROPERTY Name="ValueName" Format="String">MaxSize</PROPERTY><PROPERTY Name="RegValue" Format="Integer">5046272</PROPERTY><PROPERTY Name="RegType" Format="Integer">4</PROPERTY><PROPERTY Name="RegOp" Format="Integer">1</PROPERTY><PROPERTY Name="RegCond" Format="Integer">1</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{2C10DB69-39AB-48A4-A83F-9AB3ACBA7C45}" BuildTypeMask="819" Name="RegKey(819):&quot;HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog\Application&quot;,&quot;PrimaryModule&quot;" Localize="0"><PROPERTY Name="KeyPath" Format="String">HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog\Application</PROPERTY><PROPERTY Name="ValueName" Format="String">PrimaryModule</PROPERTY><PROPERTY Name="RegValue" Format="String">Application</PROPERTY><PROPERTY Name="RegType" Format="Integer">1</PROPERTY><PROPERTY Name="RegOp" Format="Integer">1</PROPERTY><PROPERTY Name="RegCond" Format="Integer">1</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{2C10DB69-39AB-48A4-A83F-9AB3ACBA7C45}" BuildTypeMask="819" Name="RegKey(819):&quot;HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog\Application&quot;,&quot;Retention&quot;" Localize="0"><PROPERTY Name="KeyPath" Format="String">HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog\Application</PROPERTY><PROPERTY Name="ValueName" Format="String">Retention</PROPERTY><PROPERTY Name="RegValue" Format="Integer">0</PROPERTY><PROPERTY Name="RegType" Format="Integer">4</PROPERTY><PROPERTY Name="RegOp" Format="Integer">1</PROPERTY><PROPERTY Name="RegCond" Format="Integer">1</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{2C10DB69-39AB-48A4-A83F-9AB3ACBA7C45}" BuildTypeMask="819" Name="RegKey(819):&quot;HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog\Security&quot;,&quot;DisplayNameFile&quot;" Localize="0"><PROPERTY Name="KeyPath" Format="String">HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog\Security</PROPERTY><PROPERTY Name="ValueName" Format="String">DisplayNameFile</PROPERTY><PROPERTY Name="RegValue" Format="String">%SystemRoot%\system32\els.dll</PROPERTY><PROPERTY Name="RegType" Format="Integer">2</PROPERTY><PROPERTY Name="RegOp" Format="Integer">1</PROPERTY><PROPERTY Name="RegCond" Format="Integer">1</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{2C10DB69-39AB-48A4-A83F-9AB3ACBA7C45}" BuildTypeMask="819" Name="RegKey(819):&quot;HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog\Security&quot;,&quot;DisplayNameID&quot;" Localize="0"><PROPERTY Name="KeyPath" Format="String">HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog\Security</PROPERTY><PROPERTY Name="ValueName" Format="String">DisplayNameID</PROPERTY><PROPERTY Name="RegValue" Format="Integer">257</PROPERTY><PROPERTY Name="RegType" Format="Integer">4</PROPERTY><PROPERTY Name="RegOp" Format="Integer">1</PROPERTY><PROPERTY Name="RegCond" Format="Integer">1</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{2C10DB69-39AB-48A4-A83F-9AB3ACBA7C45}" BuildTypeMask="819" Name="RegKey(819):&quot;HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog\Security&quot;,&quot;File&quot;" Localize="0"><PROPERTY Name="KeyPath" Format="String">HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog\Security</PROPERTY><PROPERTY Name="ValueName" Format="String">File</PROPERTY><PROPERTY Name="RegValue" Format="String">%SystemRoot%\System32\config\SecEvent.Evt</PROPERTY><PROPERTY Name="RegType" Format="Integer">2</PROPERTY><PROPERTY Name="RegOp" Format="Integer">1</PROPERTY><PROPERTY Name="RegCond" Format="Integer">1</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{2C10DB69-39AB-48A4-A83F-9AB3ACBA7C45}" BuildTypeMask="819" Name="RegKey(819):&quot;HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog\Security&quot;,&quot;MaxSize&quot;" Localize="0"><PROPERTY Name="KeyPath" Format="String">HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog\Security</PROPERTY><PROPERTY Name="ValueName" Format="String">MaxSize</PROPERTY><PROPERTY Name="RegValue" Format="Integer">5046272</PROPERTY><PROPERTY Name="RegType" Format="Integer">4</PROPERTY><PROPERTY Name="RegOp" Format="Integer">1</PROPERTY><PROPERTY Name="RegCond" Format="Integer">1</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{2C10DB69-39AB-48A4-A83F-9AB3ACBA7C45}" BuildTypeMask="819" Name="RegKey(819):&quot;HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog\Security&quot;,&quot;PrimaryModule&quot;" Localize="0"><PROPERTY Name="KeyPath" Format="String">HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog\Security</PROPERTY><PROPERTY Name="ValueName" Format="String">PrimaryModule</PROPERTY><PROPERTY Name="RegValue" Format="String">Security</PROPERTY><PROPERTY Name="RegType" Format="Integer">1</PROPERTY><PROPERTY Name="RegOp" Format="Integer">1</PROPERTY><PROPERTY Name="RegCond" Format="Integer">1</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{2C10DB69-39AB-48A4-A83F-9AB3ACBA7C45}" BuildTypeMask="819" Name="RegKey(819):&quot;HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog\Security&quot;,&quot;Retention&quot;" Localize="0"><PROPERTY Name="KeyPath" Format="String">HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog\Security</PROPERTY><PROPERTY Name="ValueName" Format="String">Retention</PROPERTY><PROPERTY Name="RegValue" Format="Integer">0</PROPERTY><PROPERTY Name="RegType" Format="Integer">4</PROPERTY><PROPERTY Name="RegOp" Format="Integer">1</PROPERTY><PROPERTY Name="RegCond" Format="Integer">1</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{2C10DB69-39AB-48A4-A83F-9AB3ACBA7C45}" BuildTypeMask="819" Name="RegKey(819):&quot;HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog\System&quot;,&quot;DisplayNameFile&quot;" Localize="0"><PROPERTY Name="KeyPath" Format="String">HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog\System</PROPERTY><PROPERTY Name="ValueName" Format="String">DisplayNameFile</PROPERTY><PROPERTY Name="RegValue" Format="String">%SystemRoot%\system32\els.dll</PROPERTY><PROPERTY Name="RegType" Format="Integer">2</PROPERTY><PROPERTY Name="RegOp" Format="Integer">1</PROPERTY><PROPERTY Name="RegCond" Format="Integer">1</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{2C10DB69-39AB-48A4-A83F-9AB3ACBA7C45}" BuildTypeMask="819" Name="RegKey(819):&quot;HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog\System&quot;,&quot;DisplayNameID&quot;" Localize="0"><PROPERTY Name="KeyPath" Format="String">HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog\System</PROPERTY><PROPERTY Name="ValueName" Format="String">DisplayNameID</PROPERTY><PROPERTY Name="RegValue" Format="Integer">258</PROPERTY><PROPERTY Name="RegType" Format="Integer">4</PROPERTY><PROPERTY Name="RegOp" Format="Integer">1</PROPERTY><PROPERTY Name="RegCond" Format="Integer">1</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{2C10DB69-39AB-48A4-A83F-9AB3ACBA7C45}" BuildTypeMask="819" Name="RegKey(819):&quot;HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog\System&quot;,&quot;File&quot;" Localize="0"><PROPERTY Name="KeyPath" Format="String">HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog\System</PROPERTY><PROPERTY Name="ValueName" Format="String">File</PROPERTY><PROPERTY Name="RegValue" Format="String">%SystemRoot%\system32\config\SysEvent.Evt</PROPERTY><PROPERTY Name="RegType" Format="Integer">2</PROPERTY><PROPERTY Name="RegOp" Format="Integer">1</PROPERTY><PROPERTY Name="RegCond" Format="Integer">1</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{2C10DB69-39AB-48A4-A83F-9AB3ACBA7C45}" BuildTypeMask="819" Name="RegKey(819):&quot;HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog\System&quot;,&quot;MaxSize&quot;" Localize="0"><PROPERTY Name="KeyPath" Format="String">HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog\System</PROPERTY><PROPERTY Name="ValueName" Format="String">MaxSize</PROPERTY><PROPERTY Name="RegValue" Format="Integer">5046272</PROPERTY><PROPERTY Name="RegType" Format="Integer">4</PROPERTY><PROPERTY Name="RegOp" Format="Integer">1</PROPERTY><PROPERTY Name="RegCond" Format="Integer">1</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{2C10DB69-39AB-48A4-A83F-9AB3ACBA7C45}" BuildTypeMask="819" Name="RegKey(819):&quot;HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog\System&quot;,&quot;PrimaryModule&quot;" Localize="0"><PROPERTY Name="KeyPath" Format="String">HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog\System</PROPERTY><PROPERTY Name="ValueName" Format="String">PrimaryModule</PROPERTY><PROPERTY Name="RegValue" Format="String">System</PROPERTY><PROPERTY Name="RegType" Format="Integer">1</PROPERTY><PROPERTY Name="RegOp" Format="Integer">1</PROPERTY><PROPERTY Name="RegCond" Format="Integer">1</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{2C10DB69-39AB-48A4-A83F-9AB3ACBA7C45}" BuildTypeMask="819" Name="RegKey(819):&quot;HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog\System&quot;,&quot;Retention&quot;" Localize="0"><PROPERTY Name="KeyPath" Format="String">HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog\System</PROPERTY><PROPERTY Name="ValueName" Format="String">Retention</PROPERTY><PROPERTY Name="RegValue" Format="Integer">0</PROPERTY><PROPERTY Name="RegType" Format="Integer">4</PROPERTY><PROPERTY Name="RegOp" Format="Integer">1</PROPERTY><PROPERTY Name="RegCond" Format="Integer">1</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{90D8E195-E710-4AF6-B667-B1805FFC9B8F}" BuildTypeMask="819" Name="RawDep(819):&quot;File&quot;,&quot;els.dll&quot;"><PROPERTY Name="RawType" Format="String">File</PROPERTY><PROPERTY Name="Value" Format="String">els.dll</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{90D8E195-E710-4AF6-B667-B1805FFC9B8F}" BuildTypeMask="819" Name="RawDep(819):&quot;File&quot;,&quot;advapi32.dll&quot;"><PROPERTY Name="RawType" Format="String">File</PROPERTY><PROPERTY Name="Value" Format="String">advapi32.dll</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{90D8E195-E710-4AF6-B667-B1805FFC9B8F}" BuildTypeMask="819" Name="RawDep(819):&quot;File&quot;,&quot;WS2_32.dll&quot;"><PROPERTY Name="RawType" Format="String">File</PROPERTY><PROPERTY Name="Value" Format="String">WS2_32.dll</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{90D8E195-E710-4AF6-B667-B1805FFC9B8F}" BuildTypeMask="819" Name="RawDep(819):&quot;File&quot;,&quot;PSAPI.DLL&quot;"><PROPERTY Name="RawType" Format="String">File</PROPERTY><PROPERTY Name="Value" Format="String">PSAPI.DLL</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{90D8E195-E710-4AF6-B667-B1805FFC9B8F}" BuildTypeMask="819" Name="RawDep(819):&quot;File&quot;,&quot;NETAPI32.dll&quot;"><PROPERTY Name="RawType" Format="String">File</PROPERTY><PROPERTY Name="Value" Format="String">NETAPI32.dll</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{90D8E195-E710-4AF6-B667-B1805FFC9B8F}" BuildTypeMask="819" Name="RawDep(819):&quot;File&quot;,&quot;netmsg.dll&quot;"><PROPERTY Name="RawType" Format="String">File</PROPERTY><PROPERTY Name="Value" Format="String">netmsg.dll</PROPERTY></RESOURCE><GROUPMEMBER GroupVSGUID="{E01B4103-3883-4FE8-992F-10566E7B796C}"/><GROUPMEMBER GroupVSGUID="{64668FB9-9289-45F0-BEF9-23745D272E3D}"/></COMPONENT></DCARRIER>
