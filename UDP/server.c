#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int server_fd = -1;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    int opt = 1;

    // 1. Mở file chế độ đọc nhị phân (rb)
    FILE *file = fopen("text.txt", "rb");
    if (file == NULL) {
        perror("[LỖI] Không thể mở file text.txt");
        exit(EXIT_FAILURE);
    }

    // 2. Tạo socket UDP (SOCK_DGRAM)
    if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("[LỖI] Tạo socket thất bại");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("[LỖI] setsockopt thất bại");
        close(server_fd);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // 3. Cấu hình địa chỉ server
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // 4. Bind địa chỉ tới socket
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("[LỖI] Bind thất bại");
        close(server_fd);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // 5. Nhận tin nhắn từ client
    printf("[UDP SERVER] Đang lắng nghe tại cổng %d...\n", PORT);
    ssize_t bytes_received = recvfrom(server_fd, buffer, BUFFER_SIZE - 1, 0,
                                      (struct sockaddr *)&client_addr, &client_len);
    if (bytes_received < 0) {
        perror("[LỖI] Nhận dữ liệu thất bại");
        close(server_fd);
        fclose(file);
        exit(EXIT_FAILURE);
    }
    buffer[bytes_received] = '\0';

    char client_ip[INET_ADDRSTRLEN];
    uint16_t client_port = ntohs(client_addr.sin_port);
    inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);

    printf("[UDP SERVER] Nhận được tin từ [%s:%d]: %s\n", client_ip, client_port, buffer);

    // 6. Gửi dữ liệu file bằng fread
    size_t bytes_read;
    long total_bytes_sent = 0;
    int send_error = 0;

    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        ssize_t bytes_sent = sendto(server_fd, buffer, bytes_read, 0,
                                    (struct sockaddr *)&client_addr, client_len);
        if (bytes_sent < 0) {
            perror("[LỖI] Gửi gói tin thất bại");
            send_error = 1;
            break;
        }
        total_bytes_sent += bytes_sent;
        
        // Tránh nghẽn mạng UDP làm rơi gói tin ở phía Client
        usleep(1000); // Tạm dừng 1ms giữa các gói
    }

    if (!send_error) {
        // Gửi thông điệp EOF báo cho Client dừng nhận lập tức
        sendto(server_fd, "EOF", 3, 0, (struct sockaddr *)&client_addr, client_len);
        printf("[UDP SERVER] Đã gửi phản hồi thành công.\n");
        printf("[SERVER] Đã truyền xong. Tổng byte đã gửi: %ld bytes\n", total_bytes_sent);
    }

    // 7. Dọn dẹp tài nguyên
    fclose(file);
    close(server_fd);
    printf("[UDP SERVER] Đã đóng thành công.\n");
    return 0;
}