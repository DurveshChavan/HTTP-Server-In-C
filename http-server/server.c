#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define RESPONSE_SIZE 65536

const char *get_content_type(const char *path) {
    /* Find the last '.' in the filename to get the extension */
    const char *ext = strrchr(path, '.');
    if (!ext) return "application/octet-stream";
    if (strcmp(ext, ".html") == 0) return "text/html";
    if (strcmp(ext, ".css") == 0) return "text/css";
    if (strcmp(ext, ".js") == 0) return "application/javascript";
    if (strcmp(ext, ".png") == 0) return "image/png";
    if (strcmp(ext, ".jpg") == 0) return "image/jpeg";
    if (strcmp(ext, ".gif") == 0) return "image/gif";
    return "application/octet-stream";
}

void send_response(int client_fd, int status_code, const char *status_text,
                   const char *content_type, const char *body, size_t body_len) {
    char header[BUFFER_SIZE];
    /* Build the HTTP response header with status, content type, and length */
    int header_len = snprintf(header, sizeof(header),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n",
        status_code, status_text, content_type, body_len);

    /* Send the header first, then the body */
    send(client_fd, header, header_len, 0);
    if (body && body_len > 0) {
        send(client_fd, body, body_len, 0);
    }
}

void send_error(int client_fd, int status_code, const char *status_text) {
    char body[256];
    int body_len = snprintf(body, sizeof(body),
        "<html><body><h1>%d %s</h1></body></html>",
        status_code, status_text);
    send_response(client_fd, status_code, status_text, "text/html", body, body_len);
}

void serve_directory(int client_fd, const char *filepath, const char *url_path) {
    DIR *dir = opendir(filepath);
    if (!dir) {
        send_error(client_fd, 500, "Internal Server Error");
        return;
    }

    char body[RESPONSE_SIZE];
    int offset = 0;

    // Build the HTML head and opening tags
    offset += snprintf(body + offset, sizeof(body) - offset,
        "<html><head><title>Index of %s</title></head>"
        "<body><h1>Index of %s</h1><ul>",
        url_path, url_path);

    // Loop through each directory entry
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip the current directory entry
        if (strcmp(entry->d_name, ".") == 0) continue;

        // Build the full link path for this entry
        char link_path[512];
        if (strcmp(url_path, "/") == 0) {
            snprintf(link_path, sizeof(link_path), "/%s", entry->d_name);
        } else {
            snprintf(link_path, sizeof(link_path), "%s/%s", url_path, entry->d_name);
        }

        // Append an anchor tag for this entry
        offset += snprintf(body + offset, sizeof(body) - offset,
            "<li><a href=\"%s\">%s</a></li>",
            link_path, entry->d_name);
    }

    // Close the HTML tags
    offset += snprintf(body + offset, sizeof(body) - offset,
        "</ul></body></html>");

    closedir(dir);

    send_response(client_fd, 200, "OK", "text/html", body, offset);
}

void serve_file(int client_fd, const char *path) {
    char filepath[512];

    /* Map URL path to filesystem path inside public/ */
    if (strcmp(path, "/") == 0) {
        snprintf(filepath, sizeof(filepath), "public/index.html");
    } else {
        snprintf(filepath, sizeof(filepath), "public%s", path);
    }

    /* Check if the file exists and get its size */
    struct stat st;
    if (stat(filepath, &st) == -1) {
        send_error(client_fd, 404, "Not Found");
        return;
    }
    // If the path is a directory, show its contents instead
    if (S_ISDIR(st.st_mode)) {
        serve_directory(client_fd, filepath, path);
        return;
    }
    /* Open the file in binary mode */
    FILE *file = fopen(filepath, "rb");
    if (!file) {
        send_error(client_fd, 404, "Not Found");
        return;
    }

    /* Allocate a buffer to hold the file contents */
    char *body = malloc(st.st_size);
    if (!body) {
        fclose(file);
        send_error(client_fd, 500, "Internal Server Error");
        return;
    }

    /* Read the entire file into the buffer */
    fread(body, 1, st.st_size, file);
    fclose(file);

    /* Determine content type and send the response */
    const char *content_type = get_content_type(filepath);
    send_response(client_fd, 200, "OK", content_type, body, st.st_size);

    free(body);
}

void handle_client(int client_fd){
	char buffer[BUFFER_SIZE];
	ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

	if (bytes_read <=0) {
		close(client_fd);
		return;
	}

	buffer[bytes_read] = '\0';

	char method[16];
	char path[256];
	sscanf(buffer, "%s %s", method, path);

	printf("%s %s\n", method, path);
    if (strcmp(method, "GET") != 0) {
        send_error(client_fd, 405, "Method Not Allowed");
        close(client_fd);
        return;
    }
	serve_file(client_fd, path);
    	close(client_fd);

}

int main(){
	// Create a TCP socket
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	//AF_INET means IPv4, SOCK_STREAM means reliable
	if (server_fd == -1) {
		perror("socket");
		return 1;
	}

	// Configure the address struct for PORT 8080
	struct sockaddr_in addr;
	int opt = 1;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(PORT);
	
	// Bind the socaket to the address and port
	if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1){
		perror("bind");
		close(server_fd);
		return 1;
	}
		
	// Start listening for incoming connections
	if (listen(server_fd, 10) == -1) {
		perror("listen");
		close(server_fd);
		return 1;
	}

	printf("Server listeing on http://loacalhost:%d\n", PORT);
	while(1){
		struct sockaddr_in client_addr;
		socklen_t client_len = sizeof(client_addr);
		int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);

			if(client_fd == -1){
				perror("accept");
				continue;
			}
			
			handle_client(client_fd);
			
	}
	close(server_fd);

	return 0;
}
