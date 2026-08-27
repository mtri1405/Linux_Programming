#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h> // Dùng cho struct timeval
#include <errno.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define SERVER_IP "172.28.101.217"

int main() {
    int sock = -1;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    FILE *log_file = fopen("log.txt", "w");
    if (log_file == NULL) {
        perror("[LỖI] Không thể tạo file log");
        return -1;
    }

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("[LỖI] Tạo socket thất bại");
        fclose(log_file);
        return -1;
    }

    // ========================================================
    // CẤU HÌNH TIMEOUT 3 GIÂY CHO HÀM RECVFROM
    // ========================================================
    struct timeval timeout;
    timeout.tv_sec = 3;  // Chờ tối đa 3 giây
    timeout.tv_usec = 0;

    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("[LỖI] Cài đặt Timeout thất bại");
        close(sock);
        fclose(log_file);
        return -1;
    }
    // ========================================================

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("[LỖI] IP không hợp lệ");
        close(sock);
        fclose(log_file);
        return -1;
    }

    // Gửi tin nhắn mồi
    char *message = "Xin chao, day la tin nhan gui qua UDP Socket!";
    socklen_t addr_len = sizeof(server_addr);

    if (sendto(sock, message, strlen(message), 0, (struct sockaddr *)&server_addr, addr_len) < 0) {
        perror("[LỖI] Gửi tin nhắn thất bại");
        close(sock);
        fclose(log_file);
        return -1;
    }
    printf("[UDP CLIENT] Đã gửi request. Đang nhận dữ liệu...\n");
    printf("--------------------------------------------------\n");

    ssize_t bytes_received;
    long total_bytes_received = 0;

    // Vòng lặp nhận dữ liệu
    while (1) {
        bytes_received = recvfrom(sock, buffer, BUFFER_SIZE - 1, 0,
                                  (struct sockaddr *)&server_addr, &addr_len);

        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            if (strncmp(buffer, "EOF", 4) == 0) {
                break;
            }
            // In ra màn hình & Ghi log
            fwrite(buffer, 1, bytes_received, stdout);
            fflush(stdout);

            fwrite(buffer, 1, bytes_received, log_file);
            fflush(log_file);

            total_bytes_received += bytes_received;
        } else {
            // Kiểm tra nếu lỗi trả về là do HẾT THỜI GIAN CHỜ (TIMEOUT)
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                printf("\n[THÔNG BÁO] Hết thời gian chờ (Timeout 3s). Server đã ngừng gửi dữ liệu.\n");
            } else {
                perror("\n[LỖI] Xảy ra lỗi nhận dữ liệu");
            }
            break; // Thoát khỏi vòng lặp nhận
        }
    }

    printf("\n--------------------------------------------------\n");
    printf("[UDP CLIENT] Hoàn tất! Tổng byte đã nhận: %ld bytes\n", total_bytes_received);

    close(sock);
    fclose(log_file);
    printf("[UDP CLIENT] Đã đóng socket an toàn.\n");
    return 0;
}