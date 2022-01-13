#include "utils/pwd.h"
#include <pwd.h>
#include <grp.h>

int
upwd_get_uid(const char * __restrict user, uid_t * __restrict uid)
{
	upwd_assert(upwd_validate_user(user) > 0);
	upwd_assert(uid);

	struct passwd * pwd;

	pwd = getpwnam(user);
	if (!pwd) {
		/*
		 * See section «NOTES» of getgrnam(3) man page for infos about
		 * possible errno values.
		 */
		switch (errno) {
		case 0:
		case ENOENT:
		case EBADF:
		case ESRCH:
		case EWOULDBLOCK:
		case EPERM:
			return -ENOENT;

		default:
			return -errno;

		}

		unreachable();
	}

	*uid = pwd->pw_uid;

	return 0;
}

int
upwd_get_gid(const char * __restrict group, gid_t * __restrict gid)
{
	upwd_assert(upwd_validate_group(group) > 0);
	upwd_assert(gid);

	struct group * grp;

	errno = 0;

	grp = getgrnam(group);
	if (!grp) {
		/*
		 * See section «NOTES» of getgrnam(3) man page for infos about
		 * possible errno values.
		 */
		switch (errno) {
		case 0:
		case ENOENT:
		case EBADF:
		case ESRCH:
		case EWOULDBLOCK:
		case EPERM:
			return -ENOENT;

		default:
			return -errno;

		}

		unreachable();
	}

	*gid = grp->gr_gid;

	return 0;
}
