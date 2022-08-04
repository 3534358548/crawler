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
string g_sHost;					//http主机名
string g_sObject;				//http域名
char g_Host[MAX_PATH];			//https主机名
char g_Object[MAX_PATH];		//https域名
SOCKET g_sock;					//客户端套接字
SSL* sslHandle;
SSL_CTX* sslContext;
BIO* bio;

//欢迎函数	声明
void welcom();

//检查路路径	声明
bool file_path();

//开始抓取所输入的URL	声明
void StartCatch_https(string startUrl);		//使用https开始抓取所输入的URL	声明
void StartCatch_http(string startUrl);		//使用http开始抓取所输入的URL	声明

//解析URL	声明
bool Analyse(string url,string startUrl);

//连接服务器	声明
bool Connect_https();					//https协议建立TCP连接
bool Connect_http();					//http协议建立TCP连接

//建立SSl连接	声明
bool SSL_Connect();

//得到html	声明
bool Get_https_html(string& html);		//https得到html	声明
string Get_http_html(string& url);		//http得到html	声明

//UTF转GBK	声明
std::string UtfToGbk(const char* utf8);
LPCWSTR stringToLPCWSTR(std::string orig);

//分析URL结果	声明
bool results_https(string html, string startUrl);		//使用https协议分析URL结果	声明
bool results_http(string html, string startUrl);		//使用http协议分析URL结果	声明

//下载图片	声明
bool Download(string imagurl);

//释放网络
bool Release(string url);

int main()
{
	//欢迎使用
	 welcom();

	cout << "一共下载了" << number-1 << "张图片" << endl;

	system("pause");
	return 0;
}

void welcom()
{
	printf("**********************************\n");
	printf("*                                *\n");
	printf("*      你好，欢迎使用！          *\n");
	printf("*                                *\n");
	printf("*         随遇而安。             *\n");
	printf("*                                *\n");
	printf("*      QQ：3534358548            *\n");
	printf("*                                *\n");
	printf("**********************************\n");
	printf("请给我一个支点（URL入口）：");

	//查看是否有存储下载照片的目录
	if (access("G:\\tupian", 0) != 0)
	{
		CreateDirectory(L"G:\\tupian", NULL);
	}

	//开始抓取
	string starturl;
	cin >> starturl;

	//判断是http协议还是https协议
	if (starturl.find("https://") == string::npos)
	{
		if (starturl.find("http://") == string::npos)
		{
			printf("请入完整的URL：将URL前面加上“http://”或者“https://”\n");
		}
		else
		{
			//使用http协议抓取
			StartCatch_http(starturl);
		}

	}
	else
	{
		//使用https协议抓取
		StartCatch_https(starturl);
	}

}

//检查路径
bool file_path()
{
	if (access("G:\\tupian", 0) == 0)
	{
		CreateDirectory(L"G:\\tupian", NULL);
	}
	return true;
}

//使用https协议开始抓取所输入的URL
void StartCatch_https(string startUrl)
{
	queue<string> q;
	q.push(startUrl);

	while (!q.empty())
	{
		string cururl = q.front();
		q.pop();

		//解析URL
		if (false == Analyse(cururl, startUrl))
		{
			cout << "解析URL失败，错误码：" << GetLastError() << endl;
			continue;
		}

		//连接服务器
		if (false == Connect_https())
		{
			cout << "连接服务器失败，错误代码：" << GetLastError() << endl;
			continue;
		}

		//建立ssl连接
		if (false == SSL_Connect())
		{
			cout << "建立SSL连接失败，错误代码：" << GetLastError() << endl;
			continue;
		}

		//获取网页
		string html;
		if (false == (Get_https_html(html)))
		{
			cout << "获取网页数据失败，错误代码：" << GetLastError() << endl;
			continue;
		}
		
		//获取图片的URL结果并下载图片
		results_https(html, startUrl);

	}
	while (!q.empty())
	{
		string cururl = q.front();
		Release(cururl);
	}

}

