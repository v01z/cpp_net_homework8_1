#include "torrent_downloader.h"

//--------------------------------------------------------------------------------

int main(int argc, char const* argv[]){

    if (argc != 3)
    {
        std::cerr
        << "Usage:\nDownload via magnet url:\n"
        <<  argv[0] << " -m <magnet-url>\n"
        << "Download via torrent file:\n"
        <<  argv[0] << " -t <torrent-file>\n";
        return EXIT_FAILURE;
    }

    bool is_magnet_link{};

    if(std::strlen(argv[1]) == 2 &&
    std::strncmp(argv[1], "-m", 2) == 0)
    {
        is_magnet_link = true;
    }

    std::string link_or_file{ argv[2] };
    TorrentDownloader blank(link_or_file, is_magnet_link);
    blank.run();

    return EXIT_SUCCESS;
}

//--------------------------------------------------------------------------------
