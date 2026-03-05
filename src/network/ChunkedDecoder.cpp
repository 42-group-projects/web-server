#include "NetworkManager.hpp"

// ============================================================================
// Chunked Transfer Encoding Decoder
// ============================================================================

NetworkManager::ChunkDecodeResult NetworkManager::tryDecodeChunked(ConnState &st, std::string &decodedBody, size_t &totalConsumed)
{
    size_t p = st.chunkParsePos;
    decodedBody.clear();

    while (true) {
        size_t lineEnd = st.buf.find("\r\n", p);
        if (lineEnd == std::string::npos) return CHUNK_NEED_MORE;

        std::string sizeLine = st.buf.substr(p, lineEnd - p);
        sizeLine = trim(sizeLine);

        // Strip chunk extensions: "<hex>;..."
        size_t semi = sizeLine.find(';');
        if (semi != std::string::npos)
            sizeLine.erase(semi);
        sizeLine = trim(sizeLine);

        if (sizeLine.empty())
            return CHUNK_INVALID;

        // Validate hex digits strictly
        for (size_t i = 0; i < sizeLine.size(); ++i) {
            char c = sizeLine[i];
            bool isHex = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
            if (!isHex)
                return CHUNK_INVALID;
        }

        size_t chunkSize = 0;
        {
            std::istringstream iss(sizeLine);
            iss >> std::hex >> chunkSize;
            if (!iss.eof() && iss.fail())
                return CHUNK_INVALID;
        }

        // Debug: show where we are and the parsed chunk size
        std::cerr << "[NM][chunked] p=" << p << " lineEnd=" << lineEnd
                  << " sizeLine='" << sizeLine << "' chunkSize=" << chunkSize
                  << " buf.size=" << st.buf.size() << std::endl;

        p = lineEnd + 2;

        if (chunkSize == 0) {
            // RFC 7230: after last-chunk (0\r\n), there may be trailer-fields, then a final CRLF.
            // The minimal valid ending is "0\r\n\r\n" (no trailers).
            if (st.buf.size() < p + 2)
                return CHUNK_NEED_MORE;

            // If trailers are empty, we should see CRLF immediately.
            if (st.buf.compare(p, 2, "\r\n") == 0) {
                totalConsumed = p + 2;
                std::cerr << "[NM][chunked] end (no trailers) totalConsumed=" << totalConsumed << std::endl;
                return CHUNK_OK;
            }

            // Otherwise, read trailers until CRLFCRLF.
            size_t trailerEnd = st.buf.find("\r\n\r\n", p);
            std::cerr << "[NM][chunked] chunkSize=0 trailer search from p=" << p
                      << " trailerEnd=" << (trailerEnd == std::string::npos ? -1 : (long long)trailerEnd)
                      << " buf.size=" << st.buf.size() << std::endl;
            if (trailerEnd == std::string::npos)
                return CHUNK_NEED_MORE;
            totalConsumed = trailerEnd + 4;
            std::cerr << "[NM][chunked] end (with trailers) totalConsumed=" << totalConsumed << std::endl;
            return CHUNK_OK;
        }

        // Prevent decoded body from growing without bound
        if (decodedBody.size() + chunkSize > kMaxBodyBytes)
            return CHUNK_INVALID;

        if (st.buf.size() < p + chunkSize + 2) return CHUNK_NEED_MORE;

        decodedBody.append(st.buf, p, chunkSize);
        p += chunkSize;

        if (st.buf.substr(p, 2) != "\r\n") return CHUNK_INVALID;
        p += 2;
    }
}
