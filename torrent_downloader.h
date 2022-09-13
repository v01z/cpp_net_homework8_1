#pragma once

#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>
#include <csignal>

#include <libtorrent/session.hpp>
#include <libtorrent/session_settings.hpp>
#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/torrent_status.hpp>
#include <libtorrent/read_resume_data.hpp>
#include <libtorrent/write_resume_data.hpp>
#include <libtorrent/error_code.hpp>
#include <libtorrent/magnet_uri.hpp>
#include <libtorrent/torrent_info.hpp>

//--------------------------------------------------------------------------------

class TorrentDownloader{
private:
    lt::session ses_{};

    char const* state(lt::torrent_status::state_t);
    const std::vector<char> load_file(char const*)const;
public:
    TorrentDownloader() = delete;
    TorrentDownloader(std::string&, bool);
    void run();
};

//--------------------------------------------------------------------------------
