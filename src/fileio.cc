#include "fileio.h"

#include "helper/cpp.h"

#include <fcntl.h>

#ifndef O_SEQUENTIAL
    #define O_SEQUENTIAL (0)
#endif

// --------------------------------------------------------------------------------

#ifndef _WIN32
// based on https://stackoverflow.com/a/66514731
#include <unistd.h>

size_t filelength(int fp)
{
    off_t currPos = lseek(fp, 0, SEEK_CUR);
    if (currPos == off_t(-1))
    {
        assert(false); // it's undefined to call filelength on a file that's being read
        return size_t(0);
    }
    
    off_t size = lseek(fp, 0, SEEK_END);
    lseek(fp, 0, SEEK_SET);
    return size_t(size);
}
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
