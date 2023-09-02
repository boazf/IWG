#ifdef USE_SFT
#include <Arduino.h>
#include <SD.h>
#include <time.h>
#include <SFT.h>
#include <TimeUtil.h>
#include <SDUtil.h>

EthServer SFT::server(765);    // The server object for TCP communication
char SFT::curPath[MAX_PATH + 1];    // The path of the current directory

#define SERVER_MAJOR_VERSION 1
#define SERVER_MINOR_VERSION 0

#define PATH_SEP_CHAR "/"

// Connect a client to the server
void SFT::Connect(EthClient &client)
{
#ifdef DEBUG_SFT    
   Traceln("Connect");
#endif
  // Reply with the server major.minor version
  uint16_t rep = ((SERVER_MAJOR_VERSION) << 8) | (SERVER_MINOR_VERSION);
  client.write((uint8_t *)&rep, sizeof(rep));
}

// Wait for any information data from the client to become available
// Waiting for at most 2 seconds (1.5 seconds on average).
bool SFT::WaitForClient(EthClient &client)
{
  time_t t0 = t_now;
  while(client.available() == 0 && t_now - t0 < 2);
  return client.available() != 0;
}

#define FILE_TRANSFER_BUFF_SIZE 256     // Size for the send and receive buffers over TCP
#define MAKE_LONG(b0, b1, b2, b3) ((long)(b0) | ((long)(b1)) << 8 | ((long)(b2)) << 16) | ((long)(b3) << 24)

char *combinePath(char *dstPath, const char *srcPath1, const char *srcPath2)
{
  if (strlen(srcPath1) + strlen(srcPath2) + (srcPath1[1] != '\0') > MAX_PATH)
    return NULL;
  strcpy(dstPath, srcPath1);
  if (srcPath1[1] != '\0')
    strcat(dstPath, PATH_SEP_CHAR);
  strcat(dstPath, srcPath2);
  return dstPath;
}

// Upload a file from the client to Arduino
// Currently the destination file can only be put in the current directory.
// It is not possible to include a path in the destination, only a file name
void SFT::Upload(EthClient &client)
{
  byte buff[FILE_TRANSFER_BUFF_SIZE];

  // Read the file name
  unsigned int len = client.read(buff, sizeof(buff));
  char *fileName = (char *)buff;
#ifdef DEBUG_SFT    
  {
		LOCK_TRACE();
    Trace("Upload: ");
    Traceln(fileName);
  }
#endif
  int fileNameLen = strlen(fileName);

  // File length is stored in four bytes after the file name.
  long length = MAKE_LONG(buff[fileNameLen + 1], buff[fileNameLen + 2], buff[fileNameLen +3], buff[fileNameLen + 4]);

  // Open the file
  SdFile file;
  uint8_t res;
  char filePath[MAX_PATH];
  if (combinePath(filePath, curPath, fileName) == NULL)
    res = 0;
  else
  {
    file = SD.open(filePath, FILE_WRITE);
    res = file;
  }

  // Send status indicating whether the file could be opened
  buff[0] = res == 1 ? 220 : 100;
  client.write(buff, 1);

  // Wait for the first data block with file content to be received
  if (!WaitForClient(client))
  {
#ifdef DEBUG_SFT    
    Traceln("Failed to receive first buffer");
#endif
    file.close();
    SD.remove(filePath);
    return;
  }

  // Loop through the blocks with file content
  long offset = 0;
  while (offset < length && (len = client.read(buff, sizeof(buff))) > 0)
  {
    offset += len;
#ifdef DEBUG_SFT    
    {
      LOCK_TRACE();
      Trace("Receiving file ");
      Traceln(offset);
    }
#endif
    // Write file content
    size_t res = file.write(buff, len);
#ifdef DEBUG_SFT    
    {
      LOCK_TRACE();
      Trace("Written to file: ");
      Traceln(res);
    }
#endif

    // Send indication whether the block could successfully be written
    buff[0] = res == len ? 220 : 100;
    client.write(buff, 1);

    // Wait for the next block to be received
    if (!WaitForClient(client))
    {
#ifdef DEBUG_SFT    
      Traceln("Failed to receive buffer");
#endif
      break;
    }
  }

  // If the file was not entirely received, delete it and exit
  if (offset != length)
  {
    file.close();
    SD.remove(filePath);
    return;
  }

#ifdef DEBUG_SFT    
  Traceln("Completed file upload");
#endif
  file.close();

  // Completion sequence
  char c = client.read();
#ifdef DEBUG_SFT    
  {
		LOCK_TRACE();
    Trace("Completion: ");
    Traceln((char)c);
  }
#endif
  buff[0] = c == 'u' ? 220 : 100;
  client.write(buff, 1);
}

