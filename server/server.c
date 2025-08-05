#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "../include/event.h"
#include "../include/db.h"
#include "../include/statement.h"

#define PORT 8080
#define BUFFER_SIZE 16384  // Increased from 4096 to 16KB
#define MAX_CLIENTS 10

typedef struct {
    char method[10];
    char path[256];
    char content_type[64];
    int content_length;
    char body[BUFFER_SIZE];
} HTTPRequest;

typedef struct {
    int status_code;
    char status_text[64];
    char content_type[64];
    char body[BUFFER_SIZE];
    int body_length;
} HTTPResponse;

void parse_http_request(char* buffer, HTTPRequest* req) {
    char* line = strtok(buffer, "\n");
    if (line) {
        sscanf(line, "%s %s", req->method, req->path);
    }
    
    // Default content type
    strcpy(req->content_type, "text/plain");
    req->content_length = 0;
    req->body[0] = '\0';
    
    // Parse headers
    while ((line = strtok(NULL, "\n")) != NULL) {
        if (strncmp(line, "Content-Type:", 13) == 0) {
            sscanf(line, "Content-Type: %s", req->content_type);
        } else if (strncmp(line, "Content-Length:", 15) == 0) {
            sscanf(line, "Content-Length: %d", &req->content_length);
        }
        if (strcmp(line, "\r") == 0 || strlen(line) == 0) break;
    }
    
    // Extract body if present
    char* body_start = strstr(buffer, "\r\n\r\n");
    if (body_start && req->content_length > 0) {
        body_start += 4; // Skip \r\n\r\n
        strncpy(req->body, body_start, req->content_length);
        req->body[req->content_length] = '\0';
    }
}

void send_http_response(int client_socket, HTTPResponse* resp) {
    char response[BUFFER_SIZE];
    int len = snprintf(response, BUFFER_SIZE,
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Content-Length: %d\r\n"
        "\r\n"
        "%s",
        resp->status_code, resp->status_text,
        resp->content_type,
        resp->body_length,
        resp->body);
    
    send(client_socket, response, len, 0);
}

void send_json_response(int client_socket, int status_code, const char* json_data) {
    HTTPResponse resp;
    resp.status_code = status_code;
    strcpy(resp.status_text, status_code == 200 ? "OK" : "Not Found");
    strcpy(resp.content_type, "application/json");
    strcpy(resp.body, json_data);
    resp.body_length = strlen(json_data);
    
    send_http_response(client_socket, &resp);
}

