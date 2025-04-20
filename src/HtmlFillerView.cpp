#include <HtmlFillerView.h>
#ifdef DEBUG_HTTP_SERVER
#include <Trace.h>
#endif

size_t HtmlFillerView::viewHandler(byte *buff, size_t buffSize)
{
    for(size_t i = 0; i < buffSize; i++)
    {
        if (buff[i] == (byte)'%')
        {
            size_t j = i + 1;
            for (; j < buffSize; j++)
                if (buff[j] == (byte)' ')
                    break;
            if (j == buffSize)
                return i;

            int nFill;
            if (sscanf((const char *)buff + i + 1, "%d", &nFill) != 1)
            {
                continue;
            }

            String fill;
            if (!DoFill(nFill, fill))
            {
#ifdef DEBUG_HTTP_SERVER
                Traceln("Failed to fill view!");
#endif
                continue;
            }
            
            if (i + fill.length() >= buffSize)
                return i;

            for(j = i; buff[j] != (byte)' '; j++)
                buff[j] = (byte)' ';
            for (unsigned int j = 0; j < fill.length(); j++)
            {
                if (buff[i + j] != (char)' ')
                {
#ifdef DEBUG_HTTP_SERVER
                    Traceln("Not enough spaces for filled value!");
#endif
                    break;
                }
                buff[i + j] = fill.c_str()[j];
            }
        }
    }

    return buffSize;
}

bool HtmlFillerView::DoFill(int nFill, String &fill)
{
    const ViewFiller *fillers;
    int nFillers = getFillers(fillers);
    if (nFill >= nFillers)
        return false;
    fillers[nFill](fill);
    return true;
}

bool HtmlFillerView::open(byte *buff, int buffSize)
{
    offset = buffSize;
    return FileView::open(buff, buffSize);
}

int HtmlFillerView::read()
{
    memcpy(buff, buff + offset, buffSize - offset);
    offset = buffSize - offset;
    size_t nBytes = file.read(buff + offset, buffSize - offset) + offset;
    offset = viewHandler(buff, nBytes);
    return offset;
}

