#ifndef FileView_h
#define FileView_h

#include <View.h>
#include <FileViewReader.h>

class FileView : public View
{
public:
    FileView(const String viewFilePath) : View(new FileViewReader(viewFilePath))
    {
    }
};
#endif // FileView_h