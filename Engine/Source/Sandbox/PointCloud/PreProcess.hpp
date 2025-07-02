#pragma once
#include "Log.hpp"
#include "Math/Math.hpp"
#include "lasreader.hpp"

namespace worse::pc
{

    static void load()
    {
        LASreader* reader = nullptr;

        LASreadOpener opener;
        opener.set_file_name(
            "/Users/w6rsty/dev/Cpp/worse_pc/Engine/Binary/LAS/0_0_1_1.las");
        reader = opener.open();

        WS_LOG_INFO("LAS",
                    "Open {} successfully, count {}",
                    opener.get_file_name(),
                    reader->npoints);
    }

} // namespace worse::pc