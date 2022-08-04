#pragma warning(disable:4996)
#include<windows.h>
#include<iostream>
#include<string>
#include<queue>
#include <regex>

#include <openssl/ssl.h>
#include <openssl/err.h>
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib, "urlmon.lib")
#pragma comment( lib, "libcrypto.lib" )
#pragma comment( lib, "libssl.lib" )

#ifdef  __cplusplus
extern "C" {
#endif
#include <openssl/applink.c>
#ifdef  __cplusplus
}
#endif

using namespace std;
int number = 1;
string g_sHost;					//http������
string g_sObject;				//http����
char g_Host[MAX_PATH];			//https������
char g_Object[MAX_PATH];		//https����
SOCKET g_sock;					//�ͻ����׽���
SSL* sslHandle;
SSL_CTX* sslContext;
BIO* bio;

//��ӭ����	����
void welcom();

//���··��	����
bool file_path();

//��ʼץȡ�������URL	����
void StartCatch_https(string startUrl);		//ʹ��https��ʼץȡ�������URL	����
void StartCatch_http(string startUrl);		//ʹ��http��ʼץȡ�������URL	����

//����URL	����
bool Analyse(string url,string startUrl);

//���ӷ�����	����
bool Connect_https();					//httpsЭ�齨��TCP����
bool Connect_http();					//httpЭ�齨��TCP����

//����SSl����	����
bool SSL_Connect();

//�õ�html	����
bool Get_https_html(string& html);		//https�õ�html	����
string Get_http_html(string& url);		//http�õ�html	����

//UTFתGBK	����
std::string UtfToGbk(const char* utf8);
LPCWSTR stringToLPCWSTR(std::string orig);

//����URL���	����
bool results_https(string html, string startUrl);		//ʹ��httpsЭ�����URL���	����
bool results_http(string html, string startUrl);		//ʹ��httpЭ�����URL���	����

//����ͼƬ	����
bool Download(string imagurl);

//�ͷ�����
bool Release(string url);

int main()
{
	//��ӭʹ��
	 welcom();

	cout << "һ��������" << number-1 << "��ͼƬ" << endl;

	system("pause");
	return 0;
}

void welcom()
{
	printf("**********************************\n");
	printf("*                                *\n");
	printf("*      ��ã���ӭʹ�ã�          *\n");
	printf("*                                *\n");
	printf("*         ����������             *\n");
	printf("*                                *\n");
	printf("*      QQ��3534358548            *\n");
	printf("*                                *\n");
	printf("**********************************\n");
	printf("�����һ��֧�㣨URL��ڣ���");

	//�鿴�Ƿ��д洢������Ƭ��Ŀ¼
	if (access("G:\\tupian", 0) != 0)
	{
		CreateDirectory(L"G:\\tupian", NULL);
	}

	//��ʼץȡ
	string starturl;
	cin >> starturl;

	//�ж���httpЭ�黹��httpsЭ��
	if (starturl.find("https://") == string::npos)
	{
		if (starturl.find("http://") == string::npos)
		{
			printf("����������URL����URLǰ����ϡ�http://�����ߡ�https://��\n");
		}
		else
		{
			//ʹ��httpЭ��ץȡ
			StartCatch_http(starturl);
		}

	}
	else
	{
		//ʹ��httpsЭ��ץȡ
		StartCatch_https(starturl);
	}

}

//���·��
bool file_path()
{
	if (access("G:\\tupian", 0) == 0)
	{
		CreateDirectory(L"G:\\tupian", NULL);
	}
	return true;
}

//ʹ��httpsЭ�鿪ʼץȡ�������URL
void StartCatch_https(string startUrl)
{
	queue<string> q;
	q.push(startUrl);

	while (!q.empty())
	{
		string cururl = q.front();
		q.pop();

		//����URL
		if (false == Analyse(cururl, startUrl))
		{
			cout << "����URLʧ�ܣ������룺" << GetLastError() << endl;
			continue;
		}

		//���ӷ�����
		if (false == Connect_https())
		{
			cout << "���ӷ�����ʧ�ܣ�������룺" << GetLastError() << endl;
			continue;
		}

		//����ssl����
		if (false == SSL_Connect())
		{
			cout << "����SSL����ʧ�ܣ�������룺" << GetLastError() << endl;
			continue;
		}

		//��ȡ��ҳ
		string html;
		if (false == (Get_https_html(html)))
		{
			cout << "��ȡ��ҳ����ʧ�ܣ�������룺" << GetLastError() << endl;
			continue;
		}
		
		//��ȡͼƬ��URL���������ͼƬ
		results_https(html, startUrl);

	}
	while (!q.empty())
	{
		string cururl = q.front();
		Release(cururl);
	}

}

