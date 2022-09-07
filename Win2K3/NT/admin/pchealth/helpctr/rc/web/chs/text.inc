<% @LANGUAGE="VBScript" @EnableSessionState="False"  %>
<% 'Option Explicit

Function GetString(ByVal sStringName)
	If InStr(sStringName, "L_") <> 1 Then sStringName = "L_" & sStringName & "_Text"

	'' Texts
	Const L_StartRemoteAssistance_Text  		= "����Զ��Э��"
	Const L_ProductName_Text			= "Զ��Э��"
	Const L_Title1Paragraph1_Text		= "��Զ��Э���ṩ����"
	Const L_Paragraph1_Text				= "��ʱ���������˽���������Ѱ취����ʾ�����ʽ��Զ��Э���ṩ��ȫ���ӣ�������������һ������������⡣"
	Const L_Paragraph2_Text				= "����ǳ���ʹ��Զ��Э�������Ķ����е�ע�����"
	Const L_StartRAMessage_Text			= "<font color=""#000000"" size=""2"">Ҫ���ӵ����ѵļ�����ϣ��뵥��</font><b><font color=""#000000"" size=""2""><b>������Զ��Э������</b></font></b>"
	Const L_StartRAButton_Text			= "����Զ��Э��"
	Const L_JScriptNotAvail1_Text		= "��ǰ�� Internet Explorer ��ȫ�������̫�ߣ����޷�ʹ��Զ��Э����Ҫʹ��Զ��Э������������ã������վ����Ϊ������վ�����ӡ�"
	Const L_IEHelp_Text					= "�йظ��İ�ȫ���õ���Ϣ����μ� Internet Explorer ������"
	Const L_TicketError1_Text			= "����Զ��Э�������Ҳ�������Ч��Ҫʹ��Զ��Э��������Ҫһ����Ч���롣"
	Const L_InstallIE_Text				= "Ҫ����Զ��Э�����룬�����밲װ Internet Explorer 5.5��"
	Const L_TicketErrorMsg_Text			= "Ҫ��һ���˽�Զ��Э�����뵥�� Windows XP �е�<B>��ʼ</B>������<B>������֧��</B>��Ȼ����<B>����</B>���м���<B>Զ��Э��</B>��"
	
	Const L_NotesTitle_Text				= "ע��"
	Const L_NotesLine1_Text				= "ֻ���������ε��������������롣"
	
	Const L_NotesLine2_Text				= "��ʹ��Զ��Э��ʱ���������������˶��������ӵ� Internet �ϡ��������������˿��Խ��������ӵ���/���ļ�����������������Կ�����/���ļ������Ļ��������/������ʵʱ�Ի���"
	
	Const L_NotesLine6_Text				= "����һ����ȫ�ĵ������ӡ����κ�ʱ�������ӵ��ļ�������������޷�����/���ļ�����������ļ�����������ʹ�õ��Ǿ�����������ǽ���ܻ�ʹ���޷�ʹ��Զ��Э��������������£�����������Ԫ��ϵ��"
	
	Const L_LearnXP_Text				= "Ҫ��һ���˽�Զ��Э�����뵥�� Windows XP �е�<B>��ʼ</B>������<B>������֧��</B>��Ȼ����<B>����</B>���м���<B>Զ��Э��</B>��"
	
	Const L_ContactUsMsg_Text			= "��������ϵ"
	

	Const L_XPNewsletterMsg_Text		= "����ʼ�"
	Const L_EmailPage_Text				= "���ͱ�ҳ������"
	
	Const L_CopyRightMsg_Text			= "(C) 2001 Microsoft Corporation. ��������Ȩ����ʹ�ù涨��"
	
	Const L_PrivacyMsg_Text				= "��˽����"

	Const L_AccMsg_Text					= "��������"

	Const L_NotIEError1_Text			= "Ҫ����Զ��Э�����룬�����밲װ Internet Explorer 5.5 ����µİ汾��"

	Const L_NotXPError1_Text			= "Microsoft Զ��Э���� Windows&nbsp;XP ��һ�����ܡ�Ϊ��ʹ��Զ��Э�����������Ȱ�װ Windows&nbsp;XP��"

	Const L_IELinkMsg_Text				= "�������°汾�� Internet Explorer"
	Const L_XPLinkMsg_Text				= "Ҫ��һ���˽� Windows XP����ι�  "
	Const L_XPLinkMsg2_Text				= "Microsoft Windows ��վ"
	Const L_XPLinkMsg3_Text				= "��"
	
	Const L_IntroMsg1_Text              = "����һ�������Ҫʹ��Զ��Э����������ĵ��ƽ������ 28.8 kbps �ģ����ؿ��ܻỨһ���ӡ�"
    Const L_IntroMsg2_Text              = "Ҫ����Զ��Э�����������û�ű�������ڹ�������ʱ��֮�����ҳ�滹��ȷ����������ɣ�����֤�ű��Ƿ������á�"
    Const L_IntroMsg3_Text              = "Ҫ���� Internet Explorer ��"
    Const L_IntroMsg4_Text              = "<B>����</B>�˵��ϵĻ�ű����뵥��<B>Internet ѡ��</B>��"
    Const L_IntroMsg5_Text              = "����<B>��ȫ</B>ѡ���"
    Const L_IntroMsg6_Text              = "����<STRONG>Internet</STRONG>ͼ�꣬Ȼ�󵥻�<STRONG>�Զ��弶��</STRONG>��"
    Const L_IntroMsg7_Text              = "�ڡ��ű��������У�<STRONG>��ű�</STRONG>�£�����<STRONG>����</STRONG>��"
    
	' **************************************
	' Links - No need for localization
	' **************************************
	Const L_AccLink_Text				= "http://www.microsoft.com/enable/"		
	Const L_XPLink_Text					= "http://www.microsoft.com/windowsxp/default.asp"
	Const L_PrivacyLink_Text			= "http://www.microsoft.com/info/privacy.htm"
	Const L_ContactUsLink_Text			= "http://register.microsoft.com/contactus30/contactus.asp?domain=windows&amp;lcid=1033&amp;cu=http://register.microsoft.com/contactus30/contactus.asp?domain=windows"
	Const L_MailTo1_Text				= "mailto:?subject=���� Microsoft ������&body=����������� Microsoft Windows XP ��վ"
	Const L_CopyRightLink_Text			= "http://www.microsoft.com/info/cpyright.htm"
	Const L_XPNewsletterLink_Text		= "http://www.microsoft.com/windowsxp/newsletters.asp"
	Const L_LinkToIE_Text				= "http://www.microsoft.com/windows/ie/default.htm"
	'*************************************	
	
	GetString = Eval(sStringName)
End Function

%>