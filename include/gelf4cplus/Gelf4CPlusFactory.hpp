/* 
 * File:   Gelf4CPlusFactory.hpp
 * Author: sbidny
 *
 * Created on May 23, 2012, 4:32 PM
 */

#if !defined(GELF4CPLUSFACTORY_HPP)
#define	GELF4CPLUSFACTORY_HPP

/*- HEADER FILES -------------------------------------------------------------*/

// Third-party Header Files
#include <log4cplus/spi/appenderattachable.h>

// Other Header Files

#include "Gelf4CPlusAppender.hpp"

/*- NAMESPACES ---------------------------------------------------------------*/

namespace gelf4cplus
{
namespace appender
{

/**
 * A simple factory to create a UDP GELF appender.
 * @param aDstHost A destination host name.
 * @param aDstPort A destination port.
 * @param aMaxChunkSize A maximum size before we chunk the message into parts.
 * @param includeLocationInformation Should we include file and line info?
 * @return A shared pointer to the new UDP GELF appender.
 */
log4cplus::SharedAppenderPtr createUdpAppender(const std::string &aDstHost = "",
                                               const int &aDstPort = transport::DEFAULT_GRAYLOG2_PORT,
                                               const int &aMaxChunkSize = transport::DEFAULT_CHUNK_SIZE,
                                               const bool &includeLocationInformation = INCLUDE_LOCATION_DEFAULT)
{
    return log4cplus::SharedAppenderPtr(new Gelf4CPlusAppender<transport::UdpTransport>(
                                        new transport::UdpTransport(aDstHost, aDstPort, aMaxChunkSize)));
}

} // namespace appender
} // namespace gelf4cplus

#endif                                  // #if !defined(GELF4CPLUSFACTORY_HPP)
