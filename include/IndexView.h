#ifndef IndexView_h
#define IndexView_h

#include <HtmlFillerView.h>
#include <atomic>

class IndexView : public HtmlFillerView
{
public:
    IndexView();     
    bool redirect(EthClient &client, const String &_id);
    bool isSingleton() { return false; }
    static HttpController *getInstance() { return new IndexView(); }

protected:
    static int getFillers(const ViewFiller *&fillers);

private:
    static ViewFiller fillers[];

private:
    static std::atomic<int> id;
};
#endif // IndexView_h