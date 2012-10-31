/* 
 * File:   GelfMessage.h
 * Author: Steven Bidny
 *
 * Created on May 22, 2012, 12:57 PM
 */

#if !defined(GELF4CPLUSAPPENDER_HPP)
#define GELF4CPLUSAPPENDER_HPP

/*- HEADER FILES -------------------------------------------------------------*/

// System Header Files

#include <string>
#include <vector>
#include <stdint.h>

// Third-party Header Files

#include <log4cplus/appender.h>
#include <log4cplus/syslogappender.h>
#include <log4cplus/configurator.h>
#include <log4cplus/helpers/stringhelper.h>
#include <log4cplus/helpers/property.h>
#include <log4cplus/tstring.h>
#include <boost/algorithm/string.hpp>
#include <boost/unordered_map.hpp>
#include <boost/asio.hpp>
#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>

// Other Header Files

#include "ITransport.hpp"
#include "GelfMessage.hpp"

/*- NAMESPACES ---------------------------------------------------------------*/

namespace gelf4cplus
{
namespace appender
{

using transport::ITransport;
using log4cplus::tstring;
using log4cplus::helpers::Properties;
using std::string;

/*- CLASSES ------------------------------------------------------------------*/

/**
 * Dummy class to give public access to the getSysLogLevel() method
 */
class SysLogLevel : public log4cplus::SysLogAppender
{
public:

    // Constructors & Destructor

    /**
     * The default constructor.
     */
    SysLogLevel() : log4cplus::SysLogAppender("SysLogLevel")
    {
    }

    /**
     * We want to make getSysLogLevel() public.
     */
    using log4cplus::SysLogAppender::getSysLogLevel;
};

/**
 * Dummy constant to give static access to getSysLogLevel() method
 */
const SysLogLevel SYSLOG_LEVEL;

/**
 * This class defines the GELF appender, which creates GELF messages and sends
 * them using the specified transport.
 */

class Gelf4CPlusAppender : public log4cplus::Appender
{
public:

    // Type Definitions

    typedef boost::unordered_map<string, string> Dictionary;

    // Constructors and Destructor

    /**
     * The default constructor.
     * @param aTransport A transport to use when sending GELF messages.
     * @param properties Some properties to use when constructing this object.
     */
    Gelf4CPlusAppender(ITransport *aTransport = NULL,
                       const Properties &properties = Properties()) :
                       m_transport(aTransport)
    {
        // Try to get the host name
        try
        {
            m_loggingHostName = boost::asio::ip::host_name();
        }
        catch (...)
        {
            m_loggingHostName = properties.getProperty("loggingHostName",
                                                       message::UNKNOWN_HOST);
        }

        // Get the facility property
        m_facility = properties.getProperty("facility", "");

        // Get the includeLocationInformation property
        tstring includeLocationInformation =
                properties.getProperty("includeLocationInformation", "false");

        // Parse the includeLocationInformation property
        m_includeLocationInformation =
                log4cplus::helpers::toLower(includeLocationInformation)[0] == 't';
 
        // Get the subset of additional field properties
        Properties additionalFields = properties.getPropertySubset("additionalField.");

        // Get the list of property names for the additional fields
        std::vector<tstring> propertyNames = additionalFields.propertyNames();

        // For each property name...
        BOOST_FOREACH(tstring propertyName, propertyNames)
        {
            // Add an additional field
            additionalField(propertyName, additionalFields.getProperty(propertyName));
        }
    }

    /**
     * A virtual destructor in case someone wants to derive from this class.
     */
    virtual ~Gelf4CPlusAppender()
    {
        close();
    }

    // Methods

    /**
     * Gets a const reference to the additional fields dictionary.
     * @return 
     */
    virtual const Dictionary& additionalFields() const
    {
        return m_additionalFields;
    }

    /**
     * Parses a string of comma separated key:value pairs and adds them in the
     * additional fields dictionary.
     * @param aValue
     * @return 
     */
    virtual bool additionalFields(const string &aValue)
    {
        // Tokenize the string into individual fields
        boost::tokenizer< boost::char_separator<char> > fields(aValue,
                                                               boost::char_separator<char>(","));

        // For each field...

        BOOST_FOREACH(string field, fields)
        {
            // ...split the field into a key and value
            std::vector<string> keyValue(2);
            boost::algorithm::split(keyValue, field, boost::is_any_of(":"));

            // ...make sure we have exactly a key and value
            if (keyValue.size() != 2)
            {
                return false;
            }

            // ...trim any whitespace
            boost::algorithm::trim(keyValue[0]);
            boost::algorithm::trim(keyValue[1]);

            // ...add the field to the map
            m_additionalFields[keyValue[0]] = keyValue[1];
        }

        return true;
    }

