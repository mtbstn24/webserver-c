//mtbstn24
//webserver in c
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>

#define PORT 3000
#define BUFFER_SIZE 104857600
#define FILE_DIR "."

//function to return the file pointer with respect to the requested file path
FILE* getFile(const char *file_path){
	
	FILE *file;
	char *path = (char *)malloc(BUFFER_SIZE * 3 * sizeof(char));
	path[256] = '\0';
	strcpy(path, file_path);
	file = fopen(path, "rb");
	return file;
	
}

//fucntion to get the suitable content type (mime type) depending on the requested file
char* getContentType(const char *file_path){
	
	char *response_mime = (char *)malloc(BUFFER_SIZE * 1 * sizeof(char));
	memset(response_mime, '\0', BUFFER_SIZE * 1 * sizeof(char));
	
	if(strstr(file_path, ".html") || strstr(file_path, ".htm")){
		response_mime = "text/html";
	}else if(strstr(file_path, ".jpg") || strstr(file_path, "jpeg") || strstr(file_path, "JPG") || strstr(file_path, "JPEG")){
		response_mime = "image/jpeg";
	}else if(strstr(file_path, ".png")){
		response_mime = "image/png";
	}else{
		response_mime = "text/html";
	}
	
	return response_mime;
}

//function to get the suitable http response for a file request
void getFileResponse(char *file_path, char *response){
	
	FILE *file = NULL;
	char *content_type = getContentType(file_path);
	memset(response, '\0', BUFFER_SIZE * 1 * sizeof(char));
	
	if((file = getFile(file_path)) != NULL){
		
		size_t bytes = 0;
		while (fgetc(file) != EOF){
			bytes++;
		}
		fseek(file, 0, SEEK_SET);
		
		sprintf(response, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %ld\r\n\r\n", content_type, bytes);
		char file_content[1024];
		memset(file_content, 0, sizeof(file_content));
		while(fread(file_content, 1, sizeof(file_content), file) > 0){
			strcat(response, file_content);
		}
		fclose(file);
		printf("\nresponse-content: %s", response);
				
	}else{
		
		perror("File open error");
		printf("\n%s",file_path);
		strcpy(response, "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n<html><head><title>404 Not Found</title></head><body><div style='padding: 50px;'><h1>404 Not Found</h1><p>Requested file could not be found.</p></div></body></html>\r\n");
		
	}
		
}

int main(int argc, char const* argv[]){
	
	int server_sock_fd;
	int *client_sock_fd = malloc(sizeof(int));
	struct sockaddr_in server_addr, client_addr;
	socklen_t sockaddr_len = sizeof(client_addr);
	time_t t;
	
	//create tcp (SOCK_STREAM) server socket that uses internet address domain (AF_INET) using socket() function that returns a file descriptor
	if((server_sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("Socket creation Unsuccessful");
		exit(EXIT_FAILURE);
	}
	printf("Socket creation Successful.");
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT); //htons used to convert IP port number in host byte order to be in network byte order
	server_addr.sin_addr.s_addr = INADDR_ANY;
	//INADDR_ANY can be used if the socket is not needed to be bound to a specific IP.
	
	//bind the server address to the created socket
	if((bind(server_sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr))) < 0){
		perror("\nSocket binding Unsuccessful");
		exit(EXIT_FAILURE);
	}
	printf("\nSocket binding successful.");
	
	//setup connection-mode socket to listen for socket connections with limiting the incoming connection queue
	if((listen(server_sock_fd, 5)) < 0){
		perror("\nServer socket listen failed");
		exit(EXIT_FAILURE);
	}
	printf("\nServer socket listening successfully on port %d...", PORT);
	
	while(1){
		//accept fist connection in queue and creates a new socket (client socket) for the specific connection
		if((*client_sock_fd = accept(server_sock_fd, (struct sockaddr *)&client_addr, &sockaddr_len)) < 0){
			perror("\nConnection accept failed");
			exit(EXIT_FAILURE);
		}
		printf("\nConnection accept successful.");
		
		char *client_request = (char *)malloc(BUFFER_SIZE * 3 * sizeof(char));
		int request_bytes;
		
		//store the client request as a string and get the request byte count
		if((request_bytes = recv(*client_sock_fd, client_request, request_bytes-1, 0)) < 0){
			perror("\nRequest receive unsuccessful");
			continue;
		}
		printf("\nRequest received successfully.");
		client_request[request_bytes] = '\0';
		printf("\nClient Request: \n%s",client_request);
		
		char *request_file_path = (char *)malloc(BUFFER_SIZE * 3 * sizeof(char));
//		char *full_file_path = (char *)malloc(BUFFER_SIZE * 2 * sizeof(char));
		
		request_file_path[256] = '\0';
//		full_file_path[256] = '\0';

		time(&t);
		char *server_response = (char *)malloc(BUFFER_SIZE * 3 * sizeof(char));
		memset(server_response, '\0', BUFFER_SIZE * 3 * sizeof(char));
		
		//get the file path from the client request string
		sscanf(client_request, "GET %s ", request_file_path);
		
		//get the correct server response for different client requests for different file types
		if((strcmp(request_file_path,"/"))){
			sscanf(client_request, "GET /%s ", request_file_path);
			getFileResponse(request_file_path, server_response);
		}else{
			sprintf(server_response, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><head><title>webserver-c</title></head><body><p>Server Connection Success. <br/> Date %s \r\n<ul>List of file types the server can handle in the same folder<br/><li>html/ htm files</li><li>txt files</li><li>image files(jpg, png)</li></ul></p></body></html>\r\n", ctime(&t));
		}
		
		printf("\n%s",request_file_path);
		
//		strcat(full_file_path, FILE_DIR);
//		strcat(full_file_path, request_file_path);
		
		//send the response to the client socket
		if((send(*(client_sock_fd), server_response, strlen(server_response), 0)) < 0){
			perror("\nResponce send failed");
			exit(EXIT_FAILURE);
		}
		printf("\nRespond sent successfully.");
		
		//free the allocated memory spaces and set pointers to null
		memset(server_response, '\0', strlen(server_response));
		free(server_response);
		free(request_file_path);
		free(client_request);
		server_response = request_file_path = client_request = NULL;
		
		printf("\nresponse-content: %s", server_response);
		
		//close the client socket connection after handling the respective request
		close(*(client_sock_fd));
		printf("\nClient connection closed\n");
	}
	
	//close the server connection	
	close(server_sock_fd);
	printf("\nServer connection closed");
	
	//shutdown the request receiving and respond sending from socket
	shutdown(server_sock_fd, SHUT_RDWR);
	
	return 0;
}
