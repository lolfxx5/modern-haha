
XMLCOMPILER=$(SDXROOT)\admin\pchealth\PCHMars\target\obj\i386\comptree.exe

XMLCOMPILER_IN=HelpCtr.xml
XMLCOMPILER_OUT=HelpCtr.mmf

$(XMLCOMPILER_OUT) : $(XMLCOMPILER_IN)
	$(XMLCOMPILER) $(XMLCOMPILER_IN) $(XMLCOMPILER_OUT)