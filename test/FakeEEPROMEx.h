#ifndef FakeEEPROMEx_h
#define FakeEEPROMEx_h

#include <Arduino.h>

/// @brief Fake EEPROM class for testing purposes.
/// This class simulates the EEPROM functionality for unit tests.
/// It replaces the read/write functionality of the standard EEPROM fake class.
/// The standard EEPROM fake class doesn't actually reads or writes data, it just counts the number of calls to read/write.
/// This class provides a buffer to store data and allows reading/writing to it.
class EEPROMClassEx : public EEPROMClass
{
public:
    /// @brief Reads a byte from the buffer
    /// @param idx The index to read from the buffer
    /// @return The byte read from the buffer at the specified index.
    uint8_t read(int idx) override
    {
        if (idx < 0 || idx >= length())
        {
            return 0; // Return 0 if index is out of bounds
        }   
        return buffer[idx];
    }

    /// @brief Writes a byte to the buffer
    /// @param idx The index to write to the buffer
    /// @param val The byte value to write to the buffer at the specified index
    /// This function does not perform any actual EEPROM write operation.
    void write(int idx, uint8_t val) override
    {
        if (idx >= 0 && idx < length())
        {
            buffer[idx] = val;
        }
    }

    /// @brief Reads a value of type T from the buffer at the specified index.
    /// This function reads sizeof(T) bytes from the buffer starting at the specified index and stores
    /// @tparam T The type of the value to read from the buffer.
    /// @param idx The index to read from the buffer.
    /// @param t A reference to a variable of type T where the read value will be stored.
    /// @return A reference to the variable of type T containing the read value.
    /// If the index is out of bounds, it returns the unmodified variable.
    template <typename T>
    T &get(int idx, T &t)
    {
        if (idx < 0 || idx + sizeof(T) > length())
        {
            return t; // Return unmodified t if index is out of bounds
        }

        // Read sizeof(T) bytes from the buffer starting at idx
        // and store them in the variable t.
        uint8_t *buff = reinterpret_cast<uint8_t *>(&t);
        for (size_t i = 0; i < sizeof(T); i++)
        {
            buff[i] = read(idx + i);
        }

        return t;
    }

    /// @brief Writes a value of type T to the buffer at the specified index.
    /// This function writes sizeof(T) bytes to the buffer starting at the specified index.
    /// @tparam T The type of the value to write to the buffer.
    /// @param idx The index to write to the buffer.
    /// @param t A reference to a variable of type T containing the value to write.
    /// @return A reference to the variable of type T containing the written value.
    /// If the index is out of bounds, it returns the unmodified variable.  
    /// @note This function does not perform any actual EEPROM write operation.
    template <typename T>
    const T &put(int idx, const T &t)
    {
        if (idx < 0 || idx + sizeof(T) > length())
        {
            return t; // Return unmodified t if index is out of bounds
        }
        const uint8_t *buff = reinterpret_cast<const uint8_t *>(&t);
        for (size_t i = 0; i < sizeof(T); i++)
        {
            write(idx + i, buff[i]);
        }
        return t;
    }

    void commit()
    {
        // In the context of tests, we don't need to commit changes to EEPROM.
        // This is a no-op to avoid unnecessary writes during tests.
    }

    /// @brief Returns the length of the buffer.
    /// @return The size of the buffer in bytes that is used to simulate the EEPROM.
    uint16_t length()
    {
        return buffLen;
    }

    /// @brief Clears the buffer by setting all bytes to zero.
    /// This function resets the buffer to its initial state, effectively simulating a fresh EEPROM.
    void clear()
    {
        memset(buffer, -1, sizeof(buffer));
    }

    void begin(int size = 4096)
    {
        // Initialize the buffer with the specified size.
        // The default size is 4096 bytes, which is the maximum size of the buffer.
        if (size > sizeof(buffer))
        {
            size = sizeof(buffer);
        }
        buffLen = size;
    }
private:
    uint16_t buffLen = sizeof(buffer); ///< Length of the buffer.
    /// @brief Buffer to simulate EEPROM storage.
    uint8_t buffer[4096];
};

#define EEPROM EEPROMEx

/// @brief Global instance of the EEPROMClassEx.
extern EEPROMClassEx EEPROMEx;

#endif // FakeEEPROMEx_h