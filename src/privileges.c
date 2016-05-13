/*
    This file is part of fspupsmon.

    Copyright (C) 2016 Vadim Kuznetsov <vimusov@gmail.com>

    fspupsmon is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    fspupsmon is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with fspupsmon.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "log.h"
#include "privileges.h"


typedef struct {
    gid_t *groups;  /* list of users groups */
    int groups_count;  /* amount of users groups */
    uid_t ruid, euid, suid;  /* real, effective and saved user IDs */
    gid_t rgid, egid, sgid;  /* real, effective and saved group IDs */
}
privileges_t;


static privileges_t *root_privileges = NULL;
static privileges_t *user_privileges = NULL;


/*
 * Create and fill new privileges structure.
 * Take user name or NULL if it's root's account.
 * Return pointer to a new privileges structure on success or NULL on error.
 */
static privileges_t* get_privileges(const char* user_name)
{
    int is_root = 0;
    gid_t *groups = NULL;
    int groups_count = 1;
    privileges_t *priv = NULL;

    if (user_name == NULL) {
        is_root = 1;
        user_name = "root";
    }

    struct passwd *pwd = getpwnam(user_name);
    if (pwd == NULL) {
        LOG_E("unable to get user name '%s', error '%m'", user_name);
        return NULL;
    }

    groups = (gid_t*) calloc(groups_count, sizeof(gid_t));
    if (groups == NULL) {
        LOG_E("unable to allocate memory for groups, error '%m'");
        return NULL;
    }

    const gid_t gid = pwd->pw_gid;

    if (getgrouplist(user_name, gid, groups, &groups_count) == -1) {
        if (groups_count <= 0) {
            LOG_E("invalid amount %d of user groups", groups_count);
            goto on_error;
        }

        free(groups);
        LOG_D("reallocating memory for store %d groups", groups_count);

        groups = (gid_t*) calloc(groups_count, sizeof(gid_t));
        if (groups == NULL) {
            LOG_E("unable to allocate memory for groups, error '%m'");
            goto on_error;
        }

        if (getgrouplist(user_name, gid, groups, &groups_count) <= 0) {
            LOG_E("unable to get groups for user '%s'", user_name);
            goto on_error;
        }
    }

    if (groups_count <= 0) {
        LOG_E("invalid amount %d of user groups", groups_count);
        goto on_error;
    }

    priv = (privileges_t*) calloc(1, sizeof(privileges_t));
    if (priv == NULL) {
        LOG_E("unable to allocate memory for privileges, error '%m'");
        goto on_error;
    }

    if (is_root) {
        if (getresuid(&(priv->ruid), &(priv->euid), &(priv->suid)) < 0) {
            LOG_E("unable to get resuids, error '%m'");
            goto on_error;
        }

        if (getresgid(&(priv->rgid), &(priv->egid), &(priv->sgid)) < 0) {
            LOG_E("unable to get resgids, error '%m'");
            goto on_error;
        }
    }
    else {
        priv->ruid = priv->euid = priv->suid = pwd->pw_uid;
        priv->rgid = priv->egid = priv->sgid = gid;
    }

    priv->groups = groups;
    priv->groups_count = groups_count;

    LOG_D("privileges initialized for user '%s'", user_name);

    return priv;

on_error:

    free(groups);

    free(priv);

    return NULL;
}


int init_privileges(const char *user_name)
{
    if (user_name == NULL) {
        LOG_E("invalid user_name, null pointer");
        return 1;
    }

    if (getgid()) {
        LOG_E("unable to initialize privileges (you aint root)");
        return 1;
    }

    if ((root_privileges = get_privileges(NULL)) == NULL)
        return 1;

    if ((user_privileges = get_privileges(user_name)) == NULL)
        goto on_error;

    if (set_user_privileges())
        goto on_error;

    LOG_D("privileges initialized for user '%s'", user_name);

    return 0;

on_error:

    free_privileges();

    return 1;
}


int set_root_privileges(void)
{
    if (root_privileges == NULL) {
        LOG_D("do not set root privileges because they have not been saved");
        return 0;
    }

    if (setresuid(root_privileges->ruid, root_privileges->euid, root_privileges->suid)) {
        LOG_E("unable to set uid, error '%m'");
        return 1;
    }

    if (setresgid(root_privileges->rgid, root_privileges->egid, root_privileges->sgid)) {
        LOG_E("unable to set gid, error '%m'");
        return 1;
    }

    if (setgroups(root_privileges->groups_count, root_privileges->groups)) {
        LOG_E("unable to set groups, error '%m'");
        return 1;
    }

    LOG_D("privileges set to root");

    return 0;
}


int set_user_privileges(void)
{
    if (user_privileges == NULL) {
        LOG_D("do not set user privileges because they have not been saved");
        return 0;
    }

    if (setgroups(user_privileges->groups_count, user_privileges->groups)) {
        LOG_E("unable to set groups, error '%m'");
        return 1;
    }

    /* WARN: set suid/sgid to 0 to be able to restore root privileges  */

    if (setresgid(user_privileges->rgid, user_privileges->egid, 0)) {
        LOG_E("unable to set gid, error '%m'");
        return 1;
    }

    if (setresuid(user_privileges->ruid, user_privileges->euid, 0)) {
        LOG_E("unable to set uid, error '%m'");
        return 1;
    }

    LOG_D("privileges set to user");

    return 0;
}


void free_privileges(void)
{
    if (user_privileges != NULL) {
        free(user_privileges->groups);
        free(user_privileges);
        user_privileges = NULL;
    }

    if (root_privileges != NULL) {
        free(root_privileges->groups);
        free(root_privileges);
        root_privileges = NULL;
    }
}
