#ifndef FileView_h
#define FileView_h

#include <View.h>

class FileView : public View
{
public:
    FileView(const char *viewPath, const char *viewFilePath) :
        View(viewPath, mapper(viewFilePath))
    {
    }

private:
    const String mapper(const String _filePath)
    {
        String filePath(_filePath);
        int fileIndex = filePath.indexOf("GLYPHICONS-HALFLINGS-REGULAR");
        if (fileIndex != -1)
        {
            int dotIndex = filePath.lastIndexOf('.');
            String ext = filePath.substring(dotIndex + 1);
            if (ext.equals("WOFF"))
                ext = "WOF";
            else if (ext.equals("WOFF2"))
                ext = "WF2";
            filePath = filePath.substring(0, fileIndex) + "GHR." + ext;
        }
        return filePath;
    }
};

#endif // FileView_h