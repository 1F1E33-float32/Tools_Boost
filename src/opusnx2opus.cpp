#include <nanobind/nanobind.h>
#include <random>
#include <string>
#include <vector>

// =============================================================================
// Constants
// =============================================================================

static constexpr uint32_t TAG_OPUS_BASIC = 0x80000001;   // basic info
static constexpr uint32_t TAG_OPUS_CONTEXT = 0x80000003; // loop/num_samples
static constexpr uint32_t TAG_OPUS_DATA = 0x80000004;    // data info
static constexpr uint32_t TAG_OPUS_MULTI = 0x80000005;   // multistream info

// =============================================================================
// Structures
// =============================================================================

struct MappingInfo {
    uint8_t stream_count = 0;
    uint8_t coupled_count = 0;
    std::vector<uint8_t> channel_mapping; // size == channels
};

struct OpusnxMeta {
    uint8_t channels = 0;
    uint8_t frame_size = 0;       // 0 = VBR (not supported here)
    uint32_t sample_rate = 48000; // normalized to 48000
    uint16_t pre_skip = 0;
    std::vector<std::vector<uint8_t>> packets; // raw Opus packets
    bool has_mapping = false;
    MappingInfo mapping;
    bool loop_flag = false;
    int32_t loop_start = 0;
    int32_t loop_end = 0;
    int32_t playable_num_samples = -1; // <0 if unknown
};

// ============================================================================
// Utility Functions
// ============================================================================

static inline uint8_t rd_u8(const uint8_t *b, size_t off) { return b[off]; }
static inline uint16_t rd_u16le(const uint8_t *b, size_t off) { return (uint16_t)(b[off] | (b[off + 1] << 8)); }
static inline uint32_t rd_u32le(const uint8_t *b, size_t off) { return (uint32_t)(b[off] | (b[off + 1] << 8) | (b[off + 2] << 16) | (b[off + 3] << 24)); }
static inline int32_t rd_s32le(const uint8_t *b, size_t off) { return (int32_t)(b[off] | (b[off + 1] << 8) | (b[off + 2] << 16) | (b[off + 3] << 24)); }

// Ogg CRC32 (polynomial 0x04C11DB7, big-endian shift; see RFC 3533)
static inline uint32_t ogg_crc32(const uint8_t *p, size_t n) {
    uint32_t crc = 0;
    for (size_t i = 0; i < n; ++i) {
        crc ^= (uint32_t)(p[i] & 0xFF) << 24;
        for (int j = 0; j < 8; ++j) {
            if (crc & 0x80000000U)
                crc = (crc << 1) ^ 0x04C11DB7U;
            else
                crc <<= 1;
        }
    }
    return crc;
}

// Split a packet into Ogg lacing values (<=255 each)
static inline void packet_to_lacing(const std::vector<uint8_t> &pkt, std::vector<uint8_t> &segs) {
    for (size_t i = 0; i < pkt.size(); i += 255)
        segs.push_back((uint8_t)std::min<size_t>(255, pkt.size() - i));
}

// Opus duration helpers (48kHz timebase). Ported from opus_packet_get_* logic.
static inline int opus_packet_get_samples_per_frame(const uint8_t *packet, int Fs) {
    const int b0 = packet[0];
    int audiosize;
    if (b0 & 0x80) {
        audiosize = (b0 >> 3) & 0x03;
        audiosize = (Fs << audiosize) / 400;
    } else if ((b0 & 0x60) == 0x60) {
        audiosize = (b0 & 0x08) ? (Fs / 50) : (Fs / 100);
    } else {
        audiosize = (b0 >> 3) & 0x03;
        if (audiosize == 3)
            audiosize = Fs * 60 / 1000;
        else
            audiosize = (Fs << audiosize) / 100;
    }
    return audiosize;
}
static inline int opus_packet_get_nb_frames(const uint8_t *packet, size_t len) {
    if (len < 1)
        throw std::runtime_error("Opus packet too short");
    const int code = packet[0] & 0x03;
    if (code == 0)
        return 1;
    else if (code != 3)
        return 2;
    else {
        if (len < 2)
            throw std::runtime_error("Opus packet too short for code=3");
        return packet[1] & 0x3F;
    }
}
static inline int opus_packet_get_nb_samples(const uint8_t *packet, size_t len, int Fs = 48000) {
    const int frames = opus_packet_get_nb_frames(packet, len);
    const int spf = opus_packet_get_samples_per_frame(packet, Fs);
    const int samples = frames * spf;
    if ((int64_t)samples * 25 > (int64_t)Fs * 3)
        throw std::runtime_error("Opus packet duration >120ms?");
    return samples;
}

