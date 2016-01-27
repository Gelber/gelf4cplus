/* 
 * File:   Gelf4CPlusAppenderFactory.hpp
 * Author: Steven Bidny
 *
 * Created on May 22, 2012, 12:57 PM
 */

#if !defined(GELF4CPLUSAPPENDERFACTORY_HPP)
#define	GELF4CPLUSAPPENDERFACTORY_HPP

/*- HEADER FILES -------------------------------------------------------------*/

// Third-party Header Files

#include <log4cplus/spi/appenderattachable.h>
#include <log4cplus/spi/factory.h>
#include <boost/lexical_cast.hpp>

// Other Header Files

#include "Gelf4CPlusAppender.hpp"
#include "UdpTransport.hpp"

/*- NAMESPACES ---------------------------------------------------------------*/

namespace gelf4cplus
{
namespace appender
{

using log4cplus::tstring;
using log4cplus::helpers::Properties;
using boost::lexical_cast;

/*- CLASSES ------------------------------------------------------------------*/

class Gelf4CPlusAppenderFactory : public log4cplus::spi::AppenderFactory
{
public:

    // Constructors & Destructor

    /**
     * A virtual destructor in case someone wants to derive from this class.
     */
    virtual ~Gelf4CPlusAppenderFactory()
    { 
    }

    // Methods

    log4cplus::SharedAppenderPtr createObject(const Properties &properties)
    {
        // Currently unused
        tstring transport = properties.getProperty("transport", "UDP");

        // Get the subset of UDP properties
        Properties udpProperties = properties.getPropertySubset("udp.");

        // Get the UDP host
        tstring host = udpProperties.getProperty("host",
                                                 transport::DEFAULT_GRAYLOG2_HOST);

        // Get the UDP port
        int port = lexical_cast<int>(
                udpProperties.getProperty("port", lexical_cast<std::string>(transport::DEFAULT_GRAYLOG2_PORT)));

        // Create and return the appender
        return log4cplus::SharedAppenderPtr(
                new gelf4cplus::appender::Gelf4CPlusAppender(new transport::UdpTransport(host, port), properties));
    }

    virtual log4cplus::tstring const & getTypeName() const
    {
    	static tstring typeName = "log4cplus::Gelf4CPlusAppender";
        return typeName;
    }

};

} // namespace appender
} // namespace gelf4cplus

#endif                                  // #if !defined(GELF4CPLUSAPPENDERFACTORY_HPP)
