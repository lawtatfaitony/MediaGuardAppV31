#include "NatHeartBean.h"

NatHeartBean::~NatHeartBean()
{
    // �R�c���
}

NatHeartBean::NatHeartBean()
{
    // �c�y���
} 
 

/*
* �}�o����: �ثe
*/


// ������aInternet IP �M�ݤf // STUN NAT �ثe���I���D
void NatHeartBean::get_local_internet_ip_and_port(char* &local_ip, int &local_port) {
     
    char local_internet_ip[INET_ADDRSTRLEN]; // �w�q local_internet_ip
    unsigned short local_internet_port; // �w�q local_port
    
    // �Ыؤ@�� UDP socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cerr << "Failed to create socket." << std::endl;
        return;
    }

    // �]�m���a�a�}���c
    struct sockaddr_in local_internet_addr;
    memset(&local_internet_addr, 0, sizeof(local_internet_addr));
    local_internet_addr.sin_family = AF_INET;
    local_internet_addr.sin_port = 0; // ���t�Φ۰ʤ��t�ݤf
    local_internet_addr.sin_addr.s_addr = htonl(INADDR_ANY); // �j�w��Ҧ��i�α��f
     
    // ����j�w���ݤf
    socklen_t addr_len = sizeof(local_internet_addr);
    if (bind_socket(sockfd, (struct sockaddr*)&local_internet_addr, sizeof(local_internet_addr)) < 0) {
        std::cerr << "Failed to bind socket." << std::endl;

#ifdef _WIN32
        closesocket(sockfd); // �b Windows ���ϥ� closesocket
#else
        close(sockfd); // �b Linux ���ϥ� close
#endif
        return;
    }
     
    // ������� IP �a�}�M�ݤf
    inet_ntop(AF_INET, &local_internet_addr.sin_addr, local_internet_ip, sizeof(local_internet_ip));
    local_internet_port = ntohs(local_internet_addr.sin_port);

#ifdef DEBUG
    std::cout << "local_internet_ip: " << local_internet_ip << std::endl;
    std::cout << "local_internet_port: " << local_internet_port << std::endl;
#endif // DEBUG
     
    // �ˬd�O�_�O���Ī����p�� IP
    if (strcmp(local_internet_ip, "127.0.0.1") == 0 ||
        strncmp(local_internet_ip, "10.", 3) == 0 ||
        strncmp(local_internet_ip, "172.", 4) == 0 ||
        strncmp(local_internet_ip, "192.168.", 8) == 0) {
        std::cerr << "Obtained a private or loopback IP: " << local_internet_ip << std::endl;
        // �ھڻݨD�B�z�p���Φ^���a�}
    }
    else {
        // ���t���s�ë��� INTERNET IP �a�} 
        strcpy(local_ip, local_internet_ip);
        local_port = local_internet_port;
    }
     
    // ���� socket
#ifdef _WIN32
    closesocket(sockfd);
#else
   close(sockfd); // ���� socket �y�`
#endif
}

// �]�� bind ��� �קKsocket��bind��ƦW�ٽĬ�
int  NatHeartBean::bind_socket(int sockfd, const struct sockaddr* addr, socklen_t addrlen) {
    return ::bind(sockfd, addr, addrlen);
}

