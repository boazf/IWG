#ifndef DirectFileView_h
#define DirectFileView_h

#include <FileView.h>

class DirectFileView : public FileView
{
public:
    DirectFileView(const char *viewFilePath) :
        FileView(mapper(viewFilePath))
    {
    }
    bool isSingleton() { return false; }

private:
    static const String mapper(const String _filePath)
    {
        String filePath = _filePath;
        filePath.toUpperCase();
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

#endif // DirectFileView_h