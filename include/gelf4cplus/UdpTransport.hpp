/* 
 * File:   UdpTransport.hpp
 * Author: Steven Bidny
 *
 * Created on May 22, 2012, 12:57 PM
 */

#if !defined(UDPTRANSPORT_HPP)
#define UDPTRANSPORT_HPP

/*- HEADER FILES -------------------------------------------------------------*/

// System Headers

#include <string>
#include <cstdlib>
#include <sstream>
#include <stdint.h>

// Third-party Headers

#define BOOST_SYSTEM_NO_LIB
#include <boost/functional/hash.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/interprocess/detail/os_thread_functions.hpp>

// Other Headers

#include "ITransport.hpp"

/*- NAMESPACES ---------------------------------------------------------------*/

namespace gelf4cplus
{
namespace transport
{

using std::string;

/*- CONSTANTS ----------------------------------------------------------------*/

const uint16_t DISABLE_CHUNKING = 0; ///< Constant used to disable chunking.
const uint16_t DEFAULT_CHUNK_SIZE = 1024; ///< The default size of chunks.
const int DEFAULT_GRAYLOG2_PORT = 12201; ///< The default Graylog2 port.
const string DEFAULT_GRAYLOG2_HOST = "localhost"; ///< The default Graylog2 host.

/*- CLASSES ------------------------------------------------------------------*/

/**
 * This class defines a UDP transport for use with the GELF appender.
 */
class UdpTransport : public ITransport
{
public:

    // Constructors & Destructor

    /**
     * The default constructor.
     * @param aDstHost A destination host name.
     * @param aDstPort A destination port.
     * @param aMaxChunkSize The maximum size of each chunk.
     */
    UdpTransport(const string &aDstHost = "localhost",
                 const int &aDstPort = DEFAULT_GRAYLOG2_PORT,
                 const uint16_t &aMaxChunkSize = DEFAULT_CHUNK_SIZE) :
                 m_maxChunkSize(aMaxChunkSize)
    {
        // Build the id string using the IP, PID, and TID
        std::ostringstream ss;
        ss << boost::asio::ip::host_name() <<
                boost::interprocess::ipcdetail::get_current_process_id() <<
                boost::interprocess::ipcdetail::get_current_thread_id();
        m_threadId = ss.str();

        // Set up the Boost Asio stuff
        boost::asio::ip::udp::resolver resolver(m_service);
        boost::asio::ip::udp::resolver::query query(boost::asio::ip::udp::v4(),
                                                    aDstHost,
                                                    boost::lexical_cast<string>(aDstPort));
        m_endpoint = *resolver.resolve(query);
        m_socket = new boost::asio::ip::udp::socket(m_service, m_endpoint.protocol());
    }

    /**
     * A virtual destructor in case someone wants to derive from this class.
     */
    virtual ~UdpTransport()
    {
        delete m_socket;
        m_socket = NULL;
    }

    // Methods

    /**
     * Gets the maximum chunk size.
     * @return The maximum chunk size.
     */
    virtual uint16_t maxChunkSize()
    {
        return m_maxChunkSize;
    }

    /**
     * Sets the maximum chunk size.
     * @param aValue The new maximum chunk size.
     */
    virtual void maxChunkSize(const uint16_t &aValue)
    {
        m_maxChunkSize = aValue;
    }

    /**
     * Sends a message using this transport.
     * @param aMessage The message to send.
     */
    virtual void send(const string &aMessage)
    {
        size_t length = aMessage.length();

        if (m_maxChunkSize != DISABLE_CHUNKING &&
                length > m_maxChunkSize)
        {
            size_t chunkCount = (length / m_maxChunkSize) + 1;
            string messageId;
            generateMessageId(messageId);

            for (size_t i = 0; i < chunkCount; ++i)
            {
                string messageChunkPrefix;
                createChunkedMessagePart(messageId, i, chunkCount, messageChunkPrefix);
                size_t skip = i * m_maxChunkSize;

                // Send the message chunk to the UDP endpoint
                m_socket->async_send_to(boost::asio::buffer(messageChunkPrefix + aMessage.substr(skip, m_maxChunkSize)),
                                        m_endpoint, boost::bind(&UdpTransport::handler, this));
            }
        }
        else
        {
            // Send the message to the UDP endpoint
            m_socket->async_send_to(boost::asio::buffer(aMessage), m_endpoint, boost::bind(&UdpTransport::handler, this));
        }
    }

protected:

    // Constant Static Members

    const static uint8_t MAX_HEADER_SIZE = 8; ///< Maximum message ID size.

    // Members

    uint16_t m_maxChunkSize; ///< The maximum chunk size.
    boost::asio::ip::udp::endpoint m_endpoint; ///< The Boost endpoint.
    boost::asio::ip::udp::socket *m_socket; ///< The Boost socket.
    boost::asio::io_service m_service; ///< The Boost IO service.
    string m_threadId; ///< The thread ID.

    // Methods

    /**
     * Creates the prefix for the specific chunk.
     * @param aMessageId The unique ID of this message.
     * @param anIndex This chunk index.
     * @param aChunkCount The total chunk count.
     * @param aResult The resultant chunk.
     */
    virtual void createChunkedMessagePart(const string &aMessageId,
                                          const size_t &anIndex,
                                          const size_t &aChunkCount,
                                          string &aResult)
    {
        // Chunked GELF ID: 0x1e 0x0f (identifying this message as a chunked GELF message)
        aResult.push_back(0x1e);
        aResult.push_back(0x0f);

        // Message ID: 8 bytes 
        aResult += aMessageId;

        // Sequence Number: 1 byte (The sequence number of this chunk)
        aResult.push_back((char) anIndex);

        // Total Number: 1 byte (How many chunks does this message consist of in total)
        aResult.push_back((char) aChunkCount);
    }

    /**
     * Generates a unique 8-byte message ID by hashing the host name, process
     * ID, thread ID, and time.
     * @param aMessageId The resultant message ID
     */
    virtual void generateMessageId(string &aMessageId)
    {
        // Create the "unique" string using the host name and current time
        std::ostringstream ss;
        ss << m_threadId << boost::posix_time::microsec_clock::universal_time();

        // Create a hash of the "unique" string using Boost
        size_t hash = boost::hash<string>()(ss.str());

        // Return a byte array of the hash using std::string
        aMessageId.assign((char*) &hash, MAX_HEADER_SIZE);
    }

    /**
     * Dummy handler for the Boost async_send_to()
     */
    virtual void handler()
    {
    }
};

} // namespace transport
} // namespace gelf4cplus

#endif // #if !defined(UDPTRANSPORT_HPP)
