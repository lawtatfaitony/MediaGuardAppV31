#pragma once
#include "NatHeartBean.h"

NatHeartBean::~NatHeartBean()
{
    // �R�c���
}
NatHeartBean::NatHeartBean()
{
	// �c�y���
}
  
//���STUN��ĳ�������a�}�ഫIP
int NatHeartBean::get_stun_ip(){

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cerr << "Failed to create socket." << std::endl;
        return 1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(STUN_SERVER_PORT);
    inet_pton(AF_INET, STUN_SERVER_IP, &server_addr.sin_addr);

    // �c�� STUN �ШD
    // ���B�ٲ��ШD������c�عL�{�A�аѦ� RFC 5389
    // ���] stun_request �M request_length �w�g���T�]�m

    // �o�e�ШD
    sendto(sockfd, stun_request, request_length, 0, (struct sockaddr*)&server_addr, sizeof(server_addr));

    // �����T��
    char buffer[1024];
    socklen_t addr_len = sizeof(server_addr);
    recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&server_addr, &addr_len);

    // ���]�T�����]�t�F�Ȥ�ݪ� IP �M�ݤf
    // ���B�ݭn�ھ� STUN �T���榡�ѪR�X IP �M�ݤf
    // �H�U�N�X�Ȭ��ܽd�A��ڸѪR�ݭn�ھ� RFC 5389 ��{
    // ���]�ڭ̤w�g�ѪR�X client_ip �M client_port

    char client_ip[INET_ADDRSTRLEN];
    //IP�榡�ഫ���r�Ŧ�
    inet_ntop(AF_INET, &server_addr.sin_addr, client_ip, sizeof(client_ip));
    unsigned short client_port = ntohs(server_addr.sin_port);

    std::cout << "����� IP: " << client_ip << std::endl;
    std::cout << "������ݤf: " << client_port << std::endl;

    close(sockfd);
    return 0;
}

 