//使用http开始抓取所输入的URL	声明
void StartCatch_http(string startUrl) 
{
	queue<string> q;
	q.push(startUrl);

	while (!q.empty())
	{
		string cururl = q.front();
		q.pop();

		//string html = Get_http_html(html);


		//解析URL
		if (false == Analyse(cururl, startUrl))
		{
			cout << "解析URL失败，错误码：" << GetLastError() << endl;
			continue;
		}

		//连接服务器
		if (false == Connect_http())
		{
			cout << "连接服务器失败，错误代码：" << GetLastError() << endl;
			continue;
		}

		//获取网页
		string html= Get_http_html(html);
		//cout << html << endl;				//输出网页
		if (html == "")
		{
			cout << "获取网页数据失败，错误代码：" << GetLastError() << endl;
			continue;
		}

		//获取图片的URL结果并下载图片
		results_http(html, startUrl);
	}
}

bool Release(string url)
{
	char* pUrl = new char[url.length() + 1];
	if (char* pos = strstr(pUrl, "https://"))
	{
		//释放
		SSL_shutdown(sslHandle);
		SSL_free(sslHandle);
		SSL_CTX_free(sslContext);
	}
	//关掉套接字
	closesocket(g_sock);
	//释放网络
	WSACleanup();
	
	return true;
}

//解析url
bool Analyse(string url, string startUrl)
{
	char* pUrl = new char[url.length() + 1];
	strcpy(pUrl, url.c_str());

	if (char* pos = strstr(pUrl, "https://"))//找到https://开头的字符串
	{
		//char* po = strstr(pUrl, "https://");
		if (pos == NULL) return false;
		else pos += 8;//将http://开头省略

		sscanf(pos, "%[^/]%s", g_Host, g_Object);
		printf("域名：%s\n", g_Host);
		printf("资源：%s\n", g_Object);
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
		
		cout << "域名：" << g_sHost << endl;
		cout << "资源：" << g_sObject << endl;

	
	}
	return true;
}

//建立TCP连接
bool Connect_https()					//https协议建立TCP连接
{
	//初始化套接字
	WSADATA wsadata;
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsadata))
		return false;

	//创建套接字
	g_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (g_sock == INVALID_SOCKET)
		return false;

	//将域名转换为IP地址
	hostent* p = gethostbyname(g_Host);					//https
	//https协议
	for (int i = 0; p->h_aliases[i]; i++)
	{
		printf("网站名 %d: %s\n", i + 1, p->h_aliases[i]);
	}
	//地址类型
	printf("网站类型: %s\n", (p->h_addrtype == AF_INET) ? "AF_INET" : "AF_INET6");
	//网站IP地址
	for (int i = 0; p->h_addr_list[i]; i++)
	{
		printf("网站IP%d: %s\n", i + 1, inet_ntoa(*(struct in_addr*)p->h_addr_list[i]));
	}

	sockaddr_in sa;
	memcpy(&sa.sin_addr, p->h_addr, 4);
	sa.sin_family = AF_INET;
	sa.sin_port = htons(443);
	//连接网络
	if (SOCKET_ERROR == connect(g_sock, (sockaddr*)&sa, sizeof(sockaddr))) 
		return false;

	return true;
}
bool Connect_http()			//http协议建立TCP连接
{
	//初始化套接字
	WSADATA wsadata;
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsadata))
		return false;

	//创建套接字
	g_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (g_sock == INVALID_SOCKET)
		return false;

	//将域名转换为IP地址
	hostent* p = gethostbyname(g_sHost.c_str());
	//http协议
	if (p == NULL)
		return false;
	//连接web服务器
	sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(80);
	memcpy(&sa.sin_addr, p->h_addr, 4);

	//连接网络 
	if(connect(g_sock,(sockaddr*) &sa,sizeof(sockaddr)))
		return false;

	return true;
}

