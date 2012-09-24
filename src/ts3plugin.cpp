#ifdef _WIN32
#pragma warning (disable : 4100)  /* Disable Unreferenced parameter warning */
#include <Windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <process.h>
#include "public_errors.h"
#include "public_errors_rare.h"
#include "public_definitions.h"
#include "public_rare_definitions.h"
#include "ts3_functions.h"
#include "ts3plugin.h"

static struct TS3Functions ts3Functions;

#ifdef _WIN32
#define _strcpy(dest, destSize, src) strcpy_s(dest, destSize, src)
#define snprintf sprintf_s
#else
#define _strcpy(dest, destSize, src) { strncpy(dest, src, destSize-1); (dest)[destSize-1] = '\0'; }
#endif

#define PLUGIN_API_VERSION 18

#define PATH_BUFSIZE 512
#define COMMAND_BUFSIZE 128
#define INFODATA_BUFSIZE 128
#define SERVERINFO_BUFSIZE 256
#define CHANNELINFO_BUFSIZE 512
#define RETURNCODE_BUFSIZE 128

static char* pluginID = NULL;
SOCKET receivingSocket;
uint64 connectionHandlerID;
uintptr_t threadPtr = NULL;

void ts3plugin_udpListen(void *pParams);

/*********************************** Required functions START ************************************/
/*
 * If any of these required functions is not implemented, TS3 will refuse to load the plugin
 */

/* Unique name identifying this plugin */
const char* ts3plugin_name() {
	return "A2TS Rebuild";
}

/* Plugin version */
const char* ts3plugin_version() {
    return "0.1";
}

/* Plugin API version. Must be the same as the clients API major version, else the plugin fails to load. */
int ts3plugin_apiVersion() {
	return PLUGIN_API_VERSION;
}

/* Plugin author */
const char* ts3plugin_author() {
	/* If you want to use wchar_t, see ts3plugin_name() on how to use */
    return "Swapp && MSC";
}

/* Plugin description */
const char* ts3plugin_description() {
	/* If you want to use wchar_t, see ts3plugin_name() on how to use */
    return "A2TS Rebuild: This plugin will (not) work.";
}

/* Set TeamSpeak 3 callback functions */
void ts3plugin_setFunctionPointers(const struct TS3Functions funcs) {
    ts3Functions = funcs;
}

int ts3plugin_init()
{
	/* Plugin init code goes here */
	printf("PLUGIN: Init\n");

	if((ts3Functions.spawnNewServerConnectionHandler(0, &connectionHandlerID)) == ERROR_unable_to_bind_network_port)
	{
		printf("PLUGIN: Failed to get connection handler ID.\n");
		return 1;
	}

	/*	Winsock init	*/
	WORD wVersionRequested = MAKEWORD(2,2);
	WSADATA wsaData;

	// Attempt to start-up WSA
	if (WSAStartup(wVersionRequested, &wsaData) != 0)
	{
	    printf("PLUGIN: WSAStartup failed.\n");
		printf("PLUGIN: Error code: %s\n",WSAStartup);	
		return 1;
	}
	else
	{
	    printf("PLUGIN: Winsock launch OK!.\n");
		printf("PLUGIN: Status: %s.\n", wsaData.szSystemStatus);
	}
		
	// Confirm winsock version.
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2 )
	{
		/* Tell the user that we could not find a usable WinSock DLL.*/
		printf("PLUGIN: Unsupported dll version. Version: %u.%u!\n", LOBYTE(wsaData.wVersion), HIBYTE(wsaData.wVersion));
		WSACleanup();
		return 1;
	}
	else
	{
       printf("PLUGIN: dll version %u.%u!\n", LOBYTE(wsaData.wVersion), HIBYTE(wsaData.wVersion));
	}

	// Create a socket for UDP.
	receivingSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP); 

	// Check for errors to ensure that the socket is a valid socket.
	if (receivingSocket == INVALID_SOCKET)
	{
		printf("PLUGIN: Error at socket(): %ld\n", WSAGetLastError());
		return 1;
	}
	else
	{
		printf("PLUGIN: socket() is OK!\n"); 
	}

	// Create a sockaddr_in object and set its values.
	sockaddr_in endPoint;

	endPoint.sin_family = AF_INET;
	endPoint.sin_addr.s_addr = inet_addr("127.0.0.1");
	endPoint.sin_port = htons(55555);

	// Bind the socket to the endpoint
	if (bind(receivingSocket, (SOCKADDR*)&endPoint, sizeof(endPoint)) == SOCKET_ERROR)
	{
		printf("PLUGIN: bind() failed: %ld.\n", WSAGetLastError());
		closesocket(receivingSocket);
		return 1;
	}
	else
	{
		printf("PLUGIN: bind() is OK!\n");
	}

	/*	 Winsock init end */

    return 0;  /* 0 = success, 1 = failure */
}

/* Custom code called right before the plugin is unloaded */
void ts3plugin_shutdown()
{
    /* Your plugin cleanup code here */
    printf("PLUGIN: Shutting down..\n");

	closesocket(receivingSocket);
	WSACleanup();

	/* Free pluginID if we registered it */
	if(pluginID) {
		free(pluginID);
		pluginID = NULL;
	}
}

/*********************************** Required functions END ************************************/

void ts3plugin_onConnectStatusChangeEvent(uint64 connectionHandlerID, int STATUS_CONNECTION_ESTABLISHED,unsigned int ERROR_ok)
{
	printf("PLUGIN: TS3 is connected to a server.\n");

	//FIXME This function is called everytime the connection status changes. Any ideas why?

	if(threadPtr == NULL)
	{
		threadPtr = _beginthread(ts3plugin_udpListen, 0, NULL);
		printf("PLUGIN: Creating listener thread. \n");
	}
	
}

void ts3plugin_udpListen(void *pParams)
{
	printf("PLUGIN: Listener thread started.\n");

	// Prepare to receive data.
	char recvbuf[256] = "";
	int bytesRecv = SOCKET_ERROR;
	bool stopRequested = false;

	while(!stopRequested)
	{
		printf("PLUGIN: Receiving data.\n");
		bytesRecv = recv(receivingSocket, recvbuf, 256, 0);

		if (bytesRecv == SOCKET_ERROR)
		{
			printf("PLUGIN: recv() error %ld.\n", WSAGetLastError());
		}
		else
		{
			printf("PLUGIN: recv() is OK.\n");
			printf("PLUGIN: Received data is: %s", recvbuf);
			printf("PLUGIN: Bytes received: %ld.\n", bytesRecv);

			if(strcmp(recvbuf,"stop") == 0)
			{
				printf("PLUGIN: Stopping receiving.\n");
				stopRequested = true;
			}

			memset(&recvbuf[0],0,sizeof(recvbuf));
		}
	}
}