// ��� STUN ���A���� IP �M�ݤf
void NatHeartBean::get_server_internet_ip(char*& stun_server_ip, int& stun_server_port) {

    // �Ы� STUN �ШD
    create_stun_request();

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cerr << "Failed to create socket." << std::endl;
        return;
    }

    struct addrinfo hints, * res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP

    // �ѪR�D���W
    if (getaddrinfo(STUN_SERVER_IP, nullptr, &hints, &res) != 0) {
        std::cerr << "Failed to resolve hostname." << std::endl;
        return;
    }

    // �N�ѪR���a�}�]�m�� server_addr ��
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(STUN_SERVER_PORT);
    server_addr.sin_addr = ((struct sockaddr_in*)res->ai_addr)->sin_addr;
    // ����귽
    freeaddrinfo(res);
  
	// �N STUN ���A���� IP �M�ݤf�]�m�� server_addr ��
    inet_pton(AF_INET, STUN_SERVER_IP, &server_addr.sin_addr);
     
    // �o�e�ШD 
    sendto(sockfd, reinterpret_cast<const char*>(stun_request), request_length,  0, (struct sockaddr*)&server_addr, sizeof(server_addr));
     
    // �����T��
    char buffer[1024];
    socklen_t addr_len = sizeof(server_addr);

    struct timeval timeout;
    timeout.tv_sec = 5; // �W�ɮɶ��A��쬰��
    timeout.tv_usec = 0; // �L��
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)); 
    size_t recv_len = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&server_addr, &addr_len);

    if (recv_len < 0) {
        perror("recvfrom failed");
        // �B�z���~
    }
    else {
        // ���L�����쪺 buffer ���e
        std::cout << "[Stun Protocol] Receive the socket data: recv_len = "<< recv_len <<"\n" << std::endl;
		//���e�Ӥj �����L
       /* for (size_t i = 0; i < recv_len; ++i) {
            printf("%02x ", static_cast<unsigned char>(buffer[i]));
        }*/
    }

    // ���] buffer �O�����쪺�T��
    if (recv_len > 0) {
         
        char c_server_ip[INET_ADDRSTRLEN]; 
       
        // �T�O�����쪺�ƾڨ�����
        if (recv_len >= 20) { // STUN �������̤p����
            // �ˬd���������M�лx
            uint16_t message_type = (buffer[0] << 8) | buffer[1];
            if (message_type == 0x0101) { // 0x0101 �O���\�T������������
                // ���� Mapped Address
                uint16_t attribute_type = (buffer[28] << 8) | buffer[29]; // ���] Mapped Address �b�� 28 �M 29 �r�`
                uint16_t address_length = (buffer[30] << 8) | buffer[31]; // �a�}����
                uint32_t client_ip_addr = *(uint32_t*)&buffer[32]; // IP �a�}
                unsigned short u_server_port = ntohs(*(uint16_t*)&buffer[36]); // �ݤf 
                inet_ntop(AF_INET, &client_ip_addr, c_server_ip, sizeof(c_server_ip));
                std::cout << "stun_server_ip: " << c_server_ip << std::endl;
                std::cout << "stun_server_port: " << u_server_port << std::endl;
            }
        }
    }
     

#ifdef _WIN32
    closesocket(sockfd);
#else
    close(sockfd); // ���� socket �y�`
#endif

    return;
}

// ...
void NatHeartBean::create_stun_request() {
    // �M�ŽШD
    memset(stun_request, 0, STUN_REQUEST_SIZE);

    // �]�m�ШD�����Y
    // stun_request �����c�]�m�p�U�G
    // stun_request[0]�G�����M����
    // stun_request[1]�G��k�X�]Binding Request�^
    // stun_request[2] �M stun_request[3]�G�������ס]���B�]�m�� 0�A�]���S���B�~�ݩʡ^
    // stun_request[8]�G�ѧO�Ū��}�l��m
	//--------------------------------------------------------------------------------
     // �]�m�ШD�����Y
    stun_request[0] = 0x00; // �����M����
    stun_request[1] = 0x01; // ��k�X�]Binding Request�^

    // �]�m�������ס]�o�̬� 0�A�]���S���B�~���ݩʡ^
    stun_request[2] = 0x00;
    stun_request[3] = 0x00;
     
    // �]�m�ѧO�š]�H���ͦ��A�o��²�ƳB�z�^
    uint32_t transaction_id = rand(); // �H���ͦ��@���ѧO��
    memcpy(&stun_request[8], &transaction_id, sizeof(transaction_id)); // �N�ѧO�ũ�J�ШD��

    // �]�m�ШD����
    request_length = STUN_REQUEST_SIZE; 
}

//�������IP,�ê�^IP OK ���������� 2024-12-29
std::string NatHeartBean::get_public_ip_by_curl() {
    // �ϥ� curl �R�O������@ IP
    system("curl -s http://ifconfig.me > public_ip_by_curl.txt");

    // Ū�� ip.txt ��󤤪����e
    FILE* file = fopen("public_ip_by_curl.txt", "r");
    if (!file) {
        std::cerr << "Failed to open ip.txt" << std::endl;
        return "127.0.0.1";
    }

    char ip[16]; // IPv4 �a�}�̤j����
    if (fgets(ip, sizeof(ip), file) != nullptr) {
        fclose(file);
        return std::string(ip);
    }

    fclose(file);
    return "127.0.0.1"; //�q�{���Ѫ��ȦӤ��O string.empty
}

// �q�L���s��� curl�Ȥ�ݩR�O���G �����w��curl(windows ���m��,Linux�ݭn�w��package)
// curl �R�O��o����IP OK  ���������� 2024-12-29
std::string NatHeartBean::get_public_ip_by_curl_memory() {

    // �ϥ� curl �R�O������@ IP
    const char* command = "curl -s http://ifconfig.me";
     
    // ���}�޹D�H����R�O
#ifdef WIN32
    int _pclose(FILE * stream);
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(command, "r"), _pclose);
#endif

#ifdef _linux_
    int pclose(FILE * stream);
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command, "r"), pclose);
#endif
    

    if (!pipe) {
        throw std::runtime_error("_popen() failed!");
    }

    char buffer[128];
    std::string result;

    // Ū���R�O����X
    while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) {
        result += buffer;
    }
    
    return result; 
}