    /**
     * Clears the additional fields dictionary.
     */
    virtual void clearAdditionalFields()
    {
        m_additionalFields.clear();
    }

    /**
     * Adds or changes a single additional field in the dictionary.
     * @param aKey A key for the additional field.
     * @param aValue A value for the additional field.
     */
    virtual void additionalField(const string &aKey, const string &aValue)
    {
        m_additionalFields[aKey] = aValue;
    }

    /**
     * Should we include file and line information?
     * @return True if including file and line info, false if not.
     */
    virtual bool includeLocationInformation() const
    {
        return m_includeLocationInformation;
    }

    /**
     * Set the boolean to include or exclude file and line information.
     * @param aValue True if including file and line info, false if not.
     */
    virtual void includeLocationInformation(const bool &aValue)
    {
        m_includeLocationInformation = aValue;
    }

    /**
     * Closes this appender.
     */
    virtual void close()
    {
        m_transport.reset();
    }

    /**
     * Sets the transport to a new transport.
     * @param aValue The new transport value.
     */
    virtual void transport(ITransport *aValue)
    {
        // Close the existing transport
        close();

        // Set the new transport
        m_transport.reset(aValue);
    }

    /**
     * Is this instance valid?
     * @return True if valid, false if not.
     */
    virtual bool isValid() const
    {
        return m_transport != 0;
    }

protected:

    // Attributes

    boost::shared_ptr<ITransport> m_transport; ///< Shared pointer to transport.
    string m_loggingHostName; ///< Name of this host.
    string m_facility; ///< Facility for this appender.
    bool m_includeLocationInformation; ///< Should we include file and line?
    Dictionary m_additionalFields; ///< Dictionary of additional fields.

    // Methods

    /**
     * The overridden method for appending.
     * @param anEvent The logging event to append.
     */
    virtual void append(const log4cplus::spi::InternalLoggingEvent &anEvent)
    {
        // Can't append if not valid
        if (!isValid())
        {
            return;
        }

        // Get the compressed JSON
        string gelfJsonString;
        createGelfJsonFromLoggingEvent(anEvent, gelfJsonString);

        // Send the message using the transport
        m_transport->send(gelfJsonString);
    }

    /**
     * Creates the JSON String for a given logging event.
     * The short message of the GELF message is a maximum of 250 chars long.
     * Message building and skipping of additional fields etc is based on
     * https://github.com/Graylog2/graylog2-docs/wiki/GELF from May 21, 2012.
     * @param anEvent The logging event to base the JSON creation on.
     * @param anEvent GELF message as compressed JSON.
     */
    virtual void createGelfJsonFromLoggingEvent(const log4cplus::spi::InternalLoggingEvent &anEvent,
                                                string &aGelfJsonString) const
    {
        // Get the full message
        tstring fullMessage = anEvent.getMessage();

        const log4cplus::helpers::Time &time = anEvent.getTimestamp();

        // Create the basic GELF message
        message::GelfMessage gelfMessage(fullMessage.substr(0, message::SHORT_MESSAGE_LENGTH - 1),
                                         m_loggingHostName,
                                         time.sec() + (time.usec() / 1000000.0),
                                         fullMessage,
                                         SYSLOG_LEVEL.getSysLogLevel(anEvent.getLogLevel()),
                                         m_facility.empty() ? anEvent.getLoggerName() : m_facility);

        // Only include location information if configured
        if (m_includeLocationInformation)
        {
            gelfMessage.file(anEvent.getFile());
            gelfMessage.line(anEvent.getLine());
        }

        // Add additional fields
        BOOST_FOREACH(Dictionary::value_type field, m_additionalFields)
        {
            gelfMessage[field.first] = field.second;
        }

        // Add the event type
        gelfMessage["type"] = (int64_t) anEvent.getType();

        // Add the thread
        gelfMessage["thread"] = anEvent.getThread();

        // Add the logger name
        gelfMessage["logger_name"] = anEvent.getLoggerName();

        // Add NDC properties
        tstring ndc = anEvent.getNDC();

        if (!ndc.empty())
        {
            gelfMessage["ndc"] = ndc;
        }

        // Serialize the message
        gelfMessage.serialize(aGelfJsonString);
    }
};

} // namespace appender
} // namespace gelf4cplus

#endif // if !defined(GELF4CPLUSAPPENDER_HPP)
