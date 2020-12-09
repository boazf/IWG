#ifndef ESP32
#ifndef w5100ex_h
#define w5100ex_h

#include <Ethernet.h>
#include <utility/w5100.h>


class W5100ClassEx : public W5100Class
{
public:
  /**
   * @brief	 This function is being called by send() and sendto() function also. 
   * 
   * This function read the Tx write pointer register and after copy the data in buffer update the Tx write pointer
   * register. User should read upper byte first and lower byte later to get proper value.
   */
  void send_data_processing(SOCKET s, const uint8_t *data, uint16_t len);
  /**
   * @brief A copy of send_data_processing that uses the provided ptr for the
   *        write offset.  Only needed for the "streaming" UDP API, where
   *        a single UDP packet is built up over a number of calls to
   *        send_data_processing_ptr, because TX_WR doesn't seem to get updated
   *        correctly in those scenarios
   * @param ptr value to use in place of TX_WR.  If 0, then the value is read
   *        in from TX_WR
   * @return New value for ptr, to be used in the next call
   */
// FIXME Update documentation
  void send_data_processing_offset(SOCKET s, uint16_t data_offset, const uint8_t *data, uint16_t len);

  /**
   * @brief	This function is being used for copy the data form Receive buffer of the chip to application buffer.
   * 
   * It calculate the actual physical address where one has to read
   * the data from Receive buffer. Here also take care of the condition while it exceed
   * the Rx memory uper-bound of socket.
   */
  void read_data(SOCKET s, volatile uint16_t src, volatile uint8_t * dst, uint16_t len);

  uint16_t getRXReceivedSize(SOCKET s);

private:
  static const uint16_t RMASK = 0x07FF; // Rx buffer MASK
  static const uint16_t RSIZE = 2048; // Max Rx buffer size
};

extern W5100ClassEx W5100Ex;

#endif // w5100ex_h
#endif // ESP32