# --------- DO NOT EDIT THIS FILE ------------ DO NOT EDIT THIS FILE ---------
# --------- DO NOT EDIT --- see layout.txt --- DO NOT EDIT -------------------
# --------- DO NOT EDIT THIS FILE ------------ DO NOT EDIT THIS FILE ---------
$(O)\sdkmake.inc : sdkmake.inc
	copy $** $@
	$(BINPLACE_CMD)
$(O)\sdkpost.mak : sdkpost.mak
	copy $** $@
	$(BINPLACE_CMD)
$(O)\msisdk.msi : $(BUILD_PACKAGEDIR)\msisdk.msi
	copy $** $@
	$(BINPLACE_CMD)
$(O)\ReadMe.txt : $(SDXROOT)\admin\darwin\src\sdk\ReadMe.txt
	copy $** $@
	$(BINPLACE_CMD)
$(O)\Redist.txt : $(SDXROOT)\admin\darwin\src\sdk\Redist.txt
	copy $** $@
	$(BINPLACE_CMD)
$(O)\License.rtf : $(SDXROOT)\admin\darwin\src\sdk\License.rtf
	copy $** $@
	$(BINPLACE_CMD)
$(O)\INSTLR1.ADM : $(SDXROOT)\admin\darwin\src\sdk\INSTLR1.ADM
	copy $** $@
	$(BINPLACE_CMD)
$(O)\instlr11.adm : $(SDXROOT)\admin\darwin\src\sdk\instlr11.adm
	copy $** $@
	$(BINPLACE_CMD)
$(O)\msi.h : $(SDXROOT)\public\sdk\inc\msi.h
	copy $** $@
	$(BINPLACE_CMD)
$(O)\msiquery.h : $(SDXROOT)\public\sdk\inc\msiquery.h
	copy $** $@
	$(BINPLACE_CMD)
$(O)\msidefs.h : $(SDXROOT)\public\sdk\inc\msidefs.h
	copy $** $@
	$(BINPLACE_CMD)
$(O)\patchwiz.h : $(SDXROOT)\admin\darwin\src\msitools\patchwiz\patchwiz.h
	copy $** $@
	$(BINPLACE_CMD)
$(O)\patchapi.h : $(SDXROOT)\public\sdk\inc\patchapi.h
	copy $** $@
	$(BINPLACE_CMD)
$(O)\Mergemod.h : $(SDXROOT)\admin\darwin\build\common\obj\i386\Mergemod.h
	copy $** $@
	$(BINPLACE_CMD)
$(O)\msi.lib : $(SDXROOT)\public\sdk\lib\i386\msi.lib
	copy $** $@
	$(BINPLACE_CMD)
$(O)\mspatchc.lib : $(SDXROOT)\public\sdk\lib\i386\mspatchc.lib
	copy $** $@
	$(BINPLACE_CMD)
$(O)\msi.chm : $(SDXROOT)\admin\darwin\doc\msi.chm
	copy $** $@
	$(BINPLACE_CMD)
$(O)\msi.chi : $(SDXROOT)\admin\darwin\doc\msi.chi
	copy $** $@
	$(BINPLACE_CMD)
$(O)\msitool.mak : $(SDXROOT)\admin\darwin\src\msitools\msitool.mak
	copy $** $@
	$(BINPLACE_CMD)
$(O)\darice.cub : $(SDXROOT)\admin\darwin\data\archive\cubes\110\darice.cub
	copy $** $@
	$(BINPLACE_CMD)
$(O)\logo.cub : $(SDXROOT)\admin\darwin\data\archive\cubes\110\logo.cub
	copy $** $@
	$(BINPLACE_CMD)
$(O)\mergemod.cub : $(SDXROOT)\admin\darwin\data\archive\cubes\110\mergemod.cub
	copy $** $@
	$(BINPLACE_CMD)
$(O)\darice.cub1 : $(SDXROOT)\admin\darwin\data\archive\cubes\120\darice.cub
	copy $** $@
	$(BINPLACE_CMD)
$(O)\logo.cub1 : $(SDXROOT)\admin\darwin\data\archive\cubes\120\logo.cub
	copy $** $@
	$(BINPLACE_CMD)
$(O)\mergemod.cub1 : $(SDXROOT)\admin\darwin\data\archive\cubes\120\mergemod.cub
	copy $** $@
	$(BINPLACE_CMD)
$(O)\preadme.txt : $(SDXROOT)\admin\darwin\src\sdk\preadme.txt
	copy $** $@
	$(BINPLACE_CMD)
$(O)\makecab.exe : $(SDXROOT)\admin\darwin\bin\i386\makecab.exe
	copy $** $@
	$(BINPLACE_CMD)