//建立SSl连接
bool SSL_Connect()
{
	// Register the error strings for libcrypto & libssl

	ERR_load_BIO_strings();
	// SSl库的初始化，载入SSL的所有算法，载入所有的SSL错误信息
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

//得到 html
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

	//获取网页发送Get请求
	string info;
	info += "GET " + g_sObject + " HTTP/1.1\r\n";
	info += "Host: " + g_sHost + "\r\n";
	info += "Connection: Close\r\n\r\n";

	if (SOCKET_ERROR == send(g_sock, info.c_str(), info.length(), 0))
		return "";

	//接收数据
	char ch=0;
	string html;
	while (recv(g_sock,&ch,sizeof(char),0))
	{
		html += ch;
	}

	return html;
}

//UTF转GBK
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

//分析URL结果
bool results_https(string html, string startUrl)
{
	//匹配出所有的URL   正则表达式
	smatch mat;
	string::const_iterator start = html.begin();
	string::const_iterator end = html.end();
	regex gex("https://[^\\s'\"<>()]+");
	vector<string> vecurl;
	int number = 1;		//定义一个链接索引
	if (regex_search(start, end, mat, gex) != 0)
	{
		while (regex_search(start, end, mat, gex))
		{
			string newurl(mat[0].first, mat[0].second);

			//获取到的图片链接以	“图片链接1：”格式打印出来
			cout << "图片链接" << number << ":" << newurl << endl;
			number++;

			//把获取到的URL存起来
			vecurl.push_back(newurl);
			start = mat[0].second;

		}

		//遍历所有的URL
		for (int x = 0; x < vecurl.size() - 1; x++)
		{
			for (int j = x + 1; j < vecurl.size(); j++)
			{
				if (vecurl[x] != vecurl[j])
				{
					//判断是不是图片
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

	//匹配出所有的URL   正则表达式
	smatch mat;
	string::const_iterator start = html.begin();
	string::const_iterator end = html.end();
	regex gex("http://[^\\s'\"<>()]+");
	vector<string> vecurl;
	int number = 1;		//定义一个链接索引

	while (regex_search(start, end, mat, gex))
	{
		string newurl(mat[0].first, mat[0].second);

		//获取到的图片链接以	“图片链接1：”格式打印出来
		cout << "图片链接" << number << ":" << newurl << endl;
		number++;

		//把获取到的URL存起来
		vecurl.push_back(newurl);
		start = mat[0].second;
	}

	//遍历所有的URL
	for (int x = 0; x < vecurl.size() - 1; x++)
	{
		for (int j = x + 1; j < vecurl.size(); j++)
		{
			if (vecurl[x] != vecurl[j])
			{
				//判断是不是图片
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

//下载图片
bool Download(string imagurl)
{
	//如果是图片
	//对URL进行转义
	string url = imagurl;
	size_t len = url.length();//获取字符串长度
	int nmlen = MultiByteToWideChar(CP_ACP, 0, url.c_str(), len + 1, NULL, 0);
	wchar_t* buffer = new wchar_t[nmlen];
	MultiByteToWideChar(CP_ACP, 0, url.c_str(), len + 1, buffer, nmlen);
	//对URL转义结束

	//设置文件路径  将文件名按照1、2、3......命名
	char filename[128], tempfp[128];
	strcpy(filename, R"+*(G:\\tupian\\)+*");  //下载路径
	sprintf(tempfp, "%d", number);
	strcat(filename, tempfp);
	strcat(filename, ".jpg");   //下载的文件的格式类型
	printf("%s\n", filename);

	//转义文件名字（下载路径）
	string name = filename;
	size_t lenname = name.length();//获取字符串长度
	int namelen = MultiByteToWideChar(CP_ACP, 0, name.c_str(), lenname + 1, NULL, 0);
	wchar_t* buffer1 = new wchar_t[namelen];
	MultiByteToWideChar(CP_ACP, 0, name.c_str(), lenname + 1, buffer1, nmlen);
	//转义文件名字（下载路径）  转义结束

	HRESULT hr = URLDownloadToFile(NULL, buffer, buffer1, 0, NULL);			//下载函数
	number++;

	//检测是否下载成功
	if (hr == S_OK)
	{
		cout << "-------ok" << endl;

	}
	return true;
}