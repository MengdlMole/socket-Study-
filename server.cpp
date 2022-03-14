#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

using namespace std;

#define BUF_SIZE 1024

int main() {

	// 1. 创建服务端的 socket	
	int server_socket = socket(AF_INET, SOCK_STREAM, 0);
	// int server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if ( server_socket == -1 )
	{
		printf("创建 socket 失败 \n");
		return -1;
	}

	// 2. 把服务端用于通信的地址和端口绑定到 socket 上
	struct sockaddr_in server_addr; // 服务器地址的结构体
	
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 任意通信 ip 地址
	//server_addr.sin_addr.s_addr = inet_addr("1.1.1.1"); // 指定通信 ip 地址
	server_addr.sin_port = htons(22); //指定通信端口
	bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)); // 绑定地址和端口到 socket 上

	// 3. 监听通信端口
	int h = listen(server_socket, 20);
	
	if ( h != 0 )
	{
		perror("listen");
		close(server_socket);
		return -1;
	}


	// 4. 收到客户端请求时与客户端生成一个新的 socket 与客户端通信
	struct sockaddr_in client_addr;
	socklen_t client_addr_size = sizeof(client_addr);
	int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_size);

	printf("客户端( %s )已连接 \n", inet_ntoa(client_addr.sin_addr));
    string i(50, '=');
    cout << i << endl;


	// 4. 实现功能：与客户端进行文件传输
    char buffer[BUF_SIZE] = {0};
    while(1){

        printf("等待客户端请求... \n \n");
            
        memset(buffer, 0, sizeof(buffer));
        // 接收客户端请求
        if( recv(client_socket, buffer, 1, 0) <= 0 ){
            cout << buffer << endl;
            printf("接收请求失败 \n");
            break;
        }
        char str_temp[1];
        strcpy(str_temp, buffer);
        
        // 1> 客户端上传文件
        if( strcmp(str_temp, "1") == 0 ){
                      
            printf("接收：客户端请求上传文件 \n");

            memset(buffer, 0, sizeof(buffer));
            strcpy(buffer, "服务端已接收请求");
            // cout << sizeof(buffer) << endl;
            if( send(client_socket, buffer, strlen(buffer), 0) <= 0 ){
                printf("发送反馈失败 \n");
                break;
            }
            cout << "发送：" << buffer << endl << endl;
            
            // 接收保存文件的路径
            memset(buffer, 0, sizeof(buffer));
            if( recv(client_socket, buffer, BUF_SIZE, 0) <= 0 ){
                printf("接收文件保存路径失败 \n");
                break;
            }
            cout <<"接收：文件保存路径 " << buffer << endl;
            char save_path[100];
            strcpy(save_path, buffer);

            // 检测是否能够创建文件
            FILE *fl = fopen(save_path, "wb");
            memset(buffer, 0, sizeof(buffer));
            if( fl == NULL ){
                strcpy(buffer, "服务端无法创建文件");
                if( send(client_socket, buffer, strlen(buffer),0) <= 0 ){
                    printf("发送反馈失败 \n");
                    break;
                }
                cout << "发送：" << buffer << endl << endl;
            }
            else{
                memset(buffer, 0, sizeof(buffer));
                strcpy(buffer, "请发送文件");
                if( send(client_socket, buffer, strlen(buffer), 0) <= 0 ){
                    printf("发送反馈失败 \n");
                    break;
                }
                cout << "发送：" << buffer << endl << endl;

                // 接收文件
                memset(buffer, 0, sizeof(buffer));
                int nCount;
                while( (nCount = recv(client_socket, buffer, BUF_SIZE, 0)) > 0 ){
                    fwrite(buffer, nCount, 1, fl);
                }
                cout <<"接收：文件已接收" << endl;

                memset(buffer, 0, sizeof(buffer));
                strcpy(buffer, "服务端已接收文件");
                if( send(client_socket, buffer, strlen(buffer), 0) <= 0 ){
                    printf("发送反馈失败 \n");
                    break;
                }
                cout << "发送：" << buffer << endl << endl;
                printf("客户端( %s )已上传文件 \n", inet_ntoa(client_addr.sin_addr));
                break;
            }         
        }
        
        // 2> 客户端下载文件
        else {
                      
            printf("接收：客户端请求下载文件 \n");

            memset(buffer, 0, sizeof(buffer));
            strcpy(buffer, "服务端已接收请求");
            // cout << sizeof(buffer) << endl;
            if( send(client_socket, buffer, strlen(buffer), 0) <= 0 ){
                printf("发送反馈失败 \n");
                break;
            }
            cout << "发送：" << buffer << endl << endl;
            
            // 接收目标文件的路径
            memset(buffer, 0, sizeof(buffer));
            if( recv(client_socket, buffer, BUF_SIZE, 0) <= 0 ){
                printf("接收目标文件路径失败 \n");
                break;
            }
            cout <<"接收：目标文件路径 " << buffer << endl;
            char file_name[100];
            strcpy(file_name, buffer);

            // 检测是否有目标文件
            FILE *fl = fopen(file_name, "rb");
            memset(buffer, 0, sizeof(buffer));
            if( fl == NULL ){
                strcpy(buffer, "服务端无此文件");
                if( send(client_socket, buffer, strlen(buffer),0) <= 0 ){
                    printf("发送反馈失败 \n");
                    break;
                }
                cout << "发送：" << buffer << endl << endl;
            }
            else{
                memset(buffer, 0, sizeof(buffer));
                strcpy(buffer, "请客户端接收文件...");
                if( send(client_socket, buffer, strlen(buffer), 0) <= 0 ){
                    printf("发送反馈失败 \n");
                    break;
                }
                cout << "发送：" << buffer << endl << endl;
                
                memset(buffer, 0, sizeof(buffer));
                if( recv(client_socket, buffer, BUF_SIZE, 0) <= 0 ){
                    printf("客户端无回应 \n");
                    break;
                }
                cout << "接收：" << buffer << endl;

                // 发送文件
                memset(buffer, 0, sizeof(buffer));
                int nCount;
                int i_temp = 1;
                while ( (nCount = fread(buffer, 1, BUF_SIZE, fl)) > 0 ){
                    if( send(client_socket, buffer, nCount, 0) <= 0 ){
                        printf("文件发送失败 \n");
                        i_temp = 0;
                        break;
                    }
                }
                if ( i_temp == 0 ){ break; }
                shutdown(client_socket, SHUT_WR); // 文件读取完毕，断开输出流，向服务端发送 FIN 包
                cout <<"发送：文件已发送" << endl << endl;

                memset(buffer, 0, sizeof(buffer));
                if( recv(client_socket, buffer, BUF_SIZE, 0) <= 0 ){
                    printf("客户端无回应 \n");
                    break;
                }
                cout << "接收：" << buffer << endl << endl;
                printf("客户端( %s )已下载文件 \n", inet_ntoa(client_addr.sin_addr));
                break;
            }          
        }
    }



	// 5. 关闭 socket ，释放资源
	close(server_socket);
	close(client_socket);

    cout << i << endl;
    printf("通信结束 \n");
	return 0;

}