$(O)\TEMPLATE.PCP : $(SDXROOT)\admin\darwin\src\msitools\PatchWiz\TEMPLATE.PCP
	copy $** $@
	$(BINPLACE_CMD)
$(O)\EXAMPLE.PCP : $(SDXROOT)\admin\darwin\src\msitools\PatchWiz\EXAMPLE.PCP
	copy $** $@
	$(BINPLACE_CMD)
$(O)\WiReadMe.txt : $(SDXROOT)\admin\darwin\src\msitools\scripts\WiReadMe.txt
	copy $** $@
	$(BINPLACE_CMD)
$(O)\WiCompon.vbs : $(SDXROOT)\admin\darwin\src\msitools\scripts\WiCompon.vbs
	copy $** $@
	$(BINPLACE_CMD)
$(O)\WiDialog.vbs : $(SDXROOT)\admin\darwin\src\msitools\scripts\WiDialog.vbs
	copy $** $@
	$(BINPLACE_CMD)
$(O)\WiDiffDb.vbs : $(SDXROOT)\admin\darwin\src\msitools\scripts\WiDiffDb.vbs
	copy $** $@
	$(BINPLACE_CMD)
$(O)\WiExport.vbs : $(SDXROOT)\admin\darwin\src\msitools\scripts\WiExport.vbs
	copy $** $@
	$(BINPLACE_CMD)
$(O)\WiFilVer.vbs : $(SDXROOT)\admin\darwin\src\msitools\scripts\WiFilVer.vbs
	copy $** $@
	$(BINPLACE_CMD)
$(O)\WiFeatur.vbs : $(SDXROOT)\admin\darwin\src\msitools\scripts\WiFeatur.vbs
	copy $** $@
	$(BINPLACE_CMD)
$(O)\WiGenXfm.vbs : $(SDXROOT)\admin\darwin\src\msitools\scripts\WiGenXfm.vbs
	copy $** $@
	$(BINPLACE_CMD)
$(O)\WiImport.vbs : $(SDXROOT)\admin\darwin\src\msitools\scripts\WiImport.vbs
	copy $** $@
	$(BINPLACE_CMD)
$(O)\WiLangId.vbs : $(SDXROOT)\admin\darwin\src\msitools\scripts\WiLangId.vbs
	copy $** $@
	$(BINPLACE_CMD)
$(O)\WiLstPrd.vbs : $(SDXROOT)\admin\darwin\src\msitools\scripts\WiLstPrd.vbs
	copy $** $@
	$(BINPLACE_CMD)
$(O)\WiLstScr.vbs : $(SDXROOT)\admin\darwin\src\msitools\scripts\WiLstScr.vbs
	copy $** $@
	$(BINPLACE_CMD)
$(O)\WiLstXfm.vbs : $(SDXROOT)\admin\darwin\src\msitools\scripts\WiLstXfm.vbs
	copy $** $@
	$(BINPLACE_CMD)
$(O)\WiMakCab.vbs : $(SDXROOT)\admin\darwin\src\msitools\scripts\WiMakCab.vbs
	copy $** $@
	$(BINPLACE_CMD)
$(O)\WiMerge.vbs : $(SDXROOT)\admin\darwin\src\msitools\scripts\WiMerge.vbs
	copy $** $@
	$(BINPLACE_CMD)
$(O)\WiPolicy.vbs : $(SDXROOT)\admin\darwin\src\msitools\scripts\WiPolicy.vbs
	copy $** $@
	$(BINPLACE_CMD)
$(O)\WiRunSQL.vbs : $(SDXROOT)\admin\darwin\src\msitools\scripts\WiRunSQL.vbs
	copy $** $@
	$(BINPLACE_CMD)
$(O)\WiStream.vbs : $(SDXROOT)\admin\darwin\src\msitools\scripts\WiStream.vbs
	copy $** $@
	$(BINPLACE_CMD)
$(O)\WiSubStg.vbs : $(SDXROOT)\admin\darwin\src\msitools\scripts\WiSubStg.vbs
	copy $** $@
	$(BINPLACE_CMD)
$(O)\WiSumInf.vbs : $(SDXROOT)\admin\darwin\src\msitools\scripts\WiSumInf.vbs
	copy $** $@
	$(BINPLACE_CMD)
$(O)\WiTextIn.vbs : $(SDXROOT)\admin\darwin\src\msitools\scripts\WiTextIn.vbs
	copy $** $@
	$(BINPLACE_CMD)
$(O)\WiToAnsi.vbs : $(SDXROOT)\admin\darwin\src\msitools\scripts\WiToAnsi.vbs
	copy $** $@
	$(BINPLACE_CMD)