// Download a file from Arduino to the client
// File will be looked up only in the current directory.
// Currently the source file cannot contain path, only a file name
void SFT::Download(EthClient &client)
{
  byte buff[FILE_TRANSFER_BUFF_SIZE];
  memset(buff, 0, sizeof(buff));

  // Read the file name
  client.read(buff, sizeof(buff));
  char *fileName = (char *)buff;
#ifdef DEBUG_SFT    
  {
		LOCK_TRACE();
    Trace("Download: ");
    Traceln(fileName);
  }
#endif

  // Try to open the file
  SdFile file;
  uint8_t ores;
  {
    char filePath[MAX_PATH + 1];
    if (combinePath(filePath, curPath, fileName) == NULL)
      ores = 0;
    else
    {
      file = SD.open(filePath);
      ores = file;
    }
  }
  if (ores == 0)
  {
    // Send indication that the file could not be opened
    byte ret = 100;
    client.write(&ret, 1);
    return;
  }

  // Send status indicating the file is opened and the file size
  long size = file.size();
  byte ret[5];
  ret[0] = 220;
  *((long *)(ret + 1)) = size;
  client.write(ret, sizeof(ret));

  // Read the first block of the file content
  int16_t rres = file.read(buff, sizeof(buff));
  long offset = 0;
  bool fail = false;
  while (!fail && rres != 0 && offset < size)
  {
    // Loop through the file content blocks
    offset += rres;
#ifdef DEBUG_SFT    
    {
      LOCK_TRACE();
      Trace("Read file: ");
      Trace(offset);
      Trace(" bytes out of ");
      Traceln(size);
    }
#endif
    // Wait for handshake
    if (!WaitForClient(client))
    {
#ifdef DEBUG_SFT    
      Traceln("Failed to receive download buffer request");
#endif
      fail = true;
      continue;
    }
    // Validate correct handshake
    byte req;
    int reqRes = client.read(&req, 1);
    if (reqRes != 1 && req != (byte)'w')
    {
#ifdef DEBUG_SFT    
  		LOCK_TRACE();
      Trace("Unexpected buffer request: res = ");
      Trace(reqRes);
      Trace(", data = ");
      Traceln((char) req);
#endif
      fail = true;
      continue;
    }
    // Send the current file content block
    size_t sres = client.write(buff, rres);
    if (sres != (size_t)rres)
    {
#ifdef DEBUG_SFT    
  		LOCK_TRACE();
      Trace("Failed to write file content to client, write result: ");
      Trace(sres);
      Trace(", sent bytes: ");
      Traceln(rres);
#endif
      fail = true;
      continue;
    }

    // Read the next file content block, if the file was not read yet completely.
    if (offset < size)
      rres = file.read(buff, sizeof(buff));
  }

  // Close the file
  file.close();

  if (fail)
    return;

#ifdef DEBUG_SFT    
  Traceln("Completed file download");
#endif

  // Wait for completion handshake
  if (!WaitForClient(client))
  {
#ifdef DEBUG_SFT    
    Traceln("Failed to receive completion status request");
#endif
    return;
  }
  // Completion sequence
#ifdef DEBUG_SFT    
  int len = 
#endif
  client.read(ret, sizeof(ret));
#ifdef DEBUG_SFT    
  {
		LOCK_TRACE();
    Trace("Completion: len = ");
    Trace(len);
    Trace(" content: ");
    for (int i = 0; i < len; i++)
      Trace((char)ret[i]);
    Traceln();
  }
#endif
  buff[0] = ret[0] == (byte)'?' ? 220 : 100;
  client.write(buff, 1);
  delay(1);
}

#define MAX_FILE_NAME 128
// File information sent to client
typedef struct FILE_INFO_
{
  byte signature[2];            // Indication that this is the first word of the file info structure
  byte day;                     // File last write day
  byte month;                   // File last write month
  byte year[2];                 // File last write year
  byte hour;                    // File last write hour
  byte minute;                  // File last write minute
  byte size[4];                 // File size
  byte isDir;                   // File is a directory
  char name[MAX_FILE_NAME];     // File name
} __attribute__((__packed__)) FILE_INFO;

