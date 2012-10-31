/* 
 * File:   GelfMessage.hpp
 * Author: Steven Bidny
 *
 * Created on May 22, 2012, 12:57 PM
 */

#if !defined(GELFMESSAGE_HPP)
#define	GELFMESSAGE_HPP

/*- HEADER FILES -------------------------------------------------------------*/

// System Header Files

#include <string>
#include <exception>
#include <stdint.h>
#include <climits>
#include <sstream>

// Third-party Header Files

#define BOOST_IOSTREAM_NO_LIB

#include "json_spirit/json_spirit_writer_template.h"
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>

/*- NAMESPACES ---------------------------------------------------------------*/

namespace gelf4cplus
{
namespace message
{

using std::string;

/*- CONSTANTS ----------------------------------------------------------------*/

const int NO_LINE = -1;
const string DEFAULT_FACILITY = "";
const double USE_SERVER_TIMESTAMP = -1.0;
const string GELF_VERSION = "1.0";
const string UNKNOWN_HOST = "unknown_host";
const string DEFAULT_SHORT_MESSAGE = "empty";
const size_t SHORT_MESSAGE_LENGTH = 250;

// Fields

const string VERSION = "version";
const string HOST = "host";
const string SHORT_MESSAGE = "short_message";
const string TIMESTAMP = "timestamp";
const string FULL_MESSAGE = "full_message";
const string LEVEL = "level";
const string FACILITY = "facility";
const string FILE = "file";
const string LINE = "line";

/*- CLASSES ------------------------------------------------------------------*/

/**
 * A class representing a GELF message with compressed JSON serialization.
 */
class GelfMessage
{
public:

    // Type Definitions

    typedef json_spirit::mValue Value; ///< Value type
    typedef json_spirit::mObject Object; ///< JSON object type
    typedef json_spirit::Value_type ValueType; ///< Type of value

    // Constructors & Destructor

    /**
     * The constructor.
     * @param aShortMessage A short message.
     * @param aHost The host name of the sender.
     * @param aTimestamp The message timestamp.
     * @param aFullMessage A full message.
     * @param aLevel The severity level.
     * @param aFacility The message facility.
     * @param aFile The filename.
     * @param aLine The line number.
     * @param aVersion The GELF version.
     */
    GelfMessage(const string &aShortMessage = DEFAULT_SHORT_MESSAGE,
                const string &aHost = UNKNOWN_HOST,
                const double &aTimestamp = USE_SERVER_TIMESTAMP,
                const string &aFullMessage = "",
                const uint8_t &aLevel = 1,
                const string &aFacility = DEFAULT_FACILITY,
                const string &aFile = "",
                const int &aLine = NO_LINE,
                const string &aVersion = GELF_VERSION)
    {
        // If any of the sets fails, throw an exception
        if (!version(aVersion) ||
                !host(aHost) ||
                !shortMessage(aShortMessage) ||
                !timestamp(aTimestamp) ||
                !level(aLevel) ||
                !facility(aFacility) ||
                !fullMessage(aFullMessage) ||
                !file(aFile) ||
                !line(aLine))
        {
            throw std::invalid_argument("Bad argument to GelfMessage()");
        }
    }

    /**
     * A virtual destructor in case someone wants to derive from this class.
     */
    virtual ~GelfMessage()
    {
    }

    // Methods

    /**
     * Set the GELF version.
     * @param aVersion The new GELF version.
     * @return True if the new GELF version was set.
     */
    virtual bool version(const string &aVersion)
    {
        // If there is no version, use the default GELF_VERSION
        if (aVersion == "")
        {
            m_object[VERSION] = GELF_VERSION;

            return true;
        }

        m_object[VERSION] = aVersion;

        return true;
    }

    /**
     * Set the host name.
     * @param aHost The new host name.
     * @return True if the new host name was set.
     */
    virtual bool host(const string &aHost)
    {
        // If there is no host, use the default UNKNOWN_HOST
        if (aHost == "")
        {
            m_object[HOST] = UNKNOWN_HOST;

            return true;
        }

        m_object[HOST] = aHost;

        return true;
    }

    /**
     * Set the short message.
     * @param aShortMessage A new short message.
     * @return True if the new short message was set.
     */
    virtual bool shortMessage(const string &aShortMessage)
    {
        // If there is no short message, use the default DEFAULT_MESSAGE
        if (aShortMessage == "")
        {
            m_object[SHORT_MESSAGE] = DEFAULT_SHORT_MESSAGE;

            return true;
        }

        m_object[SHORT_MESSAGE] = aShortMessage;

        return true;
    }

