#ifndef HTTP_PARSER_MULTIPART_HPP
#define HTTP_PARSER_MULTIPART_HPP

#include <string>
#include <fstream>
#include <boost/uuid/uuid.hpp>           
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>        
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

namespace websocketpp {
namespace http {
namespace parser {

namespace fs = boost::filesystem;

class multipart_file_saver {
public:
    multipart_file_saver() : m_state(0) {}
    multipart_file_saver(const multipart_file_saver&) {}

    void set_boundary(const std::string& boundary) { m_boundary = std::string("\r\n--") + boundary + "\r\n"; }
    void set_base_folder(const std::string& base_folder) { m_base_folder = base_folder; }
    size_t process_body(char const * buf, size_t len, std::string& body);

private:
    inline void on_header_done()
    {
        if (m_content_disposition_header) {
            auto filename_start = m_cur_header_value.find("filename=\"");
            if (filename_start != std::string::npos) {
                filename_start += 10;
                auto filename_end = m_cur_header_value.find_first_of("\"", filename_start );
                if (filename_end != std::string::npos) {
                    if (m_subfolder.empty()) {
                        boost::uuids::random_generator generator;
                        boost::uuids::uuid uuid1 = generator();
                        m_subfolder = m_base_folder + boost::uuids::to_string(uuid1) + '/';
                        fs::path data_dir(m_subfolder);
                        fs::create_directories(data_dir);
                    }
                    m_file_name = m_subfolder + m_cur_header_value.substr(filename_start, filename_end - filename_start);
                    m_data_file.open(m_file_name);
                    m_is_file_part = true;
                }
            }
        }
        m_cur_header_value.clear();
    }

    inline void on_body_begin()
    {
        m_subfolder.clear();
    }

    inline void on_part_begin()
    {
        m_cur_header_value.clear();
        m_content_disposition_header = false;
        m_is_file_part = false;
    }

    inline void on_header_field(const char* data, size_t size)
    {
        on_header_done();
        if (strncmp(data, "Content-Disposition", size) == 0)
            m_content_disposition_header = true;
        else
            m_content_disposition_header = false;
    }

    inline void on_header_value(const char* data, size_t size)
    {
        m_cur_header_value.append(data, size);
    }

    inline void on_headers_complete()
    {
        on_header_done();
    }

    inline void on_part_end()
    {
        if (m_is_file_part && m_data_file.is_open()) {
            m_data_file.close();
        }
        m_is_file_part = false;
    }

    inline void on_body_end() {
        if (m_data_file.is_open()) {
            m_data_file.close();
        }
    }


    std::string m_boundary;
    std::string m_base_folder;
    std::string m_subfolder;
    size_t      m_boundary_index = 2;
    uint16_t    m_state = 0;

    bool        m_content_disposition_header = false;
    bool        m_is_file_part = false;
    std::string m_cur_header_value;
    std::string m_file_name;
    std::ofstream m_data_file;
};

} // namespace parser
} // namespace http
} // namespace websocketpp

#include <websocketpp/http/impl/multipart_file_saver.hpp>

#endif // HTTP_PARSER_MULTIPART_HPP
