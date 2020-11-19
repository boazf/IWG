#include <Arduino.h>
#include <SD.h>
#include <time.h>
#include <SFT.h>
#include <TimeUtil.h>
#include <SDUtil.h>
#include <AppConfig.h>
#include <WString.h>
#include <IPAddress.h>

EthernetServer SFT::server(765);    // The server object for TCP communication
SdVolume SFT::vol;                  // The SD card volume
SdFile SFT::curDir;                 // The current directory object
char SFT::curPath[MAX_PATH + 1];    // The path of the current directory

#define SERVER_MAJOR_VERSION 1
#define SERVER_MINOR_VERSION 0

// Connect a client to the server
void SFT::Connect(EthernetClient &client)
{
#ifdef DEBUG_SFT    
   Serial.println("Connect");
#endif
  // Reply with the server major.minor version
  uint16_t rep = ((SERVER_MAJOR_VERSION) << 8) | (SERVER_MINOR_VERSION);
  client.write((uint8_t *)&rep, sizeof(rep));
}

// Wait for any information data from the client to become available
// Waitnig for at most 2 seconds (1.5 seconds on average).
bool SFT::WaitForClient(EthernetClient &client)
{
  time_t t0 = t;
  while(client.available() == 0 && t - t0 < 2);
  return client.available() != 0;
}

#define FILE_TRANSFER_BUFF_SIZE 256     // Size for the send and receive buffers over TCP
#define MAKE_LONG(b0, b1, b2, b3) ((long)(b0) | ((long)(b1)) << 8 | ((long)(b2)) << 16) | ((long)(b3) << 24)

// Upload a file from the client to Arduino
// Currently the destination file can only be put in the currect directoy.
// It is not possible to include a path in the destination, only a file name
void SFT::Upload(EthernetClient &client)
{
#ifdef DEBUG_SFT    
  Serial.print("Upload: ");
#endif
  byte buff[FILE_TRANSFER_BUFF_SIZE];

  // Read the file name
  unsigned int len = client.read(buff, sizeof(buff));
  char *fileName = (char *)buff;
#ifdef DEBUG_SFT    
  Serial.println(fileName);
#endif
  int fileNameLen = strlen(fileName);

  // File length is stored in four bytes after the file name.
  long length = MAKE_LONG(buff[fileNameLen + 1], buff[fileNameLen + 2], buff[fileNameLen +3], buff[fileNameLen + 4]);

  // Open the file
  SdFile file, org = curDir;
  uint8_t res = file.open(curDir, fileName, O_CREAT | O_WRITE);
  curDir = org; // For some unknown reason file.open changes the directory object passed to it

  // Send status indicating whether the file could be opened
  buff[0] = res == 1 ? 220 : 100;
  client.write(buff, 1);

  // Wait for the first data block with file content to be received
  if (!WaitForClient(client))
  {
#ifdef DEBUG_SFT    
    Serial.println("Failed to receive first buffer");
#endif
    file.close();
    file.remove();
    return;
  }

  // Erase file content
  file.truncate(0); 

  // Loop through the blocks with file content
  long offset = 0;
  while (offset < length && (len = client.read(buff, sizeof(buff))) > 0)
  {
    offset += len;
#ifdef DEBUG_SFT    
    Serial.print("Receiving file ");
    Serial.println(offset);
#endif
    // Write file content
    size_t res = file.write(buff, len);
#ifdef DEBUG_SFT    
    Serial.print("Written to file: ");
    Serial.println(res);
#endif

    // Send indication whether the block could successfully be written
    buff[0] = res == len ? 220 : 100;
    client.write(buff, 1);

    // Wait for the next block to be received
    if (!WaitForClient(client))
    {
#ifdef DEBUG_SFT    
      Serial.println("Failed to receive buffer");
#endif
      break;
    }
  }

  // If the file was not entirely received, delete it and exit
  if (offset != length)
  {
    file.close();
    file.remove();
    return;
  }

#ifdef DEBUG_SFT    
  Serial.println("Completed file upload");
#endif
  file.close();

  // Completion sequence
  char c = client.read();
#ifdef DEBUG_SFT    
  Serial.print("Completion: ");
  Serial.println((char)c);
#endif
  buff[0] = c == 'u' ? 220 : 100;
  client.write(buff, 1);
}