// ============================================================================
// Utility Functions
// ============================================================================

static std::vector<uint8_t> build_ogg_page(const std::vector<std::vector<uint8_t>> &packets, uint64_t granulepos, uint32_t serial, uint32_t seqno, bool bos, bool eos, bool continued) {
    // Build segment table and payload
    std::vector<uint8_t> segment_table;
    std::vector<uint8_t> payload;
    for (auto &pkt : packets) {
        packet_to_lacing(pkt, segment_table);
        payload.insert(payload.end(), pkt.begin(), pkt.end());
    }
    if (segment_table.size() > 255)
        throw std::runtime_error("Too many segments for one Ogg page");

    // Header with zeroed checksum
    std::vector<uint8_t> hdr;
    hdr.insert(hdr.end(), {'O', 'g', 'g', 'S'});
    hdr.push_back(0x00);
    uint8_t flags = 0;
    if (continued)
        flags |= 0x01;
    if (bos)
        flags |= 0x02;
    if (eos)
        flags |= 0x04;
    hdr.push_back(flags);
    for (int i = 0; i < 8; ++i)
        hdr.push_back((uint8_t)((granulepos >> (i * 8)) & 0xFF));
    for (int i = 0; i < 4; ++i)
        hdr.push_back((uint8_t)((serial >> (i * 8)) & 0xFF));
    for (int i = 0; i < 4; ++i)
        hdr.push_back((uint8_t)((seqno >> (i * 8)) & 0xFF));
    hdr.insert(hdr.end(), {0, 0, 0, 0}); // checksum placeholder
    hdr.push_back((uint8_t)segment_table.size());
    hdr.insert(hdr.end(), segment_table.begin(), segment_table.end());

    // Compute CRC over header+payload (checksum bytes are zero)
    std::vector<uint8_t> all = hdr;
    all.insert(all.end(), payload.begin(), payload.end());
    const uint32_t crc = ogg_crc32(all.data(), all.size());
    hdr[22] = (uint8_t)(crc & 0xFF);
    hdr[23] = (uint8_t)((crc >> 8) & 0xFF);
    hdr[24] = (uint8_t)((crc >> 16) & 0xFF);
    hdr[25] = (uint8_t)((crc >> 24) & 0xFF);

    // Return page bytes
    hdr.insert(hdr.end(), payload.begin(), payload.end());
    return hdr;
}

static std::vector<uint8_t> build_opus_head(uint8_t channels, uint16_t pre_skip, uint32_t input_sample_rate, const MappingInfo *map_opt) {
    std::vector<uint8_t> o;
    o.insert(o.end(), {'O', 'p', 'u', 's', 'H', 'e', 'a', 'd'});
    o.push_back(1); // version
    o.push_back(channels);
    o.push_back((uint8_t)(pre_skip & 0xFF));
    o.push_back((uint8_t)(pre_skip >> 8));
    for (int i = 0; i < 4; ++i)
        o.push_back((uint8_t)((input_sample_rate >> (i * 8)) & 0xFF));
    o.push_back(0);
    o.push_back(0); // output_gain (le16, 0)

    if (!map_opt || channels <= 2) {
        o.push_back(0); // mapping_family=0
    } else {
        o.push_back(1); // mapping_family=1
        o.push_back(map_opt->stream_count);
        o.push_back(map_opt->coupled_count);
        o.insert(o.end(), map_opt->channel_mapping.begin(), map_opt->channel_mapping.end());
    }
    return o;
}

