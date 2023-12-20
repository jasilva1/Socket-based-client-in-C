/*
 * client.c
 * Josh Silva
 * 33435438
 */
#include <stdio.h>
#include <stdlib.h>
#include "csapp.h"
#include <string.h>

int main(int argc, char **argv) 
{

    int clientfd;
    char *num1, *num2;
    char *host, *port;

    if (argc != 3) {
        fprintf(stderr, "usage: %s <num1> <num2>\n", argv[0]);
        exit(0);
    }

    num1 = argv[1];
    num2 = argv[2];

    host = "localhost";
    port = "8080";

    clientfd = Open_clientfd(host, port); //opening the client socket

    rio_t rio; //Robust I/0 for the socket
    Rio_readinitb(&rio, clientfd);

    char xmlrequest[1024];//XML reuest to be sent to the server
    sprintf(xmlrequest,
            "<?xml version=\"1.0\"?>"
            "<methodCall>"
            "<methodName>sample.addmultiply</methodName>"
            "<params>"
            "<param><value><double>%s</double></value></param>"
            "<param><value><double>%s</double></value></param>"
            "</params>"
            "</methodCall>", num1, num2);

    //HTTP request including the XML content
    char httprequest[MAXLINE * 2];
    sprintf(httprequest,
            "POST /RPC2 HTTP/1.1\r\n"
            "Host: localhost:8080\r\n"
            "User-Agent: XMLRPC C Client\r\n"
            "Content-Type: text/xml\r\n"
            "Content-Length: %ld\r\n\r\n"
            "%s", strlen(xmlrequest), xmlrequest);

    //Sending the HTTP request to the server
    Rio_writen(clientfd, httprequest, strlen(httprequest));
    //reading response
    char buf[MAXLINE];
    char response[MAXLINE];
    int total_bytes_read = 0;
    ssize_t res;

    //removing HTTP headers
    do {
        res = Rio_readlineb(&rio, buf, MAXLINE);
    } while (res > 0 && strcmp(buf, "\r\n") != 0);

    //reading XML response body
    while ((res = Rio_readlineb(&rio, buf, MAXLINE)) > 0) {
         if (total_bytes_read + res < MAXLINE) {
             memcpy(response + total_bytes_read, buf, res);//copying response to buffer
             total_bytes_read += res;
         } else {
             printf("Response is too large\n");//error msg
             break;
         }
    }

    response[total_bytes_read] = '\0';
    //check for response
    if (total_bytes_read > 0) {
         //parsing XML response
         char* start = strstr(response, "<double>");
         char* end;
         float sum, product;
         if (start) {
             //taking the value of the first <double>
             start += strlen("<double>");
             end = strstr(start, "</double>");
             if (end) {
                 *end = '\0';
                 sum = atof(start);
                 //taking the value of the second double
                 start = strstr(end + strlen("</double>"), "<double>");
                 if (start) {
                     start += strlen("<double>");
                     end = strstr(start, "</double>");
                     if (end) {
                         *end = '\0';
                         product = atof(start);
                         printf("%.1f %.1f\n", sum, product);//print and format
                     }
                 }
             }
         }
     } else {
         printf("Error occurred.\n");
     }

     Close(clientfd);
     exit(0);

}

