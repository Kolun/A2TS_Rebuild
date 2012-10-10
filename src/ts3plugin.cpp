#ifdef _WIN32
#pragma warning (disable : 4100)  /* Disable Unreferenced parameter warning */
#include <Windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <process.h>
#include <queue>
#include <string>
#include "public_errors.h"
#include "public_errors_rare.h"
#include "public_definitions.h"
#include "public_rare_definitions.h"
#include "ts3_functions.h"
#include "ts3plugin.h"
#include "parser.h"

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
#define SERVER_NAME         L"."
#define PIPE_NAME           L"a2ts"
#define FULL_PIPE_NAME      L"\\\\" SERVER_NAME L"\\pipe\\" PIPE_NAME
#define BUFFER_SIZE     512

static char* pluginID = NULL;
uint64 connectionHandlerID;
anyID clientId;
HANDLE clientPipe = INVALID_HANDLE_VALUE;
HANDLE receiverThreadHndl;
DWORD dwError = ERROR_SUCCESS;
HANDLE senderThreadHndl;
BOOL stopRequested = FALSE;

std::queue<std::wstring> incomingMessages;
std::queue<std::wstring> outgoingMessages;
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

    return 0;  /* 0 = success, 1 = failure */
}

/* Custom code called right before the plugin is unloaded */
void ts3plugin_shutdown()
{
	printf("PLUGIN: Shutdown called.\n");   
	// Request thread stop.
	stopRequested = TRUE;
	// Cancel all awaiting IO request for the thread.
	CancelSynchronousIo(receiverThreadHndl);

	while(receiverThreadHndl != NULL)
	{
		printf("PLUGIN: Awaiting thread shutdown.\n");
		Sleep(100);
	}

	printf("PLUGIN: Thread shutdown confirmed.\n");
	
	/* Free pluginID if we registered it */
	if(pluginID) {
		free(pluginID);
		pluginID = NULL;
	}
}

/*********************************** Required functions END ************************************/

void ts3plugin_registerPluginID(const char* id) {
	const size_t sz = strlen(id) + 1;
	pluginID = (char*)malloc(sz * sizeof(char));
	_strcpy(pluginID, sz, id);  /* The id buffer will invalidate after exiting this function */
	printf("PLUGIN: registerPluginID: %s\n", pluginID);
}

void ts3plugin_onConnectStatusChangeEvent(uint64 serverConnectionHandlerID, int newStatus, unsigned int errorNumber)
{
	printf("PLUGIN: newStatus is %d\n",newStatus);
	if(newStatus == STATUS_CONNECTION_ESTABLISHED)
	{
		// Check if threads are started.
		// If not - start.
		printf("PLUGIN: Connected to server.\n");

		// Receive client id
		if(ts3Functions.getClientID(connectionHandlerID, &clientId) != ERROR_ok)
		{
			printf("PLUGIN: Failed to receive client ID.\n"); 
			// FIXME <--------------- Client ID must begin from 1.
		}
		else
		{
			printf("PLUGIN: Current client ID: %d\n", clientId);
		}

		if(receiverThreadHndl == NULL)
		{
			printf("PLUGIN: Receiver handle is unassigned. Assigning..\n");
			receiverThreadHndl = (HANDLE)_beginthread(ts3plugin_receiveCommand, 0, NULL);
		}
		else
		{
			printf("PLUGIN: Receiver handle already assigned. \n");
			if(GetThreadId(receiverThreadHndl) != NULL)
			{
				printf("PLUGIN: Thread id: %d\n", GetThreadId(receiverThreadHndl));
			}
		}
		
/*		if(senderThreadHndl == NULL)
		{
			printf("PLUGIN: Sender handle is unassigned. Assigning..\n");
			senderThreadHndl = (HANDLE)_beginthread(ts3plugin_sendCommand, 0 , NULL);
		}
		else
		{
			printf("PLUGIN: Sender handle already assigned. \n");
			if(GetThreadId(senderThreadHndl) != NULL)
			{
				printf("PLUGIN: Thread id: %d\n", GetThreadId(senderThreadHndl));
			}
		}*/
	}
}