void send_file_response(int client_socket, const char* file_path, const char* content_type) {
    FILE* file = fopen(file_path, "rb");
    if (!file) {
        HTTPResponse resp;
        resp.status_code = 404;
        strcpy(resp.status_text, "Not Found");
        strcpy(resp.content_type, "text/plain");
        strcpy(resp.body, "File not found");
        resp.body_length = strlen(resp.body);
        send_http_response(client_socket, &resp);
        return;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (file_size <= 0) {
        fclose(file);
        HTTPResponse resp;
        resp.status_code = 500;
        strcpy(resp.status_text, "Internal Server Error");
        strcpy(resp.content_type, "text/plain");
        strcpy(resp.body, "File is empty");
        resp.body_length = strlen(resp.body);
        send_http_response(client_socket, &resp);
        return;
    }
    
    // For files that fit in buffer, send them directly
    if (file_size <= BUFFER_SIZE - 1) {
        char* file_content = malloc(file_size + 1);
        if (!file_content) {
            fclose(file);
            HTTPResponse resp;
            resp.status_code = 500;
            strcpy(resp.status_text, "Internal Server Error");
            strcpy(resp.content_type, "text/plain");
            strcpy(resp.body, "Memory allocation failed");
            resp.body_length = strlen(resp.body);
            send_http_response(client_socket, &resp);
            return;
        }
        
        size_t bytes_read = fread(file_content, 1, file_size, file);
        fclose(file);
        
        if (bytes_read != file_size) {
            free(file_content);
            HTTPResponse resp;
            resp.status_code = 500;
            strcpy(resp.status_text, "Internal Server Error");
            strcpy(resp.content_type, "text/plain");
            strcpy(resp.body, "Failed to read file");
            resp.body_length = strlen(resp.body);
            send_http_response(client_socket, &resp);
            return;
        }
        
        file_content[file_size] = '\0';
        
        HTTPResponse resp;
        resp.status_code = 200;
        strcpy(resp.status_text, "OK");
        strcpy(resp.content_type, content_type);
        strcpy(resp.body, file_content);
        resp.body_length = file_size;
        
        send_http_response(client_socket, &resp);
        free(file_content);
    } else {
        // For larger files, send headers first, then stream the content
        char headers[512];
        snprintf(headers, sizeof(headers),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: %s\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
            "Access-Control-Allow-Headers: Content-Type\r\n"
            "Content-Length: %ld\r\n"
            "\r\n",
            content_type, file_size);
        
        send(client_socket, headers, strlen(headers), 0);
        
        // Stream file content in chunks
        char chunk[BUFFER_SIZE];
        size_t bytes_read;
        while ((bytes_read = fread(chunk, 1, sizeof(chunk), file)) > 0) {
            send(client_socket, chunk, bytes_read, 0);
        }
        
        fclose(file);
    }
}

char* get_content_type(const char* path) {
    if (strcmp(path, "/") == 0 || strstr(path, ".html")) return "text/html";
    if (strstr(path, ".css")) return "text/css";
    if (strstr(path, ".js")) return "application/javascript";
    if (strstr(path, ".json")) return "application/json";
    return "text/plain";
}

void handle_api_events(int client_socket, HTTPRequest* req) {
    if (strcmp(req->method, "GET") == 0) {
        // Get all events
        Table* table = load_table("causal.cdb");
        if (!table) {
            send_json_response(client_socket, 500, "{\"error\":\"Failed to load database\"}");
            return;
        }
        
        char json_response[BUFFER_SIZE] = "[";
        int first = 1;
        
        for (size_t i = 0; i < table->num_events; i++) {
            Event* event = &table->events[i];
            
            if (!first) strcat(json_response, ",");
            first = 0;
            
            char event_json[512];
            snprintf(event_json, sizeof(event_json),
                "{\"id\":%u,\"data\":\"%s\",\"parent_count\":%u,\"parents\":[",
                event->id, event->data, event->parent_count);
            
            strcat(json_response, event_json);
            
            for (int j = 0; j < event->parent_count; j++) {
                if (j > 0) strcat(json_response, ",");
                char parent_id[16];
                snprintf(parent_id, sizeof(parent_id), "%u", event->parents[j]);
                strcat(json_response, parent_id);
            }
            
            strcat(json_response, "]}");
        }
        
        strcat(json_response, "]");
        send_json_response(client_socket, 200, json_response);
        
        free(table);
    } else if (strcmp(req->method, "POST") == 0) {
        // Add new event
        // Parse JSON from request body
        char* json_start = strstr(req->body, "{");
        if (!json_start) {
            send_json_response(client_socket, 400, "{\"error\":\"Invalid JSON\"}");
            return;
        }
        
        // Simple JSON parsing (in production, use a proper JSON library)
        uint32_t id = 0;
        char data[MAX_DATA_LENGTH] = {0};
        uint32_t parents[MAX_PARENTS] = {0};
        int parent_count = 0;
        
        // Extract ID
        char* id_start = strstr(json_start, "\"id\":");
        if (id_start) {
            sscanf(id_start, "\"id\":%u", &id);
        }
        
        // Extract data
        char* data_start = strstr(json_start, "\"data\":\"");
        if (data_start) {
            data_start += 8; // Skip "data":"
            char* data_end = strchr(data_start, '"');
            if (data_end) {
                int data_len = data_end - data_start;
                if (data_len >= MAX_DATA_LENGTH) data_len = MAX_DATA_LENGTH - 1;
                strncpy(data, data_start, data_len);
                data[data_len] = '\0';
            }
        }
        
        // Extract parents
        char* parents_start = strstr(json_start, "\"parents\":[");
        if (parents_start) {
            parents_start += 11; // Skip "parents":[
            char* parents_end = strchr(parents_start, ']');
            if (parents_end) {
                char parents_str[256] = {0};
                int parents_len = parents_end - parents_start;
                if (parents_len >= sizeof(parents_str)) parents_len = sizeof(parents_str) - 1;
                strncpy(parents_str, parents_start, parents_len);
                parents_str[parents_len] = '\0';
                
                // Parse parent IDs
                char* token = strtok(parents_str, ",");
                while (token && parent_count < MAX_PARENTS) {
                    parents[parent_count++] = atoi(token);
                    token = strtok(NULL, ",");
                }
            }
        }
        
        // Validate input
        if (id == 0) {
            send_json_response(client_socket, 400, "{\"error\":\"Invalid event ID\"}");
            return;
        }
        
        if (strlen(data) == 0) {
            send_json_response(client_socket, 400, "{\"error\":\"Event data cannot be empty\"}");
            return;
        }
        
        // Create and insert event
        Event event;
        event.id = id;
        strncpy(event.data, data, MAX_DATA_LENGTH - 1);
        event.data[MAX_DATA_LENGTH - 1] = '\0';
        event.parent_count = parent_count;
        for (int i = 0; i < parent_count; i++) {
            event.parents[i] = parents[i];
        }
        
        Table* table = load_table("causal.cdb");
        if (!table) {
            send_json_response(client_socket, 500, "{\"error\":\"Failed to load database\"}");
            return;
        }
        
        insert_event(&event, table);
        free(table);
        
        send_json_response(client_socket, 201, "{\"message\":\"Event created successfully\"}");
    }
}

void handle_request(int client_socket) {
    char buffer[BUFFER_SIZE];
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received <= 0) return;
    
    buffer[bytes_received] = '\0';
    
    HTTPRequest req;
    memset(&req, 0, sizeof(HTTPRequest)); // Initialize to zero
    parse_http_request(buffer, &req);
    
    // Validate request
    if (strlen(req.method) == 0 || strlen(req.path) == 0) {
        HTTPResponse resp;
        resp.status_code = 400;
        strcpy(resp.status_text, "Bad Request");
        strcpy(resp.content_type, "text/plain");
        strcpy(resp.body, "Invalid request");
        resp.body_length = strlen(resp.body);
        send_http_response(client_socket, &resp);
        return;
    }
    
    // Handle CORS preflight
    if (strcmp(req.method, "OPTIONS") == 0) {
        HTTPResponse resp;
        resp.status_code = 200;
        strcpy(resp.status_text, "OK");
        strcpy(resp.content_type, "text/plain");
        strcpy(resp.body, "");
        resp.body_length = 0;
        send_http_response(client_socket, &resp);
        return;
    }
    
    // Handle API routes
    if (strncmp(req.path, "/api/events", 11) == 0) {
        handle_api_events(client_socket, &req);
        return;
    }
    
    // Handle static files
    char file_path[512];
    if (strcmp(req.path, "/") == 0) {
        strcpy(file_path, "../frontend/index.html");
    } else {
        snprintf(file_path, sizeof(file_path), "../frontend%s", req.path);
    }
    
    send_file_response(client_socket, file_path, get_content_type(req.path));
}

int main() {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    // Set socket options
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    
    printf("CausalDB HTTP Server running on http://localhost:%d\n", PORT);
    printf("Frontend available at http://localhost:%d\n", PORT);
    
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }
        
        handle_request(client_socket);
        close(client_socket);
    }
    
    close(server_socket);
    return 0;
} 