//ʹ��http��ʼץȡ�������URL	����
void StartCatch_http(string startUrl) 
{
	queue<string> q;
	q.push(startUrl);

	while (!q.empty())
	{
		string cururl = q.front();
		q.pop();

		//string html = Get_http_html(html);


		//����URL
		if (false == Analyse(cururl, startUrl))
		{
			cout << "����URLʧ�ܣ������룺" << GetLastError() << endl;
			continue;
		}

		//���ӷ�����
		if (false == Connect_http())
		{
			cout << "���ӷ�����ʧ�ܣ�������룺" << GetLastError() << endl;
			continue;
		}

		//��ȡ��ҳ
		string html= Get_http_html(html);
		//cout << html << endl;				//�����ҳ
		if (html == "")
		{
			cout << "��ȡ��ҳ����ʧ�ܣ�������룺" << GetLastError() << endl;
			continue;
		}

		//��ȡͼƬ��URL���������ͼƬ
		results_http(html, startUrl);
	}
}

bool Release(string url)
{
	char* pUrl = new char[url.length() + 1];
	if (char* pos = strstr(pUrl, "https://"))
	{
		//�ͷ�
		SSL_shutdown(sslHandle);
		SSL_free(sslHandle);
		SSL_CTX_free(sslContext);
	}
	//�ص��׽���
	closesocket(g_sock);
	//�ͷ�����
	WSACleanup();
	
	return true;
}

//����url
bool Analyse(string url, string startUrl)
{
	char* pUrl = new char[url.length() + 1];
	strcpy(pUrl, url.c_str());

	if (char* pos = strstr(pUrl, "https://"))//�ҵ�https://��ͷ���ַ���
	{
		//char* po = strstr(pUrl, "https://");
		if (pos == NULL) return false;
		else pos += 8;//��http://��ͷʡ��

		sscanf(pos, "%[^/]%s", g_Host, g_Object);
		printf("������%s\n", g_Host);
		printf("��Դ��%s\n", g_Object);
		delete[] pUrl;
	}
	else {
		if (string::npos == url.find("http://"))
			return false;
		if(url.length()<=7)
			return false;
		int pos1 = url.find('/', 7);
		if (pos1 == string::npos)
		{
			g_sHost = url.substr(7);
			g_sObject = "/";

		}
		else
		{
			g_sHost = url.substr(7, pos1 - 7);
			g_sObject = url.substr(pos1);
		}
		if (g_sHost.empty() || g_sObject.empty())
			return false;
		
		cout << "������" << g_sHost << endl;
		cout << "��Դ��" << g_sObject << endl;

	
	}
	return true;
}

//����TCP����
bool Connect_https()					//httpsЭ�齨��TCP����
{
	//��ʼ���׽���
	WSADATA wsadata;
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsadata))
		return false;

	//�����׽���
	g_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (g_sock == INVALID_SOCKET)
		return false;

	//������ת��ΪIP��ַ
	hostent* p = gethostbyname(g_Host);					//https
	//httpsЭ��
	for (int i = 0; p->h_aliases[i]; i++)
	{
		printf("��վ�� %d: %s\n", i + 1, p->h_aliases[i]);
	}
	//��ַ����
	printf("��վ����: %s\n", (p->h_addrtype == AF_INET) ? "AF_INET" : "AF_INET6");
	//��վIP��ַ
	for (int i = 0; p->h_addr_list[i]; i++)
	{
		printf("��վIP%d: %s\n", i + 1, inet_ntoa(*(struct in_addr*)p->h_addr_list[i]));
	}

	sockaddr_in sa;
	memcpy(&sa.sin_addr, p->h_addr, 4);
	sa.sin_family = AF_INET;
	sa.sin_port = htons(443);
	//��������
	if (SOCKET_ERROR == connect(g_sock, (sockaddr*)&sa, sizeof(sockaddr))) 
		return false;

	return true;
}
bool Connect_http()			//httpЭ�齨��TCP����
{
	//��ʼ���׽���
	WSADATA wsadata;
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsadata))
		return false;

	//�����׽���
	g_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (g_sock == INVALID_SOCKET)
		return false;

	//������ת��ΪIP��ַ
	hostent* p = gethostbyname(g_sHost.c_str());
	//httpЭ��
	if (p == NULL)
		return false;
	//����web������
	sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(80);
	memcpy(&sa.sin_addr, p->h_addr, 4);

	//�������� 
	if(connect(g_sock,(sockaddr*) &sa,sizeof(sockaddr)))
		return false;

	return true;
}

