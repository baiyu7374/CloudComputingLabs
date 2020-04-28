#pragma once

#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/wait.h>
#include <ctype.h>

// 获取当前路径函数
#ifdef WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

const int BUFFER_SIZE = 4096;

std::string GetCurrentWorkingDir() {
  char buff[FILENAME_MAX];
  GetCurrentDir( buff, FILENAME_MAX );
  std::string current_working_dir(buff);
  return current_working_dir;
}

// main 函数中将 client_fd 交给 task
class task
{
private:
	int client_fd;

public:
	task(){};
	task(int fd) : client_fd(fd){}
	~task(){};

	void Response(char *message, int status, int size);

	void StartDeal();

	void response_get(char *filename);

	void response_post(char *filename);

	void deal_error(int status, char *message);
};

// response message to client   status: 200, 404, 501...
void task::Response(char *message, int status, int size)
{
	char buf[512];
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "HTTP/1.1 %d ", status);
	switch(status) {
		case 200: sprintf(buf, "%sOK\n", buf); break;
		case 404: sprintf(buf, "%sNot Found\n", buf); break;
		case 501: sprintf(buf, "%sNot Implemented\n", buf); break;
	}
	sprintf(buf, "%sServer: Team139's Web Server\n", buf);
	sprintf(buf, "%sContent-length: %d\n", buf, size);
	sprintf(buf, "%sContent-type: text/html\n", buf);
	sprintf(buf, "%s\n", buf);
	sprintf(buf, "%s%s", buf, message);

	// sleep(1);

	write(client_fd, buf, strlen(buf));
}

// 整个处理的开始!
void task::StartDeal()
{
	char buffer[BUFFER_SIZE];
	int size;

	if ((size = read(client_fd, buffer, BUFFER_SIZE - 1)) > 0) {
		char method[5];
		char filename[50];
		int i = 0, j = 0;

		// parse request line
		while (buffer[j] != ' ' && buffer[j] != '\0') {
			method[i++] = buffer[j++];
		}
		j++;
		method[i] = '\0';

		// parse quring string
		i = 0;
		while (buffer[j] != ' ' && buffer[j] != '\0') {
			filename[i++] = buffer[j++];
		}
		filename[i] = '\0';

		// GET, POST, Other
		if (strcmp(method, "GET") == 0) // GET
		{
			response_get(filename);
		} 
		else if (strcmp(method, "POST") == 0) // POST
		{
			if (strcmp(filename, "/Post_show") != 0) 
			{
				deal_error(404, (char*)"Not Found");
			} 
			else 
			{
				// parse Username; ID....
				// data's length: Content-Length: 指定
				char *argvs;

				// 不带数据, 404
				if ((argvs = strstr(buffer, "Content-Length")) == NULL) 
				{
					deal_error(404, (char*)"Not Found");
				} else response_post(argvs);
			}
		}
		else 
		{	// 501 page
			deal_error(501, (char*)"Not Implemented");
		}
	}

	close(client_fd);
}

// parse username, id...
void task::response_post(char *argvs)
{
	argvs = strchr(argvs, ':');		// find ':'
	argvs++;	// jump ':'
	// printf("%s\n", buffer);

	int length;
	sscanf(argvs, "%d", &length);
	// printf("%d\n", length);

	// 拿到最终用户传来的数据 **&**
	argvs = argvs + strlen(argvs) - length;
	// printf("%s\n", argvs);
	char name[20];
	char id[20];
	int i = 0, j = 0;
	if (strncmp(argvs, "Name=", 5) != 0) {
		deal_error(404, (char*)"Not Found");
		return;
	}
	j+=5;
	while (argvs[j] != '\0' && argvs[j] != '&') {
		name[i++] = argvs[j++];
	}
	if (argvs[j] != '&') {
		deal_error(404, (char*)"Not Found");
		return ;
	}
	j++;
	argvs += j;
	if (strncmp(argvs, "ID=", 3) != 0) {
		deal_error(404, (char*)"Not Fount");
		return ;
	}
	i = 0, j = 3;
	while (argvs[j] != '\0' && isdigit(argvs[j])) {
		id[i++] = argvs[j++];
	}
	if (argvs[j] != '\0') {
		deal_error(404, (char*)"Not Found");
		return ;
	}

	char message[512];
	memset(message, 0, sizeof(message));
	sprintf(message, "<html>\n<title>POST method</title>\n<body bgcolor=ffffff>\n");
	sprintf(message, "%sYour Name: %s\n", message, name);
	sprintf(message, "%sID: %s\n", message, id);
	sprintf(message, "%s<hr><em>HTTP Web Server</em>\n", message);
	sprintf(message, "%s</body></html>\n", message);

	Response(message, 200, strlen(message));
}


void task::response_get(char *filename)
{
	// current work directory.
	char path[1000];
	getcwd(path, sizeof(path));


	// full path & deal directory
	strcat(path, filename);
	struct stat statbuf;
	if (stat(path, &statbuf) != 0) {
		deal_error(404, (char*)"Not Found");
	}
	if (S_ISDIR(statbuf.st_mode)) {
		strcat(path, "/index.html");
	}

	// file exist ? 
	if (access(path, R_OK) == -1) // 404 Not Found
	{
		deal_error(404, (char*)"Not Found");
	}
	else // file exist
	{
		// get file's size
		int size;
		stat(path, &statbuf);
		size = (int)statbuf.st_size;

		// send file to client
		Response((char*)"", 200, size);
		int filefd = open(path, O_RDONLY);
		// response(size, 200);

		sendfile(client_fd, filefd, 0, statbuf.st_size);
	}
}

// 封装处理错误信息页
void task::deal_error(int status, char *error_msg)
{
	char message[512];
	memset(message, 0, sizeof(message));
	sprintf(message, "<html>\n<title>%d  %s</title>\n", status, error_msg);
	sprintf(message, "%s<body bgcolor=ffffff>\n", message);
	sprintf(message, "%s %s \n", message, error_msg);
	sprintf(message, "%s<hr><em>HTTP Web Server</em>\n", message);
	sprintf(message, "%s</body>\n</html>\n", message);

	Response(message, status, strlen(message));
}