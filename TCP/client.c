#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define SERVER_IP "172.28.101.217"
int main() {
    int sock = -1;
    FILE *log_file = NULL;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE];

    // 1. Tạo socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("[LỖI] Tạo socket thất bại");
        return -1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        perror("[LỖI] Địa chỉ IP không hợp lệ");
        close(sock);
        return -1;
    }

    // 2. Kết nối đến server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("[LỖI] Kết nối tới Server thất bại");
        close(sock);
        return -1;
    }

    printf("[CLIENT] Đã kết nối tới Server thành công.\n");
    printf("[CLIENT] Đang lắng nghe dữ liệu theo thời gian thực...\n");
    printf("--------------------------------------------------\n");

    // 3. Mở file log để ghi nhận dữ liệu
    log_file = fopen("log.txt", "w+");
    if (log_file == NULL) {
        perror("[LỖI] Không thể tạo/mở file log");
        close(sock);
        return -1;
    }

    ssize_t bytes_received;
    long total_bytes_received = 0;

    // 4. Vòng lặp nhận dữ liệu theo thời gian thực
    while ((bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        // Ghi dữ liệu vào file log
        fwrite(buffer, 1, bytes_received, log_file);
        total_bytes_received += bytes_received;
    }

    if (bytes_received < 0) {
        perror("\n[LỖI] Xảy ra lỗi trong quá trình nhận dữ liệu");
    } else {
        printf("\n--------------------------------------------------\n");
        printf("[CLIENT] Server đã đóng kết nối.\n");
        printf("[CLIENT] Tổng số byte đã nhận và ghi log: %ld bytes\n", total_bytes_received);
    }

    // 5. Dọn dẹp và đóng tài nguyên
    if (log_file != NULL) {
        fclose(log_file);
        log_file = NULL;
    }

    if (sock >= 0) {
        shutdown(sock, SHUT_RDWR);
        close(sock);
        sock = -1;
    }

    printf("[CLIENT] Đã giải phóng tài nguyên và đóng Socket.\n");
    return 0;
}