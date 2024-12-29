#pragma once
#include <iostream> 
#include <cstring> 
#include <cstdint> // �Ω� uint16_t, uint32_t ������
#include <io.h> 
#include <cerrno>
 
#include <stdio.h> 
#include <cstdio>    // �]�t popen �M pclose ���w�q 
#include <memory>    // �]�t std::unique_ptr
#include <stdexcept> // �]�t std::runtime_error
#include <string>    // �]�t std::string
#include <array>

#ifdef _WIN32
#include <winsock2.h>  // Windows socket
#include <ws2tcpip.h>  // Windows TCP/IP
#pragma comment(lib, "ws2_32.lib") // Windows socket library
#else
#include <arpa/inet.h> // POSIX socket
#include <unistd.h>    // POSIX standard 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> 

#endif
 
//https://baike.baidu.com/item/stun/3131387?fr=ge_ala ��z
//https://www.bilibili.com/opus/727141412236165127
 

class NatHeartBean {
 
public:
	explicit NatHeartBean(); 
	~NatHeartBean();
private:
	const char* STUN_SERVER_IP = "stun.l.google.com"; // STUN ���A�� IP
	const int STUN_SERVER_PORT = 19302; // STUN ���A���ݤf 
	// STUN �ШD���j�p
	const int STUN_REQUEST_SIZE = 20; // �ھڻݭn�վ�j�p 
public:
	
	// �ϥ�STUN ��ĳ����,�O�d�o��Ө�ƥH���u�ƩΪ̧�y ���G 204.204.204.204
	// ��� IP and PORT 
	void get_server_internet_ip(char*& loacal_ip, int& local_port); 
	// �ϥ�STUN ��ĳ����,�O�d�o��Ө�ƥH���u�ƩΪ̧�y ���G 0.0.0.0 
	void get_local_internet_ip_and_port(char*& loacal_ip, int& local_port);
	
	int bind_socket(int sockfd, const struct sockaddr* addr, socklen_t addrlen);

	//�]�m STUN �ШD�����e
	void create_stun_request();

	// �q�Lcurl �Ȥ�ݩR�O ����~��ip���覡�O�i�Ϊ� ok 2024-12-29
	// �O�dlog��󪺤覡
	std::string get_public_ip_by_curl();
	// �q�Lcurl �Ȥ�ݩR�O ����~��ip���覡�O�i�Ϊ� ok 2024-12-29
	// �q�L���s��� �Ȥ�ݩR�O���G
	std::string get_public_ip_by_curl_memory();

	char stun_request[20];
	int request_length;
};