// Send to the client information about all entries in the current directory
void SFT::ListDirectory(EthClient &client)
{
#ifdef DEBUG_SFT
  Traceln("List directory...");
#endif
  bool fail = false;
  // Loop through the directory entries
  File dir = SD.open(curPath);
  File file = dir.openNextFile();
  while (file)
  {
    FILE_INFO fileInfo;
    fileInfo.signature[0] = 222;
    fileInfo.signature[1] = 0;
    char filePath[MAX_PATH + 1];
    strcpy(filePath, file.name());
    char *slash = strrchr(filePath, *PATH_SEP_CHAR);
    strncpy(fileInfo.name, slash + 1, MAX_FILE_NAME);
    fileInfo.isDir = file.isDirectory();
    long size = file.size();
    memcpy(&fileInfo.size, &size, 4);
    time_t fileTime = file.getLastWrite();
    tm tr;
    localtime_r(&fileTime, &tr);
    fileInfo.day = tr.tm_mday;
    fileInfo.month = tr.tm_mon + 1;
    word year = tr.tm_year + 1900;
    memcpy(&fileInfo.year, &year, 2);
    fileInfo.hour = tr.tm_hour;
    fileInfo.minute = tr.tm_min;
    // Send the file information to the client
    int res = client.write((byte *)&fileInfo, sizeof(fileInfo));
    if (res != sizeof(fileInfo))
    {
      fail = true;
    }
    delay(1);

    // Wait for handshake
    if (!WaitForClient(client))
    {
#ifdef DEBUG_SFT
      Traceln("Failed to receive continuation");
#endif
      fail = true;
      break;
    }

    // Validate handshake
    char c = client.read();
    if (c != 'l')
    {
#ifdef DEBUG_SFT
      Traceln("Unexpected continuation");
#endif
      fail = true;
      break;
    }
    file.close();
    file = dir.openNextFile();
  }

  dir.close();
  // Send finalizing indication
  byte ret = fail ? 100 : 220;
  client.write(&ret, 1);
  delay(1);
}

// Change curent directory
// Current the requested directory can only be relative to the current directory
// and can contain only one directory down (sending the directory name), or onr directory
// up (sending ..). The client can also send no directory, this will result in sending the
// path of the curent directory. In any case the path of the new current directory is sent
// back to the client
void SFT::ChangeDirectory(EthClient &client)
{
  bool fail = false;
  char path[MAX_PATH + 2];
  memset(path, 0, sizeof(path));
  // Read the requested new directory
  int len = client.read((byte *)path, sizeof(path));
  if (len == 0)
  {
#ifdef DEBUG_SFT  
    Traceln("Change directory: path was not received");
#endif
    fail = true;
  }
  else
  {
#ifdef DEBUG_SFT  
    {
      LOCK_TRACE();
      Trace("Change directory: ");
      Traceln(path);
    }
#endif

    if (path[0] != '\0')
    {
      // Client sent a new directory to change to...
      if (strcmp(path, "..") == 0)
      {
        // Request to go up to the parent directory
        if (strcmp(curPath, PATH_SEP_CHAR) == 0)
        {
          // cannot go up in case we're currently in the root directory
#ifdef DEBUG_SFT  
          Traceln("can't go up directory from root directory");
#endif
          fail = true;
        }
        else
        {
          // Get the last backslash character in the current directory path
          char *backSlash = strrchr(curPath, *PATH_SEP_CHAR);
          if (backSlash != curPath)
            // The curent path contains more than one backslash.
            // Trim the path one directory less from the end
            *backSlash = '\0';
          else
            // The current path contained only one sub-directory.
            // We should go up one directory to the root directory.
            curPath[1] = '\0';
          
        }
      }
      else
      {
        // Verify that the path length will not exceed the maximum
        char tempPath[MAX_PATH + 1];
        if (combinePath(tempPath, curPath, path) != NULL)
        {
          File newPath = SD.open(tempPath);
          fail = !newPath || !newPath.isDirectory();
          if (!fail)
          {
            strcpy(curPath, tempPath);
            if (newPath)
              newPath.close();
          }
        }
        else
        {
            // Path will become too long, we do not allow it
            fail = true;
        }
      }
    }
  }

  // Send pass/fail indication and the current path
  path[0] = fail ? 100 : 220;
  strcpy(path+1, curPath);
#ifdef DEBUG_SFT  
  {
    LOCK_TRACE();
    Trace("Current path: ");
    Traceln(curPath);
  }
#endif
  // Send finalizing indication to the client
  client.write(path, strlen(path));
  delay(1);
}