$(O)\WiUseXfm.vbs : $(SDXROOT)\admin\darwin\src\msitools\scripts\WiUseXfm.vbs
	copy $** $@
	$(BINPLACE_CMD)
$(O)\tutorial.cpp : $(SDXROOT)\admin\darwin\src\samples\tutorial.dll\tutorial.cpp
	copy $** $@
	$(BINPLACE_CMD)
$(O)\tutorial.rc : $(SDXROOT)\admin\darwin\src\samples\tutorial.dll\tutorial.rc
	copy $** $@
	$(BINPLACE_CMD)
$(O)\tutorial.def : $(SDXROOT)\admin\darwin\src\samples\tutorial.dll\tutorial.def
	copy $** $@
	$(BINPLACE_CMD)
$(O)\makefile3 : $(SDXROOT)\admin\darwin\src\samples\tutorial.dll\SDKFiles\makefile
	copy $** $@
	$(BINPLACE_CMD)
$(O)\readme.txt3 : $(SDXROOT)\admin\darwin\src\samples\tutorial.dll\SDKFiles\readme.txt
	copy $** $@
	$(BINPLACE_CMD)
$(O)\process.cpp : $(SDXROOT)\admin\darwin\src\samples\process.dll\process.cpp
	copy $** $@
	$(BINPLACE_CMD)
$(O)\process.rc : $(SDXROOT)\admin\darwin\src\samples\process.dll\process.rc
	copy $** $@
	$(BINPLACE_CMD)
$(O)\process.def : $(SDXROOT)\admin\darwin\src\samples\process.dll\process.def
	copy $** $@
	$(BINPLACE_CMD)
$(O)\makefile4 : $(SDXROOT)\admin\darwin\src\samples\process.dll\SDKFiles\makefile
	copy $** $@
	$(BINPLACE_CMD)
$(O)\readme.txt4 : $(SDXROOT)\admin\darwin\src\samples\process.dll\SDKFiles\readme.txt
	copy $** $@
	$(BINPLACE_CMD)
$(O)\remove.cpp : $(SDXROOT)\admin\darwin\src\samples\remove.dll\remove.cpp
	copy $** $@
	$(BINPLACE_CMD)
$(O)\remove.rc : $(SDXROOT)\admin\darwin\src\samples\remove.dll\remove.rc
	copy $** $@
	$(BINPLACE_CMD)
$(O)\remove.def : $(SDXROOT)\admin\darwin\src\samples\remove.dll\remove.def
	copy $** $@
	$(BINPLACE_CMD)
$(O)\makefile5 : $(SDXROOT)\admin\darwin\src\samples\remove.dll\SDKFiles\makefile
	copy $** $@
	$(BINPLACE_CMD)
$(O)\readme.txt5 : $(SDXROOT)\admin\darwin\src\samples\remove.dll\SDKFiles\readme.txt
	copy $** $@
	$(BINPLACE_CMD)
$(O)\create.cpp : $(SDXROOT)\admin\darwin\src\samples\create.dll\create.cpp
	copy $** $@
	$(BINPLACE_CMD)
$(O)\create.rc : $(SDXROOT)\admin\darwin\src\samples\create.dll\create.rc
	copy $** $@
	$(BINPLACE_CMD)
$(O)\create.def : $(SDXROOT)\admin\darwin\src\samples\create.dll\create.def
	copy $** $@
	$(BINPLACE_CMD)
$(O)\makefile6 : $(SDXROOT)\admin\darwin\src\samples\create.dll\SDKFiles\makefile
	copy $** $@
	$(BINPLACE_CMD)
$(O)\readme.txt6 : $(SDXROOT)\admin\darwin\src\samples\create.dll\SDKFiles\readme.txt
	copy $** $@
	$(BINPLACE_CMD)
$(O)\msiloc.cpp : $(SDXROOT)\admin\darwin\src\msitools\msiloc\msiloc.cpp
	copy $** $@
	$(BINPLACE_CMD)
$(O)\msiloc.rc : $(SDXROOT)\admin\darwin\src\msitools\msiloc\msiloc.rc
	copy $** $@
	$(BINPLACE_CMD)
$(O)\Makefile7 : $(SDXROOT)\admin\darwin\src\msitools\msiloc\SDKFiles\Makefile
	copy $** $@
	$(BINPLACE_CMD)
$(O)\readme.txt7 : $(SDXROOT)\admin\darwin\src\msitools\msiloc\SDKFiles\readme.txt
	copy $** $@
	$(BINPLACE_CMD)
$(O)\setup.cpp : $(SDXROOT)\admin\darwin\src\msitools\setup.exe\setup.cpp
	copy $** $@
	$(BINPLACE_CMD)