// IPC Implementation
void ts3plugin_pipeConnect()
{
	// Try to open the named pipe identified by the pipe name.
	while (stopRequested != true) 
    {
        clientPipe = CreateFile( 
            FULL_PIPE_NAME,                 // Pipe name 
            GENERIC_READ | GENERIC_WRITE,   // Read and write access
            0,                              // No sharing 
            NULL,                           // Default security attributes
            OPEN_EXISTING,                  // Opens existing pipe
            0,                              // Default attributes
            NULL                            // No template file
            );

		if(clientPipe != INVALID_HANDLE_VALUE)
		{
			printf("PLUGIN: Connected to a server pipe.\n");
			// Set the read mode and the blocking mode of the named pipe.
			DWORD dwMode = PIPE_READMODE_MESSAGE;
			if (!SetNamedPipeHandleState(clientPipe, &dwMode, NULL, NULL))
			{
				dwError = GetLastError();
				printf("PLUGIN: SetNamedPipeHandleState failed w/err 0x%08lx\n", dwError);
			}
			break;
		}
		else
		{
			Sleep(500);
		}
    }
}

void ts3plugin_receiveCommand(void* pArguments)
{
	// Call pipe connection function.
	ts3plugin_pipeConnect();

	BOOL fFinishRead = FALSE;
	wchar_t chResponse[BUFFER_SIZE];
    DWORD cbResponse, cbRead;
    cbResponse = sizeof(chResponse) -1;
	int errCode;

	while(stopRequested != true)
	{
        fFinishRead = ReadFile(
            clientPipe,             // Handle of the pipe
            chResponse,             // Buffer to receive the reply
            cbResponse,             // Size of buffer in bytes
            &cbRead,                // Number of bytes read 
            NULL                    // Not overlapped 
            );

        if (fFinishRead)
        {
			chResponse[cbRead/2] = '\0';
			printf("PLUGIN: Received %d bytes from server.\n",cbRead);
			wprintf(L"PLUGIN: Received message: \"%s\"\n",chResponse);
			incomingMessages.push(chResponse);
			printf("PLUGIN: Size of incoming messages queue: %d\n", incomingMessages.size());
			printf("PLUGIN: Message queue last message: \"%S\"\n",incomingMessages.front().c_str());
		}
		else
		{
			errCode = GetLastError();
			if(errCode == 109)
			{
				ts3plugin_pipeConnect();
			}
			else
			{
				printf("PLUGIN: Read failed. Error code: %d\n",GetLastError());
			}
		}
    }

	receiverThreadHndl = NULL;
}

void ts3plugin_sendCommand(void* pArguments) // FIXME Currently disabled.
{
	while(clientPipe == INVALID_HANDLE_VALUE || stopRequested)
	{
		Sleep(500);
	}

	printf("PLUGIN: Connected to a server pipe.\n");

	while(!stopRequested)
	{
		if(!outgoingMessages.empty())
		{
			const wchar_t* chRequest = outgoingMessages.front().c_str();
			DWORD cbWritten;
			outgoingMessages.pop();

			if (!WriteFile(
				clientPipe,											// Handle of the pipe
				chRequest,											// Message to be written
				sizeof(chRequest),									// Number of bytes to write
				&cbWritten,											// Number of bytes written
				NULL												// Not overlapped
				))
			{
				dwError = GetLastError();
				wprintf(L"WriteFile to pipe failed w/err 0x%08lx\n", dwError);
			}
		}
		else
		{
			Sleep(100);
		}
	}

	senderThreadHndl = INVALID_HANDLE_VALUE;
}

// Main loop implementation
void ts3plugin_pos()
{
	while(stopRequested)
	{
		if(incomingMessages.size() != 0)
		{
			// Check if there are incoming messages in the queue.
			// If they are - parse them.
			// FIXME Add code which would get the arguments from the parser.


		}
	}
}
