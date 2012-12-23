#if !defined __FSUTIL_H__
#define __FSUTIL_H__

namespace energonsoftware {

boost::filesystem::path home_dir();

bool is_hidden(const boost::filesystem::path& path);

// returns a list of all the files in a directory
// does *not* include directories in the list
// set subdirs to true to recursively list subdirectory files
// set hidden to true to include hidden files (files that begin with a .)
void list_directory(const boost::filesystem::path& path, std::vector<boost::filesystem::path>& files, bool subdirs=false, bool hidden=false);

bool file_to_string(const boost::filesystem::path& path, std::string& str);
bool file_to_strings(const boost::filesystem::path& path, std::vector<std::string>& s);

}

#endif
