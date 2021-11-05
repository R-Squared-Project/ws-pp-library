#ifndef HTTP_PARSER_MULTIPART_IMPL_HPP
#define HTTP_PARSER_MULTIPART_IMPL_HPP

#include <string>
#include <fstream>

namespace websocketpp {
namespace http {
namespace parser {

#define CR '\r'
#define LF '\n'
#define SP ' '
#define TAB '\t'
#define HYPHEN '-'


#define APPEND_DATA(P, S) \
        if (m_is_file_part && m_data_file.is_open()) \
            m_data_file.write(P, S); \
        else \
            body.append(P, S);

enum mp_state {
    s_first_boundary,
    s_header_field_start,
    s_header_field,
    s_header_value_start,
    s_header_value,
    s_header_value_cr,
    s_headers_done,
    s_data_start,
    s_data,
    s_data_cr,
    s_data_cr_lf,
    s_data_cr_lf_hy,
    s_data_boundary_start,
    s_data_boundary,
    s_data_boundary_done,
    s_data_boundary_done_cr_lf,
    s_data_boundary_done_hy_hy,
    s_epilogue
};

/* Header field name as defined by rfc 2616. Also lowercases them.
 *     field-name   = token
 *     token        = 1*<any CHAR except CTLs or tspecials>
 *     CTL          = <any US-ASCII control character (octets 0 - 31) and DEL (127)>
 *     tspecials    = "(" | ")" | "<" | ">" | "@"
 *                  | "," | ";" | ":" | "\" | DQUOTE
 *                  | "/" | "[" | "]" | "?" | "="
 *                  | "{" | "}" | SP | TAB
 *     DQUOTE       = <US-ASCII double-quote mark (34)>
 *     SP           = <US-ASCII SP, space (32)>
 *     TAB           = <US-ASCII TAB, horizontal-tab (9)>
 */
static const char header_field_chars[256] = {
/*  0 nul   1 soh   2 stx   3 etx   4 eot   5 enq   6 ack   7 bel   */
    0,      0,      0,      0,      0,      0,      0,      0,
/*  8 bs    9 ht    10 nl   11 vt   12 np   13 cr   14 so   15 si   */
    0,      0,      0,      0,      0,      0,      0,      0,
/*  16 dle  17 dc1  18 dc2  19 dc3  20 dc4  21 nak  22 syn  23 etb  */
    0,      0,      0,      0,      0,      0,      0,      0,
/*  24 can  25 em   26 sub  27 esc  28 fs   29 gs   30 rs   31 us   */
    0,      0,      0,      0,      0,      0,      0,      0,
/*  32 sp   33 !    34 "    35 #    36 $    37 %    38 &    39 '    */
    0,      '!',    0,      '#',    '$',    '%',    '&',    '\'',
/*  40 (    41 )    42 *    43 +    44 ,    45 -    46 .    47 /    */
    0,      0,      '*',    '+',    0,      '-',    '.',    0,
/*  48 0    49 1    50 2    51 3    52 4    53 5    54 6    55 7    */
    '0',    '1',    '2',    '3',    '4',    '5',    '6',    '7',
/*  56 8    57 9    58 :    59 ;    60 <    61 =    62 >    63 ?    */
    '8',    '9',    0,      0,      0,      0,      0,      0,
/*  64 @    65 A    66 B    67 C    68 D    69 E    70 F    71 G    */
    0,      'A',    'B',    'C',    'D',    'E',    'F',    'G',
/*  72 H    73 I    74 J    75 K    76 L    77 M    78 N    79 O    */
    'H',    'I',    'J',    'K',    'L',    'M',    'N',    'O',
/*  80 P    81 Q    82 R    83 S    84 T    85 U    86 V    87 W    */
    'P',    'Q',    'R',    'S',    'T',    'U',    'V',    'W',
/*  88 X    89 Y    90 Z    91 [    92 \    93 ]    94 ^    95 _    */
    'X',    'Y',    'Z',     0,     0,      0,      '^',    '_',
/*  96 `    97 a    98 b    99 c    100 d   101 e   102 f   103 g   */
    '`',    'a',    'b',    'c',    'd',    'e',    'f',    'g',
/*  104 h   105 i   106 j   107 k   108 l   109 m   110 n   111 o   */
    'h',    'i',    'j',    'k',    'l',    'm',    'n',    'o',
/*  112 p   113 q   114 r   115 s   116 t   117 u   118 v   119 w   */
    'p',    'q',    'r',    's',    't',    'u',    'v',    'w',
/*  120 x   121 y   122 z   123 {   124 |   125 }   126 ~   127 del */
    'x',    'y',    'z',    0,      '|',     0,     '~',    0
};

size_t multipart_file_saver::process_body(const char* data, size_t data_len, std::string& body)
{
    const char*   mark;
    const char*   p;
    unsigned char c;
    const char* data_start = data;

    for (p = data; p < data + data_len; ++p) {
        c = *p;

reexecute:
        switch (m_state) {

            case s_first_boundary:
                if (m_boundary_index < m_boundary.size()) {
                    if (c == m_boundary[m_boundary_index]) {
                        m_boundary_index++;
                        break;
                    } else if (m_boundary_index < 4) {
                        m_boundary_index = 2;
                        break;
                    }
                } else if (m_boundary_index == m_boundary.size()) {
                    on_body_begin();
                    on_part_begin();
                    m_boundary_index = 0;
                    m_state = s_header_field_start;
                    // fallthrough;
                } else
                    goto error;

            case s_header_field_start:
                if (c == CR) {
                    m_state = s_headers_done;
                    break;
                }
                m_state = s_header_field;
                // fallthrough;

            case s_header_field:
                mark = p;
                while (p < data + data_len) {
                    c = *p;
                    if (header_field_chars[c] == 0)
                        break;
                    ++p;
                }
                if (p > mark) {
                    on_header_field(mark, p - mark);
                }
                if (p == data + data_len) {
                    break;
                }
                if (c == ':') {
                    m_state = s_header_value_start;
                    break;
                }
                goto error;

            case s_header_value_start:
                if (c == SP || c == TAB) {
                    break;
                }
                m_state = s_header_value;
                // fallthrough;

            case s_header_value:
                mark = p;
                while (p < data + data_len) {
                    c = *p;
                    if (c == CR) {
                        m_state = s_header_value_cr;
                        break;
                    }
                    ++p;
                }
                if (p > mark) {
                    on_header_value(mark, p - mark);
                }
                break;

            case s_header_value_cr:
                if (c == LF) {
                    m_state = s_header_field_start;
                    break;
                }
                goto error;

            case s_headers_done:
                if (c == LF) {
                    on_headers_complete();
                    m_state = s_data_start;
                    break;
                }
                goto error;

            case s_data_start:
                body.append(data_start, p - data_start);
                data_start = p;
                if (m_is_file_part)
                    body.append(m_file_name);
                m_state = s_data;
                // fallthrough;

            case s_data:
                while (p < data + data_len) {
                    c = *p;
                    ++p;
                    if (c == CR) {
                        m_boundary_index = 1;
                        m_state = s_data_boundary;
                        break;
                    }
                }
                --p;
                break;

            case s_data_boundary:
                if (m_boundary_index < m_boundary.size()) {
                    if (c == m_boundary[m_boundary_index]) {
                        ++m_boundary_index;
                        break;
                    } else if (c == HYPHEN && m_boundary_index == m_boundary.size()-2) {
                        m_state = s_data_boundary_done_hy_hy;
                        break;
                    } else {
                        // part of "boundary" from previous chunk
                        if (static_cast<size_t>(p - data) < m_boundary_index) {
                            APPEND_DATA(&m_boundary[0], m_boundary_index);
                            data_start = p;
                        }
                        m_boundary_index = 0;
                        m_state = s_data;
                        goto reexecute;
                    }                    
                } else if (m_boundary_index == m_boundary.size()) {
                    m_boundary_index = 0;
                    m_state = s_data_boundary_done;
                    goto reexecute;
                } else
                    goto error;
            
            case s_data_boundary_done:
                {
                    auto data_size = p - data_start - m_boundary.size();
                    if (data_size > 0) {
                        APPEND_DATA(data_start, data_size);
                    }
                    body.append(m_boundary);
                    data_start = p;
                    on_part_end();
                    on_part_begin();
                    m_state = s_header_field_start;
                }
                goto reexecute;

            case s_data_boundary_done_hy_hy:
                if (c == HYPHEN) {
                    auto data_size = p - data_start - (m_boundary.size() - 1);
                    if (data_size > 0) {
                        APPEND_DATA(data_start, data_size);
                    }
                    body.append(m_boundary, 0, m_boundary.size()-2);
                    body.append("-");
                    data_start = p;
                    on_part_end();
                    on_body_end();
                    m_state = s_epilogue;
                    break;
                }
                goto error;

            case s_epilogue:
                // Must be ignored according to rfc 1341.
                break;
        }
    }

    if (m_is_file_part && (m_state == s_data || m_state == s_data_boundary)) {
        APPEND_DATA(data_start, p - data_start - m_boundary_index);
    } else {
        body.append(data_start, p - data_start);
    }

    return data_len;

error:
    return p - data;
}

} // namespace parser
} // namespace http
} // namespace websocketpp

#endif // HTTP_PARSER_MULTIPART_IMPL_HPP