$(O)\setupui.cpp : $(SDXROOT)\admin\darwin\src\msitools\setup.exe\setupui.cpp
	copy $** $@
	$(BINPLACE_CMD)
$(O)\upgrdmsi.cpp : $(SDXROOT)\admin\darwin\src\msitools\setup.exe\upgrdmsi.cpp
	copy $** $@
	$(BINPLACE_CMD)
$(O)\utils.cpp : $(SDXROOT)\admin\darwin\src\msitools\setup.exe\utils.cpp
	copy $** $@
	$(BINPLACE_CMD)
$(O)\vertrust.cpp : $(SDXROOT)\admin\darwin\src\msitools\setup.exe\vertrust.cpp
	copy $** $@
	$(BINPLACE_CMD)
$(O)\common.h : $(SDXROOT)\admin\darwin\src\msitools\setup.exe\common.h
	copy $** $@
	$(BINPLACE_CMD)
$(O)\resource.h : $(SDXROOT)\admin\darwin\src\msitools\setup.exe\resource.h
	copy $** $@
	$(BINPLACE_CMD)
$(O)\setup.h : $(SDXROOT)\admin\darwin\src\msitools\setup.exe\setup.h
	copy $** $@
	$(BINPLACE_CMD)
$(O)\setupui.h : $(SDXROOT)\admin\darwin\src\msitools\setup.exe\setupui.h
	copy $** $@
	$(BINPLACE_CMD)
$(O)\setup.rc : $(SDXROOT)\admin\darwin\src\msitools\setup.exe\setup.rc
	copy $** $@
	$(BINPLACE_CMD)
$(O)\setup.ico : $(SDXROOT)\admin\darwin\src\msitools\setup.exe\setup.ico
	copy $** $@
	$(BINPLACE_CMD)
$(O)\msistuff.cpp : $(SDXROOT)\admin\darwin\src\msitools\msistuff\msistuff.cpp
	copy $** $@
	$(BINPLACE_CMD)
$(O)\msistuff.rc : $(SDXROOT)\admin\darwin\src\msitools\msistuff\msistuff.rc
	copy $** $@
	$(BINPLACE_CMD)
$(O)\Makefile1 : $(SDXROOT)\admin\darwin\src\msitools\setup.exe\SDKFiles\Makefile
	copy $** $@
	$(BINPLACE_CMD)
$(O)\ReadMe.htm1 : $(SDXROOT)\admin\darwin\src\msitools\setup.exe\SDKFiles\ReadMe.htm
	copy $** $@
	$(BINPLACE_CMD)
$(O)\Makefile2 : $(SDXROOT)\admin\darwin\src\msitools\msistuff\SDKFiles\Makefile
	copy $** $@
	$(BINPLACE_CMD)
$(O)\ReadMe.htm2 : $(SDXROOT)\admin\darwin\src\msitools\msistuff\SDKFiles\ReadMe.htm
	copy $** $@
	$(BINPLACE_CMD)
$(O)\dbreadme.txt : $(SDXROOT)\admin\darwin\src\sdk\dbreadme.txt
	copy $** $@
	$(BINPLACE_CMD)
$(O)\schema.msi : $(SDXROOT)\admin\darwin\build\packages\obj\i386\schema.msi
	copy $** $@
	$(BINPLACE_CMD)
$(O)\schema.msi1 : $(SDXROOT)\admin\darwin\data\archive\packages\100\schema.msi
	copy $** $@
	$(BINPLACE_CMD)
$(O)\schema.msi2 : $(SDXROOT)\admin\darwin\data\archive\packages\110\schema.msi
	copy $** $@
	$(BINPLACE_CMD)
$(O)\schema.msi3 : $(SDXROOT)\admin\darwin\data\archive\packages\120\schema.msi
	copy $** $@
	$(BINPLACE_CMD)
$(O)\sequence.msi : $(SDXROOT)\admin\darwin\build\packages\obj\i386\sequence.msi
	copy $** $@
	$(BINPLACE_CMD)
$(O)\sequence.msi1 : $(SDXROOT)\admin\darwin\data\archive\packages\100\sequence.msi
	copy $** $@
	$(BINPLACE_CMD)
$(O)\sequence.msi2 : $(SDXROOT)\admin\darwin\data\archive\packages\110\sequence.msi
	copy $** $@
	$(BINPLACE_CMD)
$(O)\sequence.msi3 : $(SDXROOT)\admin\darwin\data\archive\packages\120\sequence.msi
	copy $** $@
	$(BINPLACE_CMD)
