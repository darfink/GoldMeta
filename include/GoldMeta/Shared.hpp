#pragma once

#include <boost/serialization/strong_typedef.hpp>

namespace gm {
    /// <summary>
    /// Describes the values a plugin routine can set
    /// </summary>
    // TODO: Make C++11 optional, otherwise just use normal e-numerations
    enum class Result {
        Unset,     /* This is just the default value for a meta result */
        Ignored,   /* The plugin did nothing. This could be used to tell a subsequent plugin that the some situation hasn't been handled yet. This would be recognized only by other plugins; Metamod itself doesn't do anything special for this situation. Unless otherwise specified by a later plugin, execution of routine in the gameDLL will take place. This is valid in both normal and post routines. */
        Handled,   /* The plugin handled the situation, or did something with the information. Again, this could be used to tell a subsequent plugin that some situation has already been taken care of, and is not recognized specially by Metamod. Unless otherwise specified by a later plugin, execution of routine in the gameDLL will take place. This is valid in both normal and post routines. */
        Override,  /* The plugin is providing a return value for the routine, which should be used in place of the return value from the gameDLL's routine (the plugin "overrides" the gameDLL's return value). Unless otherwise specified by a later plugin, the gameDLL routine will still be called. Note this only makes sense for non-void routines. This is valid in both normal and post routines. */
        Supersede, /* The plugin has performed sufficient actions for the routine, and the gameDLL's routine should not be called (the plugin "supersedes" the gameDLL's routine, more or less replacing it entirely). Any return value for the routine should be specified as well by the plugin. Note this is only valid for pre routines, as post routines cannot prevent calling the gameDLL's routine (as it has already happened!). Also note, this doesn't prevent subsequent plugins from being called for this routine; it supersedes only the gameDLL. */
    };

    /// <summary>
    /// Describes the game information a plugin can query for
    /// </summary>
    enum class GameInfo {
        Name,
        Description,
        Directory,
        LibraryPath,
        LibraryFilename,
    };

    /// <summary>
    /// The unique identifier for any plugin
    /// </summary>
    // TODO: Plugins should not be required to use boost
    BOOST_STRONG_TYPEDEF(unsigned int, PluginId)

    // This function does not use upper camel case to match the other C++ casts
    template<typename Target, typename Source>
    inline Target brute_cast(const Source s) {
        static_assert(sizeof(Target) == sizeof(Source), "The size of the both types must match");
        union { Target t; Source s; } u;

        u.s = s;
        return u.t;
    }
}
