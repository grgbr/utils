#include "utils/pwd.h"

#if defined(CONFIG_UTILS_ASSERT_INTERN)

#define upwd_assert_intern(_expr) \
	stroll_assert("utils:upwd", _expr)

#else  /* !defined(CONFIG_UTILS_ASSERT_INTERN) */

#define upwd_assert_intern(_expr)

#endif /* defined(CONFIG_UTILS_ASSERT_INTERN) */

int
upwd_parse_uid(const char * __restrict string, uid_t * __restrict uid)
{
	upwd_assert_api(string);
	upwd_assert_api(uid);

	int      err;
	uint32_t val;

	/* TODO: build a compile time assertion. */
	upwd_assert_intern(sizeof(uid_t) == sizeof(uint32_t));

	err = ustr_parse_uint32(string, &val);
	if (err)
		return err;

	*uid = (uid_t)val;

	return 0;
}

int
upwd_parse_gid(const char * __restrict string, gid_t * __restrict gid)
{
	upwd_assert_api(string);
	upwd_assert_api(gid);

	int      err;
	uint32_t val;

	/* TODO: build a compile time assertion. */
	upwd_assert_intern(sizeof(gid_t) == sizeof(uint32_t));

	err = ustr_parse_uint32(string, &val);
	if (err)
		return err;

	*gid = (gid_t)val;

	return 0;
}

/*
 * See section «NOTES» of getpwuid(3), getgrgid(3), getpwnam(3) and getgrnam(3)
 * man pages for infos about possible error values.
 */
static __nothrow
void
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
	upwd_assert_api(upwd_validate_user_name(name) > 0);

	const struct passwd * ent;

	errno = 0;

	ent = getpwnam(name);
	if (!ent) {
		upwd_normalize_errno();
		return NULL;
	}

	return ent;
}

int
upwd_get_uid_byname(const char * __restrict name, uid_t * __restrict uid)
{
	upwd_assert_api(upwd_validate_user_name(name) > 0);
	upwd_assert_api(uid);

	const struct passwd * pwd;

	pwd = upwd_get_user_byname(name);
	if (!pwd)
		return -errno;

	*uid = pwd->pw_uid;

	return 0;
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
	upwd_assert_api(upwd_validate_group_name(name) > 0);

	const struct group * ent;

	errno = 0;

	ent = getgrnam(name);
	if (!ent) {
		upwd_normalize_errno();
		return NULL;
	}

	return ent;
}

int
upwd_get_gid_byname(const char * __restrict name, gid_t * __restrict gid)
{
	upwd_assert_api(upwd_validate_group_name(name) > 0);
	upwd_assert_api(gid);

	const struct group * grp;

	grp = upwd_get_group_byname(name);
	if (!grp)
		return -errno;

	*gid = grp->gr_gid;

	return 0;
}