$(O)\schema.msm : $(SDXROOT)\admin\darwin\build\packages\obj\i386\schema.msm
	copy $** $@
	$(BINPLACE_CMD)
$(O)\schema.msm1 : $(SDXROOT)\admin\darwin\data\archive\packages\110\schema.msm
	copy $** $@
	$(BINPLACE_CMD)
$(O)\schema.msm2 : $(SDXROOT)\admin\darwin\data\archive\packages\120\schema.msm
	copy $** $@
	$(BINPLACE_CMD)
$(O)\uisample.msi : $(SDXROOT)\admin\darwin\build\packages\obj\i386\uisample.msi
	copy $** $@
	$(BINPLACE_CMD)
$(O)\error.ara : $(SDXROOT)\admin\darwin\data\intl\error.ara
	copy $** $@
	$(BINPLACE_CMD)
$(O)\error.cat : $(SDXROOT)\admin\darwin\data\intl\error.cat
	copy $** $@
	$(BINPLACE_CMD)
$(O)\error.chs : $(SDXROOT)\admin\darwin\data\intl\error.chs
	copy $** $@
	$(BINPLACE_CMD)
$(O)\error.cht : $(SDXROOT)\admin\darwin\data\intl\error.cht
	copy $** $@
	$(BINPLACE_CMD)
$(O)\error.csy : $(SDXROOT)\admin\darwin\data\intl\error.csy
	copy $** $@
	$(BINPLACE_CMD)
$(O)\error.dan : $(SDXROOT)\admin\darwin\data\intl\error.dan
	copy $** $@
	$(BINPLACE_CMD)
$(O)\error.deu : $(SDXROOT)\admin\darwin\data\intl\error.deu
	copy $** $@
	$(BINPLACE_CMD)
$(O)\error.ell : $(SDXROOT)\admin\darwin\data\intl\error.ell
	copy $** $@
	$(BINPLACE_CMD)
$(O)\error.enu : $(SDXROOT)\admin\darwin\data\intl\error.enu
	copy $** $@
	$(BINPLACE_CMD)
$(O)\error.esn : $(SDXROOT)\admin\darwin\data\intl\error.esn
	copy $** $@
	$(BINPLACE_CMD)
$(O)\Error.eti : $(SDXROOT)\admin\darwin\data\intl\Error.eti
	copy $** $@
	$(BINPLACE_CMD)
$(O)\error.euq : $(SDXROOT)\admin\darwin\data\intl\error.euq
	copy $** $@
	$(BINPLACE_CMD)
$(O)\error.fin : $(SDXROOT)\admin\darwin\data\intl\error.fin
	copy $** $@
	$(BINPLACE_CMD)
$(O)\error.fra : $(SDXROOT)\admin\darwin\data\intl\error.fra
	copy $** $@
	$(BINPLACE_CMD)
$(O)\error.heb : $(SDXROOT)\admin\darwin\data\intl\error.heb
	copy $** $@
	$(BINPLACE_CMD)
$(O)\error.hrv : $(SDXROOT)\admin\darwin\data\intl\error.hrv
	copy $** $@
	$(BINPLACE_CMD)
$(O)\error.hun : $(SDXROOT)\admin\darwin\data\intl\error.hun
	copy $** $@
	$(BINPLACE_CMD)
$(O)\error.idt : $(SDXROOT)\admin\darwin\data\intl\error.idt
	copy $** $@
	$(BINPLACE_CMD)
$(O)\error.ita : $(SDXROOT)\admin\darwin\data\intl\error.ita
	copy $** $@
	$(BINPLACE_CMD)
$(O)\error.jpn : $(SDXROOT)\admin\darwin\data\intl\error.jpn
	copy $** $@
	$(BINPLACE_CMD)
$(O)\error.kor : $(SDXROOT)\admin\darwin\data\intl\error.kor
	copy $** $@
	$(BINPLACE_CMD)
$(O)\Error.lth : $(SDXROOT)\admin\darwin\data\intl\Error.lth
	copy $** $@
	$(BINPLACE_CMD)
$(O)\Error.lvi : $(SDXROOT)\admin\darwin\data\intl\Error.lvi
	copy $** $@
	$(BINPLACE_CMD)
$(O)\error.nld : $(SDXROOT)\admin\darwin\data\intl\error.nld
	copy $** $@
	$(BINPLACE_CMD)
$(O)\error.nor : $(SDXROOT)\admin\darwin\data\intl\error.nor
	copy $** $@
	$(BINPLACE_CMD)
$(O)\error.plk : $(SDXROOT)\admin\darwin\data\intl\error.plk
	copy $** $@
	$(BINPLACE_CMD)
