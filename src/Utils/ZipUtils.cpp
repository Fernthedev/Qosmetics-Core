#include "Utils/ZipUtils.hpp"
#include "beatsaber-hook/shared/utils/utils.h"
#include "logging.hpp"
#include "zip/shared/zip.h"

namespace Qosmetics::Core::ZipUtils
{
    namespace
    {
        bool ReadZipFile(zip* z, std::string_view fileName, std::vector<uint8_t>& out)
        {
            int index = zip_name_locate(z, fileName.data(), 0);

            if (index == -1)
            {
                return false;
                INFO("file '%s' did not exist in zip, are you sure you passed in the right name?", fileName.data());
            }
            INFO("Reading file '%s'", fileName.data());
            struct zip_stat st;
            zip_stat_init(&st);
            zip_stat(z, fileName.data(), 0, &st);

            zip_file* f = zip_fopen(z, st.name, 0);
            out.resize(st.size);
            zip_fread(f, out.data(), st.size);
            INFO("Found %lu bytes", st.size);
            zip_fclose(f);

            return true;
        }
    }

    bool GetBytesFromZipFile(std::string_view zipPath, std::string_view fileName, std::vector<uint8_t>& out)
    {
        if (zipPath.ends_with("/"))
        {
            ERROR("Path '%s' was a folder path!", zipPath.data());
            return false;
        }

        if (!fileexists(zipPath) && !direxists(zipPath))
        {
            ERROR("Path '%s' did not exist!", zipPath.data());
            return false;
        }
        if (fileName == "")
        {
            ERROR("File Name was empty!");
            return false;
        }

        int err = 0;
        zip* z = zip_open(zipPath.data(), 0, &err);

        if (!ReadZipFile(z, fileName, out))
        {
            zip_close(z);
            return false;
        }
        zip_close(z);
        return true;
    }
}