static std::vector<uint8_t> build_opus_tags(const std::string &vendor, const std::vector<std::string> &comments) {
    auto put_u32le = [](std::vector<uint8_t> &v, uint32_t x) {
        v.push_back(x & 0xFF);
        v.push_back((x >> 8) & 0xFF);
        v.push_back((x >> 16) & 0xFF);
        v.push_back((x >> 24) & 0xFF);
    };
    std::vector<uint8_t> o;
    o.insert(o.end(), {'O', 'p', 'u', 's', 'T', 'a', 'g', 's'});
    put_u32le(o, (uint32_t)vendor.size());
    o.insert(o.end(), vendor.begin(), vendor.end());
    put_u32le(o, (uint32_t)comments.size());
    for (auto &s : comments) {
        put_u32le(o, (uint32_t)s.size());
        o.insert(o.end(), s.begin(), s.end());
    }
    return o;
}

static OpusnxMeta parse_opusnx(const std::vector<uint8_t> &data) {
    if (data.size() < 0x24)
        throw std::runtime_error("Not Nintendo OPUS: file too small");
    if (rd_u32le(data.data(), 0x00) != TAG_OPUS_BASIC)
        throw std::runtime_error("Not Nintendo OPUS (0x80000001 missing)");

    OpusnxMeta m{};
    m.channels = rd_u8(data.data(), 0x09);
    m.frame_size = rd_u8(data.data(), 0x0A);
    uint32_t sr = rd_u32le(data.data(), 0x0C);
    if (sr != 48000)
        sr = 48000;
    m.sample_rate = sr;
    uint32_t data_rel = rd_u32le(data.data(), 0x10);
    uint32_t context_rel = rd_u32le(data.data(), 0x18);
    m.pre_skip = rd_u16le(data.data(), 0x1C);

    // multistream at 0x20?
    if (rd_u32le(data.data(), 0x20) == TAG_OPUS_MULTI && m.channels <= 8) {
        size_t ms = 0x20;
        m.has_mapping = true;
        m.mapping.stream_count = rd_u8(data.data(), ms + 0x08);
        m.mapping.coupled_count = rd_u8(data.data(), ms + 0x09);
        m.mapping.channel_mapping.resize(m.channels);
        for (int i = 0; i < m.channels; ++i)
            m.mapping.channel_mapping[i] = rd_u8(data.data(), ms + 0x0A + i);
    }

    // context info (loop + num_samples)
    if (context_rel != 0 && rd_u32le(data.data(), context_rel + 0x00) == TAG_OPUS_CONTEXT) {
        m.loop_flag = rd_u8(data.data(), context_rel + 0x09) != 0;
        int32_t ns = rd_s32le(data.data(), context_rel + 0x0C);
        m.loop_start = rd_s32le(data.data(), context_rel + 0x10);
        m.loop_end = rd_s32le(data.data(), context_rel + 0x14);
        if (ns > 0)
            m.playable_num_samples = ns; // else unknown
    }

    // data chunk
    const uint32_t data_abs = data_rel;
    if (rd_u32le(data.data(), data_abs) != TAG_OPUS_DATA)
        throw std::runtime_error("Missing 0x80000004 data chunk");
    const uint32_t data_size_total = rd_u32le(data.data(), data_abs + 0x04);
    size_t pos = (size_t)data_abs + 0x08;
    size_t end = pos + data_size_total;
    if (end > data.size())
        throw std::runtime_error("OPUS data size out of range");

    // OPUS_SWITCH packet stream: [be32 pkt_size][u32 state?][pkt bytes...]
    while (pos < end) {
        if (pos + 8 > end)
            throw std::runtime_error("Truncated OPUS_SWITCH packet header");
        uint32_t pkt_be = (uint32_t)(data[pos] << 24 | data[pos + 1] << 16 | data[pos + 2] << 8 | data[pos + 3]);
        pos += 4;
        pos += 4; // skip state
        if (pos + pkt_be > end)
            throw std::runtime_error("Invalid OPUS_SWITCH packet size");
        std::vector<uint8_t> p(data.begin() + pos, data.begin() + pos + pkt_be);
        m.packets.emplace_back(std::move(p));
        pos += pkt_be;
    }
    return m;
}

