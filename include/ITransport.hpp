#if !defined(TRANSPORT_HPP)
#define TRANSPORT_HPP

/*- HEADER FILES -------------------------------------------------------------*/

// System Headers

#include <string>

/*- NAMESPACES ---------------------------------------------------------------*/

namespace gelf4cplus
{
namespace transport
{

/*- CLASSES ------------------------------------------------------------------*/

/**
 * An interface to create a new transport type.
 */
class ITransport
{
public:

    // Constructors & Destructors

    /**
     * A virtual destructor since this is an interface.
     */
    virtual ~ITransport()
    {
    }

    // Methods

    /**
     * A pure virtual send method to send a message.
     * @param aMessage The message to send.
     */
    virtual void send(const std::string &aMessage) = 0;
};

} // namespace transport
} // namespace gelf4cplus

#endif // #if !defined(TRANSPORT_HPP)