// Make new sub-directory
// Currently it is possible to only request to create a new sub-directory
// that is directly under the current directory. The requested directory
// cannot contain path, only one directory name.
void SFT::MakeDirectory(EthClient &client)
{
  bool fail = false;
  char path[MAX_PATH];
  memset(path, 0, sizeof(path));
  // Read the name of the requested sub-directory
  int len = client.read((byte *)path, sizeof(path));
#ifdef DEBUG_SFT    
  {
    LOCK_TRACE();
    Trace("Make directory: ");
    Traceln(path);
  }
#endif
  if (len == 0)
  {
    // No name was sent.
#ifdef DEBUG_SFT    
    Traceln("path was not received");
#endif
    fail = true;
  }
  else
  {
    char newPath[MAX_PATH + 1];
    fail = combinePath(newPath, curPath, path) == NULL || !SD.mkdir(newPath);
  }
  
  // Send pass/fail indication to the client
  byte ret = fail ? 100 : 220;
  client.write(&ret, 1);
  delay(1);
}

// Delete a file
// Currently files can only be deleted from the current directory.
// It is not possible to send a path to a file.
void SFT::Delete(EthClient &client)
{
  bool fail = false;
  char path[MAX_PATH];
  memset(path, 0, sizeof(path));
  // Read the file name to be deleted
  int len = client.read((byte *)path, sizeof(path));
#ifdef DEBUG_SFT    
  {
    LOCK_TRACE();
    Trace("Delete file: ");
    Traceln(path);
  }
#endif
  if (len == 0)
  {
#ifdef DEBUG_SFT    
    Traceln("file name was not received");
#endif
    fail = true;
  }
  else
  {
    char tempPath[MAX_PATH + 1];
    fail = combinePath(tempPath, curPath, path) == NULL || !SD.remove(tempPath);
#ifdef DEBUG_SFT    
    if (fail)
    {   
      LOCK_TRACE();
      Trace("Failed to delete file: ");
      Traceln(path);
    }
#endif
  }
  
  // Send pass/fail indication to the client
  byte ret = fail ? 100 : 220;
  client.write(&ret, 1);
  delay(1);
}

// Delete a sub-directory
// Currently it is possible to only delete a sub-directory of the current directory.
// It is not possible to send a path to a directory for deletion
void SFT::RemoveDirectory(EthClient &client)
{
  bool fail = false;
  char path[MAX_PATH];
  memset(path, 0, sizeof(path));
  // Read the name of the sub-directory to be deleted
  int len = client.read((byte *)path, sizeof(path));
#ifdef DEBUG_SFT    
  {
    LOCK_TRACE();
    Traceln(path);
    Trace("Remove directory: ");
  }
#endif
  if (len == 0)
  {
    // No sub-directory name was received
#ifdef DEBUG_SFT    
    Traceln("file name was not received");
#endif
    fail = true;
  }
  else
  {
    char tempPath[MAX_PATH + 1];
    fail = combinePath(tempPath, curPath, path) == NULL || !SD.rmdir(tempPath);
  }
#ifdef DEBUG_SFT    
  if (fail)
  {
    LOCK_TRACE();
    Trace("Failed to remove directory: ");
    Traceln(path);
  }
#endif
  
  // Send pass/fail indication to the client
  byte ret = fail ? 100 : 220;
  client.write(&ret, 1);
  delay(1);
}

// Initialize the server
void SFT::Init()
{
  // Start the server object to listen for requests from the client
  server.begin();

  // Set current path to the root directory
  memset(curPath, 0, sizeof(curPath));
  curPath[0] = *PATH_SEP_CHAR;
#ifdef DEBUG_SFT    
  Traceln("SFT Initialized");
#endif
}

// See if the client sent a request and handle it
void SFT::DoService()
{
    EthClient client = server.available();
    if (!client)
    {
        return;
    }
#ifdef DEBUG_SFT    
    Traceln("New client");
#endif
    AutoSD autoSD;
    while (client.connected())
    {
        if (!client.available())
        {
            continue;
        }
        // The first byte of each request indicates the type of request
        uint8_t c = client.read();
        switch(c)
        {
          case 'C':
            Connect(client);
            break;

          case 'U':
            Upload(client);
            break;

          case 'W':
            Download(client);
            break;

          case 'L':
            ListDirectory(client);
            break;

          case 'D':
            ChangeDirectory(client);
            break;

          case 'M':
            MakeDirectory(client);
            break;

          case 'x':
            Delete(client);
            break;

          case 'X':
            RemoveDirectory(client);
            break;
        }
        break;
    }
    // Close the connection:
    client.stop();
#ifdef DEBUG_SFT    
    Traceln("Client disconnected");
#endif
}

void InitSFT()
{
  SFT::Init();
}

void DoSFTService()
{
  SFT::DoService();
}
#endif // USE_SFT