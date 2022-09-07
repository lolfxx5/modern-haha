;**************************************************************************
;/*
; *                      Microsoft Confidential
; *                      Copyright (C) Microsoft Corporation 1991
; *                      All Rights Reserved.
; */
;   Change Log:
;
;     Date    Who   #                   Description
;   --------  ---  ---  ------------------------------------------------------
;   04/01/90  DIC  C01  These variables are used to determine if enough memory
;                       is available to write a file out to disk. (Compaq STR
;                       #1889) (MS Bug #774)
;*****************************************************************************/
           PAGE    80,132 ;
	   TITLE DEBCONST.ASM
	   IF1
	       %OUT COMPONENT=DEBUG, MODULE=DEBCONST
	   ENDIF
.XLIST
.XCREF
include	version.inc		; cas -- missing equates
include	syscall.inc		; cas -- missing equates
INCLUDE    DOSSYM.INC
INCLUDE    debug.inc
.LIST
.CREF

CODE	   SEGMENT PUBLIC BYTE
CODE	   ENDS

CONST	   SEGMENT PUBLIC BYTE
CONST	   ENDS

CSTACK	   SEGMENT STACK
CSTACK	   ENDS

DATA	   SEGMENT PUBLIC BYTE
DATA	   ENDS

DG	   GROUP CODE,CONST,CSTACK,DATA

CODE	   SEGMENT PUBLIC  BYTE

	   EXTRN ALUFROMREG:NEAR, ALUTOREG:NEAR, ACCIMM:NEAR, SEGOP:NEAR
	   EXTRN ESPRE:NEAR, SSPRE:NEAR, CSPRE:NEAR, DSPRE:NEAR
	   EXTRN REGOP:NEAR, NOOPERANDS:NEAR, SAVHEX:NEAR, SHORTJMP:NEAR
	   EXTRN MOVSEGTO:NEAR, WORDTOALU:NEAR, MOVSEGFROM:NEAR
	   EXTRN GETADDR:NEAR, XCHGAX:NEAR, LONGJMP:NEAR, LOADACC:NEAR
	   EXTRN STOREACC:NEAR, REGIMMB:NEAR, SAV16:NEAR, MEMIMM:NEAR
	   EXTRN INT3:NEAR, SAV8:NEAR, CHK10:NEAR, M8087:NEAR
	   EXTRN M8087_D9:NEAR, M8087_DB:NEAR, M8087_DD:NEAR
	   EXTRN M8087_DF:NEAR, INFIXB:NEAR, INFIXW:NEAR, OUTFIXB:NEAR
	   EXTRN OUTFIXW:NEAR, JMPCALL:NEAR, INVARB:NEAR, INVARW:NEAR
	   EXTRN OUTVARB:NEAR, OUTVARW:NEAR, PREFIX:NEAR, IMMED:NEAR
	   EXTRN SIGNIMM:NEAR, SHIFT:NEAR, SHIFTV:NEAR, GRP1:NEAR
	   EXTRN GRP2:NEAR, REGIMMW:NEAR, DOORG:NEAR


	   EXTRN DB_OPER:NEAR, DW_OPER:NEAR, ASSEMLOOP:NEAR, GROUP2:NEAR
	   EXTRN NO_OPER:NEAR, GROUP1:NEAR, FGROUPP:NEAR, FGROUPX:NEAR
	   EXTRN FGROUPZ:NEAR, FD9_OPER:NEAR, FGROUPB:NEAR, FGROUP:NEAR
	   EXTRN FGROUPDS:NEAR, DCINC_OPER:NEAR, INT_OPER:NEAR
	   EXTRN IN_OPER:NEAR, DISP8_OPER:NEAR, JMP_OPER:NEAR, L_OPER:NEAR
	   EXTRN MOV_OPER:NEAR, OUT_OPER:NEAR, PUSH_OPER:NEAR
	   EXTRN GET_DATA16:NEAR, FGROUP3:NEAR, FGROUP3W:NEAR
	   EXTRN FDE_OPER:NEAR, ESC_OPER:NEAR, AA_OPER:NEAR
	   EXTRN CALL_OPER:NEAR, FDB_OPER:NEAR, POP_OPER:NEAR, ROTOP:NEAR
	   EXTRN TST_OPER:NEAR, EX_OPER:NEAR

CODE	   ENDS

CONST	   SEGMENT PUBLIC BYTE

	   PUBLIC REG8, REG16, SREG, SIZ8, DISTAB, DBMN, ADDMN, ADCMN, SUBMN
	   PUBLIC SBBMN, XORMN, ORMN, ANDMN, AAAMN, AADMN, AASMN, CALLMN, CBWMN
	   PUBLIC UPMN, DIMN, CMCMN, CMPMN, CWDMN, DAAMN, DASMN, DECMN, DIVMN
	   PUBLIC ESCMN, HLTMN, IDIVMN, IMULMN, INCMN, INTOMN, INTMN, INMN
	   PUBLIC IRETMN, JAMN, JCXZMN, JNCMN, JBEMN, JZMN, JGEMN, JGMN, JLEMN
	   PUBLIC JLMN, JMPMN, JNZMN, JPEMN, JNZMN, JPEMN, JPOMN, JNSMN, JNOMN
	   PUBLIC JOMN, JSMN, LAHFMN, LDSMN, LEAMN, LESMN, LOCKMN, LODBMN
	   PUBLIC LODWMN, LOOPNZMN, LOOPZMN, LOOPMN, MOVBMN, MOVWMN, MOVMN
	   PUBLIC MULMN, NEGMN, NOPMN, NOTMN, OUTMN, POPFMN, POPMN, PUSHFMN
	   PUBLIC PUSHMN, RCLMN, RCRMN, REPZMN, REPNZMN, RETFMN, RETMN, ROLMN
	   PUBLIC RORMN, SAHFMN, SARMN, SCABMN, SCAWMN, SHLMN, SHRMN, STCMN
	   PUBLIC DOWNMN, EIMN, STOBMN, STOWMN, TESTMN, WAITMN, XCHGMN, XLATMN
	   PUBLIC ESSEGMN, CSSEGMN, SSSEGMN, DSSEGMN, BADMN

	   PUBLIC M8087_TAB, FI_TAB, SIZE_TAB, MD9_TAB, MD9_TAB2, MDB_TAB
	   PUBLIC MDB_TAB2, MDD_TAB, MDD_TAB2, MDF_TAB, OPTAB, MAXOP, SHFTAB
	   PUBLIC IMMTAB, GRP1TAB, GRP2TAB, SEGTAB, REGTAB, REGTABEND, FLAGTAB
	   PUBLIC STACK

           PUBLIC DriveOfFile,FileSizeHB,FileSizeLB,TempLB,TempHB     ;C01

	   PUBLIC AXSAVE, BXSAVE, CXSAVE, DXSAVE, BPSAVE, SPSAVE, SISAVE
	   PUBLIC DISAVE, DSSAVE, ESSAVE, SSSAVE, CSSAVE, IPSAVE, FLSAVE, RSTACK
	   PUBLIC REGDIF, RDFLG, TOTREG, DSIZ, NOREGL, DISPB, LBUFSIZ, LBUFFCNT
	   PUBLIC LINEBUF, PFLAG, COLPOS, RSETFLAG

	   IF	SYSVER
	   PUBLIC CONFCB, POUT, COUT, CIN, IOBUFF, IOADDR, IOCALL, IOCOM
	   PUBLIC IOSTAT, IOCHRET, IOSEG, IOCNT
	   ENDIF

	   PUBLIC QFLAG, NEWEXEC, RETSAVE, USER_PROC_PDB, HEADSAVE, EXEC_BLOCK
	   PUBLIC COM_LINE, COM_FCB1, COM_FCB2, COM_SSSP, COM_CSIP, NEXTCS
	   PUBLIC NEXTIP, NAMESPEC

REG8	   DB	"ALCLDLBLAHCHDHBH"
REG16	   DB	"AXCXDXBXSPBPSIDI"
SREG	   DB	"ESCSSSDS",0,0
SIZ8	   DB	"BYWODWQWTB",0,0
; 0
DISTAB	   DW	OFFSET DG:ADDMN,ALUFROMREG
	   DW	OFFSET DG:ADDMN,ALUFROMREG
	   DW	OFFSET DG:ADDMN,ALUTOREG
	   DW	OFFSET DG:ADDMN,ALUTOREG
	   DW	OFFSET DG:ADDMN,ACCIMM
	   DW	OFFSET DG:ADDMN,ACCIMM
	   DW	OFFSET DG:PUSHMN,SEGOP
	   DW	OFFSET DG:POPMN,SEGOP
	   DW	OFFSET DG:ORMN,ALUFROMREG
	   DW	OFFSET DG:ORMN,ALUFROMREG
	   DW	OFFSET DG:ORMN,ALUTOREG
	   DW	OFFSET DG:ORMN,ALUTOREG
	   DW	OFFSET DG:ORMN,ACCIMM
	   DW	OFFSET DG:ORMN,ACCIMM
	   DW	OFFSET DG:PUSHMN,SEGOP
	   DW	OFFSET DG:DBMN,SAVHEX		; cas -- this has always been
;						; disassembled as a POP CS,
;						; which doesn't really exist.
;						; It is now a 386 prefix, but
;						; we don't know about 386
;						; instructions, so we'll put
;						; out a DB
; 10H
	   DW	OFFSET DG:ADCMN,ALUFROMREG
	   DW	OFFSET DG:ADCMN,ALUFROMREG
	   DW	OFFSET DG:ADCMN,ALUTOREG
	   DW	OFFSET DG:ADCMN,ALUTOREG
	   DW	OFFSET DG:ADCMN,ACCIMM
	   DW	OFFSET DG:ADCMN,ACCIMM
	   DW	OFFSET DG:PUSHMN,SEGOP
	   DW	OFFSET DG:POPMN,SEGOP
	   DW	OFFSET DG:SBBMN,ALUFROMREG
	   DW	OFFSET DG:SBBMN,ALUFROMREG
	   DW	OFFSET DG:SBBMN,ALUTOREG
	   DW	OFFSET DG:SBBMN,ALUTOREG
	   DW	OFFSET DG:SBBMN,ACCIMM
	   DW	OFFSET DG:SBBMN,ACCIMM
	   DW	OFFSET DG:PUSHMN,SEGOP
	   DW	OFFSET DG:POPMN,SEGOP
; 20H
	   DW	OFFSET DG:ANDMN,ALUFROMREG
	   DW	OFFSET DG:ANDMN,ALUFROMREG
	   DW	OFFSET DG:ANDMN,ALUTOREG
	   DW	OFFSET DG:ANDMN,ALUTOREG
	   DW	OFFSET DG:ANDMN,ACCIMM
	   DW	OFFSET DG:ANDMN,ACCIMM
	   DW	OFFSET DG:ESSEGMN,ESPRE
	   DW	OFFSET DG:DAAMN,NOOPERANDS
	   DW	OFFSET DG:SUBMN,ALUFROMREG
	   DW	OFFSET DG:SUBMN,ALUFROMREG
	   DW	OFFSET DG:SUBMN,ALUTOREG
	   DW	OFFSET DG:SUBMN,ALUTOREG
	   DW	OFFSET DG:SUBMN,ACCIMM
	   DW	OFFSET DG:SUBMN,ACCIMM
	   DW	OFFSET DG:CSSEGMN,CSPRE
	   DW	OFFSET DG:DASMN,NOOPERANDS
; 30H
	   DW	OFFSET DG:XORMN,ALUFROMREG
	   DW	OFFSET DG:XORMN,ALUFROMREG
	   DW	OFFSET DG:XORMN,ALUTOREG
	   DW	OFFSET DG:XORMN,ALUTOREG
	   DW	OFFSET DG:XORMN,ACCIMM
	   DW	OFFSET DG:XORMN,ACCIMM
	   DW	OFFSET DG:SSSEGMN,SSPRE
	   DW	OFFSET DG:AAAMN,NOOPERANDS
	   DW	OFFSET DG:CMPMN,ALUFROMREG
	   DW	OFFSET DG:CMPMN,ALUFROMREG
	   DW	OFFSET DG:CMPMN,ALUTOREG
	   DW	OFFSET DG:CMPMN,ALUTOREG
	   DW	OFFSET DG:CMPMN,ACCIMM
	   DW	OFFSET DG:CMPMN,ACCIMM
	   DW	OFFSET DG:DSSEGMN,DSPRE
	   DW	OFFSET DG:AASMN,NOOPERANDS
; 40H
	   DW	OFFSET DG:INCMN,REGOP
	   DW	OFFSET DG:INCMN,REGOP
	   DW	OFFSET DG:INCMN,REGOP
	   DW	OFFSET DG:INCMN,REGOP
	   DW	OFFSET DG:INCMN,REGOP
	   DW	OFFSET DG:INCMN,REGOP
	   DW	OFFSET DG:INCMN,REGOP
	   DW	OFFSET DG:INCMN,REGOP
	   DW	OFFSET DG:DECMN,REGOP
	   DW	OFFSET DG:DECMN,REGOP
	   DW	OFFSET DG:DECMN,REGOP
	   DW	OFFSET DG:DECMN,REGOP
	   DW	OFFSET DG:DECMN,REGOP
	   DW	OFFSET DG:DECMN,REGOP
	   DW	OFFSET DG:DECMN,REGOP
	   DW	OFFSET DG:DECMN,REGOP
; 50H
	   DW	OFFSET DG:PUSHMN,REGOP
	   DW	OFFSET DG:PUSHMN,REGOP
	   DW	OFFSET DG:PUSHMN,REGOP
	   DW	OFFSET DG:PUSHMN,REGOP
	   DW	OFFSET DG:PUSHMN,REGOP
	   DW	OFFSET DG:PUSHMN,REGOP
	   DW	OFFSET DG:PUSHMN,REGOP
	   DW	OFFSET DG:PUSHMN,REGOP
	   DW	OFFSET DG:POPMN,REGOP
	   DW	OFFSET DG:POPMN,REGOP
	   DW	OFFSET DG:POPMN,REGOP
	   DW	OFFSET DG:POPMN,REGOP
	   DW	OFFSET DG:POPMN,REGOP
	   DW	OFFSET DG:POPMN,REGOP
	   DW	OFFSET DG:POPMN,REGOP
	   DW	OFFSET DG:POPMN,REGOP
; 60H
	   DW	OFFSET DG:DBMN,SAVHEX
	   DW	OFFSET DG:DBMN,SAVHEX
	   DW	OFFSET DG:DBMN,SAVHEX
	   DW	OFFSET DG:DBMN,SAVHEX
	   DW	OFFSET DG:DBMN,SAVHEX
	   DW	OFFSET DG:DBMN,SAVHEX
	   DW	OFFSET DG:DBMN,SAVHEX
	   DW	OFFSET DG:DBMN,SAVHEX
	   DW	OFFSET DG:DBMN,SAVHEX
	   DW	OFFSET DG:DBMN,SAVHEX
	   DW	OFFSET DG:DBMN,SAVHEX
	   DW	OFFSET DG:DBMN,SAVHEX
	   DW	OFFSET DG:DBMN,SAVHEX
	   DW	OFFSET DG:DBMN,SAVHEX
	   DW	OFFSET DG:DBMN,SAVHEX
	   DW	OFFSET DG:DBMN,SAVHEX
; 70H
	   DW	OFFSET DG:JOMN,SHORTJMP
	   DW	OFFSET DG:JNOMN,SHORTJMP
	   DW	OFFSET DG:JCMN,SHORTJMP
	   DW	OFFSET DG:JNCMN,SHORTJMP
	   DW	OFFSET DG:JZMN,SHORTJMP
	   DW	OFFSET DG:JNZMN,SHORTJMP
	   DW	OFFSET DG:JBEMN,SHORTJMP
	   DW	OFFSET DG:JAMN,SHORTJMP
	   DW	OFFSET DG:JSMN,SHORTJMP
	   DW	OFFSET DG:JNSMN,SHORTJMP
	   DW	OFFSET DG:JPEMN,SHORTJMP
	   DW	OFFSET DG:JPOMN,SHORTJMP
	   DW	OFFSET DG:JLMN,SHORTJMP
	   DW	OFFSET DG:JGEMN,SHORTJMP
	   DW	OFFSET DG:JLEMN,SHORTJMP
	   DW	OFFSET DG:JGMN,SHORTJMP
; 80H
	   DW	0,IMMED
	   DW	0,IMMED
	   DW	0,IMMED
	   DW	0,SIGNIMM
	   DW	OFFSET DG:TESTMN,ALUTOREG ;ARR 2.4
	   DW	OFFSET DG:TESTMN,ALUTOREG ;ARR 2.4
	   DW	OFFSET DG:XCHGMN,ALUTOREG ;ARR 2.4
	   DW	OFFSET DG:XCHGMN,ALUTOREG ;ARR 2.4
	   DW	OFFSET DG:MOVMN,ALUFROMREG
	   DW	OFFSET DG:MOVMN,ALUFROMREG
	   DW	OFFSET DG:MOVMN,ALUTOREG
	   DW	OFFSET DG:MOVMN,ALUTOREG
	   DW	OFFSET DG:MOVMN,MOVSEGTO
	   DW	OFFSET DG:LEAMN,WORDTOALU
	   DW	OFFSET DG:MOVMN,MOVSEGFROM
	   DW	OFFSET DG:POPMN,GETADDR
; 90H
	   DW	OFFSET DG:NOPMN,NOOPERANDS
	   DW	OFFSET DG:XCHGMN,XCHGAX
	   DW	OFFSET DG:XCHGMN,XCHGAX
	   DW	OFFSET DG:XCHGMN,XCHGAX
	   DW	OFFSET DG:XCHGMN,XCHGAX
	   DW	OFFSET DG:XCHGMN,XCHGAX
	   DW	OFFSET DG:XCHGMN,XCHGAX
	   DW	OFFSET DG:XCHGMN,XCHGAX
	   DW	OFFSET DG:CBWMN,NOOPERANDS
	   DW	OFFSET DG:CWDMN,NOOPERANDS
	   DW	OFFSET DG:CALLMN,LONGJMP
	   DW	OFFSET DG:WAITMN,NOOPERANDS
	   DW	OFFSET DG:PUSHFMN,NOOPERANDS
	   DW	OFFSET DG:POPFMN,NOOPERANDS
	   DW	OFFSET DG:SAHFMN,NOOPERANDS
	   DW	OFFSET DG:LAHFMN,NOOPERANDS
; A0H
	   DW	OFFSET DG:MOVMN,LOADACC
	   DW	OFFSET DG:MOVMN,LOADACC
	   DW	OFFSET DG:MOVMN,STOREACC
	   DW	OFFSET DG:MOVMN,STOREACC
	   DW	OFFSET DG:MOVBMN,NOOPERANDS
	   DW	OFFSET DG:MOVWMN,NOOPERANDS
	   DW	OFFSET DG:CMPBMN,NOOPERANDS
	   DW	OFFSET DG:CMPWMN,NOOPERANDS
	   DW	OFFSET DG:TESTMN,ACCIMM
	   DW	OFFSET DG:TESTMN,ACCIMM
	   DW	OFFSET DG:STOBMN,NOOPERANDS
	   DW	OFFSET DG:STOWMN,NOOPERANDS
	   DW	OFFSET DG:LODBMN,NOOPERANDS
	   DW	OFFSET DG:LODWMN,NOOPERANDS
	   DW	OFFSET DG:SCABMN,NOOPERANDS
	   DW	OFFSET DG:SCAWMN,NOOPERANDS
; B0H
	   DW	OFFSET DG:MOVMN,REGIMMB
	   DW	OFFSET DG:MOVMN,REGIMMB
	   DW	OFFSET DG:MOVMN,REGIMMB
	   DW	OFFSET DG:MOVMN,REGIMMB
	   DW	OFFSET DG:MOVMN,REGIMMB
	   DW	OFFSET DG:MOVMN,REGIMMB
	   DW	OFFSET DG:MOVMN,REGIMMB
	   DW	OFFSET DG:MOVMN,REGIMMB
	   DW	OFFSET DG:MOVMN,REGIMMW
	   DW	OFFSET DG:MOVMN,REGIMMW
	   DW	OFFSET DG:MOVMN,REGIMMW
	   DW	OFFSET DG:MOVMN,REGIMMW
	   DW	OFFSET DG:MOVMN,REGIMMW
	   DW	OFFSET DG:MOVMN,REGIMMW
	   DW	OFFSET DG:MOVMN,REGIMMW
	   DW	OFFSET DG:MOVMN,REGIMMW
; C0H
	   DW	OFFSET DG:DBMN,SAVHEX
	   DW	OFFSET DG:DBMN,SAVHEX
	   DW	OFFSET DG:RETMN,SAV16
	   DW	OFFSET DG:RETMN,NOOPERANDS
	   DW	OFFSET DG:LESMN,WORDTOALU
	   DW	OFFSET DG:LDSMN,WORDTOALU
	   DW	OFFSET DG:MOVMN,MEMIMM
	   DW	OFFSET DG:MOVMN,MEMIMM
	   DW	OFFSET DG:DBMN,SAVHEX
	   DW	OFFSET DG:DBMN,SAVHEX
	   DW	OFFSET DG:RETFMN,SAV16
	   DW	OFFSET DG:RETFMN,NOOPERANDS
	   DW	OFFSET DG:INTMN,INT3
	   DW	OFFSET DG:INTMN,SAV8
	   DW	OFFSET DG:INTOMN,NOOPERANDS
	   DW	OFFSET DG:IRETMN,NOOPERANDS
; D0H
	   DW	0,SHIFT
	   DW	0,SHIFT
	   DW	0,SHIFTV
	   DW	0,SHIFTV
	   DW	OFFSET DG:AAMMN,CHK10
	   DW	OFFSET DG:AADMN,CHK10
	   DW	OFFSET DG:DBMN,SAVHEX
	   DW	OFFSET DG:XLATMN,NOOPERANDS
	   DW	0,M8087 		; d8
	   DW	0,M8087_D9		; d9
	   DW	0,M8087 		; da
	   DW	0,M8087_DB		; db
	   DW	0,M8087 		; dc
	   DW	0,M8087_DD		; dd
	   DW	0,M8087 		; de
	   DW	0,M8087_DF		; df
; E0H
	   DW	OFFSET DG:LOOPNZMN,SHORTJMP
	   DW	OFFSET DG:LOOPZMN,SHORTJMP
	   DW	OFFSET DG:LOOPMN,SHORTJMP
	   DW	OFFSET DG:JCXZMN,SHORTJMP
	   DW	OFFSET DG:INMN,INFIXB
	   DW	OFFSET DG:INMN,INFIXW
	   DW	OFFSET DG:OUTMN,OUTFIXB
	   DW	OFFSET DG:OUTMN,OUTFIXW
	   DW	OFFSET DG:CALLMN,JMPCALL
	   DW	OFFSET DG:JMPMN,JMPCALL
	   DW	OFFSET DG:JMPMN,LONGJMP
	   DW	OFFSET DG:JMPMN,SHORTJMP
	   DW	OFFSET DG:INMN,INVARB
	   DW	OFFSET DG:INMN,INVARW
	   DW	OFFSET DG:OUTMN,OUTVARB
	   DW	OFFSET DG:OUTMN,OUTVARW
; F0H
	   DW	OFFSET DG:LOCKMN,PREFIX
	   DW	OFFSET DG:DBMN,SAVHEX
	   DW	OFFSET DG:REPNZMN,PREFIX
	   DW	OFFSET DG:REPZMN,PREFIX
	   DW	OFFSET DG:HLTMN,NOOPERANDS
	   DW	OFFSET DG:CMCMN,NOOPERANDS
	   DW	0,GRP1
	   DW	0,GRP1
	   DW	OFFSET DG:CLCMN,NOOPERANDS
	   DW	OFFSET DG:STCMN,NOOPERANDS
	   DW	OFFSET DG:DIMN,NOOPERANDS
	   DW	OFFSET DG:EIMN,NOOPERANDS
	   DW	OFFSET DG:UPMN,NOOPERANDS
	   DW	OFFSET DG:DOWNMN,NOOPERANDS
	   DW	0,GRP2
	   DW	0,GRP2

DBMN	   DB	"DB",0
	   DB	"DW",0
	   DB	";",0
ORGMN	   DB	"ORG",0
ADDMN	   DB	"ADD",0
ADCMN	   DB	"ADC",0
SUBMN	   DB	"SUB",0
SBBMN	   DB	"SBB",0
XORMN	   DB	"XOR",0
ORMN	   DB	"OR",0
ANDMN	   DB	"AND",0
AAAMN	   DB	"AAA",0
AADMN	   DB	"AAD",0
AAMMN	   DB	"AAM",0
AASMN	   DB	"AAS",0
CALLMN	   DB	"CALL",0
CBWMN	   DB	"CBW",0
CLCMN	   DB	"CLC",0
UPMN	   DB	"CLD",0                 ; CLD,0
DIMN	   DB	"CLI",0
CMCMN	   DB	"CMC",0
CMPBMN	   DB	"CMPSB",0               ; CMPSB
CMPWMN	   DB	"CMPSW",0               ; CMPSW,0
CMPMN	   DB	"CMP",0
CWDMN	   DB	"CWD",0
DAAMN	   DB	"DAA",0
DASMN	   DB	"DAS",0
DECMN	   DB	"DEC",0
DIVMN	   DB	"DIV",0
ESCMN	   DB	"ESC",0
	   DB	"FXCH",0
	   DB	"FFREE",0
	   DB	"FCOMPP",0
	   DB	"FCOMP",0
	   DB	"FCOM",0
	   DB	"FICOMP",0
	   DB	"FICOM",0
	   DB	"FNOP",0
	   DB	"FCHS",0
	   DB	"FABS",0
	   DB	"FTST",0
	   DB	"FXAM",0
	   DB	"FLDL2T",0
	   DB	"FLDL2E",0
	   DB	"FLDLG2",0
	   DB	"FLDLN2",0
	   DB	"FLDPI",0
	   DB	"FLD1",0
	   DB	"FLDZ",0
	   DB	"F2XM1",0
	   DB	"FYL2XP1",0
	   DB	"FYL2X",0
	   DB	"FPTAN",0
	   DB	"FPATAN",0
	   DB	"FXTRACT",0
	   DB	"FDECSTP",0
	   DB	"FINCSTP",0
	   DB	"FPREM",0
	   DB	"FSQRT",0
	   DB	"FRNDINT",0
	   DB	"FSCALE",0
	   DB	"FINIT",0
	   DB	"FDISI",0
	   DB	"FENI",0
	   DB	"FCLEX",0
	   DB	"FBLD",0
	   DB	"FBSTP",0
	   DB	"FLDCW",0
	   DB	"FSTCW",0
	   DB	"FSTSW",0
	   DB	"FSTENV",0
	   DB	"FLDENV",0
	   DB	"FSAVE",0
	   DB	"FRSTOR",0
	   DB	"FADDP",0
	   DB	"FADD",0
	   DB	"FIADD",0
	   DB	"FSUBRP",0
	   DB	"FSUBR",0
	   DB	"FSUBP",0
	   DB	"FSUB",0
	   DB	"FISUBR",0
	   DB	"FISUB",0
	   DB	"FMULP",0
	   DB	"FMUL",0
	   DB	"FIMUL",0
	   DB	"FDIVRP",0
	   DB	"FDIVR",0
	   DB	"FDIVP",0
	   DB	"FDIV",0
	   DB	"FIDIVR",0
	   DB	"FIDIV",0
	   DB	"FWAIT",0
	   DB	"FILD",0
	   DB	"FLD",0
	   DB	"FSTP",0
	   DB	"FST",0
	   DB	"FISTP",0
	   DB	"FIST",0
HLTMN	   DB	"HLT",0
IDIVMN	   DB	"IDIV",0
IMULMN	   DB	"IMUL",0
INCMN	   DB	"INC",0
INTOMN	   DB	"INTO",0
INTMN	   DB	"INT",0
INMN	   DB	"IN",0                  ; IN
IRETMN	   DB	"IRET",0
	   DB	"JNBE",0
	   DB	"JAE",0
JAMN	   DB	"JA",0
JCXZMN	   DB	"JCXZ",0
JNCMN	   DB	"JNB",0
JBEMN	   DB	"JBE",0
JCMN	   DB	"JB",0
	   DB	"JNC",0
	   DB	"JC",0
	   DB	"JNAE",0
	   DB	"JNA",0
JZMN	   DB	"JZ",0
	   DB	"JE",0
JGEMN	   DB	"JGE",0
JGMN	   DB	"JG",0
	   DB	"JNLE",0
	   DB	"JNL",0
JLEMN	   DB	"JLE",0
JLMN	   DB	"JL",0
	   DB	"JNGE",0
	   DB	"JNG",0
JMPMN	   DB	"JMP",0
JNZMN	   DB	"JNZ",0
	   DB	"JNE",0
JPEMN	   DB	"JPE",0
JPOMN	   DB	"JPO",0
	   DB	"JNP",0
JNSMN	   DB	"JNS",0
JNOMN	   DB	"JNO",0
JOMN	   DB	"JO",0
JSMN	   DB	"JS",0
	   DB	"JP",0
LAHFMN	   DB	"LAHF",0
LDSMN	   DB	"LDS",0
LEAMN	   DB	"LEA",0
LESMN	   DB	"LES",0
LOCKMN	   DB	"LOCK",0
LODBMN	   DB	"LODSB",0               ; LODSB
LODWMN	   DB	"LODSW",0               ; LODSW,0
LOOPNZMN   DB	"LOOPNZ",0
LOOPZMN    DB	"LOOPZ",0
	   DB	"LOOPNE",0
	   DB	"LOOPE",0
LOOPMN	   DB	"LOOP",0
MOVBMN	   DB	"MOVSB",0               ; MOVSB
MOVWMN	   DB	"MOVSW",0               ; MOVSW,0
MOVMN	   DB	"MOV",0
MULMN	   DB	"MUL",0
NEGMN	   DB	"NEG",0
NOPMN	   DB	"NOP",0
NOTMN	   DB	"NOT",0
OUTMN	   DB	"OUT",0                 ; OUT
POPFMN	   DB	"POPF",0
POPMN	   DB	"POP",0
PUSHFMN    DB	"PUSHF",0
PUSHMN	   DB	"PUSH",0
RCLMN	   DB	"RCL",0
RCRMN	   DB	"RCR",0
REPZMN	   DB	"REPZ",0
REPNZMN    DB	"REPNZ",0
	   DB	"REPE",0
	   DB	"REPNE",0
	   DB	"REP",0
RETFMN	   DB	"RETF",0
RETMN	   DB	"RET",0
ROLMN	   DB	"ROL",0
RORMN	   DB	"ROR",0
SAHFMN	   DB	"SAHF",0
SARMN	   DB	"SAR",0
SCABMN	   DB	"SCASB",0               ; SCASB
SCAWMN	   DB	"SCASW",0               ; SCASW,0
SHLMN	   DB	"SHL",0
SHRMN	   DB	"SHR",0
STCMN	   DB	"STC",0
DOWNMN	   DB	"STD",0                 ; STD
EIMN	   DB	"STI",0                 ; STI
STOBMN	   DB	"STOSB",0               ; STOSB
STOWMN	   DB	"STOSW",0               ; STOSW,0
TESTMN	   DB	"TEST",0
WAITMN	   DB	"WAIT",0
XCHGMN	   DB	"XCHG",0
XLATMN	   DB	"XLAT",0
ESSEGMN    DB	"ES:",0
CSSEGMN    DB	"CS:",0
SSSEGMN    DB	"SS:",0
DSSEGMN    DB	"DS:",0
BADMN	   DB	"???",0

M8087_TAB  DB	"ADD$MUL$COM$COMP$SUB$SUBR$DIV$DIVR$"
FI_TAB	   DB	"F$FI$F$FI$"
SIZE_TAB   DB	"DWORD PTR $DWORD PTR $QWORD PTR $WORD PTR $"
	   DB	"BYTE PTR $TBYTE PTR $"

MD9_TAB    DB	"LD$@$ST$STP$LDENV$LDCW$STENV$STCW$"
MD9_TAB2   DB	"CHS$ABS$@$@$TST$XAM$@$@$LD1$LDL2T$LDL2E$"
	   DB	"LDPI$LDLG2$LDLN2$LDZ$@$2XM1$YL2X$PTAN$PATAN$XTRACT$"
	   DB	"@$DECSTP$INCSTP$PREM$YL2XP1$SQRT$@$RNDINT$SCALE$@$@$"

MDB_TAB    DB	"ILD$@$IST$ISTP$@$LD$@$STP$"
MDB_TAB2   DB	"ENI$DISI$CLEX$INIT$"

MDD_TAB    DB	"LD$@$ST$STP$RSTOR$@$SAVE$STSW$"
MDD_TAB2   DB	"FREE$XCH$ST$STP$"

MDF_TAB    DB	"ILD$@$IST$ISTP$BLD$ILD$BSTP$ISTP$"


OPTAB	   DB	11111111B		; DB
	   DW	DB_OPER
	   DB	11111111B		; DW
	   DW	DW_OPER
	   DB	11111111B		; COMMENT
	   DW	ASSEMLOOP
	   DB	11111111B		; ORG
	   DW	DOORG
	   DB	0 * 8			; ADD
	   DW	GROUP2
	   DB	2 * 8			; ADC
	   DW	GROUP2
	   DB	5 * 8			; SUB
	   DW	GROUP2
	   DB	3 * 8			; SBB
	   DW	GROUP2
	   DB	6 * 8			; XOR
	   DW	GROUP2
	   DB	1 * 8			; OR
	   DW	GROUP2
	   DB	4 * 8			; AND
	   DW	GROUP2
	   DB	00110111B		; AAA
	   DW	NO_OPER
	   DB	11010101B		; AAD
	   DW	AA_OPER
	   DB	11010100B		; AAM
	   DW	AA_OPER
	   DB	00111111B		; AAS
	   DW	NO_OPER
	   DB	2 * 8			; CALL
	   DW	CALL_OPER
	   DB	10011000B		; CBW
	   DW	NO_OPER
	   DB	11111000B		; CLC
	   DW	NO_OPER
	   DB	11111100B		; CLD
	   DW	NO_OPER
	   DB	11111010B		; DIM
	   DW	NO_OPER
	   DB	11110101B		; CMC
	   DW	NO_OPER
	   DB	10100110B		; CMPB
	   DW	NO_OPER
	   DB	10100111B		; CMPW
	   DW	NO_OPER
	   DB	7 * 8			; CMP
	   DW	GROUP2
	   DB	10011001B		; CWD
	   DW	NO_OPER
	   DB	00100111B		; DAA
	   DW	NO_OPER
	   DB	00101111B		; DAS
	   DW	NO_OPER
	   DB	1 * 8			; DEC
	   DW	DCINC_OPER
	   DB	6 * 8			; DIV
	   DW	GROUP1
	   DB	11011000B		; ESC
	   DW	ESC_OPER
	   DB	00001001B		; FXCH
	   DW	FGROUPP
	   DB	00101000B		; FFREE
	   DW	FGROUPP
	   DB	11011001B		; FCOMPP
	   DW	FDE_OPER
	   DB	00000011B		; FCOMP
	   DW	FGROUPX 		; Exception to normal P instructions
	   DB	00000010B		; FCOM
	   DW	FGROUPX
	   DB	00010011B		; FICOMP
	   DW	FGROUPZ
	   DB	00010010B		; FICOM
	   DW	FGROUPZ
	   DB	11010000B		; FNOP
	   DW	FD9_OPER
	   DB	11100000B		; FCHS
	   DW	FD9_OPER
	   DB	11100001B		; FABS
	   DW	FD9_OPER
	   DB	11100100B		; FTST
	   DW	FD9_OPER
	   DB	11100101B		; FXAM
	   DW	FD9_OPER
	   DB	11101001B		; FLDL2T
	   DW	FD9_OPER
	   DB	11101010B		; FLDL2E
	   DW	FD9_OPER
	   DB	11101100B		; FLDLG2
	   DW	FD9_OPER
	   DB	11101101B		; FLDLN2
	   DW	FD9_OPER
	   DB	11101011B		; FLDPI
	   DW	FD9_OPER
	   DB	11101000B		; FLD1
	   DW	FD9_OPER
	   DB	11101110B		; FLDZ
	   DW	FD9_OPER
	   DB	11110000B		; F2XM1
	   DW	FD9_OPER
	   DB	11111001B		; FYL2XP1
	   DW	FD9_OPER
	   DB	11110001B		; FYL2X
	   DW	FD9_OPER
	   DB	11110010B		; FPTAN
	   DW	FD9_OPER
	   DB	11110011B		; FPATAN
	   DW	FD9_OPER
	   DB	11110100B		; FXTRACT
	   DW	FD9_OPER
	   DB	11110110B		; FDECSTP
	   DW	FD9_OPER
	   DB	11110111B		; FINCSTP
	   DW	FD9_OPER
	   DB	11111000B		; FPREM
	   DW	FD9_OPER
	   DB	11111010B		; FSQRT
	   DW	FD9_OPER
	   DB	11111100B		; FRNDINT
	   DW	FD9_OPER
	   DB	11111101B		; FSCALE
	   DW	FD9_OPER
	   DB	11100011B		; FINIT
	   DW	FDB_OPER
	   DB	11100001B		; FDISI
	   DW	FDB_OPER
	   DB	11100000B		; FENI
	   DW	FDB_OPER
	   DB	11100010B		; FCLEX
	   DW	FDB_OPER
	   DB	00111100B		; FBLD
	   DW	FGROUPB
	   DB	00111110B		; FBSTP
	   DW	FGROUPB
	   DB	00001101B		; FLDCW
	   DW	FGROUP3W
	   DB	00001111B		; FSTCW
	   DW	FGROUP3W
	   DB	00101111B		; FSTSW
	   DW	FGROUP3W
	   DB	00001110B		; FSTENV
	   DW	FGROUP3
	   DB	00001100B		; FLDENV
	   DW	FGROUP3
	   DB	00101110B		; FSAVE
	   DW	FGROUP3
	   DB	00101100B		; FRSTOR
	   DW	FGROUP3
	   DB	00110000B		; FADDP
	   DW	FGROUPP
	   DB	00000000B		; FADD
	   DW	FGROUP
	   DB	00010000B		; FIADD
	   DW	FGROUPZ
	   DB	00110100B		; FSUBRP
	   DW	FGROUPP
	   DB	00000101B		; FSUBR
	   DW	FGROUPDS
	   DB	00110101B		; FSUBP
	   DW	FGROUPP
	   DB	00000100B		; FSUB
	   DW	FGROUPDS
	   DB	00010101B		; FISUBR
	   DW	FGROUPZ
	   DB	00010100B		; FISUB
	   DW	FGROUPZ
	   DB	00110001B		; FMULP
	   DW	FGROUPP
	   DB	00000001B		; FMUL
	   DW	FGROUP
	   DB	00010001B		; FIMUL
	   DW	FGROUPZ
	   DB	00110110B		; FDIVRP
	   DW	FGROUPP
	   DB	00000111B		; FDIVR
	   DW	FGROUPDS
	   DB	00110111B		; FDIVP
	   DW	FGROUPP
	   DB	00000110B		; FDIV
	   DW	FGROUPDS
	   DB	00010111B		; FIDIVR
	   DW	FGROUPZ
	   DB	00010110B		; FIDIV
	   DW	FGROUPZ
	   DB	10011011B		; FWAIT
	   DW	NO_OPER
	   DB	00011000B		; FILD
	   DW	FGROUPZ
	   DB	00001000B		; FLD
	   DW	FGROUPX
	   DB	00001011B		; FSTP
	   DW	FGROUP			;an000; dms;
	   DB	00101010B		; FST
	   DW	FGROUPX
	   DB	00011011B		; FISTP
	   DW	FGROUPZ
	   DB	00011010B		; FIST
	   DW	FGROUPZ
	   DB	11110100B		; HLT
	   DW	NO_OPER
	   DB	7 * 8			; IDIV
	   DW	GROUP1
	   DB	5 * 8			; IMUL
	   DW	GROUP1
	   DB	0 * 8			; INC
	   DW	DCINC_OPER
	   DB	11001110B		; INTO
	   DW	NO_OPER
	   DB	11001100B		; INTM
	   DW	INT_OPER
	   DB	11101100B		; IN
	   DW	IN_OPER
	   DB	11001111B		; IRET
	   DW	NO_OPER
	   DB	01110111B		; JNBE
	   DW	DISP8_OPER
	   DB	01110011B		; JAE
	   DW	DISP8_OPER
	   DB	01110111B		; JA
	   DW	DISP8_OPER
	   DB	11100011B		; JCXZ
	   DW	DISP8_OPER
	   DB	01110011B		; JNB
	   DW	DISP8_OPER
	   DB	01110110B		; JBE
	   DW	DISP8_OPER
	   DB	01110010B		; JB
	   DW	DISP8_OPER
	   DB	01110011B		; JNC
	   DW	DISP8_OPER
	   DB	01110010B		; JC
	   DW	DISP8_OPER
	   DB	01110010B		; JNAE
	   DW	DISP8_OPER
	   DB	01110110B		; JNA
	   DW	DISP8_OPER
	   DB	01110100B		; JZ
	   DW	DISP8_OPER
	   DB	01110100B		; JE
	   DW	DISP8_OPER
	   DB	01111101B		; JGE
	   DW	DISP8_OPER
	   DB	01111111B		; JG
	   DW	DISP8_OPER
	   DB	01111111B		; JNLE
	   DW	DISP8_OPER
	   DB	01111101B		; JNL
	   DW	DISP8_OPER
	   DB	01111110B		; JLE
	   DW	DISP8_OPER
	   DB	01111100B		; JL
	   DW	DISP8_OPER
	   DB	01111100B		; JNGE
	   DW	DISP8_OPER
	   DB	01111110B		; JNG
	   DW	DISP8_OPER
	   DB	4 * 8			; JMP
	   DW	JMP_OPER
	   DB	01110101B		; JNZ
	   DW	DISP8_OPER
	   DB	01110101B		; JNE
	   DW	DISP8_OPER
	   DB	01111010B		; JPE
	   DW	DISP8_OPER
	   DB	01111011B		; JPO
	   DW	DISP8_OPER
	   DB	01111011B		; JNP
	   DW	DISP8_OPER
	   DB	01111001B		; JNS
	   DW	DISP8_OPER
	   DB	01110001B		; JNO
	   DW	DISP8_OPER
	   DB	01110000B		; JO
	   DW	DISP8_OPER
	   DB	01111000B		; JS
	   DW	DISP8_OPER
	   DB	01111010B		; JP
	   DW	DISP8_OPER
	   DB	10011111B		; LAHF
	   DW	NO_OPER
	   DB	11000101B		; LDS
	   DW	L_OPER
	   DB	10001101B		; LEA
	   DW	L_OPER
	   DB	11000100B		; LES
	   DW	L_OPER
	   DB	11110000B		; LOCK
	   DW	NO_OPER
	   DB	10101100B		; LODB
	   DW	NO_OPER
	   DB	10101101B		; LODW
	   DW	NO_OPER
	   DB	11100000B		; LOOPNZ
	   DW	DISP8_OPER
	   DB	11100001B		; LOOPZ
	   DW	DISP8_OPER
	   DB	11100000B		; LOOPNE
	   DW	DISP8_OPER
	   DB	11100001B		; LOOPE
	   DW	DISP8_OPER
	   DB	11100010B		; LOOP
	   DW	DISP8_OPER
	   DB	10100100B		; MOVB
	   DW	NO_OPER
	   DB	10100101B		; MOVW
	   DW	NO_OPER
	   DB	11000110B		; MOV
	   DW	MOV_OPER
	   DB	4 * 8			; MUL
	   DW	GROUP1
	   DB	3 * 8			; NEG
	   DW	GROUP1
	   DB	10010000B		; NOP
	   DW	NO_OPER
	   DB	2 * 8			; NOT
	   DW	GROUP1
	   DB	11101110B		; OUT
	   DW	OUT_OPER
	   DB	10011101B		; POPF
	   DW	NO_OPER
	   DB	0 * 8			; POP
	   DW	POP_OPER
	   DB	10011100B		; PUSHF
	   DW	NO_OPER
	   DB	6 * 8			; PUSH
	   DW	PUSH_OPER
	   DB	2 * 8			; RCL
	   DW	ROTOP
	   DB	3 * 8			; RCR
	   DW	ROTOP
	   DB	11110011B		; REPZ
	   DW	NO_OPER
	   DB	11110010B		; REPNZ
	   DW	NO_OPER
	   DB	11110011B		; REPE
	   DW	NO_OPER
	   DB	11110010B		; REPNE
	   DW	NO_OPER
	   DB	11110011B		; REP
	   DW	NO_OPER
	   DB	11001011B		; RETF
	   DW	GET_DATA16
	   DB	11000011B		; RET
	   DW	GET_DATA16
	   DB	0 * 8			; ROL
	   DW	ROTOP
	   DB	1 * 8			; ROR
	   DW	ROTOP
	   DB	10011110B		; SAHF
	   DW	NO_OPER
	   DB	7 * 8			; SAR
	   DW	ROTOP
	   DB	10101110B		; SCAB
	   DW	NO_OPER
	   DB	10101111B		; SCAW
	   DW	NO_OPER
	   DB	4 * 8			; SHL
	   DW	ROTOP
	   DB	5 * 8			; SHR
	   DW	ROTOP
	   DB	11111001B		; STC
	   DW	NO_OPER
	   DB	11111101B		; STD
	   DW	NO_OPER
	   DB	11111011B		; EI
	   DW	NO_OPER
	   DB	10101010B		; STOB
	   DW	NO_OPER
	   DB	10101011B		; STOW
	   DW	NO_OPER
	   DB	11110110B		; TEST
	   DW	TST_OPER
	   DB	10011011B		; WAIT
	   DW	NO_OPER
	   DB	10000110B		; XCHG
	   DW	EX_OPER
	   DB	11010111B		; XLAT
	   DW	NO_OPER
	   DB	00100110B		; ESSEG
	   DW	NO_OPER
	   DB	00101110B		; CSSEG
	   DW	NO_OPER
	   DB	00110110B		; SSSEG
	   DW	NO_OPER
	   DB	00111110B		; DSSEG
	   DW	NO_OPER

ZZOPCODE   LABEL BYTE
MAXOP	   =	(ZZOPCODE-OPTAB)/3

SHFTAB	   DW	OFFSET DG:ROLMN,OFFSET DG:RORMN,OFFSET DG:RCLMN
	   DW	OFFSET DG:RCRMN,OFFSET DG:SHLMN,OFFSET DG:SHRMN
	   DW	OFFSET DG:BADMN,OFFSET DG:SARMN

IMMTAB	   DW	OFFSET DG:ADDMN,OFFSET DG:ORMN,OFFSET DG:ADCMN
	   DW	OFFSET DG:SBBMN,OFFSET DG:ANDMN,OFFSET DG:SUBMN
	   DW	OFFSET DG:XORMN,OFFSET DG:CMPMN

GRP1TAB    DW	OFFSET DG:TESTMN,OFFSET DG:BADMN,OFFSET DG:NOTMN
	   DW	OFFSET DG:NEGMN,OFFSET DG:MULMN,OFFSET DG:IMULMN
	   DW	OFFSET DG:DIVMN,OFFSET DG:IDIVMN

GRP2TAB    DW	OFFSET DG:INCMN,OFFSET DG:DECMN,OFFSET DG:CALLMN
	   DW	OFFSET DG:CALLMN,OFFSET DG:JMPMN,OFFSET DG:JMPMN
	   DW	OFFSET DG:PUSHMN,OFFSET DG:BADMN

SEGTAB	   DW	OFFSET DG:ESSAVE,OFFSET DG:CSSAVE,OFFSET DG:SSSAVE
	   DW	OFFSET DG:DSSAVE

REGTAB	   DB	"AX",0,"BX",0,"CX",0,"DX",0,"SP",0,"BP",0
	   DB	"SI",0,"DI",0,"DS",0,"ES",0,"SS",0,"CS",0,"IP",0,"PC",0
REGTABEND  LABEL WORD

; Flags are ordered to correspond with the bits of the flag
; register, most significant bit first, zero if bit is not
; a flag. First 16 entries are for bit set, second 16 for
; bit reset.

FLAGTAB    DW	0
	   DW	0
	   DW	0
	   DW	0
	   DB	"OV"
	   DB	"DN"
	   DB	"EI"                    ; "STI"
	   DW	0
	   DB	"NG"
	   DB	"ZR"
	   DW	0
	   DB	"AC"
	   DW	0
	   DB	"PE"
	   DW	0
	   DB	"CY"
	   DW	0
	   DW	0
	   DW	0
	   DW	0
	   DB	"NV"
	   DB	"UP"                    ; "CLD"
	   DB	"DI"
	   DW	0
	   DB	"PL"
	   DB	"NZ"
	   DW	0
	   DB	"NA"
	   DW	0
	   DB	"PO"
	   DW	0
	   DB	"NC"

	   DW	80H DUP(?)
STACK	   LABEL BYTE


; Register save area

AXSAVE	   DW	0
BXSAVE	   DW	0
CXSAVE	   DW	0
DXSAVE	   DW	0
SPSAVE	   DW	5AH
BPSAVE	   DW	0
SISAVE	   DW	0
DISAVE	   DW	0
DSSAVE	   DW	0
ESSAVE	   DW	0
RSTACK	   LABEL WORD			; Stack set here so registers can be saved by pushing
SSSAVE	   DW	0
CSSAVE	   DW	0
IPSAVE	   DW	100H
FLSAVE	    DW	 0F202H

RSETFLAG   DB	0

;  These variables used to determine if the file is larget than the   ;C01
;  amount of disk space available whenever a write occurs.            ;C01
								      ;C01
FileSizeLB DW   0                                                     ;C01
FileSizeHB DW   0                                                     ;C01
TempHB     DW   0                                                     ;C01
TempLB     DW   0                                                     ;C01
DriveOfFile DB  ?                                                     ;C01

REGDIF	   EQU	AXSAVE-REGTAB

; This value is initially 0, it is set to non-zero if a file is specified
;  either at debug invokation, or via the (N)ame command. It is used to
;  control the printing of the NONAMESPEC message for the (W)rite command.
NAMESPEC   DB	0

; RAM area.

RDFLG	   DB	READ
TOTREG	   DB	13
DSIZ	   DB	0FH			;changed to 7 if screen 40 col mode
NOREGL	   DB	8			;changed to 4 if screen 40 col mode
DISPB	   DW	128			;changed to 64 if screen 40 col mode

LBUFSIZ    DB	BUFLEN
LBUFFCNT   DB	0
LINEBUF    DB	0DH
	   DB	BUFLEN DUP (?)
PFLAG	   DB	0
COLPOS	   DB	0

	   IF	SYSVER
CONFCB	   DB	0
	   DB	"PRN        "
	   DB	25 DUP(0)

POUT	   DD	?
COUT	   DD	?
CIN	   DD	?
IOBUFF	   DB	3 DUP (?)
IOADDR	   DD	?

IOCALL	   DB	22
	   DB	0
IOCOM	   DB	0
IOSTAT	   DW	0
	   DB	8 DUP (0)
IOCHRET    DB	0
	   DW	OFFSET DG:IOBUFF
IOSEG	   DW	?
IOCNT	   DW	1
	   DW	0
	   ENDIF

QFLAG	   DB	0
NEWEXEC    DB	0
RETSAVE    DW	?

USER_PROC_PDB DW ?
NextCS	   DW	?
NextIP	   DW	?

HEADSAVE   DW	?

EXEC_BLOCK LABEL BYTE
	   DW	0
COM_LINE   LABEL DWORD
	   DW	80H
	   DW	?
COM_FCB1   LABEL DWORD
	   DW	FCB
	   DW	?
COM_FCB2   LABEL DWORD
	   DW	FCB + 10H
	   DW	?
COM_SSSP   DD	?
COM_CSIP   DD	?

CONST	   ENDS
	   END
