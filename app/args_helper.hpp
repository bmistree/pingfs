const uint16_t FS_ID = 55;

void run_io_service(boost::asio::io_service* io_service) {
    io_service->run();
}

/**
 * @param remote_endpt A hostname to bounce messages
 * off of.
 */
std::shared_ptr<pingfs::PingBlockService> gen_block_service(
    boost::asio::io_service* io_service,
    const std::string remote_endpt) {

    std::shared_ptr<pingfs::Ping> ping =
        std::make_shared<pingfs::Ping>(io_service);

    std::shared_ptr<pingfs::BlockPingTranslator> translator =
        std::make_shared<pingfs::PassThroughTranslator>();

    return std::make_shared<pingfs::TrackFreedService>(
        FS_ID,
        ping,
        translator,
        remote_endpt);
}

std::shared_ptr<pingfs::CounterSupplier> gen_counter_supplier() {
    return std::make_shared<pingfs::CounterSupplier>();
}

void copy_char(char** dest, const char* src,
    std::size_t src_len) {
    *dest = new char[src_len];
    strncpy(*dest, src, src_len + 1);
}

void fuse_params(const std::string& mount_point,
    bool debug, std::vector<char*>* fuse_args) {
    char* bin_name;
    copy_char(&bin_name, "dummy", 5);
    fuse_args->push_back(bin_name);
    if (debug) {
        char* debug_opt;
        copy_char(&debug_opt, "-d", 2);
        fuse_args->push_back(debug_opt);
    }

    // FIXME: It's unclear why we have to run
    // this in the foreground, but we do.
    char* helper;
    copy_char(&helper, "-f", 2);
    fuse_args->push_back(helper);

    char* mount_point_ptr;
    copy_char(
        &mount_point_ptr,
        mount_point.c_str(),
        mount_point.size());
    fuse_args->push_back(mount_point_ptr);
    fuse_args->push_back(nullptr);
}
