/*
 * Copyright (c) 2012-2022 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/error/error.inl"
#include "core/guid.h"
#include "core/platform.h"
#include <stdio.h>  // sscanf

#if CROWN_PLATFORM_POSIX
	#include <fcntl.h>
	#include <unistd.h>
	#include <errno.h>
#elif CROWN_PLATFORM_WINDOWS
	#include <objbase.h>
#endif // CROWN_PLATFORM_POSIX

namespace crown
{
namespace guid_globals
{
#if CROWN_PLATFORM_POSIX
	static int _fd = -1;
#endif

	void init()
	{
#if CROWN_PLATFORM_POSIX
		_fd = ::open("/dev/urandom", O_RDONLY);
		CE_ASSERT(_fd != -1, "open: errno = %d", errno);
#endif // CROWN_PLATFORM_POSIX
	}

	void shutdown()
	{
#if CROWN_PLATFORM_POSIX
		::close(_fd);
		_fd = -1;
#endif // CROWN_PLATFORM_POSIX
	}

} // namespace guid_globals

namespace guid
{
	Guid new_guid()
	{
		Guid guid;
#if CROWN_PLATFORM_POSIX
		CE_ASSERT(guid_globals::_fd != -1, "new_guid: library uninitialized");
		ssize_t rb = read(guid_globals::_fd, &guid, sizeof(guid));
		CE_ENSURE(rb == sizeof(guid));
		CE_UNUSED(rb);
		guid.data1 = (guid.data1 & 0xffffffffffff4fffu) | 0x4000u;
		guid.data2 = (guid.data2 & 0x3fffffffffffffffu) | 0x8000000000000000u;
#elif CROWN_PLATFORM_WINDOWS
		HRESULT hr = CoCreateGuid((GUID*)&guid);
		CE_ASSERT(hr == S_OK, "CoCreateGuid: error");
		CE_UNUSED(hr);
#endif // CROWN_PLATFORM_POSIX
		return guid;
	}

	Guid parse(const char* str)
	{
		Guid guid;
		try_parse(guid, str);
		return guid;
	}

	bool try_parse(Guid& guid, const char* str)
	{
		CE_ENSURE(NULL != str);
		u32 a, b, c, d, e, f;
		int num = sscanf(str, "%8x-%4x-%4x-%4x-%4x%8x", &a, &b, &c, &d, &e, &f);
		guid.data1  = u64(a) << 32;
		guid.data1 |= u64(b) << 16;
		guid.data1 |= u64(c) <<  0;
		guid.data2  = u64(d) << 48;
		guid.data2 |= u64(e) << 32;
		guid.data2 |= u64(f) <<  0;
		return num == 6;
	}

	const char* to_string(char* buf, u32 len, const Guid& guid)
	{
		snprintf(buf, len, "%.8x-%.4x-%.4x-%.4x-%.4x%.8x"
			, (u32)((guid.data1 & 0xffffffff00000000u) >> 32)
			, (u16)((guid.data1 & 0x00000000ffff0000u) >> 16)
			, (u16)((guid.data1 & 0x000000000000ffffu) >>  0)
			, (u16)((guid.data2 & 0xffff000000000000u) >> 48)
			, (u16)((guid.data2 & 0x0000ffff00000000u) >> 32)
			, (u32)((guid.data2 & 0x00000000ffffffffu) >>  0)
			);
		return buf;
	}

} // namespace guid

} // namespace crown