$(O)\error.ptb : $(SDXROOT)\admin\darwin\data\intl\error.ptb
	copy $** $@
	$(BINPLACE_CMD)
$(O)\error.ptg : $(SDXROOT)\admin\darwin\data\intl\error.ptg
	copy $** $@
	$(BINPLACE_CMD)
$(O)\error.rom : $(SDXROOT)\admin\darwin\data\intl\error.rom
	copy $** $@
	$(BINPLACE_CMD)
$(O)\error.rus : $(SDXROOT)\admin\darwin\data\intl\error.rus
	copy $** $@
	$(BINPLACE_CMD)
$(O)\error.sky : $(SDXROOT)\admin\darwin\data\intl\error.sky
	copy $** $@
	$(BINPLACE_CMD)
$(O)\error.slv : $(SDXROOT)\admin\darwin\data\intl\error.slv
	copy $** $@
	$(BINPLACE_CMD)
$(O)\error.sve : $(SDXROOT)\admin\darwin\data\intl\error.sve
	copy $** $@
	$(BINPLACE_CMD)
$(O)\error.tha : $(SDXROOT)\admin\darwin\data\intl\error.tha
	copy $** $@
	$(BINPLACE_CMD)
$(O)\error.trk : $(SDXROOT)\admin\darwin\data\intl\error.trk
	copy $** $@
	$(BINPLACE_CMD)
$(O)\error.vit : $(SDXROOT)\admin\darwin\data\intl\error.vit
	copy $** $@
	$(BINPLACE_CMD)
$(O)\actionte.ara : $(SDXROOT)\admin\darwin\data\intl\actionte.ara
	copy $** $@
	$(BINPLACE_CMD)
$(O)\actionte.cat : $(SDXROOT)\admin\darwin\data\intl\actionte.cat
	copy $** $@
	$(BINPLACE_CMD)
$(O)\actionte.chs : $(SDXROOT)\admin\darwin\data\intl\actionte.chs
	copy $** $@
	$(BINPLACE_CMD)
$(O)\actionte.cht : $(SDXROOT)\admin\darwin\data\intl\actionte.cht
	copy $** $@
	$(BINPLACE_CMD)
$(O)\actionte.csy : $(SDXROOT)\admin\darwin\data\intl\actionte.csy
	copy $** $@
	$(BINPLACE_CMD)
$(O)\actionte.dan : $(SDXROOT)\admin\darwin\data\intl\actionte.dan
	copy $** $@
	$(BINPLACE_CMD)
$(O)\actionte.deu : $(SDXROOT)\admin\darwin\data\intl\actionte.deu
	copy $** $@
	$(BINPLACE_CMD)
$(O)\actionte.ell : $(SDXROOT)\admin\darwin\data\intl\actionte.ell
	copy $** $@
	$(BINPLACE_CMD)
$(O)\actionte.enu : $(SDXROOT)\admin\darwin\data\intl\actionte.enu
	copy $** $@
	$(BINPLACE_CMD)
$(O)\actionte.esn : $(SDXROOT)\admin\darwin\data\intl\actionte.esn
	copy $** $@
	$(BINPLACE_CMD)
$(O)\ActionTe.eti : $(SDXROOT)\admin\darwin\data\intl\ActionTe.eti
	copy $** $@
	$(BINPLACE_CMD)
$(O)\actionte.euq : $(SDXROOT)\admin\darwin\data\intl\actionte.euq
	copy $** $@
	$(BINPLACE_CMD)
$(O)\actionte.fin : $(SDXROOT)\admin\darwin\data\intl\actionte.fin
	copy $** $@
	$(BINPLACE_CMD)
$(O)\actionte.fra : $(SDXROOT)\admin\darwin\data\intl\actionte.fra
	copy $** $@
	$(BINPLACE_CMD)
$(O)\actionte.heb : $(SDXROOT)\admin\darwin\data\intl\actionte.heb
	copy $** $@
	$(BINPLACE_CMD)
$(O)\actionte.hrv : $(SDXROOT)\admin\darwin\data\intl\actionte.hrv
	copy $** $@
	$(BINPLACE_CMD)
$(O)\actionte.hun : $(SDXROOT)\admin\darwin\data\intl\actionte.hun
	copy $** $@
	$(BINPLACE_CMD)
$(O)\actionte.idt : $(SDXROOT)\admin\darwin\data\intl\actionte.idt
	copy $** $@
	$(BINPLACE_CMD)
$(O)\actionte.ita : $(SDXROOT)\admin\darwin\data\intl\actionte.ita
	copy $** $@
	$(BINPLACE_CMD)
$(O)\actionte.jpn : $(SDXROOT)\admin\darwin\data\intl\actionte.jpn
	copy $** $@
	$(BINPLACE_CMD)
