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
#include "include/public_errors.h"
#include "include/public_errors_rare.h"
#include "include/public_definitions.h"
#include "include/public_rare_definitions.h"
#include "include/ts3_functions.h"
#include "include/ts3plugin.h"
#include "include/parser.h"
using namespace std;

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
anyID myId;
HANDLE clientPipe = INVALID_HANDLE_VALUE;
HANDLE receiverThreadHndl;
DWORD dwError = ERROR_SUCCESS;
HANDLE senderThreadHndl;
BOOL stopRequested = FALSE;
BOOL inRt = 0;
char *chname[] = {"PvP_WOG","RT",""};
uint64 newcid = 0;
uint64 oldcid = 0;
BOOL connected = 0;

queue<wstring> incomingMessages;
queue<wstring> outgoingMessages;

void ts3plugin_moveToRt();
void ts3plugin_moveFromRt();
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

	int connectionState;
	connectionHandlerID = ts3Functions.getCurrentServerConnectionHandlerID();
	if(connectionHandlerID)
	{
		ts3Functions.getConnectionStatus(connectionHandlerID, &connectionState);
		if(connectionState == STATUS_CONNECTION_ESTABLISHED)
		{
			if(ts3Functions.getClientID(connectionHandlerID, &myId) != ERROR_ok)
			{
				printf("PLUGIN: Failed to receive client ID.\n"); 
			}
			else
			{
				printf("PLUGIN: Current client ID: %d\n", myId);
			}
		}
	}
	
/*	if(receiverThreadHndl == NULL)
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
		else
		{
			return 1;
		}
	}*/

	printf("PLUGIN: Completed init().\n");

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
	if(newStatus == STATUS_CONNECTION_ESTABLISHED)
	{
		printf("PLUGIN: Connected to server.\n");

		// Receive client id
		// Overwrite if already assigned.
		if(ts3Functions.getClientID(connectionHandlerID, &myId) != ERROR_ok)
		{
			printf("PLUGIN: Failed to receive client ID.\n"); 
		}
		else
		{
			printf("PLUGIN: Current client ID: %d\n", myId);
		}
		connected = TRUE;
	}
	else
	{
		printf("PLUGIN: Disconnected from server.\n");
		connected = FALSE;
	}
}

// IPC Implementation
void ts3plugin_pipeConnect()
{
	// Try to open the named pipe identified by the pipe name.
	while (stopRequested != TRUE) 
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

	while(stopRequested != TRUE)
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

	while(stopRequested != TRUE)
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
void ts3plugin_mainLoop()
{
	while(stopRequested != TRUE && connected == TRUE)
	{
		if(incomingMessages.size() != 0)
		{
			// Check if there are incoming messages in the queue.
			// If they are - parse them.
			// FIXME Add code which would get the arguments from the parser.
			printf("PLUGIN: mainLoopExec.\n");

			if(inRt == FALSE)
			{
				ts3plugin_moveToRt();
			}
			else
			{
				printf("PLUGIN: Already in RT channel.\n");
			}

			// Set player position

		}
	}
}

void ts3plugin_moveToRt()
{
	// Move user to RT channel.
	unsigned int error = ts3Functions.getChannelIDFromChannelNames(connectionHandlerID, chname, &newcid);
	if(error == ERROR_ok)
		{
			if(newcid)
			{
				anyID *clientList = 0;
				ts3Functions.getChannelClientList(connectionHandlerID, newcid, &clientList);
				ts3Functions.requestMuteClients(connectionHandlerID, clientList, 0);
				ts3Functions.getChannelOfClient(connectionHandlerID, myId, &oldcid);
				ts3Functions.requestClientMove(connectionHandlerID, myId, newcid, "1234", 0);
				inRt = TRUE;
			}
			else
			{
				printf("PLUGIN: No RT channel found.\n");
			}
		}
	else
		{
			printf("PLUGIN: Failed to get RT channel.\n");
		}
}

void ts3plugin_moveFromRt()
{
	if(inRt = TRUE)
	{
		// Move the player from RT
		unsigned int error = ts3Functions.requestClientMove(connectionHandlerID, myId, oldcid, "", 0);
		if(error == ERROR_ok)
		{
			printf("PLUGIN: Moved user back to old channel.\n");
			inRt = FALSE;
		}
		else
		{
			printf("PLUGIN: Failed to move user back to old channel. Trying to move to default channel.\n");

			uint64* allChannels;

			if(ts3Functions.getChannelList(connectionHandlerID, &allChannels) == ERROR_ok)
			{
				int isDefault = 0,
				i = 0;
				for(; (allChannels[i] != NULL) && (isDefault == 0); i++)
				if(ts3Functions.getChannelVariableAsInt(connectionHandlerID, allChannels[i], CHANNEL_FLAG_DEFAULT, &isDefault) != ERROR_ok )
				{
					printf("PLUGIN: Failed to check default flag of channel: %d\n", allChannels[i]);
				}
				if(ts3Functions.requestClientMove(connectionHandlerID, myId, oldcid, "", 0) == ERROR_ok)
				{
					printf("PLUGIN: Moved user to default channel.\n");
					inRt = FALSE;
				}
				else
				{
					printf("PLUGIN: Failed to move user to default channel.\n");
					// ISSUE Try to disconnect in this case?
				}
				ts3Functions.freeMemory(allChannels);
			}
			else
			{
				printf("PLUGIN: Failed to get channel list.\n");
			}
		}
	}
	else
		printf("PLUGIN: Client already not in RT.\n");
}

void ts3plugin_onPluginCommandEvent(uint64 serverConnectionHandlerID, const char* , const char* pluginCommand)
{
	// Receive coordinates from other players.
	if(serverConnectionHandlerID != connectionHandlerID)
	{
		printf("PLUGIN: onUpdateClientEvent failure.\n");
		return;
	}

	char* commandText = strcpy(commandText, pluginCommand);
	char* containedID = strtok(commandText, "@");

	anyID clientId = (anyID)containedID;

	if(clientId != myId)
	{
		//ts3plugin_parseCommandText();
	}
}

void ts3plugin_parseCommandText()
{
	// Parse the command from other clients call some other function that would do something else
	// For example position other clients, determine if they are talking (or not)
}

void ts3plugin_onTalkStatusChangeEvent(uint64 serverConnectionHandlerID, int status, int, anyID clientID)
{
	if(status == STATUS_TALKING)
	{
		// Code describing what happens when any of the clients start talking
		if(clientID == myId)
		{
			printf("PLUGIN: I am talking.\n");
			// We are the ones talking.
			// Inform everyone else of our current coordinates and stuff.
			//ts3Functions.sendPluginCommand(connectionHandlerID, pluginID, "insert_command_text_here", PluginCommandTarget_CURRENT_CHANNEL,NULL,0);
		}
		else
		{
			printf("PLUGIN: Someone else talking.\n");
			// Someone else starts talking.
		}
	}
	else
	{
		// And what happens when he stops.
		if(clientID == myId)
		{
			// We were the ones talking.
			//ts3Functions.sendPluginCommand(connectionHandlerID, pluginID, "insert_command_text_here", PluginCommandTarget_CURRENT_CHANNEL,NULL,0);

			printf("PLUGIN: I stopped talking.\n");
		}
		else
		{
			// Someone else stopped talking.
			printf("PLUGIN: Someone else stopped talking.\n");
		}
	}
}
