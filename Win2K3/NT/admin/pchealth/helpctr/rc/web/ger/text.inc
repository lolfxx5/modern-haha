<% @LANGUAGE="VBScript" @EnableSessionState="False"  %>
<% 'Option Explicit

Function GetString(ByVal sStringName)
	If InStr(sStringName, "L_") <> 1 Then sStringName = "L_" & sStringName & "_Text"

	'' Texts
	Const L_StartRemoteAssistance_Text  		= "Remoteunterst�tzung starten"
	Const L_ProductName_Text			= "Remoteunterst�tzung"
	Const L_Title1Paragraph1_Text		= "Hilfe mit Remoteunterst�tzung erteilen"
	Const L_Paragraph1_Text				= "Manchmal ist es am besten, wenn man jemand anderem die L�sung f�r ein Problem zeigen kann. Remoteunterst�tzung ist eine sichere Verbindung, mit der Sie einem Freund beim L�sen eines Computerproblems helfen k�nnen."
	Const L_Paragraph2_Text				= "Lesen Sie die Anmerkungen unten, falls Sie Remoteunterst�tzung zum ersten Mal verwenden."
	Const L_StartRAMessage_Text			= "<font color=""#000000"" size=""2"">Klicken Sie auf </font><b><font color=""#000000"" size=""2""><b>Remoteunterst�tzung starten</b></font></b>, um eine Verbindung mit dem anderem Computer herzustellen. "
	Const L_StartRAButton_Text			= "Remoteunterst�tzung starten"
	Const L_JScriptNotAvail1_Text		= "Remoteunterst�tzung kann aufgrund der aktuellen Internet Explorer-Sicherheitseinstellungen nicht verwendet werden. �ndern Sie die Einstellungen, oder f�gen Sie diese Site als vertrauensw�rdige Site hinzu, um Remoteunterst�tzung verwenden zu k�nnen."
	Const L_IEHelp_Text					= "Weitere Informationen �ber das �ndern der Sicherheitseinstellungen erhalten Sie in der Internet Explorer-Hilfe."
	Const L_TicketError1_Text			= "Die Remoteunterst�tzungseinladung fehlt oder ist ung�ltig. Sie ben�tigen eine g�ltige Einladung, um Remoteunterst�tzung verwenden zu k�nnen."
	Const L_InstallIE_Text				= "Internet Explorer 5.5 muss installiert sein, damit Remoteunterst�tzungseinladungen angenommen werden k�nnen."
	Const L_TicketErrorMsg_Text			= "Klicken Sie in Windows XP auf <b>Start</b> und dann auf <b>Hilfe und Support</b>, und geben Sie <b>Remoteunterst�tzung</b> im Feld <b>Suchen</b> ein, um weitere Informationen �ber Remoteunterst�tzung zu erhalten."
	
	Const L_NotesTitle_Text				= "Anmerkungen"
	Const L_NotesLine1_Text				= "Nehmen Sie nur Einladungen von vertrauensw�rdigen Personen an."
	
	Const L_NotesLine2_Text				= "Sie und der andere Teilnehmer m�ssen �ber eine Verbindung mit dem Internet verf�gen, w�hrend Sie Remoteunterst�tzung verwenden. Falls der Absender dieser Einladung verf�gbar ist, k�nnen Sie den Bildschirm des anderen Teilnehmers einsehen und sich miteinander in einem Echtzeitchat unterhalten."
	
	Const L_NotesLine6_Text				= "Die Verbindung ist sicher und unidirektional, d. h. der andere Teilnehmer kann nicht auf Ihren Computer zugreifen. In einem LAN-Netzwerk kann es sein, dass Firewalls Sie daran hindern, Remoteunterst�tzung zu verwenden. Wenden Sie sich in diesem Fall an den Netzwerkadministrator, um weitere Informationen zu erhalten."
	
	Const L_LearnXP_Text				= "Klicken Sie in Windows XP auf <b>Start</b> und dann auf <b>Hilfe und Support</b>, und geben Sie <b>Remoteunterst�tzung</b> im Feld <b>Suchen</b> ein, um weitere Informationen �ber Remoteunterst�tzung zu erhalten."
	
	Const L_ContactUsMsg_Text			= "So erreichen Sie uns"
	

	Const L_XPNewsletterMsg_Text		= "Newsletter abonnieren"
	Const L_EmailPage_Text				= "Empfehlen Sie diese Seite"
	
	Const L_CopyRightMsg_Text			= "(C) 2001 Microsoft Corporation. Alle Rechte vorbehalten. Rechtliche Hinweise."
	
	Const L_PrivacyMsg_Text				= "Datenschutzrichtlinie"

	Const L_AccMsg_Text					= "Eingabehilfen"

	Const L_NotIEError1_Text			= "Internet Explorer 5.5 oder h�her muss installiert sein, damit Remoteunterst�tzungseinladungen angenommen werden k�nnen."

	Const L_NotXPError1_Text			= "Microsoft-Remoteunterst�tzung ist eine Funktion von Windows&nbsp;XP. Windows&nbsp;XP muss installiert sein, damit Remoteunterst�tzung verwendet werden kann."

	Const L_IELinkMsg_Text				= "Die aktuellste Version von Internet Explorer downloaden"
	Const L_XPLinkMsg_Text				= "Weitere Informationen �ber Windows XP erhalten Sie auf der "
	Const L_XPLinkMsg2_Text				= "Microsoft Windows-Website"
	Const L_XPLinkMsg3_Text				= "."
	
	Const L_IntroMsg1_Text              = "Eine f�r Remoteunterst�tzung erforderliche Komponente wird �bertragen. Die Downloadzeit betr�gt mit einem 28,8 kBit/s Modem ca. 1 Minute."
    Const L_IntroMsg2_Text              = "Active Scripting muss aktiviert sein, damit Remoteunterst�tzung ausgef�hrt werden kann. Vergewissern Sie sich, dass Scripting aktiviert ist, falls nach der gesch�tzten Downloadzeit keine Best�tigung f�r den Abschluss des Downloads angezeigt wurde."
    Const L_IntroMsg3_Text              = "So aktivieren Sie Active Scripting"
    Const L_IntroMsg4_Text              = "Klicken Sie im Men� <B>Tools</B> in Internet Explorer auf <B>Internetoptionen</B>."
    Const L_IntroMsg5_Text              = "Klicken Sie auf die Registerkarte <B>Sicherheit</B>."
    Const L_IntroMsg6_Text              = "Klicken Sie auf das Symbol <STRONG>Internet</STRONG> und dann auf <STRONG>Stufe anpassen</STRONG>."
    Const L_IntroMsg7_Text              = "Klicken Sie im Abschnitt <STRONG>Scripting</STRONG> unter <STRONG>Active Scripting</STRONG> auf <STRONG>Aktivieren</STRONG>."
    
	' **************************************
	' Links - No need for localization
	' **************************************
	Const L_AccLink_Text				= "http://www.microsoft.com/enable/"		
	Const L_XPLink_Text					= "http://www.microsoft.com/windowsxp/default.asp"
	Const L_PrivacyLink_Text			= "http://www.microsoft.com/info/privacy.htm"
	Const L_ContactUsLink_Text			= "http://register.microsoft.com/contactus30/contactus.asp?domain=windows&amp;lcid=1033&amp;cu=http://register.microsoft.com/contactus30/contactus.asp?domain=windows"
	Const L_MailTo1_Text				= "mailto:?subject=Ein Microsoft-Link&body=Dieser Link ist von der Microsoft Windows XP-Website"
	Const L_CopyRightLink_Text			= "http://www.microsoft.com/info/cpyright.htm"
	Const L_XPNewsletterLink_Text		= "http://www.microsoft.com/windowsxp/newsletters.asp"
	Const L_LinkToIE_Text				= "http://www.microsoft.com/windows/ie/default.htm"
	'*************************************	
	
	GetString = Eval(sStringName)
End Function

%>