#include <unity.h>
#include <FakeLock.h>
#include <HtmlFillerViewReaderTests.h>
#include <MemViewReader.h>
#include <HtmlFillerViewReader.h>
#include <HtmlFillerViewReader.cpp>

// Create a mock ViewReader
static const String mem = "This%1is a t%est string% with %0                  and %1                        fillers.%";
static const String expectedContent = "This%1is a t%est string% with Mock Filler1        and Mock Filler2              fillers.%";
// Create a mock GetFillers function
static const GetFillers mockGetFillers = [](const ViewFiller *&fillers) -> int {
    static ViewFiller mockFillers[] =
    { 
        [](String &fill) { fill = "Mock Filler1"; },
        [](String &fill) { fill = "Mock Filler2"; } 
    };
    fillers = mockFillers;
    return NELEMS(mockFillers); // Number of fillers
};

void htmlFillerViewReaderBasicTests()
{
    ViewReader *mockViewReader = new MemViewReader(reinterpret_cast<const byte *>(mem.c_str()), mem.length(), CONTENT_TYPE::HTML);
    HtmlFillerViewReader reader(std::unique_ptr<ViewReader>(mockViewReader), mockGetFillers);

    byte buff[256];
    TEST_ASSERT_MESSAGE(reader.open(buff, sizeof(buff)), "Failed to open HtmlFillerViewReader");

    int bytesRead = reader.read();
    TEST_ASSERT_GREATER_THAN_MESSAGE(0, bytesRead, "Read operation failed or returned zero bytes");
    std::string readContent(reinterpret_cast<const char *>(buff), bytesRead);
    
    TEST_ASSERT_EQUAL_MESSAGE(-1, reader.read(), "Expected read() to return -1 after reading all content");
    TEST_ASSERT_EQUAL_STRING_MESSAGE(
        expectedContent.c_str(), 
        readContent.c_str(),
        "Read content does not match expected content"); 
    String lastModifiedTime;
    TEST_ASSERT_MESSAGE(reader.getLastModifiedTime(lastModifiedTime) == false, "Expected getLastModifiedTime to return false for MemViewReader");
    
    CONTENT_TYPE contentType = reader.getContentType();
    TEST_ASSERT_EQUAL_MESSAGE(CONTENT_TYPE::HTML, contentType, "Expected content type to be HTML");
    
    long viewSize = reader.getViewSize();
    TEST_ASSERT_EQUAL_MESSAGE(mem.length(), viewSize, "Expected view size to match the length of the test string");
    
    reader.close();
}

void HtmlFillerViewReaderTestLoop(const String &mem, const String &expectedContent, HtmlFillerViewReader &reader)
{
    byte buff[256];
    for (int buffLen = 16; buffLen < mem.length() + 16; buffLen++)
    {
        memset(buff, 0, sizeof(buff)); // Clear the buffer
        // Open the reader with the buffer
        TEST_ASSERT_MESSAGE(reader.open(buff, buffLen), "Failed to open HtmlFillerViewReader");
        
        std::string readContent;
        for (int bytesRead = reader.read(); bytesRead != -1; bytesRead = reader.read())
        {
            // Check if the read operation was successful
            TEST_ASSERT_GREATER_THAN_MESSAGE(0, bytesRead, "Read operation failed or returned zero bytes");
            readContent.append(reinterpret_cast<const char *>(buff), bytesRead);
        }
        
        TEST_ASSERT_EQUAL_MESSAGE(-1, reader.read(), "Expected read() to return -1 after reading all content");
        TEST_ASSERT_EQUAL_STRING_MESSAGE(
            expectedContent.c_str(), 
            readContent.c_str(),
            (String("Read content does not match expected content, buffLen: ") + String(buffLen)).c_str()); 
        reader.close();
    }
}

void htmlFillerViewReaderWithVariousBuffLenTests()
{
    ViewReader *mockViewReader = new MemViewReader(reinterpret_cast<const byte *>(mem.c_str()), mem.length(), CONTENT_TYPE::HTML);
    HtmlFillerViewReader reader(std::unique_ptr<ViewReader>(mockViewReader), mockGetFillers);

    HtmlFillerViewReaderTestLoop(mem, expectedContent, reader);
}

void htmlFillerViewReaderWithNonExistingFillerTests()
{
        const String mem = "This is a test string with %2 filler.";
    ViewReader *mockViewReader = new MemViewReader(reinterpret_cast<const byte *>(mem.c_str()), mem.length(), CONTENT_TYPE::HTML);
    HtmlFillerViewReader reader(std::unique_ptr<ViewReader>(mockViewReader), mockGetFillers);

        HtmlFillerViewReaderTestLoop(mem, mem, reader);
}

void HtmlFillerViewReaderWithNotEnoughSpaceForFiller(const String &mem, const String &expectedContent)
{
    // This test checks if the reader handles cases where there is not enough space for the filler
    // The expected behavior is that it should not crash and should fill as much as possible.
    // in .
    
    ViewReader *mockViewReader = new MemViewReader(reinterpret_cast<const byte *>(mem.c_str()), mem.length(), CONTENT_TYPE::HTML);
    HtmlFillerViewReader reader(std::unique_ptr<ViewReader>(mockViewReader), mockGetFillers);

    HtmlFillerViewReaderTestLoop(mem, expectedContent, reader);
}

void htmlFillerViewReaderWithNotEnoughSpaceForFillerTests()
{
    HtmlFillerViewReaderWithNotEnoughSpaceForFiller(
        "This is a test string with %0 filler.",
        "This is a test string with Mocfiller.");
    HtmlFillerViewReaderWithNotEnoughSpaceForFiller(
        "This is a test string with %0        ", 
        "This is a test string with Mock Fille");
    HtmlFillerViewReaderWithNotEnoughSpaceForFiller(
        "This is a test string with %0 filler, but without enough space.", 
        "This is a test string with Mocfiller, but without enough space.");
}