    /**
     * Set the timestamp or use USE_SERVER_TIMESTAMP to remove the timestamp.
     * @param aTimestamp A new timestamp.
     * @return True if the new timestamp was set.
     */
    virtual bool timestamp(const double &aTimestamp)
    {
        // If there is no timestamp, erase this entry
        if (aTimestamp == USE_SERVER_TIMESTAMP)
        {
            m_object.erase(TIMESTAMP);

            return true;
        }

        m_object[TIMESTAMP] = aTimestamp;

        return true;
    }

    /**
     * Set the full message or use an emptry string to remove the full message.
     * @param aFullMessage The new full message.
     * @return True if the new full message was set.
     */
    virtual bool fullMessage(const string &aFullMessage)
    {
        // If there is no full message, erase this entry
        if (aFullMessage == "")
        {
            m_object.erase(FULL_MESSAGE);

            return true;
        }

        m_object[FULL_MESSAGE] = aFullMessage;

        return true;
    }

    /**
     * Set the severity level.
     * @param aLevel The new severity level.
     * @return True if the new severity level was set.
     */
    virtual bool level(const uint8_t &aLevel)
    {
        // The level must be between 0 and 7 inclusive according to syslog
        if (aLevel > 7)
        {
            return false;
        }

        m_object[LEVEL] = aLevel;

        return true;
    }

    /**
     * Set the facility or use an empty string to set the default facility.
     * @param aFacility The new facility.
     * @return True if the new facility was set.
     */
    virtual bool facility(const string &aFacility)
    {
        // If there is no facility, set it to the default value
        if (aFacility == "")
        {
            m_object[FACILITY] = DEFAULT_FACILITY;

            return true;
        }

        m_object[FACILITY] = aFacility;

        return true;
    }

    /**
     * Set the filename or use an empty string to remove the filename.
     * @param aFile The new filename.
     * @return True if the new filename was set.
     */
    virtual bool file(const string &aFile)
    {
        // If there is no file, erase this entry
        if (aFile == "")
        {
            m_object.erase(FILE);

            return true;
        }

        m_object[FILE] = aFile;

        return true;
    }

    /**
     * Set the line number or use NO_LINE to remove the line number.
     * @param aLine The new line number.
     * @return True if the new line number was set.
     */
    virtual bool line(const int &aLine)
    {
        if (aLine == NO_LINE)
        {
            // If there is no line, erase this entry
            m_object.erase(LINE);

            return true;
        }
        else if (aLine < 0)
        {
            // Line must be positive
            return false;
        }

        m_object[LINE] = aLine;

        return true;
    }

    /**
     * Return the GELF version.
     * @return The GELF version.
     */
    virtual string version()
    {
        return m_object[VERSION].get_str();
    }

    /**
     * Return the host name.
     * @return The host name.
     */
    virtual string host()
    {
        return m_object[HOST].get_str();
    }

    /**
     * Return the short message.
     * @return The short message.
     */
    virtual string shortMessage()
    {
        return m_object[SHORT_MESSAGE].get_str();
    }

    /**
     * Return the timestamp as <seconds since epoch>.<microseconds>.
     * @return The timestamp as <seconds since epoch>.<microseconds>
     */
    virtual double timestamp()
    {
        return m_object[TIMESTAMP].get_real();
    }

    /**
     * Return the full message.
     * @return The full message.
     */
    virtual string fullMessage()
    {
        return m_object[FULL_MESSAGE].get_str();
    }

    /**
     * Return the severity level.
     * @return The severity level.
     */
    virtual uint8_t level()
    {
        return (uint8_t) m_object[LEVEL].get_int();
    }

    /**
     * Return the facility.
     * @return The facility.
     */
    virtual string facility()
    {
        return m_object[FACILITY].get_str();
    }

    /**
     * Return the filename.
     * @return The filename.
     */
    virtual string file()
    {
        return m_object[FILE].get_str();
    }

    /**
     * Return the line number.
     * @return The line number.
     */
    virtual int line()
    {
        return m_object[LINE].get_int();
    }

    /**
     * Serialize this object using JSON and compression.
     * @param aSerializedString The output of the serialization of this object.
     */
    virtual void serialize(string &aSerializedString) const
    {
        // Get the JSON, compress it, and set it to the output buffer
        compress(json_spirit::write_string((Value) m_object, json_spirit::remove_trailing_zeros),
                 aSerializedString);
    }

