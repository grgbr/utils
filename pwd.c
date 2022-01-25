#include "utils/pwd.h"

/*
 * See section «NOTES» of getpwuid(3), getgrgid(3), getpwnam(3) and getgrnam(3)
 * man pages for infos about possible error values.
 */
static void __nothrow
upwd_normalize_errno(void)
{
	switch (errno) {
	case 0:
	case ENOENT:
	case EBADF:
	case ESRCH:
	case EWOULDBLOCK:
	case EPERM:
		errno = ENOENT;

	default:
		return;
	}
}

const struct passwd *
upwd_get_user_byid(uid_t uid)
{
	const struct passwd * ent;

	errno = 0;

	ent = getpwuid(uid);
	if (!ent) {
		upwd_normalize_errno();
		return NULL;
	}

	return ent;
}

const struct passwd *
upwd_get_user_byname(const char * __restrict name)
{
	upwd_assert(upwd_validate_user_name(name) > 0);

	const struct passwd * ent;

	errno = 0;

	ent = getpwnam(name);
	if (!ent) {
		upwd_normalize_errno();
		return NULL;
	}

	return ent;
}

const struct group *
upwd_get_group_byid(gid_t gid)
{
	const struct group * ent;

	errno = 0;

	ent = getgrgid(gid);
	if (!ent) {
		upwd_normalize_errno();
		return NULL;
	}

	return ent;
}

const struct group *
upwd_get_group_byname(const char * __restrict name)
{
	upwd_assert(upwd_validate_group_name(name) > 0);

	const struct group * ent;

	errno = 0;

	ent = getgrnam(name);
	if (!ent) {
		upwd_normalize_errno();
		return NULL;
	}

	return ent;
}
