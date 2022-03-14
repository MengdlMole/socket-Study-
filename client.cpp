#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

using namespace std;

#define BUF_SIZE 1024


int main() {


	// 1. 创建客户端的 socket
	int client_socket = socket(AF_INET, SOCK_STREAM, 0);
	
	// 创建失败则提示失败
	if ( client_socket == -1 )
	{
		printf("创建 socket 失败 \n");
		return -1;
	}

	struct sockaddr_in server_addr; // 目标服务端地址的结构体
	
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("1.1.1.1"); // 目标服务端的通信 ip 地址（按需修改）
	server_addr.sin_port = htons(22); // 目标服务端的通信端口（按需修改）
	
	
	// 2. 向服务端发起连接请求
	int h = connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
	
	// 1> 若连接失败则提示失败
	if ( h == -1 ) 
	{
		printf("与服务端( %s )连接失败 \n", inet_ntoa(server_addr.sin_addr));
		close(client_socket);
		return -1;
	}

    // 2> 若连接成功显示连接成功
    else
    {
        printf("服务端( %s )已连接 \n", inet_ntoa(server_addr.sin_addr));
        string i(50, '=');
        cout << i << endl;
    }
	
	
	// 3. 实现功能：与服务端进行文件传输

    // 0> 打印操作指引
    printf("请输入您想进行的操作【1】上传文件 【2】 下载文件：");
    char str_temp[1];
    while (1){
        cin.getline(str_temp, 2);
        string j(50, '-');
        cout << j << endl;
        // cout << str_temp << endl;
        if ( strcmp(str_temp, "1") == 0 || strcmp(str_temp, "2") == 0 ){
            break;
        }
        else{
            printf("请重新输入：");
        }
    }

    // 1> 上传文件到服务端
    if (strcmp(str_temp, "1") == 0 ){
    		
        printf("请输入您要上传的文件（路径）：");
        char file_name[100];
        while ( 1 ){
            cin.getline(file_name, 100);
            // cout << file_name << endl;

            // 检查文件是否存在
            FILE *fl = fopen(file_name, "rb");
            if ( fl == NULL ){
                printf("文件不存在，请重新输入：");
                fclose(fl);
            }

            else{
                char buffer[BUF_SIZE] = {0};
                //  若文件存在，则发送文件
                
                printf("您想要上传到服务端的位置（路径）：");
                char save_path[100];
                cin.getline(save_path, 100);

                string j(50, '-');
                cout << j << endl;

                // 向服务端发送上传文件的请求并接收反馈
                memset(buffer, 0, sizeof(buffer));
                strcpy(buffer, "1"); // "1" 表示上传文件
                if( send(client_socket, buffer, strlen(buffer), 0) <= 0 ) {
                    printf("发送上传文件请求失败 \n");
                    break;
                }
                printf("发送：客户端请求上传文件 \n");

                memset(buffer, 0, sizeof(buffer));
                if( recv(client_socket, buffer, BUF_SIZE, 0) <= 0 ){
                    // cout << buffer << endl;
                    printf("服务端无回应 \n");
                    break;
                }
                cout << "接收：" << buffer << endl << endl;

                // 向服务端发送想要保存文件的位置（路径）并接收反馈
                memset(buffer, 0, sizeof(buffer));
                strcpy(buffer, save_path);
                if( send(client_socket, buffer, strlen(buffer), 0) <= 0 ) {
                    printf("发送文件保存路径失败 \n");
                    break;
                }

                cout << "发送：文件保存路径 " << buffer << endl;

                memset(buffer, 0, sizeof(buffer));
                if( recv(client_socket, buffer, BUF_SIZE, 0) <= 0 ){
                    printf("服务端无回应 \n");
                    break;
                }
                cout << "接收：" << buffer << endl << endl;
         
                // 发送文件
                memset(buffer, 0, sizeof(buffer));
                int nCount;
                printf("发送：文件上传中... \n");

                int i_temp = 1;
                while ( (nCount = fread(buffer, 1, BUF_SIZE, fl)) > 0 ){
                    if( send(client_socket, buffer, nCount, 0) <= 0 ){
                        printf("文件上传失败 \n");
                        i_temp = 0;
                        break;
                    }
                }
                if ( i_temp == 0 ){ break; }

                shutdown(client_socket, SHUT_WR); // 文件读取完毕，断开输出流，向服务端发送 FIN 包
                
                // 阻塞，等待服务端接收完毕
                memset(buffer, 0, sizeof(buffer));
                if( recv(client_socket, buffer, BUF_SIZE, 0) <= 0 ){
                    printf("文件已上传，但服务端无反应 \n");
                }
                else{
                    cout << "接收：" << buffer << endl << endl;
                }
                fclose(fl);
                break;
            }
            printf("文件已上传 \n \n");
        }
    }

    // 2> 从服务端下载文件
    else{
    	printf("请输入您要在本地创建的文件（路径）：");
        char save_path[100];
        while ( 1 ){
            cin.getline(save_path, 100);

            // 检查能否创建文件
            FILE *fl = fopen(save_path, "wb");
            if ( fl == NULL ){
                printf("文件无法创建，请重新输入：");
                fclose(fl);
            }

            else{
                char buffer[BUF_SIZE] = {0};
                //  若文件创建成功，则从服务端下载文件
                
                printf("请输入您想要下载的服务端的目标文件（路径）：");
                char file_name[100];
                cin.getline(file_name, 100);

                string j(50, '-');
                cout << j << endl;

                // 向服务端发送下载文件的请求并接收反馈
                memset(buffer, 0, sizeof(buffer));
                strcpy(buffer, "2"); // "2" 表示下载文件
                if( send(client_socket, buffer, strlen(buffer), 0) <= 0 ) {
                    printf("发送下载文件请求失败 \n");
                    break;
                }
                printf("发送：客户端请求下载文件 \n");

                memset(buffer, 0, sizeof(buffer));
                if( recv(client_socket, buffer, BUF_SIZE, 0) <= 0 ){
                    // cout << buffer << endl;
                    printf("服务端无回应 \n");
                    break;
                }
                cout << "接收：" << buffer << endl << endl;

                // 向服务端发送目标文件的位置（路径）并接收反馈
                memset(buffer, 0, sizeof(buffer));
                strcpy(buffer, file_name);
                if( send(client_socket, buffer, strlen(buffer), 0) <= 0 ) {
                    printf("发送目标文件路径失败 \n");
                    break;
                }

                cout << "发送：目标文件路径 " << buffer << endl;

                memset(buffer, 0, sizeof(buffer));
                if( recv(client_socket, buffer, BUF_SIZE, 0) <= 0 ){
                    printf("服务端无回应 \n");
                    break;
                }
                cout << "接收：" << buffer << endl << endl;

                memset(buffer, 0, sizeof(buffer));
                strcpy(buffer, "请发送文件");
                if( send(client_socket, buffer, strlen(buffer), 0) <= 0 ){
                    printf("发送反馈失败 \n");
                    break;
                }
                cout << "发送：" << buffer << endl;
         
                // 接收文件
                memset(buffer, 0, sizeof(buffer));
                int nCount;
                printf("文件下载中... \n");

                while( (nCount = recv(client_socket, buffer, BUF_SIZE, 0)) > 0 ){
                    fwrite(buffer, nCount, 1, fl);
                }
                cout <<"接收：文件已发送" << endl << endl;
                
                memset(buffer, 0, sizeof(buffer));
                strcpy(buffer, "客户端已接收文件");
                if( send(client_socket, buffer, BUF_SIZE, 0) <= 0 ){
                    printf("文件已下载，但向服务端反馈失败 \n");
                }
                else{
                    cout << "发送：" << buffer << endl << endl;
                }
                fclose(fl);
                break;
                printf("文件已下载 \n \n");   
            }  
        }
    }

    // 4. 关闭 socket， 释放资源
	close(client_socket);

	string i(50, '=');
    cout << i << endl;
    printf("通信结束 \n");
	return 0;
}