    /**
     * Insert a key/value pair into the object.
     * Will not insert the field if it already exists.
     * @param aKey The key to a field.
     * @param aValue The value of a field.
     * @return A pair with an iterator to the inserted value and a boolean which
     * is false if the field already existed.
     */
    virtual std::pair<Object::iterator, bool> insert(const string &aKey,
                                                     const Value &aValue)
    {
        if (isAllowedKey(aKey))
        {
            return m_object.insert(json_spirit::mConfig::Pair_type(makeKey(aKey),
                                                                   aValue));
        }
        else
        {
            return std::make_pair(m_object.end(), false);
        }
    }

    /**
     * Return a constant reference to the JSON object.
     * @return A constant reference to the JSON object.
     */
    virtual const Object& getObject() const
    {
        return m_object;
    }

    /**
     * Return a reference to the existing value at this key.
     * Throws an exception if the key does not exist.
     * @param aKey A key to a field.
     * @return A reference to the value at this key.
     */
    virtual Value& at(const string &aKey)
    {
        json_spirit::mObject::iterator it = m_object.find(makeKey(aKey));

        // Return the value if the key exists
        if (it != m_object.end())
        {
            return it->second;
        }

        // Throw an exception if the key does not exist
        throw std::out_of_range("Key not found in JSON object");
    }

    /**
     * Return a reference to the existing value or a new value if the key did
     * not exist.
     * @param aKey A key to a field.
     * @return A reference to the value at this key.
     */
    virtual Value& operator [](const string &aKey)
    {
        if (isAllowedKey(aKey))
        {
            return m_object[makeKey(aKey)];
        }
        else
        {
            throw std::invalid_argument("Key is not allowed");
        }
    }

    /**
     * Return the type of field at the specified key.
     * @param aKey A key to a field.
     * @return The type of field at the specified key.
     */
    virtual ValueType type(const string &aKey)
    {
        return at(makeKey(aKey)).type();
    }

    /**
     * Erase the incoming key and its associated value from the object.
     * @param aKey Key to erase.
     * @return True if the key was not required, was found, and was erased.
     */
    virtual bool erase(const string &aKey)
    {
        // Don't delete required fields
        if (isRequiredField(aKey))
        {
            return false;
        }

        // Erase the entry
        return (bool) m_object.erase(makeKey(aKey));
    }

protected:

    // Attributes

    Object m_object; ///< Stores all key value pairs

    //Methods

    /**
     * Compress the input message.
     * @param aMessage The input message.
     * @param aCompressedMessage The compressed output message.
     * @return True if the compression succeeded.
     */
    virtual void compress(const string &aMessage,
                          string &aCompressedMessage) const
    {
        std::istringstream ss(aMessage);
        boost::iostreams::filtering_istream in;
        in.push(boost::iostreams::gzip_compressor());
        in.push(ss);
        std::ostringstream out;
        boost::iostreams::copy(in, out);
        aCompressedMessage = out.str();
    }

    /**
     * Is the field a required field in GELF?
     * @param aKey A key to a field.
     * @return True if the field is a required GELF field.
     */
    virtual bool isRequiredField(const string &aKey)
    {
        if (aKey == VERSION ||
                aKey == HOST ||
                aKey == SHORT_MESSAGE)
        {
            return true;
        }

        return false;
    }

    /**
     * Is the field a standard field in GELF?
     * @param aKey A key to a field.
     * @return True if the field is a standard GELF field.
     */
    virtual bool isStandardField(const string &aKey)
    {
        if (aKey == VERSION ||
                aKey == HOST ||
                aKey == SHORT_MESSAGE ||
                aKey == TIMESTAMP ||
                aKey == FULL_MESSAGE ||
                aKey == LEVEL ||
                aKey == FACILITY ||
                aKey == FILE ||
                aKey == LINE)
        {
            return true;
        }

        return false;
    }

    /**
     * Checks if a key is allowed in the GELF message.
     * @param aKey The key to check.
     * @return True if key is allowed, false if not.
     */
    virtual bool isAllowedKey(const string &aKey)
    {
        string key = makeKey(aKey);

        if (aKey == "_id")
        {
            return false;
        }

        return true;
    }

    /**
     * Prepend '_' to the key name if not a standard field.
     * @param aKey A key to a field.
     * @return The key prepended with '_' if not a standard field.
     */
    virtual string makeKey(const string &aKey)
    {
        string key = ((isStandardField(aKey) || aKey.at(0) == '_') ? aKey : '_' + aKey);

        return key;
    }
};

} // namespace message
} // namespace gelf4cplus

#endif // #if !defined(GELFMESSAGE_HPP)
