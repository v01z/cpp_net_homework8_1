#include "torrent_downloader.h"

//--------------------------------------------------------------------------------

std::atomic<bool> shut_down{false };

//--------------------------------------------------------------------------------

void sighandler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received.\n";
    shut_down = true;
}

//--------------------------------------------------------------------------------

TorrentDownloader::TorrentDownloader(std::string& res_getting_rule, bool use_magnet_link)
{
    lt::settings_pack sett_pack;
    sett_pack.set_int(lt::settings_pack::alert_mask,
                      lt::alert_category::error
                      | lt::alert_category::storage
                      | lt::alert_category::status);

    ses_.apply_settings(std::move (sett_pack));

    auto buf = load_file(".resume_file");

    lt::add_torrent_params torr_params;

    if(use_magnet_link)
        torr_params = lt::parse_magnet_uri(res_getting_rule);
    else
        torr_params.ti = std::make_shared<lt::torrent_info>(res_getting_rule);

    if (buf.size())
    {
        lt::add_torrent_params atp = lt::read_resume_data(buf);
        if (atp.info_hashes == torr_params.info_hashes)
            torr_params = std::move(atp);
    }

    torr_params.save_path = ".";
    ses_.async_add_torrent(std::move(torr_params));
}

//--------------------------------------------------------------------------------

void TorrentDownloader::run() {
    if (!ses_.is_valid())
    {
        std::cout << "Not valid session. Aborting.\n";
        return;
    }

    std::chrono::steady_clock::time_point  last_save_resume =
            std::chrono::steady_clock::now();

    lt::torrent_handle h;

    std::signal(SIGINT, &sighandler);

    for (bool done = false; !done;)
    {
        std::vector<lt::alert*> alerts;
        ses_.pop_alerts(&alerts);

        if (shut_down)
        {
            shut_down = false;
            auto const handles = ses_.get_torrents();
            if (1 == handles.size())
            {
                handles[0].save_resume_data(lt::torrent_handle::save_info_dict);
                done = true;
                break;
            }
        }

        for (lt::alert const* a : alerts)
        {
            if (auto at = lt::alert_cast<lt::add_torrent_alert>(a))
            {
                h = at->handle;
            }
            if (lt::alert_cast<lt::torrent_finished_alert>(a))
            {
                h.save_resume_data(lt::torrent_handle::save_info_dict);
                done = true;
            }
            if (lt::alert_cast<lt::torrent_error_alert>(a))
            {
                std::cout << a->message() << std::endl;
                done = true;
                h.save_resume_data(lt::torrent_handle::save_info_dict);
            }

            if (auto rd = lt::alert_cast<lt::save_resume_data_alert>(a))
            {
                std::ofstream of(".resume_file", std::ios_base::binary);
                of.unsetf(std::ios_base::skipws);
                auto const b = write_resume_data_buf(rd->params);
                of.write(b.data(), int(b.size()));
                if (done) break;
            }

            if (lt::alert_cast<lt::save_resume_data_failed_alert>(a))
            {
                if (done) break;
            }

            if (auto st = lt::alert_cast<lt::state_update_alert>(a))
            {
                if (st->status.empty()) continue;

                lt::torrent_status const& s = st->status[0];
                std::cout << '\r' << state(s.state) << ' '
                << (s.download_payload_rate / 1000) << " kB/s "
                << (s.total_done / 1000) << " kB ("
                << (s.progress_ppm / 10000) << "%) downloaded ("
                << s.num_peers << " peers)\x1b[K";
                std::cout.flush();
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        ses_.post_torrent_updates();

        // save resume data once every 30 seconds
        if (std::chrono::steady_clock::now() - last_save_resume > std::chrono::seconds(30))
        {
            h.save_resume_data(lt::torrent_handle::save_info_dict);
            last_save_resume = std::chrono::steady_clock::now();
        }
    }
}
//--------------------------------------------------------------------------------

char const* TorrentDownloader::state(lt::torrent_status::state_t s) {
    switch (s)
    {
        case lt::torrent_status::checking_files: return "checking";
        case lt::torrent_status::downloading_metadata: return "dl metadata";
        case lt::torrent_status::downloading: return "downloading";
        case lt::torrent_status::finished: return "finished";
        case lt::torrent_status::seeding: return "seeding";
        case lt::torrent_status::checking_resume_data: return "checking resume";
        default: return "<>";
    }
}

//--------------------------------------------------------------------------------

const std::vector<char> TorrentDownloader::load_file(char const* filename)const{
    std::ifstream ifs(filename, std::ios_base::binary);
    ifs.unsetf(std::ios_base::skipws);
    return {std::istream_iterator<char>(ifs), std::istream_iterator<char>()};
}

//--------------------------------------------------------------------------------
