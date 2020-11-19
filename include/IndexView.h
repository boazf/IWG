#ifndef IndexView_h
#define IndexView_h

#include <View.h>

class IndexView : public View
{
public:
    IndexView(const char *_viewName, const char *_viewFile);     
    bool redirect(EthernetClient &client, const String &_id);

private:
    bool DoFill(int nFill, String &fill);
    static int id;
};

extern IndexView indexView;

#endif // IndexView_h