// ============================================================================
// Main Logic
// ============================================================================

static std::vector<uint8_t> mux_to_ogg(const OpusnxMeta &meta) {
    // Build OpusHead / OpusTags
    const MappingInfo *map_opt = meta.has_mapping ? &meta.mapping : nullptr;
    auto opus_head = build_opus_head(meta.channels, meta.pre_skip, meta.sample_rate, map_opt);

    std::vector<std::string> comments;
    if (meta.loop_flag) {
        comments.push_back("LOOPSTART=" + std::to_string(meta.loop_start));
        comments.push_back("LOOPEND=" + std::to_string(meta.loop_end));
    }
    if (meta.playable_num_samples >= 0)
        comments.push_back("NUMSAMPLES=" + std::to_string(meta.playable_num_samples));
    auto opus_tags = build_opus_tags("opusnx-converted", comments);

    // final granulepos if exact num samples known (RFC 7845): pre_skip + playable
    int64_t final_gpos = -1;
    if (meta.playable_num_samples >= 0)
        final_gpos = (int64_t)meta.pre_skip + (int64_t)meta.playable_num_samples;

    // random stream serial
    std::mt19937_64 rng{std::random_device{}()};
    const uint32_t serial = (uint32_t)(rng() & 0xFFFFFFFFULL);

    std::vector<uint8_t> out;
    std::vector<std::vector<uint8_t>> page_packets;

    // Page 0: OpusHead
    page_packets = {opus_head};
    auto page0 = build_ogg_page(page_packets, 0, serial, 0, true, false, false);
    out.insert(out.end(), page0.begin(), page0.end());

    // Page 1: OpusTags
    page_packets = {opus_tags};
    auto page1 = build_ogg_page(page_packets, 0, serial, 1, false, meta.packets.empty(), false);
    out.insert(out.end(), page1.begin(), page1.end());

    // Audio pages: fill up to lacing limit (<=255 segs)
    uint32_t seqno = 2;
    int64_t gpos_acc = 0;
    size_t i = 0;
    const size_t N = meta.packets.size();
    while (i < N) {
        page_packets.clear();
        size_t seg_count = 0;
        while (i < N) {
            const auto &pkt = meta.packets[i];
            size_t need = (pkt.size() + 254) / 255; // segments for this packet
            if (seg_count + need > 255)
                break;
            page_packets.push_back(pkt);
            seg_count += need;
            gpos_acc += opus_packet_get_nb_samples(pkt.data(), pkt.size(), 48000);
            ++i;
        }
        const bool last = (i == N);
        const uint64_t gpos = (last && final_gpos >= 0) ? (uint64_t)final_gpos : (uint64_t)gpos_acc;
        auto page = build_ogg_page(page_packets, gpos, serial, seqno++, false, last, false);
        out.insert(out.end(), page.begin(), page.end());
    }
    return out;
}

static nanobind::bytes opusnx2opus_main(nanobind::bytes data) {
    std::vector<uint8_t> buf((const uint8_t *)data.c_str(), (const uint8_t *)data.c_str() + data.size());
    OpusnxMeta meta = parse_opusnx(buf);
    auto ogg = mux_to_ogg(meta);
    return nanobind::bytes((const char *)ogg.data(), ogg.size());
}

// ============================================================================
// Module Initialization
// ============================================================================

void init_opusnx2opus(nanobind::module_ &m) {
    m.def("opusnx2opus_main", &opusnx2opus_main, nanobind::arg("data"),
          R"pbdoc(
Convert OpusNX data to standard Opus data.

Args:
    data (bytes): Input OpusNX-encoded data.

Returns:
    bytes: Re-encoded Opus data.
)pbdoc");
}