//����SSl����
bool SSL_Connect()
{
	// Register the error strings for libcrypto & libssl

	ERR_load_BIO_strings();
	// SSl��ĳ�ʼ��������SSL�������㷨���������е�SSL������Ϣ
	SSL_library_init();
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();

	// New context saying we are a client, and using SSL 2 or 3
	sslContext = SSL_CTX_new(SSLv23_client_method());
	if (sslContext == NULL)
	{
		ERR_print_errors_fp(stderr);
		return false;
	}
	// Create an SSL struct for the connection
	sslHandle = SSL_new(sslContext);
	if (sslHandle == NULL)
	{
		ERR_print_errors_fp(stderr);
		return false;
	}
	// Connect the SSL struct to our connection
	if (!SSL_set_fd(sslHandle, g_sock))
	{
		ERR_print_errors_fp(stderr);
		return false;
	}
	// Initiate SSL handshake
	if (SSL_connect(sslHandle) != 1)
	{
		ERR_print_errors_fp(stderr);
		return false;
	}

	return true;
}

//�õ� html
bool Get_https_html(string& html)
{
	char temp1[100];
	sprintf(temp1, "%d", 166);
	string c_get;
	c_get = c_get
		+ "GET " + g_Object + " HTTP/1.1\r\n"
		+ "Host: " + g_Host + "\r\n"
		+ "Content-Type: text/html; charset=UTF-8\r\n"
		//+ "Content-Length:" + temp1 + "\r\n"
		//+ "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.36 Edge/16.16299\r\n"
		+ "Connection:Close\r\n\r\n";
	//+ temp;

	SSL_write(sslHandle, c_get.c_str(), c_get.length());

	char buff[101];
	int nreal = 0;

	while ((nreal = SSL_read(sslHandle, buff, 100)) > 0)
	{
		buff[nreal] = '\0';
		html += UtfToGbk(buff);
		//printf("%s\n", buff);
		memset(buff, 0, sizeof(buff));
	}

	//printf("%s\n", html);
	return true;
}
string Get_http_html(string& url)
{

	//��ȡ��ҳ����Get����
	string info;
	info += "GET " + g_sObject + " HTTP/1.1\r\n";
	info += "Host: " + g_sHost + "\r\n";
	info += "Connection: Close\r\n\r\n";

	if (SOCKET_ERROR == send(g_sock, info.c_str(), info.length(), 0))
		return "";

	//��������
	char ch=0;
	string html;
	while (recv(g_sock,&ch,sizeof(char),0))
	{
		html += ch;
	}

	return html;
}

//UTFתGBK
string UtfToGbk(const char* utf8)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wstr, len);
	len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);
	if (wstr) delete[] wstr;
	return str;
}
LPCWSTR stringToLPCWSTR(string orig)
{
	size_t origsize = orig.length() + 1;
	const size_t newsize = 100;
	size_t convertedChars = 0;
	wchar_t* wcstring = (wchar_t*)malloc(sizeof(wchar_t) * (orig.length() - 1));
	mbstowcs_s(&convertedChars, wcstring, origsize, orig.c_str(), _TRUNCATE);

	return wcstring;
}