$(O)\actionte.kor : $(SDXROOT)\admin\darwin\data\intl\actionte.kor
	copy $** $@
	$(BINPLACE_CMD)
$(O)\ActionTe.lth : $(SDXROOT)\admin\darwin\data\intl\ActionTe.lth
	copy $** $@
	$(BINPLACE_CMD)
$(O)\ActionTe.lvi : $(SDXROOT)\admin\darwin\data\intl\ActionTe.lvi
	copy $** $@
	$(BINPLACE_CMD)
$(O)\actionte.nld : $(SDXROOT)\admin\darwin\data\intl\actionte.nld
	copy $** $@
	$(BINPLACE_CMD)
$(O)\actionte.nor : $(SDXROOT)\admin\darwin\data\intl\actionte.nor
	copy $** $@
	$(BINPLACE_CMD)
$(O)\actionte.plk : $(SDXROOT)\admin\darwin\data\intl\actionte.plk
	copy $** $@
	$(BINPLACE_CMD)
$(O)\actionte.ptb : $(SDXROOT)\admin\darwin\data\intl\actionte.ptb
	copy $** $@
	$(BINPLACE_CMD)
$(O)\actionte.ptg : $(SDXROOT)\admin\darwin\data\intl\actionte.ptg
	copy $** $@
	$(BINPLACE_CMD)
$(O)\actionte.rom : $(SDXROOT)\admin\darwin\data\intl\actionte.rom
	copy $** $@
	$(BINPLACE_CMD)
$(O)\actionte.rus : $(SDXROOT)\admin\darwin\data\intl\actionte.rus
	copy $** $@
	$(BINPLACE_CMD)
$(O)\actionte.sky : $(SDXROOT)\admin\darwin\data\intl\actionte.sky
	copy $** $@
	$(BINPLACE_CMD)
$(O)\actionte.slv : $(SDXROOT)\admin\darwin\data\intl\actionte.slv
	copy $** $@
	$(BINPLACE_CMD)
$(O)\actionte.sve : $(SDXROOT)\admin\darwin\data\intl\actionte.sve
	copy $** $@
	$(BINPLACE_CMD)
$(O)\actionte.tha : $(SDXROOT)\admin\darwin\data\intl\actionte.tha
	copy $** $@
	$(BINPLACE_CMD)
$(O)\actionte.trk : $(SDXROOT)\admin\darwin\data\intl\actionte.trk
	copy $** $@
	$(BINPLACE_CMD)
$(O)\actionte.vit : $(SDXROOT)\admin\darwin\data\intl\actionte.vit
	copy $** $@
	$(BINPLACE_CMD)
$(O)\schema.log : $(SDXROOT)\admin\darwin\data\schema.idt\schema.log
	copy $** $@
	$(BINPLACE_CMD)
