#include <stdint.h>
#include "vietnam.hpp"

std::string coda_remove_tones_utf8(const char *s, size_t sz)
{
	std::string result;
	result.reserve(sz);

	const unsigned char *p = (const unsigned char *) s;
	const unsigned char *e = p + sz;

	while (p < e)
	{
		if (*p < 0x80)
		{
			result.push_back((char) *p++);
			continue;
		}

		uint32_t code = 0;
		uint32_t char_len = 0;

		switch (*p & 0xF0)
		{
		case 0xF0:
			if (p + 3 < e)
			{
				code += (*p++ & 0x07) << 18;
				code += (*p++ & 0x3F) << 12;
				code += (*p++ & 0x3F) << 6;
				code += (*p++ & 0x3F);
				char_len = 4;
			}
			break;
		case 0xE0:
			if (p + 2 < e)
			{
				code += (*p++ & 0x0F) << 12;
				code += (*p++ & 0x3F) << 6;
				code += (*p++ & 0x3F);
				char_len = 3;
			}
			break;
		default:
			if (p + 1 < e)
			{
				code += (*p++ & 0x1F) << 6;
				code += (*p++ & 0x3F);
				char_len = 2;
			}
			break;
		}

		if (code >= 0x0300 && code <= 0x036F) // ignore COMBINING_DIACRITICAL_MARKS
		{
			continue;
		}

		switch (code)
		{
		case 0x0000: /* last utf8 symbol is cut, we just continue to the end of the line */
			while (p < e)
			{
				result.push_back((char) *p++);
			}
			break;
		case 0x00D0: case 0x0110: // Ð Đ
			result.push_back('D');
			break;
		case 0x00F0: case 0x0111: // ð đ
			result.push_back('d');
			break;
		case 0x00C0: case 0x00C1: case 0x00C2: case 0x00C3: case 0x0102: case 0x1EA0: case 0x1EA2: case 0x1EA4: case 0x1EA6:
		case 0x1EA8: case 0x1EAA: case 0x1EAC: case 0x1EAE: case 0x1EB0: case 0x1EB2: case 0x1EB4: case 0x1EB6: // À Á Â Ã Ă Ạ Ả Ấ Ầ Ẩ Ẫ Ậ Ắ Ằ Ẳ Ẵ Ặ
			result.push_back('A');
			break;
		case 0x00E0: case 0x00E1: case 0x00E2: case 0x00E3: case 0x0103: case 0x1EA1: case 0x1EA3: case 0x1EA5: case 0x1EA7:
		case 0x1EA9: case 0x1EAB: case 0x1EAD: case 0x1EAF: case 0x1EB1: case 0x1EB3: case 0x1EB5: case 0x1EB7: // à á â ã ă ạ ả ấ ầ ẩ ẫ ậ ắ ằ ẳ ẵ ặ
			result.push_back('a');
			break;
		case 0x00C8: case 0x00C9: case 0x00CA: case 0x1EB8: case 0x1EBA: case 0x1EBC: case 0x1EBE: case 0x1EC0: case 0x1EC2: case 0x1EC4: case 0x1EC6: // È É Ê Ẹ Ẻ Ẽ Ế Ề Ể Ễ Ệ
			result.push_back('E');
			break;
		case 0x00E8: case 0x00E9: case 0x00EA: case 0x1EB9: case 0x1EBB: case 0x1EBD: case 0x1EBF: case 0x1EC1: case 0x1EC3: case 0x1EC5: case 0x1EC7: // è é ê ẹ ẻ ẽ ế ề ể ễ ệ
			result.push_back('e');
			break;
		case 0x00CC: case 0x00CD: case 0x0128: case 0x1EC8: case 0x1ECA: // Ì Í Ĩ Ỉ Ị
			result.push_back('I');
			break;
		case 0x00EC: case 0x00ED: case 0x0129: case 0x1EC9: case 0x1ECB: // ì í ĩ ỉ ị
			result.push_back('i');
			break;
		case 0x00D2: case 0x00D3: case 0x00D4: case 0x00D5: case 0x01A0: case 0x1ECC: case 0x1ECE: case 0x1ED0: case 0x1ED2:
		case 0x1ED4: case 0x1ED6: case 0x1ED8: case 0x1EDA: case 0x1EDC: case 0x1EDE: case 0x1EE0: case 0x1EE2: // Ò Ó Ô Õ Ơ Ọ Ỏ Ố Ồ Ổ Ỗ Ộ Ớ Ờ Ở Ỡ Ợ
			result.push_back('O');
			break;
		case 0x00F2: case 0x00F3: case 0x00F4: case 0x00F5: case 0x01A1: case 0x1ECD: case 0x1ECF: case 0x1ED1: case 0x1ED3:
		case 0x1ED5: case 0x1ED7: case 0x1ED9: case 0x1EDB: case 0x1EDD: case 0x1EDF: case 0x1EE1: case 0x1EE3: // ò ó ô õ ơ ọ ỏ ố ồ ổ ỗ ộ ớ ờ ở ỡ ợ
			result.push_back('o');
			break;
		case 0x00D9: case 0x00DA: case 0x0168: case 0x01AF: case 0x1EE4: case 0x1EE6: case 0x1EE8: case 0x1EEA: case 0x1EEC: case 0x1EEE: case 0x1EF0: // Ù Ú Ũ Ư Ụ Ủ Ứ Ừ Ử Ữ Ự
			result.push_back('U');
			break;
		case 0x00F9: case 0x00FA: case 0x0169: case 0x01B0: case 0x1EE5: case 0x1EE7: case 0x1EE9: case 0x1EEB: case 0x1EED: case 0x1EEF: case 0x1EF1: // ù ú ũ ư ụ ủ ứ ừ ử ữ ự
			result.push_back('u');
			break;
		case 0x00DD: case 0x1EF2: case 0x1EF4: case 0x1EF6: case 0x1EF8: // Ý Ỳ Ỵ Ỷ Ỹ
			result.push_back('Y');
			break;
		case 0x00FD: case 0x1EF3: case 0x1EF5: case 0x1EF7: case 0x1EF9: // ý ỳ ỵ ỷ ỹ
			result.push_back('y');
			break;
		default:
			{
				const unsigned char *pp = p - char_len;
				while (pp < p)
				{
					result.push_back((char) *pp++);
				}
			}
			break;
		}
	}

	return result;
}