// Download a file from Arduino to the client
// File will be looked up only in the current directory.
// Curently the source file cannot contain path, only a file name
void SFT::Download(EthernetClient &client)
{
#ifdef DEBUG_SFT    
  Serial.print("Download: ");
#endif
  byte buff[FILE_TRANSFER_BUFF_SIZE];
  memset(buff, 0, sizeof(buff));

  // Read the file name
  client.read(buff, sizeof(buff));
  char *fileName = (char *)buff;
#ifdef DEBUG_SFT    
  Serial.println(fileName);
#endif

  // Try to open the file
  SdFile file, org = curDir;
  uint8_t ores = file.open(curDir, fileName, O_READ);
  curDir = org;
  if (ores == 0)
  {
    // Send indication that the file could not be opened
    byte ret = 100;
    client.write(&ret, 1);
    return;
  }

  // Send status indicating the file is opened and the file size
  long size = file.fileSize();
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
    Serial.print("Read file: ");
    Serial.print(offset);
    Serial.print(" bytes out of ");
    Serial.println(size);
#endif
    // Wait for handshake
    if (!WaitForClient(client))
    {
#ifdef DEBUG_SFT    
      Serial.println("Failed to receive dowload buffer request");
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
      Serial.print("Enexpected buffer request: res = ");
      Serial.print(reqRes);
      Serial.print(", data = ");
      Serial.println((char) req);
#endif
      fail = true;
      continue;
    }
    // Send the current file content block
    size_t sres = client.write(buff, rres);
    if (sres != (size_t)rres)
    {
#ifdef DEBUG_SFT    
      Serial.print("Failed to write file content to client, write result: ");
      Serial.print(sres);
      Serial.print(", sent bytes: ");
      Serial.println(rres);
#endif
      fail = true;
      continue;
    }

    // Read the nect file content block, if the file was not read yet completely.
    if (offset < size)
      rres = file.read(buff, sizeof(buff));
  }

  // Close the file
  file.close();

  if (fail)
    return;

#ifdef DEBUG_SFT    
  Serial.println("Completed file download");
#endif

  // Wait for completion handshake
  if (!WaitForClient(client))
  {
#ifdef DEBUG_SFT    
    Serial.println("Failed to receive completion status request");
#endif
    return;
  }
  // Completion sequence
#ifdef DEBUG_SFT    
  int len = 
#endif
  client.read(ret, sizeof(ret));
#ifdef DEBUG_SFT    
  Serial.print("Completion: len = ");
  Serial.print(len);
  Serial.print(" content: ");
  for (int i = 0; i < len; i++)
    Serial.print((char)ret[i]);
  Serial.println();
#endif
  buff[0] = ret[0] == (byte)'?' ? 220 : 100;
  client.write(buff, 1);
  delay(1);
}

// File information sent to client
typedef struct FILE_INFO_
{
  word signature;   // Indication that this is the first word of the file info structure
  byte day;         // File last write day
  byte month;       // File last write month
  word year;        // File last write year
  byte hour;        // File last write hour
  byte minute;      // File last write minute
  long size;        // File size
  bool isDir;       // File is a directory
  char name[13];    // File name
} FILE_INFO;

// Send to the client information about all entries in the curernt directory
void SFT::ListDirectory(EthernetClient &client)
{
  SdFile org = curDir;
  dir_t dir;
#ifdef DEBUG_SFT
  Serial.println("List directory...");
#endif
  bool fail = false;
  // Loop through the directory entries
  while (curDir.readDir(&dir)>0)
  {
    FILE_INFO fileInfo;
    fileInfo.signature = 222;
    SdFile::dirName(dir, fileInfo.name);
#ifdef DEBUG_SFT
    Serial.println(fileInfo.name);
#endif
    fileInfo.isDir = DIR_IS_SUBDIR(&dir);
    fileInfo.size = dir.fileSize;
    fileInfo.day = FAT_DAY(dir.lastWriteDate);
    fileInfo.month = FAT_MONTH(dir.lastWriteDate);
    fileInfo.year = FAT_YEAR(dir.lastWriteDate);
    fileInfo.hour = FAT_HOUR(dir.lastWriteTime);
    fileInfo.minute = FAT_MINUTE(dir.lastWriteTime);
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
      Serial.println("Failed to receive continuation");
#endif
      fail = true;
      break;
    }

    // Validate handshake
    char c = client.read();
    if (c != 'l')
    {
#ifdef DEBUG_SFT
      Serial.println("Unexpected continuation");
#endif
      fail = true;
      break;
    }
  }

  // Send finalizing indication
  byte ret = fail ? 100 : 220;
  client.write(&ret, 1);
  delay(1);
  curDir = org; // Restore the root directory objet since enumeratin directory entries modifies the directory file object
}

