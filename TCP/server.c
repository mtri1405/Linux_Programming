#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int server_fd = -1, new_socket = -1;
    struct sockaddr_in address;
    int address_len = sizeof(address);
    char buffer[BUFFER_SIZE];
    int opt = 1;
    FILE *file = NULL;
    // 1. Tạo socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("[LỖI] Tạo socket thất bại");
        exit(EXIT_FAILURE);
    }

    // 2. Cấu hình tái sử dụng địa chỉ/port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("[LỖI] setsockopt thất bại");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Thiết lập cấu hình địa chỉ
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // 3. Bind địa chỉ và port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("[LỖI] Bind thất bại");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 4. Lắng nghe
    if (listen(server_fd, 3) < 0) {
        perror("[LỖI] Listen thất bại");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("[SERVER] Đang lắng nghe tại cổng %d...\n", PORT);

    // 5. Chấp nhận kết nối
    new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&address_len);
    if (new_socket < 0) {
        perror("[LỖI] Accept thất bại");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Trích xuất thông tin Client
    char client_ip[INET_ADDRSTRLEN];
    uint16_t client_port = ntohs(address.sin_port);
    inet_ntop(AF_INET, &(address.sin_addr), client_ip, INET_ADDRSTRLEN);
    printf("[SERVER] Client kết nối từ IP: %s | Port: %d\n", client_ip, client_port);

    // 6. Đọc file text.txt và gửi dữ liệu
    if ((file = fopen("text.txt", "r")) == NULL) {
        close(server_fd);
        close(new_socket);
        perror("[ERROR] Không thể mở file\n");
        return -1;
    }

    long total_bytes_send = 0;
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        size_t len = strlen(buffer);
        ssize_t bytes_sent = send(new_socket, buffer, len, 0);

        if (bytes_sent < 0) {
            perror("[LỖI] Gửi dữ liệu thất bại");
            break; // SỬA: Thay return -1 bằng break để xuống đóng socket
        }

        total_bytes_send += bytes_sent;
    }

    // 7. Thông báo và dọn dẹp tài nguyên
    printf("\n[SERVER] Đã truyền xong. Tổng byte đã gửi: %ld bytes\n", total_bytes_send);

    shutdown(new_socket, SHUT_RDWR);
    close(new_socket);
    close(server_fd);

    printf("[SERVER] Đã đóng socket thành công.\n");
    return 0;
}