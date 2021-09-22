#ifndef StringableEnum_h
#define StringableEnum_h

#include <map>

template <typename T>
class StringableEnum
{
public:
    StringableEnum(T e) : e(e)
    {
    }

    std::string ToString()
    {
        try
        {
            return strMap.at(e);
        }
        catch(std::out_of_range)
        {
            return "Unknown";
        }
    }

private:
    T e;
    static const std::map<T, std::string> strMap;
};

#endif // StringableEnum_h