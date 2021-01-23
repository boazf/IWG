#include <HttpFillerView.h>

int HttpFillerView::viewHandler(byte *buff, int buffSize)
{
    for(int i = 0; i < buffSize; i++)
    {
        if (buff[i] == (byte)'%')
        {
            int j = i + 1;
            for (; j < buffSize; j++)
                if (buff[j] == (byte)' ')
                    break;
            if (j == buffSize)
                return i;

            int nFill;
            if (sscanf((const char *)buff + i + 1, "%d", &nFill) != 1)
            {
#ifdef DEBUG_HTTP_SERVER
                Traceln("Bad filler index!");
#endif
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

bool HttpFillerView::DoFill(int nFill, String &fill)
{
    const ViewFiller *fillers;
    int nFillers = getFillers(fillers);
    if (nFill >= nFillers)
        return false;
    fillers[nFill](fill);
    return true;
}

int HttpFillerView::read()
{
    memcpy(buff, buff + offset, buffSize - offset);
    offset = buffSize - offset;
    int nBytes = file.read(buff + offset, buffSize - offset) + offset;
    offset = viewHandler(buff, nBytes);
    return offset;
}