layout : 	$(O)\sdkmake.inc \
	$(O)\sdkpost.mak \
	$(O)\msisdk.msi \
	$(O)\ReadMe.txt \
	$(O)\Redist.txt \
	$(O)\License.rtf \
	$(O)\INSTLR1.ADM \
	$(O)\instlr11.adm \
	$(O)\msi.h \
	$(O)\msiquery.h \
	$(O)\msidefs.h \
	$(O)\patchwiz.h \
	$(O)\patchapi.h \
	$(O)\Mergemod.h \
	$(O)\msi.lib \
	$(O)\mspatchc.lib \
	$(O)\msi.chm \
	$(O)\msi.chi \
	$(O)\msitool.mak \
	$(O)\darice.cub \
	$(O)\logo.cub \
	$(O)\mergemod.cub \
	$(O)\darice.cub1 \
	$(O)\logo.cub1 \
	$(O)\mergemod.cub1 \
	$(O)\preadme.txt \
	$(O)\makecab.exe \
	$(O)\TEMPLATE.PCP \
	$(O)\EXAMPLE.PCP \
	$(O)\WiReadMe.txt \
	$(O)\WiCompon.vbs \
	$(O)\WiDialog.vbs \
	$(O)\WiDiffDb.vbs \
	$(O)\WiExport.vbs \
	$(O)\WiFilVer.vbs \
	$(O)\WiFeatur.vbs \
	$(O)\WiGenXfm.vbs \
	$(O)\WiImport.vbs \
	$(O)\WiLangId.vbs \
	$(O)\WiLstPrd.vbs \
	$(O)\WiLstScr.vbs \
	$(O)\WiLstXfm.vbs \
	$(O)\WiMakCab.vbs \
	$(O)\WiMerge.vbs \
	$(O)\WiPolicy.vbs \
	$(O)\WiRunSQL.vbs \
	$(O)\WiStream.vbs \
	$(O)\WiSubStg.vbs \
	$(O)\WiSumInf.vbs \
	$(O)\WiTextIn.vbs \
	$(O)\WiToAnsi.vbs \
	$(O)\WiUseXfm.vbs \
	$(O)\tutorial.cpp \
	$(O)\tutorial.rc \
	$(O)\tutorial.def \
	$(O)\makefile3 \
	$(O)\readme.txt3 \
	$(O)\process.cpp \
	$(O)\process.rc \
	$(O)\process.def \
	$(O)\makefile4 \
	$(O)\readme.txt4 \
	$(O)\remove.cpp \
	$(O)\remove.rc \
	$(O)\remove.def \
	$(O)\makefile5 \
	$(O)\readme.txt5 \
	$(O)\create.cpp \
	$(O)\create.rc \
	$(O)\create.def \
	$(O)\makefile6 \
	$(O)\readme.txt6 \
	$(O)\msiloc.cpp \
	$(O)\msiloc.rc \
	$(O)\Makefile7 \
	$(O)\readme.txt7 \
	$(O)\setup.cpp \
	$(O)\setupui.cpp \
	$(O)\upgrdmsi.cpp \
	$(O)\utils.cpp \
	$(O)\vertrust.cpp \
	$(O)\common.h \
	$(O)\resource.h \
	$(O)\setup.h \
	$(O)\setupui.h \
	$(O)\setup.rc \
	$(O)\setup.ico \
	$(O)\msistuff.cpp \
	$(O)\msistuff.rc \
	$(O)\Makefile1 \
	$(O)\ReadMe.htm1 \
	$(O)\Makefile2 \
	$(O)\ReadMe.htm2 \
	$(O)\dbreadme.txt \
	$(O)\schema.msi \
	$(O)\schema.msi1 \
	$(O)\schema.msi2 \
	$(O)\schema.msi3 \
	$(O)\sequence.msi \
	$(O)\sequence.msi1 \
	$(O)\sequence.msi2 \
	$(O)\sequence.msi3 \
	$(O)\schema.msm \
	$(O)\schema.msm1 \
	$(O)\schema.msm2 \
	$(O)\uisample.msi \
	$(O)\error.ara \
	$(O)\error.cat \
	$(O)\error.chs \
	$(O)\error.cht \
	$(O)\error.csy \
	$(O)\error.dan \
	$(O)\error.deu \
	$(O)\error.ell \
	$(O)\error.enu \
	$(O)\error.esn \
	$(O)\Error.eti \
	$(O)\error.euq \
	$(O)\error.fin \
	$(O)\error.fra \
	$(O)\error.heb \
	$(O)\error.hrv \
	$(O)\error.hun \
	$(O)\error.idt \
	$(O)\error.ita \
	$(O)\error.jpn \
	$(O)\error.kor \
	$(O)\Error.lth \
	$(O)\Error.lvi \
	$(O)\error.nld \
	$(O)\error.nor \
	$(O)\error.plk \
	$(O)\error.ptb \
	$(O)\error.ptg \
	$(O)\error.rom \
	$(O)\error.rus \
	$(O)\error.sky \
	$(O)\error.slv \
	$(O)\error.sve \
	$(O)\error.tha \
	$(O)\error.trk \
	$(O)\error.vit \
	$(O)\actionte.ara \
	$(O)\actionte.cat \
	$(O)\actionte.chs \
	$(O)\actionte.cht \
	$(O)\actionte.csy \
	$(O)\actionte.dan \
	$(O)\actionte.deu \
	$(O)\actionte.ell \
	$(O)\actionte.enu \
	$(O)\actionte.esn \
	$(O)\ActionTe.eti \
	$(O)\actionte.euq \
	$(O)\actionte.fin \
	$(O)\actionte.fra \
	$(O)\actionte.heb \
	$(O)\actionte.hrv \
	$(O)\actionte.hun \
	$(O)\actionte.idt \
	$(O)\actionte.ita \
	$(O)\actionte.jpn \
	$(O)\actionte.kor \
	$(O)\ActionTe.lth \
	$(O)\ActionTe.lvi \
	$(O)\actionte.nld \
	$(O)\actionte.nor \
	$(O)\actionte.plk \
	$(O)\actionte.ptb \
	$(O)\actionte.ptg \
	$(O)\actionte.rom \
	$(O)\actionte.rus \
	$(O)\actionte.sky \
	$(O)\actionte.slv \
	$(O)\actionte.sve \
	$(O)\actionte.tha \
	$(O)\actionte.trk \
	$(O)\actionte.vit \
	$(O)\schema.log \