// Change curent directory
// Currentl the requested directory can only be relative to the current directory
// and can contain only one directory down (sending the directory name), or onr directory
// up (sending ..). The client can also send no direcotory, this will result in sending the
// path of the curent directory. In any case the path of the new current directory is sent
// back to the client
void SFT::ChangeDirectory(EthernetClient &client)
{
  bool fail = false;
  char path[MAX_PATH + 2];
  memset(path, 0, sizeof(path));
#ifdef DEBUG_SFT  
  Serial.print("Change directory: ");
#endif
  // Read the requested new directory
  int len = client.read((byte *)path, sizeof(path));
  if (len == 0)
  {
#ifdef DEBUG_SFT  
    Serial.println("path was not received");
#endif
    fail = true;
  }
  else
  {
#ifdef DEBUG_SFT  
    Serial.println(path);
#endif

    if (path[0] != '\0')
    {
      // Client sent a new directory to change to...
      if (strcmp(path, "..") == 0)
      {
        // Request to go up to the parent directory
        if (strcmp(curPath, "\\") == 0)
        {
          // cannot go up in case we're currently in the root directory
#ifdef DEBUG_SFT  
          Serial.println("can't go up directory from root directory");
#endif
          fail = true;
        }
        else
        {
          // Get the last backslash character in the current directory path
          char *backSlash = strrchr(curPath, '\\');
          if (backSlash != curPath)
            // The curent path contains more than one backslash.
            // Trim the path one directory less from the end
            *backSlash = '\0';
          else
            // The current path contained only one sub-directory.
            // We should go up one directory to the root directory.
            curPath[1] = '\0';
          
          // Open the root directory
          curDir.close();
          curDir.openRoot(vol);
          if (curPath[1] != '\0')
          {
            // We should now go down each time one sub-directory down the path
            // Copy the path to a temporary buffer, not including the first backslash
            char pathCopy[MAX_PATH];
            strcpy(pathCopy, curPath + 1);
            // Point to the name of the first sub-directory
            char *inCurPath = pathCopy;
            // Find the next backslash in the path.
            backSlash = strchr(inCurPath, '\\');
            while (!fail && backSlash != NULL)
            {
              // Change the backslash it to null character (aka '\0')
              *backSlash = '\0';
              // Open the sub-directory
              SdFile subDir;
              int res = subDir.open(curDir, inCurPath, O_READ);
              if (res == 0)
              {
                // Failed to open sub-directory
#ifdef DEBUG_SFT    
                Serial.println("Failed to open directory: ");
                Serial.println(inCurPath);
#endif
                fail = true;
                break;
              }
              else
              {
                // Close the current directory object
                curDir.close();
                // Set the current directory to be the sub-directory
                curDir = subDir;
                // Find the next backslash in the path
                inCurPath = backSlash + 1;
                backSlash = strchr(inCurPath, '\\');
              }
            }
            if (!fail)
            {
              // Open the last sub-directory in the path
              SdFile subDir;
              int res = subDir.open(curDir, inCurPath, O_READ);
              if (res == 0)
              {
#ifdef DEBUG_SFT    
                Serial.println("Failed to open directory: ");
                Serial.println(inCurPath);
#endif
                fail = true;
              }
              else
              {
                curDir.close();
                curDir = subDir;
              }
            }
          }
        }
      }
      else
      {
        // Verify that the path length will not exceed the maximum
        if (strlen(curPath) + strlen(path) <= MAX_PATH)
        {
          // Try to open the sub-directory
          SdFile subDir, org = curDir;
          if (subDir.open(curDir, path, O_READ) == 0)
          {
            // Failed to open the sub-directory, it is porbably either not a directory,
            // or it doesn't exist
            curDir = org;
            fail = true;
          }
          else
          {
            // Concatinate the new sub-directory to the path
            if (strlen(curPath) > 1)
                strcat(curPath, "\\");
            strcat(curPath, path);
            // Change current directory object to point to the new directory
            curDir.close();
            curDir = subDir;
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
  Serial.print("Current path: ");
  Serial.println(curPath);
#endif
  // Send finalizing indication to the client
  client.write(path, strlen(path));
  delay(1);
}

// Make new sub-directory
// Curerntly it is possible to only request to create a new sub-directory
// that is directly under the current directory. The requested directory
// cannot contain path, only one directory name.
void SFT::MakeDirectory(EthernetClient &client)
{
  SdFile org = curDir;
  bool fail = false;
  char path[MAX_PATH];
  memset(path, 0, sizeof(path));
#ifdef DEBUG_SFT    
  Serial.print("Make directory: ");
#endif
  // Read the name of the requested sub-directory
  int len = client.read((byte *)path, sizeof(path));
#ifdef DEBUG_SFT    
  Serial.println(path);
#endif
  if (len == 0)
  {
    // No name was sent.
#ifdef DEBUG_SFT    
    Serial.println("path was not received");
#endif
    fail = true;
  }
  else
  {
    // Create the new sub-directory
    SdFile subDir;
    if (subDir.makeDir(curDir, path) == 0)
    {
      // sub-directory creation failed.
      // Probably a sub-directory with the same name already exist
#ifdef DEBUG_SFT    
      Serial.println("Failed to create directory");
#endif
      fail = true;
    }
    else
    {
      // Close the sub-directory object
      subDir.close();
    }
  }
  
  // Send pass/fail indication to the client
  byte ret = fail ? 100 : 220;
  client.write(&ret, 1);
  delay(1);
  curDir = org;
}

// Delete a file
// Currently files can only be deleted from the current directory.
// It is not possible to send a path to a file.
void SFT::Delete(EthernetClient &client)
{
  SdFile org = curDir;
  bool fail = false;
  char path[MAX_PATH];
  memset(path, 0, sizeof(path));
#ifdef DEBUG_SFT    
  Serial.print("Delete file: ");
#endif
  // Read the file name to be deleted
  int len = client.read((byte *)path, sizeof(path));
#ifdef DEBUG_SFT    
  Serial.println(path);
#endif
  if (len == 0)
  {
#ifdef DEBUG_SFT    
    Serial.println("file name was not received");
#endif
    fail = true;
  }
  else
  {
    // Try to delete the file
    if (SdFile::remove(curDir, path) == 0)
    {
      // File deletion failed
      fail = true;
#ifdef DEBUG_SFT    
      Serial.print("Failed to delete file: ");
      Serial.println(path);
#endif
    }
  }
  
  // Send pass/fail indication to the client
  byte ret = fail ? 100 : 220;
  client.write(&ret, 1);
  delay(1);
  curDir = org;
}

// Delete a sub-directory
// Currently it is possibleto only delete a sub-directory of the current directory.
// It is not possible to send a path to a directory for deletion
void SFT::RemoveDirectory(EthernetClient &client)
{
  SdFile org = curDir;
  bool fail = false;
  char path[MAX_PATH];
  memset(path, 0, sizeof(path));
#ifdef DEBUG_SFT    
  Serial.print("Remove directory: ");
#endif
  // Read the name of the sub-directory to be deleted
  int len = client.read((byte *)path, sizeof(path));
#ifdef DEBUG_SFT    
  Serial.println(path);
#endif
  if (len == 0)
  {
    // No sub-directory name was received
#ifdef DEBUG_SFT    
    Serial.println("file name was not received");
#endif
    fail = true;
  }
  else
  {
    // Try to delete the sub-directory
    SdFile subDir;

    subDir.open(curDir, path, O_READ);
    if (subDir.rmDir() == 0)
    {
      // Failed to delete the sub-directory
      fail = true;
#ifdef DEBUG_SFT    
      Serial.print("Failed to remove directory: ");
      Serial.println(path);
#endif
    }
  }
  
  // Send pass/fail indication to the client
  byte ret = fail ? 100 : 220;
  client.write(&ret, 1);
  delay(1);
  curDir = org;
}

#define FILL_BUFF(type, val) *((type *)ptr) = val; ptr += sizeof(type)

// void MemoryDump(byte *buff, size_t len)
// {
//   for (size_t i = 0; i < len; i++)
//   {
//     if (buff[i]<16)
//       Serial.print("0");
//     Serial.print(buff[i], HEX);
//     if ((i + 1) % 16 == 0 || i == len - 1)
//       Serial.println();
//     else
//       Serial.print(", ");
//   }
// }

void SFT::ReadAppConfig(EthernetClient &client)
{
#ifdef DEBUG_SFT    
  Serial.print("Read App Config: ");
#endif
  byte buff[128];
  byte *ptr = buff;
  FILL_BUFF(bool, AppConfig::getAutoRecovery());
  FILL_BUFF(int, AppConfig::getConnectionTestPeriod());
  FILL_BUFF(uint32_t, (uint32_t)AppConfig::getLANAddr());
  FILL_BUFF(bool, AppConfig::getLimitCycles());
  FILL_BUFF(int, AppConfig::getMaxHistory());
  FILL_BUFF(int, AppConfig::getMDisconnect());
  FILL_BUFF(int, AppConfig::getMReconnect());
  FILL_BUFF(int, AppConfig::getRDisconnect());
  FILL_BUFF(int, AppConfig::getRecoveryCycles());
  FILL_BUFF(int, AppConfig::getRReconnect());
  String server1 = AppConfig::getServer1();
  strcpy((char *)ptr, server1.c_str());
  ptr += strlen(server1.c_str()) + 1;
  String server2 = AppConfig::getServer2();
  strcpy((char *)ptr, server2.c_str());
  ptr += strlen(server2.c_str()) + 1;
  FILL_BUFF(byte, 220);
  client.write(buff, ptr - buff);
#ifdef DEBUG_SFT    
  Serial.println("Done.");
#endif
}

void SFT::WriteAppConfig(EthernetClient &client)
{
#ifdef DEBUG_SFT    
  Serial.print("Write App Config: ");
#endif
  AppConfig::setAutoRecovery(true);
  AppConfig::setConnectionTestPeriod(15);
  IPAddress lanAddress(192,168,0,10);
  AppConfig::setLANAddr(lanAddress);
  AppConfig::setLimitCycles(true);
  AppConfig::setMaxHistory(10);
  AppConfig::setMDisconnect(5);
  AppConfig::setMReconnect(210);
  AppConfig::setRDisconnect(5);
  AppConfig::setRecoveryCycles(5);
  AppConfig::setRReconnect(180);
  String server1("google.com");
  AppConfig::setServer1(server1);
  String server2("yahoo.com");
  AppConfig::setServer2(server2);
  client.write(220);
#ifdef DEBUG_SFT    
  Serial.println("Done.");
#endif
}

// Initialize the server
void SFT::Init()
{
  // Start the server object to listen for requests from the client
  server.begin();

  // Set current path to the root directory
  memset(curPath, 0, sizeof(curPath));
  curPath[0] = '\\';
  vol.init(card);
  curDir.openRoot(vol);
#ifdef DEBUG_SFT    
  Serial.println("SFT Initialized");
#endif
}

// See if the clien sent a request and handle it
void SFT::DoService()
{
    EthernetClient client = server.available();
    if (!client)
    {
        return;
    }
#ifdef DEBUG_SFT    
    Serial.print("New client on socket ");
    Serial.println(client.getSocketNumber());
#endif
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

          case 'a':
            ReadAppConfig(client);
            break;

          case 'A':
            WriteAppConfig(client);
            break;
        }
        break;
    }
    // Close the connection:
    client.stop();
#ifdef DEBUG_SFT    
    Serial.println("Client disconnected");
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