#pragma once

#include <exception>
#include <string>

// A shorthand define to create class specific exceptions
#define GM_DEFINE_EXCEPTION(name) GM_DEFINE_EXCEPTION_INHERIT(name, GoldMetaException)

#define GM_DEFINE_EXCEPTION_INHERIT(name, inherit)           \
    class name : public inherit {                            \
    public:                                                  \
        name(std::string exception) : inherit(exception) {}  \
        name(const format& format) : inherit(str(format)) {} \
        ~name() throw() {}                                   \
    };

namespace gm {
    class GoldMetaException : public std::exception {
    public:
        /// <summary>
        /// Destructor for the gold meta exception object
        /// </summary>
        ~GoldMetaException() throw() { }

        /// <summary>
        /// Gets the exception message
        /// </summary>
        const char* what() const throw() {
            return mException.c_str();
        }

    protected:
        /// <summary>
        /// Constructs a gold meta exception
        /// </summary>
        GoldMetaException(std::string exception) :
            mException(exception)
        {
        }

    private:
        // Private members
        std::string mException;
    };
}
