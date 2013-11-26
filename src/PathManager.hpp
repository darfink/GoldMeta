#pragma once

#include <boost/filesystem.hpp>
#include <unordered_map>
#include <string>

#include "Exception.hpp"

namespace /* Anonymous */ {
    namespace fs = boost::filesystem;
}

namespace gm {
    class PathManager {
    public:
        /// <summary>
        /// Describes all paths available
        /// </summary>
        enum Path {
            ServerRoot,    /* Same as the current working directory */
            GameRoot,      /* The root of the game directory (e.g 'hlds/cstrike') */
            GameLibraries, /* The libraries folder (e.g 'hlds/cstrike/dlls') */
            AddonsRoot,    /* The addons folder (e.g 'hlds/cstrike/addons') */
            GoldMetaRoot,  /* The GoldMeta root folder */
            GoldMetaLogs,  /* The logs folder in GoldMeta */
            GoldMetaData,  /* The data folder in GoldMeta */
        };

        /// <summary>
        /// Describes different path types
        /// </summary>
        enum Type {
            Absolute, /* Returns the absolute path (i.e from the disk drive and deeper) */
            Relative, /* Returns the path relative to the HLDS directory */
            Naked,    /* Returns the naked path (i.e only 'cstrike') */
        };

        /// <summary>
        /// The exception class that the path manager throws
        /// </summary>
        GM_DEFINE_EXCEPTION(Exception);

        /// <summary>
        /// Constructs a path manager instance
        /// </summary>
        PathManager(const std::string& gameDirectory);

        /// <summary>
        /// Gets a specified path
        /// </summary>
        std::string GetPath(Path path, Type type) const;

        /// <summary>
        /// Gets a specified path object
        /// </summary>
        const fs::path& GetPathObject(Path path) const;

    private:
        // We want to avoid any abnormally long variable type names
        typedef std::unordered_map<Path, fs::path> PathCollection;

        // Private members
        PathCollection mPaths;
    };
}