//����URL���
bool results_https(string html, string startUrl)
{
	//ƥ������е�URL   ������ʽ
	smatch mat;
	string::const_iterator start = html.begin();
	string::const_iterator end = html.end();
	regex gex("https://[^\\s'\"<>()]+");
	vector<string> vecurl;
	int number = 1;		//����һ����������
	if (regex_search(start, end, mat, gex) != 0)
	{
		while (regex_search(start, end, mat, gex))
		{
			string newurl(mat[0].first, mat[0].second);

			//��ȡ����ͼƬ������	��ͼƬ����1������ʽ��ӡ����
			cout << "ͼƬ����" << number << ":" << newurl << endl;
			number++;

			//�ѻ�ȡ����URL������
			vecurl.push_back(newurl);
			start = mat[0].second;

		}

		//�������е�URL
		for (int x = 0; x < vecurl.size() - 1; x++)
		{
			for (int j = x + 1; j < vecurl.size(); j++)
			{
				if (vecurl[x] != vecurl[j])
				{
					//�ж��ǲ���ͼƬ
					string imagurl = vecurl[x];
					if (imagurl.find(".jpg")  != string::npos)
					{
						Download(imagurl);
					}
					else if (imagurl.find(".png") != string::npos)
					{
						Download(imagurl);
					}
				}
				break;
			}

		}
	}
	else
	{
		Download(startUrl);
		cout << startUrl << endl;
	}
	return true;
}
bool results_http(string html, string startUrl)
{

	//ƥ������е�URL   ������ʽ
	smatch mat;
	string::const_iterator start = html.begin();
	string::const_iterator end = html.end();
	regex gex("http://[^\\s'\"<>()]+");
	vector<string> vecurl;
	int number = 1;		//����һ����������

	while (regex_search(start, end, mat, gex))
	{
		string newurl(mat[0].first, mat[0].second);

		//��ȡ����ͼƬ������	��ͼƬ����1������ʽ��ӡ����
		cout << "ͼƬ����" << number << ":" << newurl << endl;
		number++;

		//�ѻ�ȡ����URL������
		vecurl.push_back(newurl);
		start = mat[0].second;
	}

	//�������е�URL
	for (int x = 0; x < vecurl.size() - 1; x++)
	{
		for (int j = x + 1; j < vecurl.size(); j++)
		{
			if (vecurl[x] != vecurl[j])
			{
				//�ж��ǲ���ͼƬ
				string imagurl = vecurl[x];
				if (imagurl.find(".jpg") != string::npos)
				{
					Download(imagurl);
				}
				else if (imagurl.find(".png") != string::npos)
				{
					Download(imagurl);
				}
			}
			break;
		}

	}
	return true;
}

//����ͼƬ
bool Download(string imagurl)
{
	//�����ͼƬ
	//��URL����ת��
	string url = imagurl;
	size_t len = url.length();//��ȡ�ַ�������
	int nmlen = MultiByteToWideChar(CP_ACP, 0, url.c_str(), len + 1, NULL, 0);
	wchar_t* buffer = new wchar_t[nmlen];
	MultiByteToWideChar(CP_ACP, 0, url.c_str(), len + 1, buffer, nmlen);
	//��URLת�����

	//�����ļ�·��  ���ļ�������1��2��3......����
	char filename[128], tempfp[128];
	strcpy(filename, R"+*(G:\\tupian\\)+*");  //����·��
	sprintf(tempfp, "%d", number);
	strcat(filename, tempfp);
	strcat(filename, ".jpg");   //���ص��ļ��ĸ�ʽ����
	printf("%s\n", filename);

	//ת���ļ����֣�����·����
	string name = filename;
	size_t lenname = name.length();//��ȡ�ַ�������
	int namelen = MultiByteToWideChar(CP_ACP, 0, name.c_str(), lenname + 1, NULL, 0);
	wchar_t* buffer1 = new wchar_t[namelen];
	MultiByteToWideChar(CP_ACP, 0, name.c_str(), lenname + 1, buffer1, nmlen);
	//ת���ļ����֣�����·����  ת�����

	HRESULT hr = URLDownloadToFile(NULL, buffer, buffer1, 0, NULL);			//���غ���
	number++;

	//����Ƿ����سɹ�
	if (hr == S_OK)
	{
		cout << "-------ok" << endl;

	}
	return true;
}