#include "fileio.h"

#include "helper/cpp.h"

#include <fcntl.h>

#ifndef O_SEQUENTIAL
    #define O_SEQUENTIAL (0)
#endif

// --------------------------------------------------------------------------------

class OSFileHandle
{
    NON_COPYABLE(OSFileHandle);
    OSFileHandle() = delete;

    int mFP = -1;


public:
    OSFileHandle(const std::string& filename, int flags)
    {
        mFP = open(filename.c_str(), flags);
        if (mFP < 0)
            throw std::invalid_argument(filename);
    }

    ~OSFileHandle()
    {
        if (mFP >= 0)
            close(mFP);
    }

    int GetFp() const { return mFP; }
};

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

std::string SlurpFile(const std::string& filename)
{
    // O_SEQUENTIAL only exists for windows, but hints at a caching mechanism that works better for slurping
    OSFileHandle fp(filename, O_RDONLY | O_SEQUENTIAL);

    const long length = filelength(fp.GetFp());
    if ((length < 0) || (length >= std::numeric_limits<int>::max()))
        throw std::invalid_argument(filename);

    std::string contents;
    contents.resize_and_overwrite(length, [&fp, length](char* buf, std::size_t buf_size) -> long
                                  {
                                      int res = read(fp.GetFp(), buf, int(length));
                                      if (res < 0)
                                        throw std::runtime_error("failed to read entire file");

                                      return res;
                                  });

    return contents;
}

// --------------------------------------------------------------------------------
